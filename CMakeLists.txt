cmake_minimum_required(VERSION 3.10)
project(ArcaneDynamics)

set(CMAKE_CXX_STANDARD 17)

# --- GLFW ---
add_subdirectory(dependencies/glfw)


# --- GLAD ---
add_library(glad
    dependencies/glad/src/glad.c
)
target_include_directories(glad PUBLIC dependencies/glad/include)

# --- IMGUI ---
file(GLOB IMGUI_SOURCES
    dependencies/imgui/*.cpp
    dependencies/imgui/backends/imgui_impl_glfw.cpp
    dependencies/imgui/backends/imgui_impl_opengl3.cpp
)
add_library(imgui ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC dependencies/imgui)
target_include_directories(imgui PUBLIC dependencies/imgui/backends)

# --- implot ---


# imgui depends on glfw and glad
target_link_libraries(imgui
    PUBLIC glfw
    PUBLIC glad
)

# --- MAIN APP ---
add_executable(ArcaneDynamics 
    src/main.cpp
    src/GuiRender.cpp

    dependencies/implot/implot.cpp
    dependencies/implot/implot_items.cpp
    dependencies/implot/implot.h


)

add_executable(mathTest 
    src/mathTest.cpp
    src/ArcaneMath.cpp
)

target_include_directories(ArcaneDynamics PUBLIC
    include
    dependencies/glad/include
    dependencies/imgui
    dependencies/imgui/backends
    dependencies/implot

)

# Link all dependencies using keyword style
target_link_libraries(ArcaneDynamics
    PRIVATE imgui
)


# macOS frameworks
if(APPLE)
    target_link_libraries(ArcaneDynamics
        PRIVATE "-framework Cocoa"
        PRIVATE "-framework OpenGL"
        PRIVATE "-framework IOKit"
        PRIVATE "-framework CoreVideo"
    )
endif()