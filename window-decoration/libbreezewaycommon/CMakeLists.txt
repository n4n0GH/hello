################# dependencies #################
### Qt/KDE
find_package(Qt5 REQUIRED CONFIG COMPONENTS Widgets)

################# configuration #################
configure_file(config-breezewaycommon.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-breezewaycommon.h )

################# breezewaystyle target #################
set(breezewaycommon_LIB_SRCS
    breezewayboxshadowrenderer.cpp
)

add_library(breezewaycommon5 ${breezewaycommon_LIB_SRCS})

generate_export_header(breezewaycommon5
    BASE_NAME breezewaycommon
    EXPORT_FILE_NAME breezewaycommon_export.h)

target_link_libraries(breezewaycommon5
    PUBLIC
        Qt5::Core
        Qt5::Gui)

set_target_properties(breezewaycommon5 PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

install(TARGETS breezewaycommon5 ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
