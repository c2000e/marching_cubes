add_library(marching_cubes_lib STATIC marching_cubes.cc)
target_compile_features(marching_cubes_lib PUBLIC cxx_std_11)
target_include_directories(marching_cubes_lib PUBLIC ../include)
