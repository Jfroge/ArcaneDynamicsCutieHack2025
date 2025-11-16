#include "../include/GUIRender.h"
#include "implot.h"
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <GLFW/glfw3.h>
#include <vector> // Required for std::vector
#include <cstring>
#include <../include/ArcaneMath.h>

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
    // Persistent state to control plot visibility
    static bool g_ShowPlots = false;
    // Persistent trajectory data arrays
    static float plot_x_data[100] = {0};
    static float plot_y_data[100] = {0};
    static int plot_data_count = 0;
    // Persistent velocity data arrays
    static float plot_t_data[100] = {0};
    static float plot_v_data[100] = {0};
    static int plot_v_count = 0; 

    static const float GROUND_HEIGHT = 50.0f;
    static const float SHOOTER_OFFSET = 50.0f;
    static const float SHOOTER_WIDTH = 50.0f;

    static float shooter_height_px       = 0.0f;

    // Ball size expressed in meters (world units). The pixel radius is computed
    // from the current px-per-meter scale so the ball appears proportionally
    // correct when constant scale is enabled or when autoscaled.
    static float FIREBALL_DIAMETER_M     = 0.5f; // default 0.5 meters diameter
    static float path_line_thickness_px  = 2.0f;

    static float padding_x_px            = 100.0f;
    static float padding_y_px            = 50.0f;

    static int   num_path_samples        = 100;
    static float ground_epsilon          = -0.01f;

    // Constant-scale option (user requested): keep px per meter fixed across runs
    static bool  g_UseConstantScale      = false;
    static float g_ScalePxPerMeter       = 20.0f; // reasonable default

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

    // Generate path based on projectile physics
    auto CalculatePath = [&]() {
        g_PathX.clear();
        g_PathY.clear();

        // Read current parameters from the mutable statics so animation matches solved values
        const float V0 = V0_MPS;
        const float Theta = THETA_DEG * (float)M_PI / 180.0f; // Convert to radians
        const float G = G_MPS2;

        // --- 1. Find the Exact Time of Impact (T_impact) ---
        // Solve: 0 = H0_Meters + (V0*sin(Theta))*t - (0.5*G)*t^2
        float A = 0.5f * G;
        float B = V0 * sinf(Theta);
        float C = H0_Meters;

        float discriminant = B * B + 4.0f * A * C;
        float T_impact = 0.0f;
        if (discriminant >= 0.0f && std::abs(A) > 1e-9f) {
            T_impact = (B + sqrtf(discriminant)) / (2.0f * A);
        }

        // --- 2. Set Max Simulation Time ---
        float T_max = std::min((float)g_SimulationDuration, T_impact > 0.0f ? T_impact : (float)g_SimulationDuration);

        for (int i = 0; i < num_path_samples; ++i) {
            float t = T_max * (float(i) / (num_path_samples - 1));

            float x = V0 * cosf(Theta) * t;
            float y = H0_Meters + V0 * sinf(Theta) * t - 0.5f * G * t * t;

            g_PathX.push_back(x);
            g_PathY.push_back(y);
        }

        // If it hit within duration, snap last point to ground
        if (T_impact > 0.0f && T_impact <= g_SimulationDuration) {
            if (!g_PathY.empty()) g_PathY.back() = 0.0f;
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

            static float theta_val = 0.0f; static bool theta_checked = false;
            static float height_val = 0.0f; static bool height_checked = false;
            static float initialVelocity_val = 0.0f; static bool initialVelocity_checked = false;
            static float deltaX_val = 0.0f; static bool deltaX_checked = false;
            static float velocityFinal_val = 0.0f; static bool velocityFinal_checked = false;
            static float deltaY_val = 0.0f; static bool deltaY_checked = false;
            static float time_val = 0.0f; static bool time_checked = false;
            static float gravity_val = 0.0f; static bool gravity_checked = false;
            
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
            DrawInputRow("finalHeight(m)", &deltaY_val, &deltaY_checked);
            DrawInputRow("gravity(m/s^2)", &gravity_val, &gravity_checked);
            ImGui::Columns(1);

            // Constant scale control: when enabled, animation and plots use this px/m value
            ImGui::Checkbox("Constant scale (px/m)", &g_UseConstantScale);
            if (g_UseConstantScale) {
                ImGui::SameLine();
                ImGui::SliderFloat("Scale", &g_ScalePxPerMeter, 1.0f, 200.0f);
            }

            if(ImGui::Button("Run", ImVec2(-1, 0))){
                float values[8];
                bool isValid[8];
                // fill input values into arrays
                values[0] = gravity_val;         isValid[0] = gravity_checked;
                values[1] = height_val;          isValid[1] = height_checked;
                values[2] = deltaY_val;          isValid[2] = deltaY_checked;
                values[3] = initialVelocity_val; isValid[3] = initialVelocity_checked;
                values[4] = velocityFinal_val;   isValid[4] = velocityFinal_checked;
                values[5] = deltaX_val;          isValid[5] = deltaX_checked;
                values[6] = theta_val;           isValid[6] = theta_checked;
                values[7] = time_val;            isValid[7] = time_checked;
                
                // pass arrays into Arcane Math to solve for the unknown values
                ArcaneMath newValues(values, isValid);
                newValues.solve();
                newValues.writeToArray(values);

                // Update the UI static variables with the newly computed values
                // Order: [0]=gravity, [1]=yi (height), [2]=yf (finalHeight), [3]=vi (initialV),
                // [4]=vf (finalV), [5]=d (deltaX), [6]=theta, [7]=time
                gravity_val = values[0];
                height_val = values[1];
                deltaY_val = values[2];
                initialVelocity_val = values[3];
                velocityFinal_val = values[4];
                deltaX_val = values[5];
                theta_val = values[6];
                time_val = values[7];

                // Generate trajectory data for plotting
                // Convert theta back to radians for calculations
                const float PI = 3.14159265358979323846f;
                float theta_rad = theta_val * PI / 180.0f;
                
                plot_data_count = 0;
                plot_v_count = 0;
                float dt = time_val / 50.0f; // 50 points along the trajectory
                if (dt > 0.0f && time_val > 0.0f) {
                    for (int i = 0; i <= 50 && plot_data_count < 100; i++) {
                        float t = i * dt;
                        float x = initialVelocity_val * std::cos(theta_rad) * t;
                        float y = height_val + initialVelocity_val * std::sin(theta_rad) * t 
                                - 0.5f * gravity_val * t * t;
                        plot_x_data[plot_data_count] = x;
                        plot_y_data[plot_data_count] = std::max(y, 0.0f); // Clamp to ground
                        
                        // Compute velocity at this time
                        float vx = initialVelocity_val * std::cos(theta_rad);
                        float vy = initialVelocity_val * std::sin(theta_rad) - gravity_val * t;
                        float v_magnitude = std::sqrt(vx*vx + vy*vy);
                        
                        plot_t_data[plot_v_count] = t;
                        plot_v_data[plot_v_count] = v_magnitude;
                        
                        plot_data_count++;
                        plot_v_count++;
                    }
                }

                // Update shared simulation parameters so animation follows solved values
                V0_MPS = initialVelocity_val;
                THETA_DEG = theta_val;
                H0_Meters = height_val;
                G_MPS2 = gravity_val;

                // Recompute the path for the simulation window
                CalculatePath();

                g_ShowPlots = true; 
                g_IsAnimationRunning = true;
                g_TimeStart = glfwGetTime();
            }
        }
        ImGui::EndChild();

        if (g_ShowPlots) {
            // Position V Time Graph (2/3 section of slice)
            float xmin=0.0f, xmax=0.0f, ymin=0.0f, ymax=0.0f;
            float xpad = 0.0f, ypad = 0.0f;
            if (plot_data_count > 0) {
                xmin = plot_x_data[0]; xmax = plot_x_data[0];
                ymin = plot_y_data[0]; ymax = plot_y_data[0];
                for (int i = 1; i < plot_data_count; ++i) {
                    xmin = std::min(xmin, plot_x_data[i]);
                    xmax = std::max(xmax, plot_x_data[i]);
                    ymin = std::min(ymin, plot_y_data[i]);
                    ymax = std::max(ymax, plot_y_data[i]);
                }
                xpad = (xmax - xmin) * 0.1f;
                if (xpad < 1e-3f) xpad = std::max(1.0f, (ymax - ymin) * 0.1f);
                ypad = (ymax - ymin) * 0.1f;
                if (ypad < 1e-3f) ypad = 1.0f;
            }

            // Disable the legend for this plot so the user cannot toggle the path on/off
            if (ImPlot::BeginPlot("Projectile Path (X vs Y)", ImVec2(-1, section_height), ImPlotFlags_NoLegend)) {
                // If constant scale is requested, set axis limits derived from px/m so
                // the ImPlot axes use the same visual scaling as the simulation canvas.
                if (g_UseConstantScale) {
                    // Available plot size roughly equals sim window width/height used elsewhere
                    float plot_avail_x = sim_window_width - padding_x_px;
                    float plot_avail_y = section_height - padding_y_px;
                    if (plot_avail_x < 1.0f) plot_avail_x = 1.0f;
                    if (plot_avail_y < 1.0f) plot_avail_y = 1.0f;

                    float x_range_m = plot_avail_x / g_ScalePxPerMeter;
                    float y_range_m = plot_avail_y / g_ScalePxPerMeter;

                    // Anchor the plot window to the data min so that the trajectory is visible
                    // while preserving the fixed px/m scale. If the data extends beyond the
                    // fixed window it will simply be clipped (consistent with constant scale).
                    float x_anchor = (plot_data_count > 0) ? xmin : 0.0f;
                    float y_anchor = (plot_data_count > 0) ? ymin : 0.0f;

                    float x0 = x_anchor;
                    float x1 = x_anchor + x_range_m;
                    float y0 = y_anchor;
                    float y1 = y_anchor + y_range_m;

                    // Ensure a minimum vertical baseline of 0 so ground is visible
                    if (y0 > 0.0f) y0 = 0.0f;

                    ImPlot::SetupAxisLimits(ImAxis_X1, x0, x1, ImPlotCond_Always);
                    ImPlot::SetupAxisLimits(ImAxis_Y1, y0, y1, ImPlotCond_Always);
                } else {
                    // If we computed ranges, lock the axis limits so ImPlot doesn't collapse a zero-width axis
                    if (plot_data_count > 0) {
                        ImPlot::SetupAxisLimits(ImAxis_X1, xmin - xpad, xmax + xpad, ImPlotCond_Always);
                        ImPlot::SetupAxisLimits(ImAxis_Y1, ymin - ypad, ymax + ypad, ImPlotCond_Always);
                    }
                }

                ImPlot::SetupAxes("Distance (m)", "Height (m)"); 
                if (plot_data_count > 0) {
                    ImPlot::PlotLine("Path", plot_x_data, plot_y_data, plot_data_count);
                }
                ImPlot::EndPlot();
            }
    
            if (ImPlot::BeginPlot("Velocity vs Time", ImVec2(-1, -1), ImPlotFlags_NoLegend)) { 
                ImPlot::SetupAxes("Time (s)", "Velocity (m/s)"); 
                if (plot_v_count > 0) {
                    ImPlot::PlotLine("Velocity", plot_t_data, plot_v_data, plot_v_count);
                }
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

        float shooter_base_y = canvas_pos.y + canvas_size.y - GROUND_HEIGHT;

        // Ground
        draw_list->AddRectFilled(
            ImVec2(canvas_pos.x, shooter_base_y),
            ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
            IM_COL32(50,40,30,255)
        );

        // (Shooter will be drawn after we compute the px-per-meter scale so its
        // displayed height matches the world initial height `H0_Meters`.)

        double current_time = glfwGetTime();

        float t = 0.0f;
        if (g_IsAnimationRunning) {
            t = (float)(current_time - g_TimeStart);
            if (t >= g_SimulationDuration) {
                t = g_SimulationDuration;
                g_IsAnimationRunning = false;
            }
        }


    // Recompute V0/Theta/G from the current UI-driven statics so animation follows inputs
    const float V0 = V0_MPS;
    const float Theta = THETA_DEG * (float)M_PI / 180.0f;
    const float G = G_MPS2;

    float MaxX = (V0 * V0 * sinf(2.0f * Theta)) / G; 
    float MaxY_peak_above_launch = (V0 * V0 * sinf(Theta) * sinf(Theta)) / (2.0f * G);

    float MaxTotalY_m = H0_Meters + MaxY_peak_above_launch;
    float MaxTotalX_m = MaxX * 1.05f;

    float avail_x = canvas_size.x - padding_x_px;
    float avail_y = canvas_size.y - GROUND_HEIGHT - padding_y_px;

    // Guard against zero horizontal extent (e.g. theta = 90 deg leads to MaxX == 0)
    const float EPS_SCALE = 1e-6f;
    if (MaxTotalY_m <= EPS_SCALE) MaxTotalY_m = 1.0f; // avoid div-by-zero

    float scale_px_per_meter;
    if (g_UseConstantScale) {
        // Use the user-selected px per meter directly. This intentionally
        // disables autoscaling so visual comparisons remain consistent.
        scale_px_per_meter = g_ScalePxPerMeter;
    } else {
        if (MaxTotalX_m > EPS_SCALE) {
            scale_px_per_meter = std::min(
                avail_x / MaxTotalX_m,
                avail_y / MaxTotalY_m
            );
        } else {
            // No horizontal spread; scale using vertical extent only
            scale_px_per_meter = avail_y / MaxTotalY_m;
        }
    }

    float shooter_base_x = canvas_pos.x + SHOOTER_OFFSET;
    // Compute shooter height in pixels from world units so the block visibly
    // represents the initial height `H0_Meters`.
    shooter_height_px = H0_Meters * scale_px_per_meter;
    float max_shooter_height_px = canvas_size.y - GROUND_HEIGHT - padding_y_px;
    if (shooter_height_px > max_shooter_height_px) shooter_height_px = max_shooter_height_px;
    if (shooter_height_px < 0.0f) shooter_height_px = 0.0f;

    float shooter_top_y = shooter_base_y - shooter_height_px;
    draw_list->AddRectFilled(
        ImVec2(shooter_base_x, shooter_top_y),
        ImVec2(shooter_base_x + SHOOTER_WIDTH, shooter_base_y),
        IM_COL32(70,0,100,255)
    );

    ImVec2 ground_origin_pix(
        shooter_base_x + SHOOTER_WIDTH / 2.0f,
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

        // Compute pixel radius from world diameter and current scale
        float fireball_radius_px = (FIREBALL_DIAMETER_M * 0.5f) * scale_px_per_meter;
        // Clamp to reasonable visibility bounds
        if (fireball_radius_px < 2.0f) fireball_radius_px = 2.0f;
        if (fireball_radius_px > 200.0f) fireball_radius_px = 200.0f;

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