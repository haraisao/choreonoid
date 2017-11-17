set(target PyPoseSeqPlugin)
add_definitions("-I..")
add_cnoid_python_module(${target} PyPoseSeqPlugin.cpp)
target_link_libraries(${target} CnoidPoseSeqPlugin CnoidPyBase)
