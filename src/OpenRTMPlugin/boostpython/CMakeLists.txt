set(target PyOpenRTMPlugin)
add_cnoid_python_module(${target} PyOpenRTMPlugin.cpp)
target_link_libraries(${target} CnoidOpenRTMPlugin CnoidPyBase)

if(MSVC)
  add_definitions("/wd4005 /wd4251")
endif()
