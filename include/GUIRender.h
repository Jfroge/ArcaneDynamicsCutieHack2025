#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class GUIRender {
    public:
        void Init(GLFWwindow* window, const char* glsl_version);
        void NewFrame();
        virtual void Update(GLFWwindow* window);
        void Render();
        void Shutdown();
    private:
        ImFont* customFont = nullptr;
};