#include <liteviz/core/detail.h>
#include <liteviz/core/image.h>

liteviz::ViewerDetail* liteviz::ViewerDetail::_detail = nullptr;

liteviz::ViewerDetail::ViewerDetail(std::string title, int width, int height):
    title(title), _viewport(width, height) {
    _detail = this;
    _notifier = std::make_shared<ViewerNotifier>();
    _config = std::make_shared<GlobalConfig>();
}

bool liteviz::ViewerDetail::init(){
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(
        _detail->_viewport.windowSize.x(), 
        _detail->_viewport.windowSize.y(), 
        _detail->title.c_str(), NULL, NULL);

    if (window == NULL){
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD init failed" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwSwapInterval(1); // Enable vsync

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetDropCallback(window, dropCallback);

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    const char* glsl_version = "#version 430";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Set Fonts
    std::string font_path = std::string(RESOURCE_DIR) + "/assets/JetBrainsMono-Regular.ttf";
    io.Fonts->AddFontFromFileTTF(font_path.c_str(), 14.0f);

    // Set Windows option
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    window_flags |= ImGuiWindowFlags_NoResize;

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowPadding = ImVec2(6.0f, 6.0f);
    style.WindowRounding = 6.0f;
    style.WindowBorderSize = 0.0f;

    return true;
}

void liteviz::ViewerDetail::updateWindowSize(){
    glfwGetWindowSize(_detail->window, &_detail->_viewport.windowSize.x(), &_detail->_viewport.windowSize.y());
    glfwGetFramebufferSize(_detail->window, &_detail->_viewport.frameBufferSize.x(), &_detail->_viewport.frameBufferSize.y());
    glViewport(0, 0, _detail->_viewport.frameBufferSize.x(), _detail->_viewport.frameBufferSize.y());
}


void liteviz::ViewerDetail::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if(_detail->any_window_active)
        return;

    if((button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        _detail->_viewport.camera.initScreenPos(liteviz::vec2f(xpos, ypos));
    }
}

void liteviz::ViewerDetail::cursorPosCallback(GLFWwindow* window, double x, double y) {
    if(_detail->any_window_active)
        return;

    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        _detail->_viewport.camera.translate(liteviz::vec2f(x, y));
    } else if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        _detail->_viewport.camera.rotate(liteviz::vec2f(x, y));
    }
}

void liteviz::ViewerDetail::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (_detail->any_window_active)
        return;

    const float raw_delta = static_cast<float>(yoffset);
    if (std::abs(raw_delta) < 1.0e-2f) return;

    // If Shift is pressed, make the scroll effect finer (0.1x)
    bool shift_pressed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ||
                         (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
    const float scale = shift_pressed ? 0.1f : 1.0f;
    const float delta = raw_delta * scale;

    _detail->_viewport.camera.zoom(delta);
}

void liteviz::ViewerDetail::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if(_detail->any_window_active)
        return;

    auto &_viewport = _detail->_viewport;
    // TODO: not implemented yet (pxk)
}

void liteviz::ViewerDetail::dropCallback(GLFWwindow* window, int count, const char** paths) {
    if(_detail->any_window_active)
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

std::string liteviz::ViewerDetail::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    auto sectime = std::chrono::duration_cast<std::chrono::seconds>(now_ms);

    std::time_t timet = sectime.count();
    struct tm curtime;
    localtime_r(&timet, &curtime);

    std::stringstream ss;
    ss << std::put_time(&curtime, "%Y-%m-%d-%H-%M-%S");
    std::string buffer = ss.str();
    return std::string(buffer);
}

liteviz::Image liteviz::ViewerDetail::getFrameBuffer() {
    int w = _viewport.frameBufferSize.x();
    int h = _viewport.frameBufferSize.y();

    liteviz::Image img(w, h, 4);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, img.ptr());

    liteviz::ImageIO::flip_vertical(img);
    return img;
}

void liteviz::ViewerDetail::controlFrameRate(liteviz::BaseConfig* config) {

    if(!config->vsync || config->targetFrameRate <= 0)
        return;

    frameTime = 1000.0f / config->targetFrameRate;

    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime).count();
    if (duration < frameTime) {
        std::this_thread::sleep_for(std::chrono::milliseconds(frameTime - duration));
    }
    lastTime = std::chrono::high_resolution_clock::now();
}

void liteviz::ViewerDetail::configuration(liteviz::BaseConfig* config) {

    if(!show_default_configuration)
        return;

    ImVec4 configBgColor = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
    if (config->transparentConfigBG) {
        configBgColor = ImVec4(0.1f, 0.1f, 0.1f, 0.3f);
    }

    ImGui::PushStyleColor(ImGuiCol_WindowBg, configBgColor);
    ImGui::Begin("Global Configuration", nullptr, window_flags);
    ImGui::SetWindowSize(ImVec2(config->x_size, 0));

    ImGui::Text("LiteViz Configuration");

    ImGui::Separator();

    if (ImGui::Button("Save snapshot", ImVec2(-FLT_MIN, config->y_size * 2.0f))) {
        Image img = getFrameBuffer();
        ImageIO::save_png("snapshot-color-" + getTimestamp() + ".png", img);
    }

    ImGui::SliderFloat("##fov_slider", &config->fov, 10.0f, 120.0f, "FoV=%.1f");
    ImGui::SameLine();
    if (ImGui::Button("Reset##fov", ImVec2(50.0f, 0.0f))) {
        config->fov = 60.0f;
    }

    ImGui::Checkbox("Vertical Synch.", &config->vsync);
    ImGui::Checkbox("Transparent Config BG", &config->transparentConfigBG);

    if (ImGui::ColorEdit4("Background", config->bgColor.data())) {
        clearColor[0] = config->bgColor[0];
        clearColor[1] = config->bgColor[1];
        clearColor[2] = config->bgColor[2];
        clearColor[3] = config->bgColor[3];
    }
    
    float frameTime = 1000.0f / ImGui::GetIO().Framerate;
    float frameRate = ImGui::GetIO().Framerate;
    ImGui::Text("Average %.3f ms/frame (%.1f FPS)", frameTime, frameRate);

    ImGui::End();
}

void liteviz::ViewerDetail::draw() {

    if (!this->init()) {
        std::cerr << "Failed to init LiteViz" << std::endl;
        return;
    }
    
    if (!this->initResources()) {
        std::cerr << "initResources returned false" << std::endl;
        return;
    }

    while (!glfwWindowShouldClose(window)) {

        glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        updateWindowSize();

        renderAll(_viewport);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
}


void liteviz::ViewerDetail::renderAll(liteviz::Viewport& _viewport) {

    for (const auto& renderer : _registeredRenderers) {
        renderer->render(_viewport);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    _detail->any_window_active = ImGui::IsAnyItemActive();

    configuration(_config.get()); 

    for (const auto& renderer : _registeredGUIRenderers) {
        renderer->render(_viewport);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapInterval(_config->vsync ? 1 : 0);

    _viewport.setFoV(_config->fov);

    controlFrameRate(_config.get());
}