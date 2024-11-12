#ifndef __VIEWER_H__
#define __VIEWER_H__

#include <filesystem>
#include <Eigen/Eigen>
#include <Eigen/Dense>
#include <memory>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <condition_variable> 
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <liteviz/core/framebuffer.h>
#include <liteviz/core/shader.h>
#include <liteviz/core/mesh.h>
#include <liteviz/core/viewport.h>

#include <GL/glew.h>  
#include <GLFW/glfw3.h>

const ImVec4 colorTables[] = {
    ImVec4(0.9059f, 0.2980f, 0.2353f, 1.0f),    // RGB: 231, 76, 60
    ImVec4(0.9451f, 0.7686f, 0.0588f, 1.0f),    // RGB: 241, 196, 15
    ImVec4(0.2196f, 0.6980f, 0.3058f, 1.0f),    // RGB: 56, 178, 78
    ImVec4(0.3647f, 0.6784f, 0.8863f, 1.0f),    // RGB: 93, 173, 226
    ImVec4(0.7900f, 0.6190f, 1.0000f, 1.0f),    // RGB: 74, 20, 140
    ImVec4(0.1137f, 0.5137f, 0.2824f, 1.0f),    // RGB: 29, 131, 72
};

struct ViewerNotifier {  
public:  
    bool ready = true;  
    std::mutex mtx;  
    std::condition_variable cv;  
};


class ViewerDetail{
public:

    std::string root = "/home/xiaokun/Workspace/Code/liteviz-dev/";

    std::string title;
    Viewport viewport;

    std::shared_ptr<Shader>             gridShader;
    std::shared_ptr<Shader>             pointShader;
    std::shared_ptr<Shader>             textureShader;
    std::shared_ptr<Shader>             triangleShader;

    std::unique_ptr<Grid>               grid;
    std::shared_ptr<Cube>               cube;
    std::unique_ptr<Frustum>            frustum;
    std::unique_ptr<CoordinateFrame>    worldFrame;
    std::unique_ptr<PointCloud>         pointcloud;

    std::shared_ptr<ViewerNotifier>     notifier;

    ViewerDetail() {}
    ViewerDetail(std::string title, int width=1280, int height=720);
    ~ViewerDetail();

    bool init();

    void preDraw3D();

    void gui();

    void saveCanvas();

    std::string getTimestamp();

    void setNotifier(std::shared_ptr<ViewerNotifier> notifier);

    static void updateWindowSize();

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    static void cursorPosCallback(GLFWwindow* window, double x, double y);

    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    bool checkWindowHovered();

    Eigen::Matrix4f computeCameraTransformation(
        const Eigen::Vector3f& cameraPosition, 
        const Eigen::Quaternionf& cameraOrientation,  
        const Eigen::Vector4f& intrinsics,
        const int viewportWidth, 
        const int viewportHeight,
        const float near=1.0e-2, 
        const float far=1.0e4);

    virtual void run();

    virtual void loadResource();

    bool show_imgui_demo_window = false;
    bool any_window_active      = false;
    bool isRunning              = true;
    bool showGrid               = true;

    // bool record = false;
    // cv::VideoWriter outputVideo;
    Eigen::Vector4f clearColor = Eigen::Vector4f(0.20f, 0.20f, 0.20f, 0.00f);
    ImGuiWindowFlags window_flags = 0;

    GLFWwindow* window;
    static ViewerDetail* viewer;

    virtual void draw3D() = 0;
    virtual void drawCanvas() = 0;
    virtual void setConfigurationPanel() = 0;
};


ViewerDetail* ViewerDetail::viewer = nullptr;

ViewerDetail::ViewerDetail(std::string title, int width, int height):
    title(title), viewport(width, height){

    viewer = this;

    frustum = std::make_unique<Frustum>();

    worldFrame = std::make_unique<CoordinateFrame>();
    worldFrame->setup();

    grid = std::make_unique<Grid>();
    grid->setup();

    cube = std::make_shared<Cube>();
    cube->setup();

    pointcloud = std::make_unique<PointCloud>();
    pointcloud->setup();

    notifier = std::make_shared<ViewerNotifier>();

}

Eigen::Matrix4f ViewerDetail::computeCameraTransformation(
    const Eigen::Vector3f& cameraPosition, 
    const Eigen::Quaternionf& cameraOrientation,  
    const Eigen::Vector4f& intrinsics,
    const int viewportWidth, 
    const int viewportHeight,
    const float near, 
    const float far) {  

    Eigen::Matrix4f projmatrix = Eigen::Matrix4f::Zero();
    projmatrix(0, 0) = 2.0 * viewportHeight / viewportWidth;
    projmatrix(1, 1) = -2;
    projmatrix(2, 2) = (far + near) / (far - near);
    projmatrix(2, 3) = 2 * far * near / (near - far);
    projmatrix(3, 2) = 1.0;

    Eigen::Matrix3f rotation = cameraOrientation.matrix();
    Eigen::Vector3f position = cameraPosition;
    Eigen::Matrix4f viewmatrix = Eigen::Matrix4f::Identity();
    viewmatrix.block<3, 3>(0, 0) = rotation;
    viewmatrix.block<3, 1>(0, 3) = position;

    Eigen::Matrix4f transform = projmatrix * viewmatrix.inverse();

    return transform;  
}  

