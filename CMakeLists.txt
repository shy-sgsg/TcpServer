cmake_minimum_required(VERSION 3.16)

project(TcpServer VERSION 0.1 LANGUAGES CXX)

# 启用自动处理功能
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找并加载 Qt 包
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

# 收集所有项目源文件和头文件
# 这是一个更清晰和更易于维护的方式
set(PROJECT_SOURCES
    main.cpp
    tcpserver.cpp
    filereceiver.cpp
    mainwindow.cpp
    imagepreviewlabel.cpp
    fullscreenimagepreview.cpp
)

set(PROJECT_HEADERS
    tcpserver.h
    filereceiver.h
    mainwindow.h
    imagepreviewlabel.h
    fullscreenimagepreview.h
)

# 1. 首先，使用 qt_add_executable() 创建可执行目标
qt_add_executable(TcpServer
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    mainwindow.ui
)

# 2. 然后，为这个目标添加头文件目录
target_include_directories(TcpServer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}) # ✅ 这行代码应该放在 qt_add_executable 后面

# 3. 接着，为目标链接库
target_link_libraries(TcpServer PRIVATE
    Qt6::Widgets
    Qt6::Network
)

# 设置目标属性
set_target_properties(TcpServer PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.TcpServer
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# 安装目标
include(GNUInstallDirs)
install(TARGETS TcpServer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 必要的 Qt 6 最终化步骤
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TcpServer)
endif()
