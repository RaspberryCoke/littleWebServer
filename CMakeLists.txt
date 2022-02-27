cmake_minimum_required(VERSION 3.14)
project(server)
set(
    SOURCES
    buffer.cpp
    epoller.cpp
    httpconn.cpp
    main.cpp
    server.cpp
    sqlpool.cpp
    httprequest.cpp
    httpresponse.cpp
)
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME}
PRIVATE
${PROJECT_SOURCE_DIR}
)
target_link_libraries(${PROJECT_NAME} pthread mysqlclient)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")