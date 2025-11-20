add_executable(server
    main.cpp
    ServerWorker.cpp
    Server.cpp
    VideoStream.cpp
)

target_link_libraries(server PRIVATE common)

target_link_libraries(server PRIVATE
    common        # common is target
    ws2_32
)
