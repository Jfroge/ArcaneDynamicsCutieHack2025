#include "../include/GUIRender.h"
#include "implot.h"
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <GLFW/glfw3.h>
#include <vector> // Required for std::vector

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

    static float ground_height_px        = 50.0f;
    static float shooter_offset_x_px     = 50.0f;
    static float shooter_width_px        = 30.0f;
    static float shooter_height_px       = 200.0f;

    static float fireball_radius_px      = 10.0f;
    static float path_line_thickness_px  = 2.0f;

    static float padding_x_px            = 100.0f;
    static float padding_y_px            = 50.0f;

    static int   num_path_samples        = 100;
    static float ground_epsilon          = -0.01f;

    static bool g_ShowPlots = false;

    static double g_TimeStart = 0.0;
    static const double g_SimulationDuration = 8.0;
    static bool g_IsAnimationRunning = false; 
    
    static std::vector<float> g_PathX;
    static std::vector<float> g_PathY;

    static float g_FinalXPix = 0.0f;
    static float g_FinalYPix = 0.0f;

    static float H0_Meters = 50.0f;
    static float V0_MPS = 40.0f; 
    static float THETA_DEG = 55.0f;
    static float G_MPS2 = 9.8f;

    #ifndef M_PI
        #define M_PI 3.14159265358979323846
    #endif

    const float V0 = V0_MPS; 
    const float Theta = THETA_DEG * (float)M_PI / 180.0f; // Convert to radians
    const float G = G_MPS2;
    
    // Generate path based on projectile physics
    auto CalculatePath = [&]() {
        g_PathX.clear();
        g_PathY.clear();
    
    // --- 1. Find the Exact Time of Impact (T_impact) ---
    // The ground is y = 0. We solve the quadratic equation for t:
    // 0 = H0_Meters + (V0*sin(Theta))*t - (0.5*G)*t^2
        float A = 0.5f * G;
        float B = V0 * sinf(Theta);
        float C = H0_Meters; 
    
    // The discriminant: sqrt(B^2 + 4AC)
        float discriminant = B * B + 4.0f * A * C; 

        float T_impact = 0.0f;
        if (discriminant >= 0) {
        // Use the positive root for time
            T_impact = (B + sqrtf(discriminant)) / (2.0f * A); 
        }

    // --- 2. Set Max Simulation Time ---
    // The simulation runs up to the shorter of the hardcoded duration or the calculated impact time.
        float T_max = std::min((float)g_SimulationDuration, T_impact);
    
    // If the projectile never lands within the simulation duration, T_max remains T_impact 
    // (or g_SimulationDuration if T_impact is very large/negative, though T_impact should be positive here).
    
        for (int i = 0; i < num_path_samples; ++i) {
            float t = T_max * (float(i) / (num_path_samples - 1));
        
        // Horizontal: (X is unchanged)
            float x = V0 * cosf(Theta) * t;
        
        // Vertical: Must include the initial height H0_Meters
            float y = H0_Meters + V0 * sinf(Theta) * t - 0.5f * G * t * t;

        // Stop checking y < ground_epsilon since T_max already sets the boundary
        // We only add points up to T_max.
        
            g_PathX.push_back(x);
            g_PathY.push_back(y);
        }
    
    // Optional: Ensure the very last point is exactly on the ground if it hit.
        if (T_impact <= g_SimulationDuration && T_impact > 0.0f) {
            g_PathY.back() = 0.0f;
        }
    };

    if (this->customFont)
        ImGui::PushFont(this->customFont);

    int display_w, display_h;
    glfwGetWindowSize(window, &display_w, &display_h);

    float main_window_width = (float)display_w / 3.0f;
    float sim_window_width  = display_w - main_window_width;
    float screen_height = (float)display_h;
    float section_height = screen_height / 3.0f;

    float x_data[] = {0, 1, 2, 3, 4, 5};
    float y_proj[] = {0, 4, 7, 9, 8, 5}; 
    float y_vel[]  = {20,18,15,10,5,0};

    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::SetNextWindowSize(ImVec2(main_window_width, screen_height));

    ImGuiWindowFlags main_flags = 
        ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoScrollbar;

    if (ImGui::Begin("MainControlPanel", nullptr, main_flags)) {

        if (ImGui::BeginChild("InputSection", ImVec2(-1, section_height), true)) {

            ImGui::Text("Arcane Dynamics"); 
            ImGui::Separator(); 
            ImGui::Text("2D Kinematics *Projectile Motion"); 
            ImGui::Spacing();

            ImGui::TextWrapped("If You know the value enter the input then check the box");
            ImGui::Spacing();

            // Original input variables kept for future problem-solving logic (not used for path calculation yet)
            static float theta_val = 0; static bool theta_checked = false;
            static float height_val = 0; static bool height_checked = false;
            static float initialVelocity_val = 0; static bool initialVelocity_checked = false;
            static float deltaX_val = 0; static bool deltaX_checked = false;
            static float velocityFinal_val = 0; static bool velocityFinal_checked = false;
            static float deltaY_val = 0; static bool deltaY_checked = false;
            static float time_val = 0; static bool time_checked = false;

            auto DrawInputRow = [](const char* label, float* value, bool* checked) {
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.4f);
                ImGui::InputFloat(label, value);
                ImGui::SameLine();
                std::string checkbox_label = std::string("##") + label + "_check";
                ImGui::Checkbox(checkbox_label.c_str(), checked);
            };

            ImGui::Columns(2, "InputCols");

            DrawInputRow("theta", &theta_val, &theta_checked);
            DrawInputRow("initialV(m/s)", &initialVelocity_val, &initialVelocity_checked);
            DrawInputRow("finalV(m/s)", &velocityFinal_val, &velocityFinal_checked);
            DrawInputRow("Time(s)", &time_val, &time_checked);

            ImGui::NextColumn();

            DrawInputRow("height(m)", &height_val, &height_checked);
            DrawInputRow("deltaX(m)", &deltaX_val, &deltaX_checked);
            DrawInputRow("deltaY(m)", &deltaY_val, &deltaY_checked);

            ImGui::Columns(1);

            if (ImGui::Button("Run", ImVec2(-1, 0))) {
                g_ShowPlots = true;
                CalculatePath();
                g_IsAnimationRunning = true;
                g_TimeStart = glfwGetTime();
            }
        }
        ImGui::EndChild();

        if (g_ShowPlots) {
            if (ImPlot::BeginPlot("Projectile Path", ImVec2(-1, section_height))) {
                // ... (Plotting logic is fine)
                ImPlot::SetupAxes("Distance (m)", "Height (m)");
                ImPlot::PlotLine("Path", x_data, y_proj, IM_ARRAYSIZE(x_data));
                ImPlot::EndPlot();
            }

            if (ImPlot::BeginPlot("Velocity vs Time", ImVec2(-1,-1))) {
                // ... (Plotting logic is fine)
                ImPlot::SetupAxes("Time (s)", "Velocity (m/s)");
                ImPlot::PlotLine("Velocity", x_data, y_vel, IM_ARRAYSIZE(y_vel));
                ImPlot::EndPlot();
            }
        }
    }
    ImGui::End();

    // ===========================================================
    //  SIMULATION DRAWING WINDOW (Refactored)
    // ===========================================================

    ImGui::SetNextWindowPos(ImVec2(main_window_width, 0));
    ImGui::SetNextWindowSize(ImVec2(sim_window_width, screen_height));

    ImGuiWindowFlags sim_flags =
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBackground;

    if (ImGui::Begin("SimulationWindow", nullptr, sim_flags)) {

        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_size = ImGui::GetContentRegionAvail();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // Sky
        draw_list->AddRectFilled(
            canvas_pos,
            ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
            IM_COL32(135, 206, 235, 255)
        );

        float shooter_base_y = canvas_pos.y + canvas_size.y - ground_height_px;

        // Ground
        draw_list->AddRectFilled(
            ImVec2(canvas_pos.x, shooter_base_y),
            ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
            IM_COL32(50,40,30,255)
        );

        // Shooter
        float shooter_base_x = canvas_pos.x + shooter_offset_x_px;
        float shooter_top_y  = shooter_base_y - shooter_height_px; // 🚀 NEW: Top Y position of shooter

        draw_list->AddRectFilled(
            ImVec2(shooter_base_x, shooter_top_y),
            ImVec2(shooter_base_x + shooter_width_px, shooter_base_y),
            IM_COL32(70,0,100,255)
        );

        double current_time = glfwGetTime();

        float t = 0.0f;
        if (g_IsAnimationRunning) {
            t = (float)(current_time - g_TimeStart);
            if (t >= g_SimulationDuration) {
                t = g_SimulationDuration;
                g_IsAnimationRunning = false;
            }
        }


    float MaxX = (V0 * V0 * sinf(2.0f * Theta)) / G; 
    float MaxY_peak_above_launch = (V0 * V0 * sinf(Theta) * sinf(Theta)) / (2.0f * G);

    float MaxTotalY_m = H0_Meters + MaxY_peak_above_launch; 
    float MaxTotalX_m = MaxX * 1.05f; 

    float avail_x = canvas_size.x - padding_x_px;
    float avail_y = canvas_size.y - ground_height_px - padding_y_px;

    float scale_px_per_meter = std::min(
        avail_x / MaxTotalX_m,
        avail_y / MaxTotalY_m
    );

    ImVec2 ground_origin_pix(
        shooter_base_x + shooter_width_px / 2.0f,
        shooter_base_y                        
    );

    // Fireball position calculation (y_m includes H0_Meters)
    float x_m = V0 * cosf(Theta) * t;
    float y_m = H0_Meters + V0 * sinf(Theta) * t - 0.5f * G * t * t;

    float x_pix = ground_origin_pix.x + x_m * scale_px_per_meter;
    float y_pix = ground_origin_pix.y - y_m * scale_px_per_meter; 


        if (y_pix >= shooter_base_y) {
            y_pix = shooter_base_y;
            if (g_IsAnimationRunning) {
                g_IsAnimationRunning = false;
                g_FinalXPix = x_pix;
                g_FinalYPix = shooter_base_y;
            }
        }
        
        ImVec2 draw_pos(x_pix, y_pix); 

        if (t > 0.0f) {
            draw_list->AddCircleFilled(
                draw_pos,
                fireball_radius_px,
                IM_COL32(255,180,0,255)
            );
        }

        ImGui::SetCursorScreenPos(ImVec2(canvas_pos.x + 10, canvas_pos.y + 10));
        ImGui::Text("Time: %.2f / %.2f", t, (float)g_SimulationDuration);
        ImGui::Text("Position: X=%.2f m, Y=%.2f m", x_m, y_m);
        ImGui::Text("Scale: %.2f px/m", scale_px_per_meter);
    }

    ImGui::End();

    if (this->customFont)
        ImGui::PopFont();
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