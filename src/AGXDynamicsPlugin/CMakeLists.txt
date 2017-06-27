option(BUILD_AGX_DYNAMICS_PLUGIN "Building AGXDynamicsPlugin" OFF)

if(NOT BUILD_AGX_DYNAMICS_PLUGIN)
  return()
endif()

IF ("$ENV{AGX_DIR}"  STREQUAL "")
  MESSAGE(WARNING "\n*** The environment initialization file setup_env.bat/bash has not been executed\n*** Errors might occur during runtime!")
ENDIF()

SET(AGX_DIR $ENV{AGX_DIR} CACHE PATH "set the top directory of the AGX Dynamics library")

if(UNIX)
set(AGX_INCLUDE_DIRS ${AGX_DIR}/include)
set(AGX_LIBRARY_DIRS ${AGX_DIR}/lib/)
set(AGX_LIBRARIES agxSabre agxCore agxPhysics agxModel) 
endif()

#add_definitions(${AGX_CFLAGS})
include_directories(${AGX_INCLUDE_DIRS} ${AGX_INCLUDE_DIRS}/1.4)
link_directories(${AGX_LIBRARY_DIRS})

set(target CnoidAGXDynamicsPlugin)

set(sources
  AGXDynamicsPlugin.cpp
  AGXSimulatorItem.cpp
  )

set(headers
  )

make_gettext_mofiles(${target} mofiles)
add_cnoid_plugin(${target} SHARED ${sources} ${headers} ${mofiles})
target_link_libraries(${target} CnoidBodyPlugin ${AGX_LIBRARIES})

get_target_property(compile_flags ${target} COMPILE_FLAGS)
if(NOT compile_flags)
  set(compile_flags "")
endif()
set_target_properties(${target} PROPERTIES COMPILE_FLAGS "${compile_flags} -std=c++11 -msse3")
apply_common_setting_for_plugin(${target} "${headers}")