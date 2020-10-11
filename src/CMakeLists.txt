add_executable( ${PROJECT_NAME} main.cpp )

find_package(Threads)

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
target_link_libraries(${PROJECT_NAME} PRIVATE xcb)

