cmake_minimum_required(VERSION 3.10)

# --- KHẮC PHỤC LỖI 1: Sửa lại đường dẫn thư viện ---
# Theo hình ảnh cũ: thư mục 'lib' và 'include' nằm ngay cạnh file CMakeLists.txt
# Em xóa chữ "Dependencies" đi vì máy em không có folder đó.
set(MY_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(MY_LIB_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/lib")

# Báo cho CMake biết chỗ tìm header (.h) và thư viện (.a / .lib)
include_directories(${MY_INCLUDE_DIR})
link_directories(${MY_LIB_DIR}) 

# --- Tạo file chạy ---
add_executable(client_app
    ClientLauncher.cpp
    Client.cpp
    Client.h
    RtpReceiver.cpp
    VideoDisplay.cpp
    # stb_image.h là thư viện header-only, không cần compile, include trong .cpp là được
)

# --- KHẮC PHỤC LỖI 2: Xử lý xung đột pthread ---
target_link_libraries(client_app PRIVATE 
    common
    ws2_32 
    mingw32         
    SDL2main        
    SDL2
    SDL2_ttf
    
    # THÊM 2 CỜ NÀY ĐỂ SỬA LỖI "multiple definition of pthread"
    -static-libgcc
    -static-libstdc++
)

# --- Copy DLL ra ngoài để chạy ---
add_custom_command(TARGET client_app POST_BUILD
    # Copy SDL2.dll
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/SDL2.dll"
        $<TARGET_FILE_DIR:client_app>
        
    # Copy SDL2_ttf.dll
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/SDL2_ttf.dll"
        $<TARGET_FILE_DIR:client_app>

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/arial.ttf"
        $<TARGET_FILE_DIR:client_app>
        
    COMMENT "Copying DLLs to build directory..."
)