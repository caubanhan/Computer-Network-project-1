#include "ServerWorker.h"
#include "Server.h"
#include "VideoStream.h"
#include <thread>
ServerWorker::ServerWorker(SOCKET clientsocket, const sockaddr_in& clientAddr)
{
    this->clientSocket = clientsocket;
    this->clientAddr = clientAddr;

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, sizeof(ipStr));

    std::cout << "ServerWorker created for "
        << ipStr << ":" << ntohs(clientAddr.sin_port)
        << std::endl;
}

ServerWorker::~ServerWorker()
{
    std::cout << "Client disconnected. Cleaning up...\n";

    // 1. Ra lệnh dừng gửi
    sending.store(false);
    state = INIT;

    // 2. Chờ luồng gửi kết thúc công việc (Tránh lỗi Abort)
    if (rtpThread.joinable()) {
        rtpThread.join();
    }

    // 3. Đóng socket
    if (rtpSocket != INVALID_SOCKET) closesocket(rtpSocket);
    if (clientSocket != INVALID_SOCKET) closesocket(clientSocket);
}

void ServerWorker::processRtspRequest()
{
    /*
        C: SETUP movie.Mjpeg RTSP/1.0
        C: CSeq: 1
        C: Transport: RTP/UDP; client_port=25000

        S: RTSP/1.0 200 OK
        S: CSeq: 1
        S: Session: 123456
    */
    char buffer[2048] = { 0 };

    while (true) {
        int recvLen = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (recvLen <= 0) return;
        std::string request(buffer);
        std::cout << "RTSP Request:\n" << request << "\n";

        // ===== 1. Parse request line =====
        size_t line1End = request.find("\r\n");
        std::string firstLine = request.substr(0, line1End); // first line = "SETUP movie.Mjpeg RTSP/1.0"

        std::istringstream iss1(firstLine);
        iss1 >> method >> fileName; // method = "SETUP", fileName = "movie.Mjpeg"

        // get "Cseq"
        size_t line2End = request.find("\r\n", line1End + 2);
        std::string secondLine = request.substr(line1End + 2, line2End - (line1End + 2));
        std::string post;
        std::istringstream iss2(secondLine);
        iss2 >> post >> cseq; // post = "CSeq:", cseq = "1"

        // get "client_port"
        size_t line3End = request.find("\r\n", line2End + 2);
        size_t portPos = request.find("client_port=", line2End + 2);
        size_t portStart = portPos + strlen("client_port=");
        std::string portStr = request.substr(portStart, line3End - portStart); // portStr = "25000"
        UDPport = std::stoi(portStr);

        executeRtspRequest();
        if (method == "TEARDOWN") break;
    }
}

void ServerWorker::executeRtspRequest()
{
    /*
        C: SETUP movie.Mjpeg RTSP/1.0
        C: CSeq: 1
        C: Transport: RTP/UDP; client_port=25000

        S: RTSP/1.0 200 OK
        S: CSeq: 1
        S: Session: 123456
    */

    // SETUP request
    if (method == "SETUP") 
    {
        std::cout << "Processing SETUP request...\n";
        sessionId = std::to_string(rand() % 1000000);

        try {
            videoStream.openFile(fileName);
            state = STATE::READY;
        }
        catch (const std::exception& e) {
            std::cerr << "Error opening video file: " << e.what() << std::endl;
            replyRtsp("FILE_NOT_FOUND_404");
            return;
        }

        // Create UDP socket
        rtpSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (rtpSocket == INVALID_SOCKET) {
            std::cerr << "Failed to create RTP socket. Error: " << WSAGetLastError() << std::endl;
            replyRtsp("500 Internal Server Error");
            return;
        }

        clientAddr.sin_port = htons(static_cast<u_short>(UDPport)); // Client RTP port
		// Send response
        replyRtsp("200 OK");

    } 
	// PLAY request
    else if (method == "PLAY") {
        if (state == STATE::READY) {
            std::cout << "Processing PLAY request...\n";
            state = STATE::PLAYING;

            // Send RTSP reply before starting the RTP packets
            replyRtsp("200 OK");

            // Start RTP sending thread
            sending.store(true);

            if (rtpThread.joinable()) rtpThread.join();
            rtpThread = std::thread([this]() {
                try {
                    while (sending.load() && state == STATE::PLAYING) {
                        this->sendRtp();
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }
                } catch (...) {
                    sending.store(false);
                }
            });

            //rtpThread.detach();   
        }
    }   

    else if (method == "PAUSE") 
    {
        if (state == STATE::PLAYING) 
        {
            std::cout << "Processing PAUSE request...\n";
			sending.exchange(false); // Stop RTP sending
            state = STATE::READY;
            replyRtsp("200 OK");
            
        }
    } 
    else if (method == "TEARDOWN") 
    {
        std::cout << "Processing TEARDOWN request...\n";
        replyRtsp("200 OK");
		sending.exchange(false); // Stop RTP sending
		state = STATE::INIT;
		// close RTP socket and clean up
		closesocket(rtpSocket);
    } 
    else 
    {
        std::cout << "Unknown RTSP method: " << method << "\n";
        replyRtsp("501 Not Implemented");
	}
}

