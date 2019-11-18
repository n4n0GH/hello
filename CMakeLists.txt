cmake_minimum_required(VERSION 3.0)
project(hello)

set(WITH_COLOR_SCHEME      yes CACHE BOOL "Build with color-scheme")
set(WITH_KSTYLE            no  CACHE BOOL "Build with kstyle")
set(WITH_KWIN_EFFECTS      yes CACHE BOOL "Build with kwin-effects")
set(WITH_PLASMA            yes CACHE BOOL "Build with plasma theme")
set(WITH_WINDOW_DECORATION yes CACHE BOOL "Build with window-decoration")

if(WITH_COLOR_SCHEME)
  add_subdirectory(color-scheme)
endif()

if(WITH_KSTYLE)
  add_subdirectory(kstyle)
endif()

if(WITH_KWIN_EFFECTS)
  add_subdirectory(kwin-effects)
endif()

if(WITH_PLASMA)
  add_subdirectory(plasma-theme)
endif()

if(WITH_WINDOW_DECORATION)
  add_subdirectory(window-decoration)
endif()
