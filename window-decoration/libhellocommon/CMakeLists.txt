################# dependencies #################
### Qt/KDE
find_package(Qt5 REQUIRED CONFIG COMPONENTS Widgets)

################# configuration #################
configure_file(config-hellocommon.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-hellocommon.h )

################# hellostyle target #################
set(hellocommon_LIB_SRCS
    helloboxshadowrenderer.cpp
)

add_library(hellocommon5 ${hellocommon_LIB_SRCS})

generate_export_header(hellocommon5
    BASE_NAME hellocommon
    EXPORT_FILE_NAME hellocommon_export.h)

target_link_libraries(hellocommon5
    PUBLIC
        Qt5::Core
        Qt5::Gui)

set_target_properties(hellocommon5 PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

install(TARGETS hellocommon5 ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
