set(hello_USE_KDE4 ${USE_KDE4})
if(hello_USE_KDE4)

  ############ Language and toolchain features
  ############ copied from ECM
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel" AND NOT WIN32)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  endif()

endif()

################# Qt/KDE #################
if(hello_USE_KDE4)

  ### XCB
  if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(XCB xcb x11-xcb)
    add_feature_info("x11-xcb" XCB_FOUND "Required to pass style properties to native Windows on X11 Platform")
    set_feature_info("x11-xcb" "Required to pass style properties to native Windows on X11 Platform" "http://xcb.freedesktop.org")
    set(hello_HAVE_X11 ${XCB_FOUND})
  else()
    set(hello_HAVE_X11 FALSE)
  endif()

  set(hello_HAVE_QTQUICK FALSE)
  set(hello_HAVE_KWAYLAND FALSE)

  ### KStyle
  set(hello_HAVE_KSTYLE FALSE)

else()

  find_package(Qt5 REQUIRED CONFIG COMPONENTS Widgets DBus)
  find_package(KF5 REQUIRED COMPONENTS
      I18n
      Config
      GuiAddons
      ConfigWidgets
      WindowSystem)

  find_package(Qt5 COMPONENTS Quick)
  set(hello_HAVE_QTQUICK ${Qt5Quick_FOUND})

  find_package( KF5FrameworkIntegration CONFIG )
  set_package_properties(KF5FrameworkIntegration PROPERTIES
      DESCRIPTION "KF5 Framework Integration"
      URL "https://projects.kde.org/projects/frameworks/frameworkintegration"
      TYPE OPTIONAL
      PURPOSE "Required to use KStyle convenience functionalities in style")

  set(hello_HAVE_KSTYLE ${KF5FrameworkIntegration_FOUND})

  find_package(XCB COMPONENTS XCB)
  set_package_properties(XCB PROPERTIES
      DESCRIPTION "X protocol C-language Binding"
      URL "http://xcb.freedesktop.org"
      TYPE OPTIONAL
      PURPOSE "Required to pass style properties to native Windows on X11 Platform"
  )

  find_package(KF5Wayland CONFIG)
  set(hello_HAVE_KWAYLAND ${KF5Wayland_FOUND})

  if(UNIX AND NOT APPLE)

    set(hello_HAVE_X11 ${XCB_XCB_FOUND})
    if (XCB_XCB_FOUND)
      find_package(Qt5 REQUIRED CONFIG COMPONENTS X11Extras)
    endif()

  else()

    set(hello_HAVE_X11 FALSE)

  endif()


endif()

################# includes #################
include_directories(
  animations
  debug
)

include_directories(${CMAKE_SOURCE_DIR}/libhellocommon)
include_directories(${CMAKE_BINARY_DIR}/libhellocommon)

################# configuration #################
configure_file(config-hello.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-hello.h )

########### next target ###############
set(hello_PART_SRCS
    animations/helloanimation.cpp
    animations/helloanimations.cpp
    animations/helloanimationdata.cpp
    animations/hellobaseengine.cpp
    animations/hellobusyindicatordata.cpp
    animations/hellobusyindicatorengine.cpp
    animations/hellodialdata.cpp
    animations/hellodialengine.cpp
    animations/helloenabledata.cpp
    animations/hellogenericdata.cpp
    animations/helloheaderviewdata.cpp
    animations/helloheaderviewengine.cpp
    animations/helloscrollbardata.cpp
    animations/helloscrollbarengine.cpp
    animations/hellospinboxengine.cpp
    animations/hellospinboxdata.cpp
    animations/hellostackedwidgetdata.cpp
    animations/hellostackedwidgetengine.cpp
    animations/hellotabbarengine.cpp
    animations/hellotabbardata.cpp
    animations/hellotoolboxengine.cpp
    animations/hellotransitiondata.cpp
    animations/hellotransitionwidget.cpp
    animations/hellowidgetstateengine.cpp
    animations/hellowidgetstatedata.cpp
    debug/hellowidgetexplorer.cpp
    helloaddeventfilter.cpp
    helloframeshadow.cpp
    hellohelper.cpp
    hellomdiwindowshadow.cpp
    hellomnemonics.cpp
    hellopropertynames.cpp
    helloshadowhelper.cpp
    hellosplitterproxy.cpp
    hellostyle.cpp
    hellostyleplugin.cpp
    hellotileset.cpp
    hellowindowmanager.cpp
)

if(NOT hello_USE_KDE4)
  set(hello_PART_SRCS
      ${hello_PART_SRCS}
      helloblurhelper.cpp
  )
endif()

if(hello_USE_KDE4)

  kde4_add_kcfg_files(hello_PART_SRCS hellostyleconfigdata.kcfgc)
  kde4_add_plugin(hello ${hello_PART_SRCS} kstylekde4compat.cpp)
  target_link_libraries(hello  ${KDE4_KDEUI_LIBS})
  target_link_libraries(hello hellocommon4)

  if(hello_HAVE_X11)
    target_link_libraries(hello ${X11_XCB_LIBRARIES})
    target_link_libraries(hello ${XCB_LIBRARIES})
  endif()

  install(TARGETS hello  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/styles/)

else()

  kconfig_add_kcfg_files(hello_PART_SRCS hellostyleconfigdata.kcfgc)
  add_library(hello MODULE ${hello_PART_SRCS})
  target_link_libraries(hello Qt5::Core Qt5::Gui Qt5::Widgets Qt5::DBus)
  if( hello_HAVE_QTQUICK )
    target_link_libraries(hello Qt5::Quick)
  endif()
  target_link_libraries(hello KF5::ConfigCore KF5::ConfigWidgets KF5::GuiAddons KF5::WindowSystem)
  target_link_libraries(hello hellocommon5)

  if( KF5FrameworkIntegration_FOUND )
  target_link_libraries(hello KF5::Style)
  endif()

  if (WIN32)
    # As stated in http://msdn.microsoft.com/en-us/library/4hwaceh6.aspx M_PI only gets defined
    # when if _USE_MATH_DEFINES is defined
    target_compile_definitions(hello PRIVATE _USE_MATH_DEFINES _BSD_SOURCE)
  endif()

  if(hello_HAVE_X11)
    target_link_libraries(hello ${XCB_LIBRARIES})
    target_link_libraries(hello Qt5::X11Extras)
  endif()

  if(hello_HAVE_KWAYLAND)
    target_link_libraries(hello KF5::WaylandClient)
  endif()

  install(TARGETS hello DESTINATION ${QT_PLUGIN_INSTALL_DIR}/styles/)

endif()

########### install files ###############
install(FILES hello.themerc  DESTINATION  ${DATA_INSTALL_DIR}/kstyle/themes)

########### subdirectories ###############
add_subdirectory(config)
