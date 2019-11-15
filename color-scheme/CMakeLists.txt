project(hello-color-scheme)

set(COLOR_THEMES
    HelloDark.colors
    HelloLight.colors)

install(FILES ${COLOR_THEMES}
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/color-schemes")
