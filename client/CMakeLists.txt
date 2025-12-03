set(SDL2_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies")
# --- CÁC PHẦN DƯỚI GIỮ NGUYÊN ---
include_directories(${SDL2_PATH}/include)
link_directories(${SDL2_PATH}/lib)

# Tạo file chạy tên là 'client_app'
add_executable(client_app
    ClientLauncher.cpp
    Client.cpp
    Client.h
    stb_image.h 
    # main_client.cpp  
)


# Link thư viện:
# 1. common_lib: Để dùng RtpPacket
# 2. ws2_32: Để dùng socket
target_link_libraries(client_app PRIVATE 
    common
    ws2_32 

    mingw32         
    SDL2main        
    SDL2
)

#copy .dll file to build directory
add_custom_command(TARGET client_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_PATH}/bin/SDL2.dll"
        $<TARGET_FILE_DIR:client_app>
    COMMENT "Copying SDL2.dll to build directory..."
)