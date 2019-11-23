project(hello-plasma)

set(THEMES
    "hellodark"
    "hellolight")

install(DIRECTORY ${THEMES}
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/plasma/desktoptheme")
