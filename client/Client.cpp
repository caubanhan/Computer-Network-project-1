#include "Client.h"
#include <conio.h> // Để dùng _getch() bắt phím

// --- Constructor ---
Client::Client(string serverAddr, int serverPort, int rtpPort, string fileName) {
    this->serverAddr = serverAddr;
    this->serverPort = serverPort;
    this->rtpPort = rtpPort;
    this->fileName = fileName;

    this->state = INIT;
    this->rtspSeq = 1;
    this->sessionId = 0;
    this->frameNum = 0;
    this->isRunning = true;
    this->rtspSocket = INVALID_SOCKET;
    this->rtpSocket = INVALID_SOCKET;

    // Khởi tạo Winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    connectToServer();
}

// --- Destructor ---
Client::~Client() {
    isRunning = false;
	if (rtpThread.joinable()) rtpThread.join(); // Chờ thread RTP kết thúc
    if (rtspSocket != INVALID_SOCKET) closesocket(rtspSocket);
    WSACleanup();
}

// --- Kết nối TCP tới Server (RTSP) ---
void Client::connectToServer() {
    rtspSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, serverAddr.c_str(), &addr.sin_addr);
    addr.sin_port = htons(serverPort);

    if (connect(rtspSocket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cout << "Connection Failed!" << endl;
    }
    else {
        cout << "Connected to Server at " << serverAddr << ":" << serverPort << endl;
    }
}

// --- Gửi lệnh RTSP ---
bool Client::sendRtspRequest(string method) {
    if (rtspSocket == INVALID_SOCKET) return false;

    string msg = method + " " + fileName + " RTSP/1.0\r\n";
    msg += "CSeq: " + to_string(rtspSeq++) + "\r\n";

    if (method == "SETUP") {
        msg += "Transport: RTP/UDP; client_port= " + to_string(rtpPort) + "\r\n";
    }
    else {
        msg += "Session: " + to_string(sessionId) + "\r\n";
    }
    msg += "\r\n";

    send(rtspSocket, msg.c_str(), (int)msg.length(), 0);
    cout << "[SENT]: " << method << endl;

    return handleServerReply();
}

// --- Xử lý phản hồi từ Server ---
bool Client::handleServerReply() {
    char buffer[1024] = { 0 };
    int len = recv(rtspSocket, buffer, 1024, 0);
    if (len > 0) {
        string reply(buffer);
        cout << "Server Reply:\n" << reply << endl; // Debug

		if (reply.find("200 OK") == string::npos) { // Không có 200 OK thì hủy
            cout << "Error:\n" << reply << endl;
            return false; 
        }

        if (reply.find("Session:") != string::npos && sessionId == 0) {
            size_t pos = reply.find("Session: ");
            string sub = reply.substr(pos + 9);
            size_t endPos = sub.find_first_of("\r\n;");
            sessionId = stoi(sub.substr(0, endPos));

            cout << "--> Connection Established! Session ID: " << sessionId << endl;
			return true; // Thành công khi nhận được Session ID
        }
		return true; // Thành công với các lệnh khác
    }
	return false; // Mặc định trả về false nếu không nhận được phản hồi đúng
}

// --- Các hàm Button Handlers ---
void Client::setup() {
    if (state == INIT) {
        if (sendRtspRequest("SETUP")) {
            state = READY;
            cout << "System READY. Press 'p' to Play." << endl;
        } else {
            cout << "SETUP Failed!" << endl;
        }
    }
}

void Client::play() {
    if (state == READY) {
        if (sendRtspRequest("PLAY")) {
            state = PLAYING;
            // Bắt đầu luồng nhận RTP nếu chưa chạy
            if (!rtpThread.joinable()) {
                rtpThread = thread(&Client::listenRtp, this);
            }
        }
        else {
            cout << "PLAY Failed!" << endl;
        }
    }
}

void Client::pause() {
    if (state == PLAYING) {
        if (sendRtspRequest("PAUSE")) {
            state = READY;
        }
        else {
			cout << "PAUSE Failed!" << endl;
        }
    }
}