// This function would send RTP packets to the client
void ServerWorker::sendRtp(){

	/* 
         Get next frame from video stream
	     build RTP packet
	     check sending flag 
	     send via rtpSocket to clientAddr
    */
    // Quick guard: socket must be valid and sending must be enabled
    if (rtpSocket == INVALID_SOCKET || !sending.load()) return;

    int frameSize = videoStream.getNextFrame(frameBuf, sizeof(frameBuf));
    if (frameSize <= 0) {
        std::cout << "End of video stream or error reading frame.\n";
        sending.store(false); // Stop sending if no more frames
        return;
    }
    rtpPacket.beginFrame(frameBuf, frameSize);

    // 3. Send all RTP packets of this frame
    uint8_t packetBuf[1500];
    int packetSize = 0;     // set mac dinh = 0 ==> Tranh undefined behavior
    while (rtpPacket.getNextPacket(packetBuf, packetSize) && sending.load()) {
        // Basic validation
        if (packetSize <= 0 || packetSize > static_cast<int>(sizeof(packetBuf))) {
            std::cerr << "Invalid RTP packet size: " << packetSize << "\n";
            break;
        }

        // Protect against socket being closed concurrently
        if (rtpSocket == INVALID_SOCKET) {
            std::cerr << "RTP socket invalidated while sending\n";
            break;
        }

        // sendto on Windows expects const char*
        int sent = sendto(
            rtpSocket,
            reinterpret_cast<const char*>(packetBuf),
            packetSize,
            0,
            reinterpret_cast<sockaddr*>(&clientAddr),
            static_cast<int>(sizeof(clientAddr))
        );

        std::this_thread::sleep_for(std::chrono::microseconds(400)); // Thêm độ trễ nhỏ để tránh gửi quá nhanh => packet loss

        if (sent == SOCKET_ERROR) {
            int err = WSAGetLastError();
            std::cerr << "sendto failed. WSAGetLastError(): " << err << "\n";
            // On certain errors (connection reset by peer) it's reasonable to stop sending
            sending.store(false);
            break;
        }
    }
}
// Reply to RTSP request and get udp port for RTP

void ServerWorker::replyRtsp(std::string stateResponse)
{
    // Tạo response đơn giản
    std::stringstream response;

    response << "RTSP/1.0 " << stateResponse << "\r\n";
    response << "CSeq: " << cseq << "\r\n";
    response << "Session: " << sessionId << "\r\n\r\n";

    std::string resp = response.str();
    int iResult = send(clientSocket, resp.c_str(), resp.length(), 0);
    if (iResult == SOCKET_ERROR)
    {
        std::cerr << "Send RTSP response failed! Error: " << WSAGetLastError() << "\n";
    }
    std::cout << "Sent RTSP response:\n" << resp << "\n";
}
