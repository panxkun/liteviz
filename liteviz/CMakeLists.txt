superbuild_depend(eigen)
superbuild_depend(imgui)
superbuild_depend(glfw)
superbuild_depend(glad)
superbuild_extern(opencv)

add_library(liteviz-core
    SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/core/detail.cpp
)

target_include_directories(liteviz-core
    PUBLIC
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(liteviz-core
    PUBLIC
    GLU
    depends::eigen
    depends::glad
    depends::glfw
    depends::imgui
    depends::opencv
)

target_compile_definitions(liteviz-core
    PUBLIC 
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    _GLIBCXX_USE_CXX11_ABI=0
)