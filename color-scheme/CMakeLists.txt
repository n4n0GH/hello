project(hello-color-scheme)

install(FILES
        HelloDark.colors
        HelloLight.colors
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/color-schemes")
