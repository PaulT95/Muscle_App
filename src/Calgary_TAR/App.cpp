#include "Calgary_TAR/App.hpp"
#include "Calgary_TAR/ImGuiTheme.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

#ifdef _WIN32
    #include <glad/glad.h>
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include <GLFW/glfw3.h>
#ifdef _WIN32
    #include <GLFW/glfw3native.h>
    #include <windows.h>
#endif

#include <imgui.h>
#include <implot.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <tinyfiledialogs.h>

namespace TARapp {

App::App() {
    init();
}

App::~App() {
    cleanup();
}

void App::init() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        m_running = false;
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    int winW = static_cast<int>(mode->width * 0.85);
    int winH = static_cast<int>(mode->height * 0.85);

    m_window = glfwCreateWindow(winW, winH, "Calgary TAR - Torque-Angle Analysis Tool", nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window\n";
        m_running = false;
        return;
    }

    int xpos = (mode->width - winW) / 2;
    int ypos = (mode->height - winH) / 2;
    glfwSetWindowPos(m_window, xpos, ypos);
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Custom dark theme
    ImGuiTheme::setFont(); //set robot as font and then dark theme(defautl)
    ImGuiTheme::ApplyTheme();

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    // Load sample data by default
    m_points = DataModel::getSampleData();
    updateFit();
}

void App::cleanup() {
    if (m_window) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
        glfwDestroyWindow(m_window);
        glfwTerminate();
        m_window = nullptr;
    }
}

void App::updateFit() {
    if (m_points.empty()) {
        m_fit.valid = false;
        return;
    }
    // Sort a temporary copy for math fitting so table row order remains stable while typing
    std::vector<TorquePoint> sorted_points = m_points;
    std::sort(sorted_points.begin(), sorted_points.end(), [](const TorquePoint& a, const TorquePoint& b) {
        return a.angle < b.angle;
    });

    m_fit = DataModel::fitPolynomial(sorted_points, m_poly_degree);
    m_reset_axes = true;
}

void App::run() {
    while (m_running && !glfwWindowShouldClose(m_window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderUI();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(m_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.10f, 0.11f, 0.13f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_window);
    }
}
//main part to control the rendering of the windows/panels
void App::renderUI() {
    // Create main fullscreen viewport window
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("MainDockSpaceWindow", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    renderMenuBar();

    // Two column split layout: Left controls & table, Right plots
    float left_panel_width = 460.0f;
    float avail_width = ImGui::GetContentRegionAvail().x;
    float avail_height = ImGui::GetContentRegionAvail().y;

    ImGui::BeginChild("LeftPanel", ImVec2(left_panel_width, avail_height), true);
    renderTablePanel();
    ImGui::Separator();
    renderQueryPanel();
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("RightPanel", ImVec2(avail_width - left_panel_width - 8.0f, avail_height), true);
    renderPlotsPanel();
    ImGui::EndChild();

    renderAboutModal();

    ImGui::End();
}

void App::renderMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load CSV File...")) {
                const char* filterPatterns[2] = { "*.csv", "*.txt" };
                const char* filePath = tinyfd_openFileDialog(
                    "Select Data CSV File",
                    "",
                    1,
                    filterPatterns,
                    "CSV Files",
                    0
                );
                if (filePath) {
                    loadCSV(filePath);
                }
            }
            if (ImGui::MenuItem("Save CSV File...")) {
                const char* filterPatterns[1] = { "*.csv" };
                const char* filePath = tinyfd_saveFileDialog(
                    "Save Data to CSV",
                    "torque_angle_data.csv",
                    1,
                    filterPatterns,
                    "CSV Files"
                );
                if (filePath) {
                    saveCSV(filePath);
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Load Sample Data")) {
                m_points = DataModel::getSampleData();
                updateFit();
                m_status_msg = "Loaded sample dataset.";
            }
            if (ImGui::MenuItem("Clear All Data")) {
                m_points.clear();
                updateFit();
                m_status_msg = "Cleared all data points.";
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                m_running = false;
            }
            ImGui::EndMenu();
        }
        // not needed anymore
        // if (ImGui::BeginMenu("Polynomial Options")) {
        //     ImGui::Text("Polynomial Fit Degree:");
        //     if (ImGui::RadioButton("1st Degree (Linear)", m_poly_degree == 1)) { m_poly_degree = 1; updateFit(); }
        //     if (ImGui::RadioButton("2nd Degree (Quadratic)", m_poly_degree == 2)) { m_poly_degree = 2; updateFit(); }
        //     if (ImGui::RadioButton("3rd Degree (Cubic)", m_poly_degree == 3)) { m_poly_degree = 3; updateFit(); }
        //     if (ImGui::RadioButton("4th Degree", m_poly_degree == 4)) { m_poly_degree = 4; updateFit(); }
        //     if (ImGui::RadioButton("5th Degree", m_poly_degree == 5)) { m_poly_degree = 5; updateFit(); }
        //     ImGui::EndMenu();
        // }
        if (ImGui::BeginMenu("Theme")) {
            if (ImGui::MenuItem("Set light theme")) {
                ImGuiTheme::ApplyLightModern();
            }
            if (ImGui::MenuItem("Set dark theme")) {
                ImGuiTheme::ApplyDarkModern();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About this app...")) {
                m_show_about = true;
            }
            ImGui::EndMenu();
        }

        ImGui::TextDisabled(" |  Status: %s", m_status_msg.c_str());
        ImGui::EndMenuBar();
    }
}

