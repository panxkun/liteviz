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
#include <condition_variable> 
#include <opencv2/opencv.hpp>
#include "shader.h"
#include "mesh.h"
#include "viewport.h"

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

    Eigen::Vector4f bgColor = Eigen::Vector4f(0.2f, 0.2f, 0.2f, 0.00f);
    
    std::shared_ptr<ViewerNotifier> notifier        = nullptr;
    std::shared_ptr<Shader>         pointShader     = nullptr;
    std::shared_ptr<Shader>         gridShader      = nullptr;
    std::shared_ptr<Grid>           grid            = nullptr;
    std::shared_ptr<Frustum>        frustum         = nullptr;
    std::shared_ptr<PointCloud>     pointCloud      = nullptr;
    std::shared_ptr<Line>           line            = nullptr;
    std::shared_ptr<ImageTexture>   colorTexture    = nullptr;
};

#endif // __VIEWER_H__



