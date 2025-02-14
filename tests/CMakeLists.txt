find_package (Boost COMPONENTS system filesystem unit_test_framework iostreams REQUIRED)

# find_package(Torch REQUIRED)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

# include_directories(${SDMS_INCLUDE_DIRS}
#                     ${Boost_INCLUDE_DIRS}
# 					          ${TORCH_INCLUDE_DIRS}
# 					          )
          
add_definitions (-DBOOST -DDENSE -DBOOST_TEST_DYN_LINK)

# Store *.cpp files in a liste
file(
	GLOB
	test_files
	*.cpp
)

# For each .cpp file (sub-programs), make the associated main program
foreach(f ${test_files})
  get_filename_component(testName ${f} NAME_WE) 
  add_executable (${testName} ${f}) 
  target_link_libraries(${testName} -L${SDMS_LIB_DIR} dl ${TORCH_LIBRARIES} ${Boost_LIBRARIES} ${SDMS_LIB_DIR}/libtb2.so ${LIB_SDMS})
  add_test( NAME ${testName} 
            COMMAND ${testName}
          )
endforeach(f)



# add_executable (test_world test_world.cpp) 
# target_link_libraries(test_world
#   fmt
#   ${TORCH_LIBRARIES}
#   ${Boost_LIBRARIES}
#   ${LIB_SDMS}
#   )

# add_executable (test_space test_space.cpp) 
# target_link_libraries(test_space
#   ${TORCH_LIBRARIES}
#   ${Boost_LIBRARIES}
#   ${LIB_SDMS}
#   )
