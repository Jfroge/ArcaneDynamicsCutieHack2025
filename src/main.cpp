#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "../include/GUIRender.h"
#include <iostream>

int main() {

    // setup winder
    if(!glfwInit()) return 1;

    // GL 3.2 + GLSL version
    const char *glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // because version 3.2
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // create window (context current)
    GLFWwindow *window = glfwCreateWindow(1280, 720, "Arcane Dynamics", NULL, NULL);
    if(window == NULL) return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw("unable to context to OpenGL");
    
    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    glViewport(0,0, screenWidth, screenHeight);

    GUIRender GUI;
    GUI.Init(window, glsl_version);

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        GUI.NewFrame();
        GUI.Update();
        GUI.Render();
        glfwSwapBuffers(window);
    }

    GUI.Shutdown();
    return 0;
}