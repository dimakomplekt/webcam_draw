cmake_minimum_required(VERSION 3.15)
project(OpenCV)

set(CMAKE_CXX_STANDARD 17)

# Пути к бинарным библиотекам
set(OpenCV_DIR "C:/opencv/build")

find_package(OpenCV REQUIRED)

add_executable(OpenCV main.cpp)

# Линкуем бинарные libs
target_link_libraries(OpenCV PRIVATE ${OpenCV_LIBS})

# Добавляем include для IntelliSense
target_include_directories(OpenCV PRIVATE
    "C:/opencv/build/include"
    "C:/opencv/build/x64/vc16/lib"
    "C:/opencv/modules/imgproc/include"
    "C:/opencv/modules/highgui/include"
    "C:/opencv/sources/include/opencv2"
)
