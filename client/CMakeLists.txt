# Tạo file chạy tên là 'client_app'
add_executable(client_app
    ClientLauncher.cpp
    Client.cpp
    Client.h
    # main_client.cpp  
)

# Find OpenCV (required for cv::imdecode / imshow)
find_package(OpenCV REQUIRED)

# Link thư viện:
# 1. common_lib: Để dùng RtpPacket
# 2. ws2_32: Để dùng socket
# 3. OpenCV_LIBS: Để hiển thị ảnh
target_include_directories(client_app PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(client_app PRIVATE 
    common
    ws2_32 
    ${OpenCV_LIBS}
)