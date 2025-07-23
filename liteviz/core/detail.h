#ifndef __VIEWER_H__
#define __VIEWER_H__

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>
#include <condition_variable> 
#include <liteviz/core/shader.h>
#include <liteviz/core/mesh.h>
#include <liteviz/core/viewport.h>

struct ViewerNotifier {  
public:  
    bool ready = true;  
    std::mutex mtx;  
    std::condition_variable cv;  
};

class ViewerDetail{

public:
    ViewerDetail(std::string title, int width, int height);

    bool init();

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    static void cursorPosCallback(GLFWwindow* window, double x, double y);

    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void dropCallback(GLFWwindow* window, int count, const char** paths);

    void loadResources();
    
    void updateWindowSize();

    void run();

    void setFrameRate(const int fps);

    void controlFrameRate();

    virtual void drawData() = 0;

public:
    std::string title;
    GLFWwindow* window;
    ImGuiWindowFlags window_flags = 0;

    static ViewerDetail* viewer;
    bool any_window_active = false;
    bool isRunning = true;

    std::mutex notifier_mutex;

    Viewport viewport;

    Eigen::Vector4f bgColor;
    
    std::shared_ptr<ViewerNotifier> notifier        = nullptr;
    std::shared_ptr<Shader>         pointShader     = nullptr;
    std::shared_ptr<Shader>         gridShader      = nullptr;
    std::shared_ptr<Grid>           grid            = nullptr;
    std::shared_ptr<Frustum>        frustum         = nullptr;
    std::shared_ptr<PointCloud>     pointCloud      = nullptr;
    std::shared_ptr<Line>           line            = nullptr;
    std::shared_ptr<ImageTexture>   colorTexture    = nullptr;

    int targetFPS = 30;
    int frameTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
};

#endif // __VIEWER_H__



