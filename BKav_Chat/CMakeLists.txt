cmake_minimum_required(VERSION 3.19)

project(BKav_Chat LANGUAGES CXX)



find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools Sql)



qt_standard_project_setup()


file(GLOB SOURCES "*.cpp")
file(GLOB HEADERS "*.h")
file(GLOB FORMS "*.ui")


qt_add_executable(BKav_Chat

    WIN32 MACOSX_BUNDLE

    main.cpp

    mainwindow.cpp

    mainwindow.h

    mainwindow.ui

    login.h login.cpp login.ui

    signup.h signup.cpp signup.ui





    errorconnectionnetwork.h errorconnectionnetwork.cpp errorconnectionnetwork.ui

    signupmodel.h signupmodel.cpp

    loginmodel.h

    loginmodel.cpp


    dashboardmodel.h
    dashboard.h
    dashboardmodel.cpp
    dashboard.cpp
    appconfig.h
    chatmodel.h
    chat.h
    chatmodel.cpp
    chat.cpp
    SocketManager.h
    SocketManager.cpp
    chatdelegate.h
    chatdelegate.cpp
    friendproxymodel.h
    friendproxymodel.cpp
    avatarcache.h
    avatarcache.cpp
    DatabaseManager.h
    DatabaseManager.cpp
    SecurityUtils.h
    SecurityUtils.cpp
    AccountManage.h
    imagecache.h



)



qt_add_translations(

    TARGETS BKav_Chat

    TS_FILES BKav_Chat_vi_VN.ts

)



target_link_libraries(BKav_Chat

    PRIVATE

        Qt::Core

        Qt::Widgets

        Qt::Sql

)



install(TARGETS BKav_Chat

    BUNDLE  DESTINATION .

    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}

    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}

)



qt_generate_deploy_app_script(

    TARGET BKav_Chat

    OUTPUT_SCRIPT deploy_script

    NO_UNSUPPORTED_PLATFORM_ERROR

)

find_package(Qt6 REQUIRED COMPONENTS Core Sql)
target_link_libraries(BKav_Chat
  PRIVATE
    Qt6::Sql
    Qt6::Sql
    Qt6::WebSockets
)

find_package(Qt6 REQUIRED COMPONENTS WebSockets Network)

#target_include_directories(BKav_Chat PRIVATE "C:/Program Files/MySQL/MySQL Server 9.7/include")
#target_link_libraries(BKav_Chat PRIVATE "C:/Program Files/MySQL/MySQL Server 9.7/lib/mysql")

#target_link_libraries(BKav_Chat PRIVATE libmysql)

install(SCRIPT ${deploy_script})