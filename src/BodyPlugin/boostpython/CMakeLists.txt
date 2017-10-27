set(target PyBodyPlugin)

add_cnoid_python_module(${target}
  PyBodyPluginModule.cpp
  PyBodyItem.cpp
  PyItems.cpp
  PySimulationClasses.cpp
)
if(WIN32)
  add_definitions("/wd4251 /wd4244 /wd4524 /wd4624")
endif()
target_link_libraries(${target} CnoidBodyPlugin CnoidPyBase)
