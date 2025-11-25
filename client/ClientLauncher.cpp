#include <iostream>
#include "Client.h"

using namespace std;

int main(int argc, char** argv) {
    // Giá trị mặc định nếu không nhập tham số
    string serverAddr = "127.0.0.1";
    int serverPort = 8554;
    int rtpPort = 25000;
    string fileName = "movie.Mjpeg";

    // Xử lý tham số đầu vào (như sys.argv trong Python)
    if (argc == 5) {
        serverAddr = argv[1];
        serverPort = stoi(argv[2]);
        rtpPort = stoi(argv[3]);
        fileName = argv[4];
    }
    else {
        cout << "[Usage: ClientLauncher.exe Server_IP Server_port RTP_port Video_file]" << endl;
        cout << "Using default values..." << endl;
    }

    // Tạo Client và chạy (tương đương root.mainloop())
    Client app(serverAddr, serverPort, rtpPort, fileName);
    app.runInterface();

    return 0;
}