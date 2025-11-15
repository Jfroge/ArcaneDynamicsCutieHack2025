#include "../include/GUIRender.h"
#include "implot.h"
#include <string>
#include <iostream>
#include <GLFW/glfw3.h>
void SetArcaneDynamicsStyle() {
    
    ImGuiStyle& style = ImGui::GetStyle();
    
    // --- Define Core Colors (retained from previous fantasy theme) ---
    ImVec4 color_parchment = ImVec4(0.95f, 0.90f, 0.75f, 1.00f); 
    ImVec4 color_wood = ImVec4(0.20f, 0.15f, 0.10f, 1.00f); 
    ImVec4 color_sigil = ImVec4(0.10f, 0.45f, 0.90f, 1.00f); 
    ImVec4 color_ink = ImVec4(0.05f, 0.05f, 0.05f, 1.00f); 

    // --- NEW BUTTON COLORS ---
    ImVec4 color_ivy_green = ImVec4(0.25f, 0.45f, 0.20f, 1.00f); // A deep, subtle green
    ImVec4 color_moss_brown = ImVec4(0.40f, 0.30f, 0.20f, 1.00f); // A rustic brown for hover
    ImVec4 color_pressed_brown = ImVec4(0.50f, 0.35f, 0.25f, 1.00f); // Slightly lighter brown for active click

    // --- Backgrounds & Borders ---
    style.Colors[ImGuiCol_Text]                  = color_ink;
    style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_WindowBg]              = color_parchment;
    style.Colors[ImGuiCol_ChildBg]               = color_parchment;
    style.Colors[ImGuiCol_Border]                = color_wood;
    style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // --- Headers & Title Bar ---
    style.Colors[ImGuiCol_TitleBg]               = color_wood;
    style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.30f, 0.25f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed]      = color_wood;
    style.Colors[ImGuiCol_Header]                = color_wood;
    style.Colors[ImGuiCol_HeaderHovered]         = color_sigil;
    style.Colors[ImGuiCol_HeaderActive]          = color_sigil;

    // --- Controls (Input fields, sliders, etc.) ---
    style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.85f, 0.80f, 0.65f, 1.00f); 
    style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.80f, 0.75f, 0.60f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.75f, 0.70f, 0.55f, 1.00f);

    // --- BUTTON COLORS (THE CHANGE) ---
    style.Colors[ImGuiCol_Button]                = color_ivy_green;      // Ivy Green by default
    style.Colors[ImGuiCol_ButtonHovered]         = color_moss_brown;     // Turns Moss Brown on hover
    style.Colors[ImGuiCol_ButtonActive]          = color_pressed_brown;  // Slightly lighter brown when clicked

    // --- Scrollbar & Checkmarks ---
    style.Colors[ImGuiCol_CheckMark]             = color_sigil;
    style.Colors[ImGuiCol_ScrollbarBg]           = color_parchment;
    style.Colors[ImGuiCol_ScrollbarGrab]         = color_wood;

    // --- Style Adjustments ---
    style.WindowRounding = 4.0f;
    style.FrameRounding = 2.0f; 
    style.GrabRounding = 2.0f;
    style.FrameBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.WindowBorderSize = 1.0f;
    
    // 2. Setup ImPlot Style (No change needed here for button colors)
    ImPlotStyle& plot_style = ImPlot::GetStyle();
    plot_style.Colors[ImPlotCol_PlotBg]     = ImVec4(0.90f, 0.85f, 0.70f, 1.00f);
    plot_style.Colors[ImPlotCol_AxisBg]     = color_parchment;
    // plot_style.Colors[ImPlotCol_Grid]       = ImVec4(0.00f, 0.00f, 0.00f, 0.15f);
    // plot_style.Colors[ImPlotCol_MinorGrid]  = ImVec4(0.00f, 0.00f, 0.00f, 0.05f); 
    // plot_style.Colors[ImPlotCol_XAxis]      = color_ink;
    // plot_style.Colors[ImPlotCol_YAxis]      = color_ink;
    // plot_style.Colors[ImPlotCol_HighlightLine] = color_sigil;
    plot_style.Colors[ImPlotCol_Selection]     = ImVec4(0.10f, 0.45f, 0.90f, 0.25f);
}

void GUIRender::Init(GLFWwindow* window, const char* glsl_version) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    SetArcaneDynamicsStyle();
    ImGuiIO &io = ImGui::GetIO();

    this->customFont = io.Fonts->AddFontFromFileTTF(
        "../include/Ancient-Medium.ttf", // <<< IMPORTANT: Update this path
        25.0f // Size of the font
    );
    // Setup platform/render bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);


}