ViewerDetail::~ViewerDetail(){

    gridShader.reset();
    pointShader.reset();
    textureShader.reset();
    triangleShader.reset();

    if (ImGui::GetCurrentContext() != nullptr) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    if (window != nullptr) {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    glfwTerminate();
}

void ViewerDetail::loadResource(){

    std::string shader_base_path = root + "liteviz/shaders/";

    gridShader = std::make_shared<Shader>(
        (shader_base_path + "draw_grid.vert").c_str(),
        (shader_base_path + "draw_grid.frag").c_str()
    );

    pointShader = std::make_shared<Shader>(
        (shader_base_path + "draw_points.vert").c_str(),
        (shader_base_path + "draw_points.frag").c_str()
    );

    textureShader = std::make_shared<Shader>(
        (shader_base_path + "draw_texture.vert").c_str(),
        (shader_base_path + "draw_texture.frag").c_str()
    );

    triangleShader = std::make_shared<Shader>(
        (shader_base_path + "draw_triangles.vert").c_str(),
        (shader_base_path + "draw_triangles.frag").c_str()
    );
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

    window = glfwCreateWindow(viewer->viewport.windowSize.x(), viewer->viewport.windowSize.y(), viewer->title.c_str(), NULL, NULL);

    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        return false;
    }

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);

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
    const char* glsl_version = "#version 400";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Set Fonts
    std::string font_path = root + "liteviz/assets/JetBrainsMono-Regular.ttf";
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

    return true;
}

void ViewerDetail::run(){

    if(!init()){
        std::cerr << "Failed to init LiteViz" << std::endl;
        return;
    }
 
    loadResource();
    
    while (!glfwWindowShouldClose(window))
    {
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_DEPTH_BUFFER_BIT);

        glClearBufferfv(GL_COLOR, 0, clearColor.data());

        updateWindowSize();

        preDraw3D();

        draw3D();

        gui();

        glfwSwapBuffers(window);

        glfwPollEvents();
    }
}

void ViewerDetail::setNotifier(std::shared_ptr<ViewerNotifier> notifier){
    viewer->notifier = notifier;
}

void ViewerDetail::preDraw3D(){

    // show some mesh example

    Eigen::Matrix4f transform = viewport.getProjectionMatrix() * viewport.getViewMatrix();
    
    if(showGrid)
        grid->draw(gridShader, transform, viewport);

    // cube->draw(triangleShader, transform, viewport);

    // point_cloud->draw(pointShader, transform, viewport);

    // surfels->draw(surfelShader, transform, viewport);

    // draw_camera(
    //     Eigen::Matrix4f::Identity(), 
    //     Eigen::Vector4f(600, 600, 320, 240), 
    //     Eigen::Vector4f(0.9059f, 0.2980f, 0.2353f, 1.0f), 
    //     2, 0.001f);

    worldFrame->setup();
    worldFrame->transform(Eigen::Matrix4f::Identity());
    worldFrame->draw(pointShader, transform, viewport);
}

void ViewerDetail::gui(){
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    any_window_active = ImGui::IsAnyItemActive() ;

    if (show_imgui_demo_window)
        ImGui::ShowDemoWindow(&show_imgui_demo_window);  

    setConfigurationPanel();

    drawCanvas();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void ViewerDetail::saveCanvas(){

    size_t w = viewport.frameBufferSize.x();
    size_t h = viewport.frameBufferSize.y();

    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    cv::Mat color_mat(h, w, CV_8UC4);
    glReadPixels(0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE, color_mat.data);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);

    cv::flip(color_mat, color_mat, 0);
    cv::imwrite("snapshot-color-" + getTimestamp() + ".png", color_mat);

}   

std::string ViewerDetail::getTimestamp(){
    auto now        = std::chrono::system_clock::now();
    auto now_ms     = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    auto sectime    = std::chrono::duration_cast<std::chrono::seconds>(now_ms);

    std::time_t timet = sectime.count();
    struct tm curtime;
    localtime_r(&timet, &curtime);

    std::stringstream ss;
    ss << std::put_time(&curtime, "%Y-%m-%d-%H-%M-%S");
    std::string buffer = ss.str();
    return std::string(buffer);
}

void ViewerDetail::updateWindowSize()
{
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

bool ViewerDetail::checkWindowHovered(){
    return ImGui::IsItemHovered() || ImGui::IsAnyItemHovered()? true: false;
}

#endif