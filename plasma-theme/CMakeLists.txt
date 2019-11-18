project(hello-plasma)

set(THEMES
    "hello dark"
    "hello light")

install(DIRECTORY ${THEMES}
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/plasma/desktoptheme")
