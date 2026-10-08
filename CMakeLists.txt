cmake_minimum_required(VERSION 3.20)

project(Calgary_TAR
        VERSION 0.1
        LANGUAGES CXX C
)

include(FetchContent)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# OpenGL
# ============================================================
find_package(OpenGL REQUIRED)

# ============================================================
# GLFW
# ============================================================
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 232164f
)
FetchContent_MakeAvailable(glfw)

# ============================================================
# Dear ImGui
# ============================================================
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG 1bcc23e
)
FetchContent_MakeAvailable(imgui)

set(IMGUI_SRC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SRC})

target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC
        glfw
        OpenGL::GL
)

# ============================================================
# Eigen 5.0.0
# ============================================================
FetchContent_Declare(
        eigen
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
        GIT_TAG 5.0.0
)
FetchContent_MakeAvailable(eigen)

# ============================================================
# ImPlot
# ============================================================
FetchContent_Declare(
        implot
        GIT_REPOSITORY https://github.com/epezent/implot.git
        GIT_TAG 81b8b19
)
FetchContent_MakeAvailable(implot)

add_library(implot STATIC
        ${implot_SOURCE_DIR}/implot.cpp
        ${implot_SOURCE_DIR}/implot_items.cpp
)

target_include_directories(implot PUBLIC
        ${implot_SOURCE_DIR}
)

target_link_libraries(implot PUBLIC imgui)

# ============================================================
# tinyfiledialogs
# ============================================================
add_library(tinyfd STATIC
        external/tinyfiledialogs/tinyfiledialogs.c
)

target_include_directories(tinyfd PUBLIC
        external/tinyfiledialogs
)

# ============================================================
# glad1 (Windows)
# ============================================================
if(WIN32)
    add_library(glad STATIC
            external/glad/src/glad.c
    )
    target_include_directories(glad PUBLIC
            external/glad/include
    )
endif()

# ============================================================
# Executable Target
# ============================================================
add_executable(Calgary_TAR
    main.cpp
    src/Calgary_TAR/App.cpp
    src/Calgary_TAR/ImGuiTheme.cpp
)

target_include_directories(Calgary_TAR PRIVATE
    include
)

target_link_libraries(Calgary_TAR PRIVATE
    imgui
    implot
    tinyfd
    Eigen3::Eigen
)

if(WIN32)
    target_link_libraries(Calgary_TAR PRIVATE
        glad
        opengl32
    )
endif()

# ============================================================
# App Icon Configuration
# ============================================================
if(APPLE)
    set(APP_ICON ${CMAKE_CURRENT_SOURCE_DIR}/icon/TAR_icon_calgary.icns)

    set_source_files_properties(${APP_ICON} PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
    )

    target_sources(Calgary_TAR PRIVATE ${APP_ICON})

    set_target_properties(Calgary_TAR PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_ICON_FILE "TAR_icon_calgary.icns"
    )
elseif(WIN32)
    set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/icon/TAR_icon_calgary.ico")
    set(WIN_RC "${CMAKE_CURRENT_BINARY_DIR}/TAR_icon_calgary.rc")
    file(WRITE "${WIN_RC}" "1 ICON DISCARDABLE \"${APP_ICON}\"")
    target_sources(Calgary_TAR PRIVATE "${WIN_RC}")
endif()