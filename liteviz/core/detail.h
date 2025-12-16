#ifndef __LITEVIZ_DETAIL_H__
#define __LITEVIZ_DETAIL_H__

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <liteviz/core/common.h>
#include <liteviz/core/shader.h>
#include <liteviz/core/viewport.h>
#include <liteviz/core/mesh.h>
#include <liteviz/core/base_renderer.h>
#include <liteviz/core/base_config.h>
#include <liteviz/core/image.h>

namespace liteviz {

struct ViewerNotifier {  
public:  
    bool ready = true;  
    std::mutex mtx;  
    std::condition_variable cv;  
};

class GlobalConfig: public BaseConfig {
public:
    GlobalConfig() = default;
    ~GlobalConfig() override = default;
};

class ViewerDetail{

public:
    ViewerDetail(std::string title, int width, int height);

    bool init();

    void updateWindowSize();

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    static void cursorPosCallback(GLFWwindow* window, double x, double y);

    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void dropCallback(GLFWwindow* window, int count, const char** paths);
    
    std::string getTimestamp();

    Image getFrameBuffer();

    void configuration(BaseConfig* config);

    void controlFrameRate(BaseConfig* config);

    void draw();

protected:

    virtual bool initResources() = 0;
    void renderAll(Viewport& viewport);
    std::vector<std::shared_ptr<BaseRenderer>> _registeredRenderers;
    std::vector<std::shared_ptr<BaseRenderer>> _registeredGUIRenderers;
    std::vector<std::shared_ptr<BaseConfig>> _registeredConfigs;

public:
    std::string title;
    GLFWwindow* window;

    Viewport _viewport;
    static ViewerDetail* _detail;
    std::shared_ptr<GlobalConfig> _config;

    ImGuiWindowFlags window_flags = 0;
    vec4f clearColor = vec4f(1.0f, 1.0f, 1.0f, 0.00f);
    bool any_window_active = false;
    bool isRunning = true;

    // Notifier for synchronization
    std::mutex notifier_mutex;
    std::shared_ptr<ViewerNotifier> _notifier = nullptr;

    // Control frame rate
    int targetFPS = 30;
    int frameTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;

    bool show_default_configuration = true;
};

} // namespace liteviz

#endif // __LITEVIZ_DETAIL_H__