void App::renderTablePanel() {
    ImGui::TextColored(ImVec4(0.2f,0.7f,1.0f,1.0f), "1. DATA INPUT & POLYNOMIAL FIT");
    ImGui::Separator();

    // Polynomial degree selector
    ImGui::SetNextItemWidth(120);
    if (ImGui::SliderInt("Fit Degree", &m_poly_degree, 1, 5)) {
        updateFit();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset Sample")) {
        m_points = DataModel::getSampleData();
        updateFit();
        m_status_msg = "Reset to sample data.";
    }

    // Action buttons above table
    if (ImGui::Button("+ Add Row")) {
        float max_a = 0.0f;
        if (!m_points.empty()) {
            max_a = m_points[0].angle;
            for (const auto& p : m_points) {
                if (p.angle > max_a) max_a = p.angle;
            }
        }
        float new_angle = m_points.empty() ? 10.0f : (max_a + 10.0f);
        float default_rest = m_points.empty() ? 0.5f : m_points.back().rest_v;
        float default_peak = m_points.empty() ? 1.5f : m_points.back().peak_v;
        m_points.push_back({ new_angle, default_rest, default_peak });
        updateFit();
    }
    ImGui::SameLine();
    if (ImGui::Button("Sort Rows")) {
        std::sort(m_points.begin(), m_points.end(), [](const TorquePoint& a, const TorquePoint& b) {
            return a.angle < b.angle;
        });
        updateFit();
        m_status_msg = "Sorted table rows by angle.";
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Table")) {
        m_points.clear();
        updateFit();
    }

    // Fit summary stats
    if (m_fit.valid) {
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "R² = %.4f | RMSE = %.4f | Range: [%.1f, %.1f]",
            m_fit.r_squared, m_fit.rmse, m_fit.min_angle, m_fit.max_angle);
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Insufficient data points for degree %d fit", m_poly_degree);
    }

    // Interactive Table
    static ImGuiTableFlags table_flags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;

    float table_height = 240.0f;
    if (ImGui::BeginTable("DataTable", 5, table_flags, ImVec2(0.0f, table_height))) {
        ImGui::TableSetupColumn("Angle(°) / Length ", ImGuiTableColumnFlags_WidthFixed, 75.0f);
        ImGui::TableSetupColumn("Rest value (Passive)", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableSetupColumn("Peak value", ImGuiTableColumnFlags_WidthFixed, 75.0f);
        ImGui::TableSetupColumn("Active value (P-R)", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableHeadersRow();

        int delete_idx = -1;
        bool data_changed = false;

        for (int i = 0; i < static_cast<int>(m_points.size()); ++i) {
            ImGui::TableNextRow();
            ImGui::PushID(i);

            // Angle
            ImGui::TableSetColumnIndex(0);
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::InputFloat("##angle", &m_points[i].angle, 0.0f, 0.0f, "%.3f")) {
                data_changed = true;
            }

            // Rest Voltage
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::InputFloat("##rest", &m_points[i].rest_v, 0.0f, 0.0f, "%.3f")) {
                data_changed = true;
            }

            // Peak Voltage
            ImGui::TableSetColumnIndex(2);
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::InputFloat("##peak", &m_points[i].peak_v, 0.0f, 0.0f, "%.3f")) {
                data_changed = true;
            }

            // Calculated Active Torque
            ImGui::TableSetColumnIndex(3);
            float active_t = (m_points[i].activeTorque());

            ImGui::Text("%.3f V", active_t);

            // Action: Delete button
            ImGui::TableSetColumnIndex(4);
            if (ImGui::Button("X")) {
                delete_idx = i;
            }

            ImGui::PopID();
        }

        if (delete_idx >= 0 && delete_idx < static_cast<int>(m_points.size())) {
            m_points.erase(m_points.begin() + delete_idx);
            data_changed = true;
        }

        if (data_changed) {
            updateFit();
        }

        ImGui::EndTable();
    }
}

