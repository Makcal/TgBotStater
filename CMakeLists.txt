cmake_minimum_required(VERSION 3.7.0)

project(TgBotStater CXX)
set(PROJECT_VERSION "0.3.4")
message(STATUS "${CMAKE_PROJECT_NAME} version: ${PROJECT_VERSION}")

find_package(tgbot REQUIRED)
find_package(brigand REQUIRED)
find_package(fmt)

add_library(${PROJECT_NAME} INTERFACE)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)
target_include_directories(${PROJECT_NAME} INTERFACE
    include/)
target_link_libraries(${PROJECT_NAME} INTERFACE
    ${tgbot_LIBRARIES}
    ${bridand_LIBRARIES}
    ${fmt_LIBRARIES})

install(DIRECTORY include/tg_stater DESTINATION include)
