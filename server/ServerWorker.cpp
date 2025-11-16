#include "ServerWorker.h"
#include "Server.h"
ServerWorker::ServerWorker(SOCKET clientsocket) : clientSocket(clientsocket)
{
    // Nếu user không truyền địa chỉ client, ta để rỗng
    ZeroMemory(&clientAddr, sizeof(clientAddr));

    std::cout << "ServerWorker created (no client address)\n";
}


// Process RtspRequest from client
void ServerWorker::processRtspRequest() {
    char buffer[1024] = {0};
    int recvLen = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (recvLen <= 0) {
        std::cout << "Client disconnected.\n";
        return;
    }

    std::string request(buffer);
    std::cout << "Received RTSP request:\n" << request << "\n";

    replyRtsp(request);
}


void ServerWorker::replyRtsp(const std::string& request)
{
    std::string cseq;

    // Lấy CSeq từ request
    size_t cseqPos = request.find("CSeq:");
    if (cseqPos != std::string::npos) { 
        size_t end = request.find("\r\n", cseqPos);
        cseq = request.substr(cseqPos + 5, end - (cseqPos + 5));
        // Xóa khoảng trắng đầu
        cseq.erase(0, cseq.find_first_not_of(" "));
    }

    // Tạo response đơn giản
    std::stringstream response;

    response << "RTSP/1.0 200 OK\r\n";
    response << "CSeq: " << cseq << "\r\n";
    response << "Session: 123456\r\n\r\n";

    std::string resp = response.str();

    send(clientSocket, resp.c_str(), resp.length(), 0);

    std::cout << "Sent RTSP response:\n" << resp << "\n";
}
