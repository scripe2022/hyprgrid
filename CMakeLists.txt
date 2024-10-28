set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

project(hyprGrid CXX)
set(CMAKE_CXX_STANDARD 23)

# add_compile_definitions(WLR_USE_UNSTABLE)
add_compile_options(-fPIC)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PIXMAN REQUIRED pixman-1)
pkg_check_modules(LIBDRM REQUIRED libdrm)
pkg_check_modules(HYPRLAND REQUIRED hyprland)

file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_library(hyprgrid SHARED ${SOURCES})

target_link_libraries(hyprgrid PRIVATE ${PIXMAN_LIBRARIES} ${LIBDRM_LIBRARIES} ${HYPRLAND_LIBRARIES})

target_include_directories(hyprgrid PRIVATE ${PROJECT_SOURCE_DIR}/include ${PIXMAN_INCLUDE_DIRS} ${LIBDRM_INCLUDE_DIRS} ${HYPRLAND_INCLUDE_DIRS})

target_compile_options(hyprgrid PRIVATE -g)

