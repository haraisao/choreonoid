#
#
#
cmake_minimum_required(VERSION 3.0.2)

if(NOT ENABLE_GUI)
  return()
endif()

option(BUILD_SIMPLE_GRAPHICS_VIEW "Building s simple graphics view plugin" OFF)

if(BUILD_SIMPLE_GRAPHICS_VIEW)
  set(CMAKE_AUTOMOC ON)
  set(plugin CnoidSimpleGraphicsPlugin)

  set(plugin_sources
    SimpleGraphicsPlugin.cpp
	SimpleGraphicsView.cpp
  )
  set(plugin_headers
    SimpleGraphicsPlugin.h
	SimpleGraphicsView.h
  )

  add_cnoid_plugin(${plugin} SHARED ${plugin_sources} ${plugin_headers} )


  QT5_ADD_RESOURCES(RC_SRCS SimpleGraphicsPlugin.qrc)
#  QT5_WRAP_CPP(
#    plugin_sources
#	SimpleGraphicsView.h
#  )

if(MSVC)
add_definitions(-D__WIN32__ -D__x86__ -D__NT__ -D__OSVERSION__=4 -D_CRT_SECURE_NO_DEPRECATE -D_WIN32_WINNT=0x0500 )
endif()

  target_link_libraries(${plugin} CnoidBase)
  apply_common_setting_for_plugin(${plugin})

  if(QT5)
    qt5_use_modules(${plugin} Widgets)
  endif()
endif()