void App::renderQueryPanel() {
    ImGui::TextColored(ImVec4(0.2f,0.7f,1.0f,1.0f), "2. INTERACTIVE QUERY & ANALYSIS");
    ImGui::Separator();

    ImGui::RadioButton("Query by specific Angle(°) / Length", &m_query_mode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Query by target Torque/Force %", &m_query_mode, 1);

    if (m_query_mode == 0) {
        // Mode A: Query by Angle
        ImGui::Text("Enter target Angle(°)/ Length:");
        ImGui::SetNextItemWidth(140);
        float min_a = m_fit.valid ? m_fit.min_angle : -90.0f;
        float max_a = m_fit.valid ? m_fit.max_angle : 180.0f;
        if (m_query_angle < min_a) m_query_angle = min_a;
        if (m_query_angle > max_a) m_query_angle = max_a;
        ImGui::SliderFloat("Angle(°) / Length", &m_query_angle, min_a, max_a, "%.1f°");

        if (m_fit.valid) {
            double norm_val = m_fit.evaluateNormalized(m_query_angle);
            float max_raw_active = DataModel::getMaxRawActive(m_points);
            float est_active = static_cast<float>(norm_val) * max_raw_active;
            float est_passive = DataModel::interpolatePassive(m_points, m_query_angle);
            float est_peak = est_passive + est_active;

            ImGui::BeginChild("QueryResultBox", ImVec2(0, 160), true);
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Results at Angle/Length = %.1f°:", m_query_angle);
            ImGui::BulletText("Normalized Torque/Force: %.1f%% (%.4f)", norm_val * 100.0, norm_val);
            ImGui::BulletText("Est. Active Torque/Force (P-to-P): %.3f ", est_active);
            ImGui::BulletText("Est. Passive Torque/Force (Rest):  %.3f ", est_passive);
            ImGui::BulletText("Est. Peak Torque/Force (Total):   %.3f ", est_peak);
            ImGui::EndChild();
        } else {
            ImGui::TextDisabled("Fit not available.");
        }
    } else {
        // Mode B: Query by Target Torqye/Force Percentage
        ImGui::Text("Enter Target Torque/Force Percentage (0..100%%):");
        ImGui::SetNextItemWidth(160);
        float pct_100 = m_query_force_pct * 100.0f;
        if (ImGui::SliderFloat("Target Torque/Force %", &pct_100, 0.0f, 100.0f, "%.1f%%")) {
            m_query_force_pct = pct_100 / 100.0f;
        }

        if (m_fit.valid) {
            std::vector<double> roots = DataModel::findRoots(m_fit, m_query_force_pct);
            float max_raw_active = DataModel::getMaxRawActive(m_points);

            ImGui::BeginChild("QueryResultBox", ImVec2(0, 160), true);
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Angle/Length(s) at %.1f%% Force/Torque:", m_query_force_pct * 100.0f);

            if (roots.empty()) {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "No valid real angles found in tested range [%.1f, %.1f]", m_fit.min_angle, m_fit.max_angle);
            } else {
                for (size_t idx = 0; idx < roots.size(); ++idx) {
                    float root_a = static_cast<float>(roots[idx]);
                    double norm_t = m_fit.evaluateNormalized(root_a);
                    float est_active = static_cast<float>(norm_t) * max_raw_active;
                    float est_passive = DataModel::interpolatePassive(m_points, root_a);

                    ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "[Root %d] Angle/Length = %.2f", static_cast<int>(idx + 1), root_a);
                    ImGui::BulletText("  Active Torque/Force (P-P): %.3f ", est_active);
                    ImGui::BulletText("  Passive Torque/Force:     %.3f ", est_passive);
                    ImGui::BulletText("  Total Peak Torque/Force:  %.3f ", est_passive + est_active);
                }
            }
            ImGui::EndChild();
        } else {
            ImGui::TextDisabled("Fit not available.");
        }
    }
}

