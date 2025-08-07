cmake_minimum_required(VERSION 3.16)
project(BrowserSelector LANGUAGES CXX)

find_package(FLTK REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(browserselector src/main.cpp)

# Include directories
include_directories(${FLTK_INCLUDE_DIR})

# Link libraries
if(TARGET nlohmann_json::nlohmann_json)
  target_link_libraries(browserselector PRIVATE ${FLTK_LIBRARIES} nlohmann_json::nlohmann_json)
else()
  target_link_libraries(browserselector PRIVATE ${FLTK_LIBRARIES})
endif()

if(WIN32)
  target_link_libraries(browserselector PRIVATE shell32)
endif()

target_compile_features(browserselector PRIVATE cxx_std_17)

