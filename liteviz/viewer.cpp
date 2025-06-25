#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>
#include <chrono>
#include <mutex>
#include <condition_variable> 
#include <opencv2/opencv.hpp>
#include "shader.h"
#include "mesh.h"
#include "viewport.h"
#include "viewer.h"


ViewerDetail::ViewerDetail(std::string title, int width, int height):
    title(title), viewport(width, height){
    viewer = this;
    notifier = std::make_shared<ViewerNotifier>();
}

bool ViewerDetail::init(){
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(
        viewer->viewport.windowSize.x(), 
        viewer->viewport.windowSize.y(), 
        viewer->title.c_str(), NULL, NULL);

    if (window == NULL){
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        // LOG_ERROR("GLAD init failed");
        std::cerr << "GLAD init failed" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSwapInterval(1); // Enable vsync

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetDropCallback(window, dropCallback);

    glEnable(GL_LINE_SMOOTH);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);  
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    const char* glsl_version = "#version 430";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Set Fonts
    std::string font_path = std::string(RESOURCE_DIR) + "/liteviz/assets/JetBrainsMono-Regular.ttf";
    io.Fonts->AddFontFromFileTTF(font_path.c_str(), 14.0f);

    // Set Windows option
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    window_flags |= ImGuiWindowFlags_NoResize;
    // window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowPadding = ImVec2(0.0f, 6.0f);
    style.WindowRounding = 6.0f;
    style.WindowBorderSize = 0.0f;
    // style.Colors[ImGuiCol_Text] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

    return true;
}

void ViewerDetail::updateWindowSize(){
    glfwGetWindowSize(viewer->window, &viewer->viewport.windowSize.x(), &viewer->viewport.windowSize.y());
    glfwGetFramebufferSize(viewer->window, &viewer->viewport.frameBufferSize.x(), &viewer->viewport.frameBufferSize.y());
    glViewport(0, 0, viewer->viewport.frameBufferSize.x(), viewer->viewport.frameBufferSize.y());
}


void ViewerDetail::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if(viewer->any_window_active)
        return;

    if((button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        viewer->viewport.camera.initScreenPos(Eigen::Vector2f(xpos, ypos));
    }
}

void ViewerDetail::cursorPosCallback(GLFWwindow* window, double x, double y) {
    if(viewer->any_window_active)
        return;

    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        viewer->viewport.camera.translate(Eigen::Vector2f(x, y));
    } else if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        viewer->viewport.camera.rotate(Eigen::Vector2f(x, y));
    }
}

void ViewerDetail::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if(viewer->any_window_active)
        return;

    const float delta = static_cast<float>(yoffset);
    if(std::abs(delta) < 1.0e-2f) return;

    viewer->viewport.camera.zoom(delta);
}

void ViewerDetail::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if(viewer->any_window_active)
        return;

    auto &viewport = viewer->viewport;

    // TODO: not implemented yet (pxk)
}

void ViewerDetail::dropCallback(GLFWwindow* window, int count, const char** paths) {
    if(viewer->any_window_active)
        return;

    if (count > 0) {
        std::string path(paths[0]);
        if (std::filesystem::exists(path)) {
            std::cout << "File dropped: " << path << std::endl;

        } else {
            std::cerr << "File does not exist: " << path << std::endl;
        }
    }
}

void ViewerDetail::loadResources() {

    std::string shader_path = std::string(RESOURCE_DIR) + "/liteviz/shaders";

    pointShader = std::make_shared<Shader>(
        (shader_path + "/draw_point.vert").c_str(), 
        (shader_path + "/draw_point.frag").c_str(), 
        true
    );

    gridShader = std::make_shared<Shader>(
        (shader_path + "/draw_grid.vert").c_str(), 
        (shader_path + "/draw_grid.frag").c_str(),
        true
    );

    grid = std::make_shared<Grid>();
    grid->setup();

    frustum = std::make_shared<Frustum>();

    pointCloud = std::make_shared<PointCloud>();

    line = std::make_shared<Line>();

    colorTexture = std::make_shared<ImageTexture>(GL_RGB, GL_BGR, GL_UNSIGNED_BYTE);
}

void ViewerDetail::run() {

    if(!init()){
        std::cerr << "Failed to init LiteViz" << std::endl;
        return;
    }

    loadResources();

    while (!glfwWindowShouldClose(window)){

        glClearBufferfv(GL_COLOR, 0, bgColor.data());

        updateWindowSize();

        drawData();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

ViewerDetail* ViewerDetail::viewer = nullptr;