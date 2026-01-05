cmake_minimum_required(VERSION 3.16)    # 规定CMake最低版本

project(UR_Control VERSION 0.1 LANGUAGES CXX)   # 定义项目名、版本、语言

# 以下三个是Qt特有
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- OpenCV 配置 ---
# 注意：在 Linux 上不需要设置 OpenCV_DIR，通常能自动找到
if(WIN32)
    set(OpenCV_DIR "E:/opencv/build")
endif()
find_package(OpenCV REQUIRED)
# 调试信息，打印一下是否找到
# message(STATUS "OpenCV library status:")
# message(STATUS "    version: ${OpenCV_VERSION}")
# message(STATUS "    libraries: ${OpenCV_LIBS}")
# message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

# --- Qt配置 ---
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

# --- 源文件配置 ---
# 1. 基础通用文件
set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui
        src/platform/CameraHelper.h
        src/RRTPlanner.h
        src/RRTPlanner.cpp
        src/tools/CalibrationHelper.h
        src/tools/CalibrationHelper.cpp
)

# 2. 根据系统加入特定实现文件(针对不同平台的相机助手)
if(WIN32)
    list(APPEND PROJECT_SOURCES src/platform/win/CameraHelper.cpp)
    message(STATUS "🖥️  Detected Windows: Added win/CameraHelper.cpp")
elseif(UNIX AND NOT APPLE)
    list(APPEND PROJECT_SOURCES src/platform/linux/CameraHelper.cpp)
    message(STATUS "🐧 Detected Linux: Added linux/CameraHelper.cpp")
endif()

# 3. 添加头文件包含路径 (代码里可直接 #include "platform/..." )
include_directories(${CMAKE_SOURCE_DIR}/src)

# --- 生成可执行文件 ---
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(UR_Control
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )

# Define target properties for Android with Qt 6 as:
#    set_property(TARGET UR_Control APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(UR_Control SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(UR_Control
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# --- 链接库文件 ---
target_link_libraries(UR_Control PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network ${OpenCV_LIBS})

# --- 单元测试配置 ---
# 1. 定义测试程序的可执行文件
# 注意：这里只包含测试入口 (test_rrt_main.cpp) 和 算法核心 (RRTPlanner)
add_executable(RRT_Test
    src/tests/test_rrt_main.cpp
    src/RRTPlanner.cpp
    src/RRTPlanner.h
)

# 2. 链接必要的库
# 算法依赖 OpenCV 进行数学计算 (cv::Point3f)
target_link_libraries(RRT_Test PRIVATE 
    ${OpenCV_LIBS}
)

# (可选) 如果你的 RRTPlanner 用到了 Qt 的 qDebug，还需要链接 QtCore
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
target_link_libraries(RRT_Test PRIVATE 
    Qt${QT_VERSION_MAJOR}::Core
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.UR_Control)
endif()
set_target_properties(UR_Control PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS UR_Control
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(UR_Control)
endif()
