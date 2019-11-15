project(hello-look-and-feel)

set(THEMES
    "hello dark"
    "hello light")

install(DIRECTORY ${THEMES}
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/plasma/desktoptheme")
