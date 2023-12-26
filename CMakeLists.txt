cmake_minimum_required(VERSION 3.5)

project(FFMpeg-Test2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavdevice
    libavfilter
    libavformat
    libavcodec
    libswresample
    libswscale
    libavutil
)


add_executable(FFMpeg-Test2 main.cpp)

target_link_libraries(FFMpeg-Test2 PkgConfig::LIBAV)

include(GNUInstallDirs)
install(TARGETS FFMpeg-Test2
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
