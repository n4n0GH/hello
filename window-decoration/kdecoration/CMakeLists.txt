add_definitions(-DTRANSLATION_DOMAIN="hello_kwin_deco")

find_package(KF5 REQUIRED COMPONENTS CoreAddons GuiAddons ConfigWidgets WindowSystem I18n)
find_package(Qt5 CONFIG REQUIRED COMPONENTS DBus)

### XCB
find_package(XCB COMPONENTS XCB)
set_package_properties(XCB PROPERTIES
  DESCRIPTION "X protocol C-language Binding"
  URL "http://xcb.freedesktop.org"
  TYPE OPTIONAL
  PURPOSE "Required to pass style properties to native Windows on X11 Platform"
)

if(UNIX AND NOT APPLE)

  set(HELLO_HAVE_X11 ${XCB_XCB_FOUND})
  if (XCB_XCB_FOUND)
    find_package(Qt5 REQUIRED CONFIG COMPONENTS X11Extras)
  endif()

else()

  set(HELLO_HAVE_X11 FALSE)

endif()

################# configuration #################
configure_file(config-hello.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-hello.h )

################# includes #################
include_directories(${CMAKE_SOURCE_DIR}/libhellocommon)
include_directories(${CMAKE_BINARY_DIR}/libhellocommon)

################# newt target #################
### plugin classes
set(hellodecoration_SRCS
    hellobutton.cpp
    hellodecoration.cpp
    helloexceptionlist.cpp
    hellosettingsprovider.cpp
    hellosizegrip.cpp)

kconfig_add_kcfg_files(hellodecoration_SRCS hellosettings.kcfgc)

### config classes
### they are kept separately because they might move in a separate library in the future
set(hellodecoration_config_SRCS
    config/helloconfigwidget.cpp
    config/hellodetectwidget.cpp
    config/helloexceptiondialog.cpp
    config/helloexceptionlistwidget.cpp
    config/helloexceptionmodel.cpp
    config/helloitemmodel.cpp
)

set(hellodecoration_config_PART_FORMS
   config/ui/helloconfigurationui.ui
   config/ui/hellodetectwidget.ui
   config/ui/helloexceptiondialog.ui
   config/ui/helloexceptionlistwidget.ui
)

ki18n_wrap_ui(hellodecoration_config_PART_FORMS_HEADERS ${hellodecoration_config_PART_FORMS})

### build library
add_library(hellodecoration MODULE
    ${hellodecoration_SRCS}
    ${hellodecoration_config_SRCS}
    ${hellodecoration_config_PART_FORMS_HEADERS})

target_link_libraries(hellodecoration
    PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::DBus
    PRIVATE
        hellocommon5
        KDecoration2::KDecoration
        KF5::ConfigCore
        KF5::CoreAddons
        KF5::ConfigWidgets
        KF5::GuiAddons
        KF5::I18n
        KF5::WindowSystem)

if(HELLO_HAVE_X11)
  target_link_libraries(hellodecoration
    PUBLIC
      Qt5::X11Extras
      XCB::XCB)
endif()


install(TARGETS hellodecoration DESTINATION ${PLUGIN_INSTALL_DIR}/org.kde.kdecoration2)
install(FILES config/hellodecorationconfig.desktop DESTINATION  ${SERVICES_INSTALL_DIR})
