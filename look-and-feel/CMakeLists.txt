project(hello-look-and-feel)

install(DIRECTORY
        "hello dark"
        "hello light"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/plasma/desktoptheme")