void App::renderPlotsPanel() {
    ImGui::TextColored(ImVec4(0.2f,0.7f,1.0f,1.0f), "3. GRAPHICAL PLOTS & VISUALIZATION");
    ImGui::SameLine();
    if (ImGui::SmallButton("Fit Axes to Data")) {
        m_reset_axes = true;
    }
    ImGui::Separator();

    float plot_h = (ImGui::GetContentRegionAvail().y - 24.0f) / 2.0f;
    ImPlotCond cond = m_reset_axes ? ImPlotCond_Always : ImPlotCond_Once;

    // ---------------- CHART 1: RAW TORQUE-ANGLE ----------------
    if (!m_points.empty()) {
        float min_x = m_points[0].angle;
        float max_x = m_points[0].angle;
        float min_y = m_points[0].rest_v;
        float max_y = m_points[0].rest_v;

        for (const auto& pt : m_points) {
            if (pt.angle < min_x) min_x = pt.angle;
            if (pt.angle > max_x) max_x = pt.angle;

            float rest = pt.rest_v;
            float peak = pt.peak_v;
            float act = pt.activeTorque();

            if (rest < min_y) min_y = rest;
            if (rest > max_y) max_y = rest;
            if (peak < min_y) min_y = peak;
            if (peak > max_y) max_y = peak;
            if (act < min_y) min_y = act;
            if (act > max_y) max_y = act;
        }

        float dx = max_x - min_x; if (dx <= 1e-4f) dx = 10.0f;
        float dy = max_y - min_y; if (dy <= 1e-4f) dy = 1.0f;

        float x_min_pad = min_x - 0.05f * dx;
        float x_max_pad = max_x + 0.05f * dx;
        float y_min_pad = min_y - 0.08f * dy;
        float y_max_pad = max_y + 0.08f * dy;

        ImPlot::SetNextAxisLimits(ImAxis_X1, x_min_pad, x_max_pad, cond);
        ImPlot::SetNextAxisLimits(ImAxis_Y1, y_min_pad, y_max_pad, cond);
    }

    if (ImPlot::BeginPlot("Raw Torque-angle/Force-length data", ImVec2(-1, plot_h))) {
        ImPlot::SetupAxes("Angle(°)/Length", "Torque / Force");

        if (!m_points.empty()) {
            std::vector<float> xs(m_points.size());
            std::vector<float> rest_ys(m_points.size());
            std::vector<float> peak_ys(m_points.size());
            std::vector<float> active_ys(m_points.size());

            for (size_t i = 0; i < m_points.size(); ++i) {
                xs[i] = m_points[i].angle;
                rest_ys[i] = m_points[i].rest_v;
                peak_ys[i] = m_points[i].peak_v;
                active_ys[i] = m_points[i].activeTorque();
            }

            // Zero baseline indicator line
            float zero_x[2] = { xs.front() - 100.0f, xs.back() + 100.0f };
            float zero_y[2] = { 0.0f, 0.0f };
            ImPlot::SetNextLineStyle(ImVec4(0.5f, 0.5f, 0.5f, 0.4f), 1.0f);
            ImPlot::PlotLine("##ZeroBaseline", zero_x, zero_y, 2);

            // Peak Voltage (Coral Red)
            ImPlot::SetNextLineStyle(ImVec4(0.95f, 0.35f, 0.35f, 1.0f), 2.0f);
            ImPlot::PlotLine("Peak ", xs.data(), peak_ys.data(), static_cast<int>(xs.size()));
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 4.5f, ImVec4(0.95f, 0.35f, 0.35f, 1.0f), 1.0f);
            ImPlot::PlotScatter("##PeakPts", xs.data(), peak_ys.data(), static_cast<int>(xs.size()));

            // Rest Voltage (Teal / Cyan)
            ImPlot::SetNextLineStyle(ImVec4(0.20f, 0.80f, 0.85f, 1.0f), 2.0f);
            ImPlot::PlotLine("Rest  (Passive)", xs.data(), rest_ys.data(), static_cast<int>(xs.size()));
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 4.5f, ImVec4(0.20f, 0.80f, 0.85f, 1.0f), 1.0f);
            ImPlot::PlotScatter("##RestPts", xs.data(), rest_ys.data(), static_cast<int>(xs.size()));

            // Active Torque (Gold)
            ImPlot::SetNextLineStyle(ImVec4(1.00f, 0.75f, 0.15f, 1.0f), 2.5f);
            ImPlot::PlotLine("Active (P-to-P)", xs.data(), active_ys.data(), static_cast<int>(xs.size()));
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 5.0f, ImVec4(1.00f, 0.75f, 0.15f, 1.0f), 1.0f);
            ImPlot::PlotScatter("##ActivePts", xs.data(), active_ys.data(), static_cast<int>(xs.size()));
        }
        ImPlot::EndPlot();
    }

    // ---------------- CHART 2: NORMALIZED TORQUE & POLYNOMIAL FIT ----------------
    if (m_fit.valid && !m_points.empty()) {
        double start_x = std::floor(m_fit.min_angle / 5.0) * 5.0;
        double end_x = std::ceil(m_fit.max_angle / 5.0) * 5.0;
        if (end_x <= start_x) end_x = start_x + 10.0;

        float min_x = static_cast<float>(start_x);
        float max_x = static_cast<float>(end_x);
        for (const auto& pt : m_points) {
            if (pt.angle < min_x) min_x = pt.angle;
            if (pt.angle > max_x) max_x = pt.angle;
        }

        float max_raw_active = DataModel::getMaxRawActive(m_points);

        std::vector<float> xs(m_points.size());
        std::vector<float> norm_ys(m_points.size());
        float min_y = 0.0f;
        float max_y = 1.0f;

        for (size_t i = 0; i < m_points.size(); ++i) {
            xs[i] = m_points[i].angle;
            float ny = m_points[i].activeTorque() / max_raw_active;
            norm_ys[i] = ny;
            if (ny < min_y) min_y = ny;
            if (ny > max_y) max_y = ny;
        }

        const int curve_pts = 150;
        std::vector<float> fit_xs(curve_pts);
        std::vector<float> fit_ys(curve_pts);
        double step = (end_x - start_x) / (curve_pts - 1);

        for (int i = 0; i < curve_pts; ++i) {
            double x = start_x + i * step;
            fit_xs[i] = static_cast<float>(x);
            float fy = static_cast<float>(m_fit.evaluateNormalized(x));
            fit_ys[i] = fy;
            if (fy < min_y) min_y = fy;
            if (fy > max_y) max_y = fy;
        }

        if (m_query_mode == 1) {
            if (m_query_force_pct < min_y) min_y = m_query_force_pct;
            if (m_query_force_pct > max_y) max_y = m_query_force_pct;
        } else {
            float q_y = static_cast<float>(m_fit.evaluateNormalized(m_query_angle));
            if (q_y < min_y) min_y = q_y;
            if (q_y > max_y) max_y = q_y;
        }

        float dx = max_x - min_x; if (dx <= 1e-4f) dx = 10.0f;
        float dy = max_y - min_y; if (dy <= 1e-4f) dy = 1.0f;

        float x_min_pad = min_x - 0.05f * dx;
        float x_max_pad = max_x + 0.05f * dx;
        float y_min_pad = min_y - 0.08f * dy;
        float y_max_pad = max_y + 0.08f * dy;

        ImPlot::SetNextAxisLimits(ImAxis_X1, x_min_pad, x_max_pad, cond);
        ImPlot::SetNextAxisLimits(ImAxis_Y1, y_min_pad, y_max_pad, cond);

        if (ImPlot::BeginPlot("Normalized Torque/Force & Polynomial Fitting Curve", ImVec2(-1, plot_h))) {
            ImPlot::SetupAxes("Angle(°)/Length", "Normalized active Torque/Force");

            // Baseline at 0.0
            float zero_x[2] = { static_cast<float>(start_x) - 100.0f, static_cast<float>(end_x) + 100.0f };
            float zero_y[2] = { 0.0f, 0.0f };
            ImPlot::SetNextLineStyle(ImVec4(0.5f, 0.5f, 0.5f, 0.4f), 1.0f);
            ImPlot::PlotLine("##ZeroNormBaseline", zero_x, zero_y, 2);

            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 5.0f, ImVec4(0.30f, 0.85f, 1.00f, 1.0f), 1.0f);
            ImPlot::PlotScatter("Raw Data Points", xs.data(), norm_ys.data(), static_cast<int>(xs.size()));

            ImPlot::SetNextLineStyle(ImVec4(0.35f, 0.90f, 0.35f, 1.0f), 2.5f); // Lime Green line
            ImPlot::PlotLine("Polynomial Fit", fit_xs.data(), fit_ys.data(), curve_pts);

            // Draw target query overlays
            if (m_query_mode == 0) {
                // Angle query marker line
                float q_y = static_cast<float>(m_fit.evaluateNormalized(m_query_angle));
                float q_x[2] = { m_query_angle, m_query_angle };
                float q_y_line[2] = { y_min_pad, q_y };
                ImPlot::SetNextLineStyle(ImVec4(0.80f, 0.40f, 1.00f, 1.0f), 2.0f); // Violet line
                ImPlot::PlotLine("Query Angle/Length", q_x, q_y_line, 2);
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 7.0f, ImVec4(0.90f, 0.50f, 1.00f, 1.0f), 1.5f);
                ImPlot::PlotScatter("Target Point", &m_query_angle, &q_y, 1);
            } else {
                // Force % horizontal dashed line & root markers
                float target_line_x[2] = { static_cast<float>(start_x), static_cast<float>(end_x) };
                float target_line_y[2] = { m_query_force_pct, m_query_force_pct };
                ImPlot::SetNextLineStyle(ImVec4(1.00f, 0.35f, 0.35f, 1.0f), 2.0f); // Bright Red line
                ImPlot::PlotLine("Target Torque/Force level", target_line_x, target_line_y, 2);

                std::vector<double> roots = DataModel::findRoots(m_fit, m_query_force_pct);
                if (!roots.empty()) {
                    std::vector<float> root_xs(roots.size());
                    std::vector<float> root_ys(roots.size());
                    for (size_t i = 0; i < roots.size(); ++i) {
                        root_xs[i] = static_cast<float>(roots[i]);
                        root_ys[i] = static_cast<float>(m_fit.evaluateNormalized(roots[i]));
                    }
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 7.0f, ImVec4(1.00f, 0.45f, 0.10f, 1.0f), 1.5f); // Orange/Red dots
                    ImPlot::PlotScatter("Identified Roots", root_xs.data(), root_ys.data(), static_cast<int>(roots.size()));
                }
            }
            ImPlot::EndPlot();
        }
    }

    m_reset_axes = false;
}