void Client::teardown() {
    sendRtspRequest("TEARDOWN");
    state = INIT;
    isRunning = false;
    exit(0); // Thoát chương trình
}

// --- RTP Listener Thread (Xử lý nhận & hiển thị Video) ---
void Client::listenRtp() {
    rtpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    DWORD timeout = 500;
    setsockopt(rtpSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    // Tăng kích thước buffer nhận của Socket lên (quan trọng để không bị drop gói)
    int buffSize = 65536;
    setsockopt(rtpSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&buffSize, sizeof(buffSize));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(rtpPort);

    if (::bind(rtpSocket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cout << "Bind RTP failed: " << WSAGetLastError() << endl;
        return;
    }

    char buffer[2048]; // Buffer nhận gói tin thô (thường chỉ tầm 1400 byte)

    int totalPackets = 0; //debug
    while (isRunning) {
        if (state != PLAYING) {
            this_thread::sleep_for(chrono::milliseconds(10));
            continue;
        }

        int len = recv(rtpSocket, buffer, sizeof(buffer), 0);
        if (len > 0) { //debug
            totalPackets++;
            if (totalPackets % 100 == 0) cout << "Da nhan " << totalPackets << " goi tin..." << endl;
        }
        else {
            // Không nhận được gì hoặc timeout
            continue;
        }
        if (len > 12) { // Phải lớn hơn Header 12 byte

            // 1. Lấy thông tin Header
            RtpHeader* header = (RtpHeader*)buffer;

            // 2. Gom dữ liệu (Payload) vào bộ đệm chung
            // Payload bắt đầu từ byte thứ 12
            uint8_t* payload = (uint8_t*)buffer + 12;
            int payloadSize = len - 12;

            frameBuffer.insert(frameBuffer.end(), payload, payload + payloadSize);

            // 3. Kiểm tra Marker Bit
            // Nếu dùng struct bit-field đôi khi không chính xác do compiler, 
            // cách chắc chắn nhất là check bit 1 của byte thứ 2 (0x80)
            // Trong buffer: buffer[1] chứa Marker (bit đầu) và PayloadType (7 bit sau)
            bool isLastPacket = (buffer[1] & 0x80) != 0;

            if (isLastPacket) {
                // Đã nhận đủ 1 Frame -> Decode
                cout << "[DEBUG] Da ghep duoc 1 Frame. Kich thuoc: " << frameBuffer.size() << " bytes.";
                // Decode từ memory buffer
                cv::Mat rawData(frameBuffer);
                cv::Mat frame = cv::imdecode(rawData, cv::IMREAD_COLOR);

                if (!frame.empty()) {
                    cout << " -> DECODE THANH CONG! (Hien thi anh)" << endl;
                    cv::imshow("Client Video", frame);
                    cv::waitKey(1); // Bắt buộc có để vẽ hình
                }
                else {
                    cout << "Decode failed (Frame incomplete?)\n";
                    if (frameBuffer.size() > 2) {
                        printf("Dau header la: %02X %02X\n", frameBuffer[0], frameBuffer[1]);
                    }
                }

                // Xóa buffer để chuẩn bị cho frame tiếp theo
                frameBuffer.clear();
            }
        }
    }
    closesocket(rtpSocket);
}

// --- Giao diện điều khiển (Giả lập GUI Mainloop) ---
void Client::runInterface() {
    cout << "\n========================================" << endl;
    cout << "          RTP CLIENT CONTROLLER         " << endl;
    cout << "========================================" << endl;
    cout << "Controls: [s] Setup, [p] Play, [u] Pause, [t] Teardown" << endl;

    while (isRunning) {
        if (_kbhit()) { // Nếu có phím nhấn
            char key = _getch();
            switch (key) {
            case 's': setup(); break;
            case 'p': play(); break;
            case 'u': pause(); break;
            case 't': teardown(); break;
            }
        }
        this_thread::sleep_for(chrono::milliseconds(50));
    }
}