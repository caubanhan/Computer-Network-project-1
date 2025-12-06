set(BUILD_SHARED_LIBS OFF)

add_executable(client_app
    ClientLauncher.cpp
    Client.cpp
    Client.h
    VideoDisplay.cpp
    RtpReceiver.cpp
)

set_target_properties(client_app PROPERTIES
    WIN32_EXECUTABLE OFF
)

target_link_options(client_app PRIVATE "-Wl,-subsystem,console")

file(TO_CMAKE_PATH ".\\lib" SDL_LIB)
file(TO_CMAKE_PATH ".\\include\\SDL2" SDL_INC)

include_directories(${SDL_INC})
target_link_directories(client_app PRIVATE ${SDL_LIB})

target_link_libraries(client_app PRIVATE
    common
    ws2_32
    SDL2main
    SDL2
    SDL2_ttf
)