void App::renderAboutModal() {
    if (!m_show_about) return;

    ImGui::OpenPopup("About this app");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(460, 190));

    if (ImGui::BeginPopupModal("About this app", &m_show_about, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "TAR/FLR - Torque-Angle/Force-length quick and dirty analysis tool");
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextWrapped("Developed by Paolo Tecchio during a reserach stay at the University of Calgary - kinesiology for experiments on animal muscle."
                           "\nI mainly developed to be fast and accurate in conditions definition with Andrew S.\n"
                           "Feel free to use it but remember to also acknowledge the repo;).");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            m_show_about = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void App::loadCSV(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        m_status_msg = "Error: Could not open file " + filepath;
        return;
    }

    std::vector<TorquePoint> new_points;
    std::string line;
    int line_num = 0;

    while (std::getline(file, line)) {
        line_num++;
        if (line.empty()) continue;

        // Replace commas or tabs with spaces for stringstream parsing
        for (char& ch : line) {
            if (ch == ',' || ch == '\t' || ch == ';') ch = ' ';
        }

        std::stringstream ss(line);
        float a, r, p;
        if (ss >> a >> r >> p) {
            new_points.push_back({ a, r, p });
        } else if (line_num == 1) {
            // Header line, ignore
            continue;
        }
    }
    //in case of fuckign?
    if (!new_points.empty()) {
        m_points = new_points;
        updateFit();
        m_status_msg = "Loaded " + std::to_string(m_points.size()) + " points from " + filepath;
    } else {
        m_status_msg = "Error: No valid data found in CSV file.";
    }
}
//simply export
void App::saveCSV(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        m_status_msg = "Error: Could not open file for writing: " + filepath;
        return;
    }

    file << "Angle,Rest_V,Peak_V,Active_V\n";
    for (const auto& pt : m_points) {
        file << pt.angle << "," << pt.rest_v << "," << pt.peak_v << "," << pt.activeTorque() << "\n";
    }

    m_status_msg = "Saved data to " + filepath;
}

} // namespace