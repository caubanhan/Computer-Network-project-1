add_executable(client
    main.cpp
    Client.cpp
)

target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(client PRIVATE common ws2_32)
