project(hello)
set(PROJECT_VERSION "5.16.5")
set(PROJECT_VERSION_MAJOR 5)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

include(GenerateExportHeader)
include(WriteBasicConfigVersionFile)
include(FeatureSummary)


find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_SOURCE_DIR}/cmake)

include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(KDecoration2 REQUIRED)
add_subdirectory(kdecoration)

add_subdirectory(libhellocommon)

find_package(KF5Package CONFIG REQUIRED)

include(ECMSetupVersion)
ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX HELLO
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/HelloConfigVersion.cmake"
                  )

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/hello")

include(ECMPackageConfigHelpers)
ecm_configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/HelloConfig.cmake.in"
                                  "${CMAKE_CURRENT_BINARY_DIR}/HelloConfig.cmake"
                                  PATH_VARS KDE_INSTALL_FULL_DATADIR
                                  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                                  )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/HelloConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/HelloConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
       )

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
