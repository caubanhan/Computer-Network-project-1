add_executable(server
    main.cpp
    ServerWorker.cpp
)

target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(server PRIVATE common ws2_32)