void GUIRender::NewFrame() {
    // feed inputs into imgui, start new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUIRender::Update(GLFWwindow* window) {
    // Persistent state to control plot visibility
    static bool g_ShowPlots = false; 

    if(this->customFont) {
        ImGui::PushFont(this->customFont);
    }

    int display_w, display_h;
    glfwGetWindowSize(window, &display_w, &display_h);

    float main_window_width = (float)display_w / 3.0f;
    float screen_height = (float)display_h;

    float section_height = screen_height / 3.0f; 

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(main_window_width, screen_height), ImGuiCond_Always);
    
    ImGuiWindowFlags main_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar;
    
    if(ImGui::Begin("MainControlPanel", nullptr, main_flags)){

        // Inputs section (top 1/3 of slice)

        if (ImGui::BeginChild("InputSection", ImVec2(-1, section_height), true)) {

            ImGui::SetWindowFontScale(1.5f); 
            ImGui::Text("Arcane Dynamics");
            ImGui::SetWindowFontScale(1.0f); 
            ImGui::Separator(); 

            ImGui::Text("2D Kinematics *Projectile Motion");
            ImGui::Spacing();

            ImGui::TextWrapped("If You know the value enter the input then check the box");
            ImGui::Spacing();

            static float theta_val = 0.0f; static bool theta_checked = false;
            static float height_val = 0.0f; static bool height_checked = false;
            static float initialVelocity_val = 0.0f; static bool initialVelocity_checked = false;
            static float deltaX_val = 0.0f; static bool deltaX_checked = false;
            static float velocityFinal_val = 0.0f; static bool velocityFinal_checked = false;
            static float deltaY_val = 0.0f; static bool deltaY_checked = false;
            static float time_val = 0.0f; static bool time_checked = false;
            
            auto DrawInputRow = [](const char* label, float* value, bool* checked) {
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.4f);
                ImGui::InputFloat(label, value, 0.0f, 0.0f, "%.3f"); 
                ImGui::SameLine(); 

                std::string checkbox_label = "##" + std::string(label) + "_check";
                ImGui::Checkbox(checkbox_label.c_str(), checked);
            };
            
            ImGui::Columns(2, "InputColumns", false); 
            // Column 1
            DrawInputRow("theta", &theta_val, &theta_checked);
            DrawInputRow("initialV(m/s)", &initialVelocity_val, &initialVelocity_checked);
            DrawInputRow("finalV(m/s)", &velocityFinal_val, &velocityFinal_checked);
            DrawInputRow("Time(s)", &time_val, &time_checked);
            ImGui::NextColumn();
            // Column 2
            DrawInputRow("height(m)", &height_val, &height_checked);
            DrawInputRow("deltaX(m)", &deltaX_val, &deltaX_checked);
            DrawInputRow("deltaY(m)", &deltaY_val, &deltaY_checked);
            ImGui::Columns(1);

            if(ImGui::Button("Run", ImVec2(-1, 0))){
                float values[7];
                float isValid[7];
                values[0] = theta_val;           isValid[0] = theta_checked;
                values[1] = height_val;          isValid[1] = height_checked;
                values[2] = initialVelocity_val; isValid[2] = initialVelocity_checked;
                values[3] = velocityFinal_val;   isValid[3] = velocityFinal_checked;
                values[4] = deltaX_val;          isValid[4] = deltaX_checked;
                values[5] = deltaY_val;          isValid[5] = deltaY_checked;
                values[6] = time_val;            isValid[6] = time_checked;

                g_ShowPlots = true; 
            }
        }
        ImGui::EndChild();
        // --- Hardcoded Plot Data ---
        float x_data[] = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
        float y_proj[] = {0.0f, 4.0f, 7.0f, 9.0f, 8.0f, 5.0f}; 
        float y_vel[]  = {20.0f, 18.0f, 15.0f, 10.0f, 5.0f, 0.0f}; 

        if (g_ShowPlots) {
            
            // Position V Time Graph (2/3 section of slice)
            if (ImPlot::BeginPlot("Projectile Path (X vs Y)", ImVec2(-1, section_height))) {
                ImPlot::SetupAxes("Distance (m)", "Height (m)"); 
                ImPlot::PlotLine("Path", x_data, y_proj, IM_ARRAYSIZE(x_data));
                ImPlot::EndPlot();
            }
    
            // Velocity V Time Graph (3/3 section of slice)
            if (ImPlot::BeginPlot("Velocity vs Time", ImVec2(-1, -1))) { 
                ImPlot::SetupAxes("Time (s)", "Velocity (m/s)"); 
                ImPlot::PlotLine("Velocity", x_data, y_vel, IM_ARRAYSIZE(x_data));
                ImPlot::EndPlot();
            }
        }
        if (this->customFont) {
            ImGui::PopFont();
        }
        
    }
    ImGui::End();
}

void GUIRender::Render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUIRender::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}



