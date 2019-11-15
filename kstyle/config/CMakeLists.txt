### configuration plugin
add_definitions(-DTRANSLATION_DOMAIN=\"hello_style_config\")
set(kstyle_hello_config_PART_SRCS hellostyleconfig.cpp)

if(hello_USE_KDE4)

  kde4_add_kcfg_files(kstyle_hello_config_PART_SRCS ../hellostyleconfigdata.kcfgc)
  kde4_add_ui_files(kstyle_hello_config_PART_SRCS ui/hellostyleconfig.ui)
  kde4_add_plugin(kstyle_hello_config ${kstyle_hello_config_PART_SRCS})

  target_link_libraries(kstyle_hello_config ${KDE4_KDEUI_LIBS} )
  install(TARGETS kstyle_hello_config DESTINATION ${PLUGIN_INSTALL_DIR})

else()

  set(kstyle_hello_config_PART_SRCS ${kstyle_hello_config_PART_SRCS} hellostyleconfigmodule.cpp)

  kconfig_add_kcfg_files(kstyle_hello_config_PART_SRCS ../hellostyleconfigdata.kcfgc)
  set(kstyle_hello_config_PART_FORMS ui/hellostyleconfig.ui)
  ki18n_wrap_ui(kstyle_hello_config_PART_FORMS_HEADERS ${kstyle_hello_config_PART_FORMS})

  add_library(kstyle_hello_config MODULE
    ${kstyle_hello_config_PART_SRCS}
    ${kstyle_hello_config_PART_FORMS_HEADERS}
 )

  target_link_libraries(kstyle_hello_config Qt5::Core Qt5::Gui Qt5::Widgets Qt5::DBus)
  target_link_libraries(kstyle_hello_config KF5::ConfigCore KF5::ConfigWidgets KF5::I18n)
  install(TARGETS kstyle_hello_config DESTINATION ${QT_PLUGIN_INSTALL_DIR})
  install(FILES hellostyleconfig.desktop DESTINATION  ${SERVICES_INSTALL_DIR})

endif()

########### next target ###############
if(NOT hello_USE_KDE4)

  find_package(KF5KCMUtils CONFIG)
  set_package_properties(KF5KCMUtils PROPERTIES
      DESCRIPTION "KF5 KCM utils Framework"
      URL "https://projects.kde.org/projects/frameworks/kcmutils"
      TYPE OPTIONAL
      PURPOSE "Required for hello-settings5 application to build")

  if( KF5KCMUtils_FOUND )

      set(hello_settings_SOURCES main.cpp)
      add_executable(hello-settings5 ${hello_settings_SOURCES} )
      target_link_libraries(hello-settings5 Qt5::Core Qt5::Gui Qt5::Widgets )
      target_link_libraries(hello-settings5 KF5::I18n KF5::KCMUtils)

      install(TARGETS hello-settings5 ${INSTALL_TARGETS_DEFAULT_ARGS})

      # icon
      ecm_install_icons(ICONS sc-apps-hello-settings.svgz
        DESTINATION ${ICON_INSTALL_DIR}
        THEME hicolor )
    endif()
endif()
