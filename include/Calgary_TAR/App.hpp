#pragma once

#include "DataModel.hpp"
#include <vector>
#include <string>
#include <mutex>
#include <memory>
#include <imgui.h>

struct GLFWwindow;

namespace TARapp {

class App {
public:
    App();
    ~App();

    void run();

private:
    void init();
    void cleanup();
    void updateFit();

    // Rendering methods
    void renderUI();
    void renderMenuBar();
    void renderTablePanel();
    void renderQueryPanel();
    void renderPlotsPanel();
    void renderAboutModal();

    // Data I/O
    void loadCSV(const std::string& filepath);
    void saveCSV(const std::string& filepath);

    // State
    GLFWwindow* m_window = nullptr;
    bool m_running = true;

    std::vector<TorquePoint> m_points;
    int m_poly_degree = 3;
    PolyFitResult m_fit;

    // Query parameters
    int m_query_mode = 0;           // 0 = By Angle, 1 = By Target Force %
    float m_query_angle = 43.0f;    // Query angle input (deg)
    float m_query_force_pct = 0.30f;// Query force % input (0..1, e.g. 0.30 = 30%)

    // Status message for feedback
    std::string m_status_msg = "Ready";
    bool m_show_about = false;
    bool m_reset_axes = true;
};

} // namespace archi
