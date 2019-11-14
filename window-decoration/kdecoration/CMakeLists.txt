add_definitions(-DTRANSLATION_DOMAIN="breezeway_kwin_deco")

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

  set(BREEZEWAY_HAVE_X11 ${XCB_XCB_FOUND})
  if (XCB_XCB_FOUND)
    find_package(Qt5 REQUIRED CONFIG COMPONENTS X11Extras)
  endif()

else()

  set(BREEZEWAY_HAVE_X11 FALSE)

endif()

################# configuration #################
configure_file(config-breezeway.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-breezeway.h )

################# includes #################
include_directories(${CMAKE_SOURCE_DIR}/libbreezewaycommon)
include_directories(${CMAKE_BINARY_DIR}/libbreezewaycommon)

################# newt target #################
### plugin classes
set(breezewaydecoration_SRCS
    breezewaybutton.cpp
    breezewaydecoration.cpp
    breezewayexceptionlist.cpp
    breezewaysettingsprovider.cpp
    breezewaysizegrip.cpp)

kconfig_add_kcfg_files(breezewaydecoration_SRCS breezewaysettings.kcfgc)

### config classes
### they are kept separately because they might move in a separate library in the future
set(breezewaydecoration_config_SRCS
    config/breezewayconfigwidget.cpp
    config/breezewaydetectwidget.cpp
    config/breezewayexceptiondialog.cpp
    config/breezewayexceptionlistwidget.cpp
    config/breezewayexceptionmodel.cpp
    config/breezewayitemmodel.cpp
)

set(breezewaydecoration_config_PART_FORMS
   config/ui/breezewayconfigurationui.ui
   config/ui/breezewaydetectwidget.ui
   config/ui/breezewayexceptiondialog.ui
   config/ui/breezewayexceptionlistwidget.ui
)

ki18n_wrap_ui(breezewaydecoration_config_PART_FORMS_HEADERS ${breezewaydecoration_config_PART_FORMS})

### build library
add_library(breezewaydecoration MODULE
    ${breezewaydecoration_SRCS}
    ${breezewaydecoration_config_SRCS}
    ${breezewaydecoration_config_PART_FORMS_HEADERS})

target_link_libraries(breezewaydecoration
    PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::DBus
    PRIVATE
        breezewaycommon5
        KDecoration2::KDecoration
        KF5::ConfigCore
        KF5::CoreAddons
        KF5::ConfigWidgets
        KF5::GuiAddons
        KF5::I18n
        KF5::WindowSystem)

if(BREEZEWAY_HAVE_X11)
  target_link_libraries(breezewaydecoration
    PUBLIC
      Qt5::X11Extras
      XCB::XCB)
endif()


install(TARGETS breezewaydecoration DESTINATION ${PLUGIN_INSTALL_DIR}/org.kde.kdecoration2)
install(FILES config/breezewaydecorationconfig.desktop DESTINATION  ${SERVICES_INSTALL_DIR})
