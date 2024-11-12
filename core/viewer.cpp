#include <GL/glew.h>  
#include <GLFW/glfw3.h>
#include "viewer.h"

ViewerDetail* ViewerDetail::viewer = nullptr;

ViewerDetail::ViewerDetail(std::string title, int width, int height):
    title(title), viewport(width, height){

    viewer = this;

    frustum = std::make_unique<Frustum>();
    world_frame = std::make_unique<CoordinateFrame>();

    grid = std::make_unique<Grid>();
    grid->setup();

    cube = std::make_shared<Cube>();
    cube->setup();

    point_cloud = std::make_unique<PointCloud>();
    point_cloud->setup();
    
    surfels = std::make_unique<Surfel>();
    surfels->setup();

    notifier = std::make_shared<ViewerNotifier>();

}

Eigen::Matrix4f ViewerDetail::compute_camera_transform(
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
    unload();
    destroy();
}

void ViewerDetail::load_shader(){
    grid_shader     = std::make_shared<Shader>(
        "./liteviz/shaders/draw_grid.vert", 
        "./liteviz/shaders/draw_grid.frag");

    points_shader   = std::make_shared<Shader>(
        "./liteviz/shaders/draw_points.vert", 
        "./liteviz/shaders/draw_points.frag");

    texture_shader  = std::make_shared<Shader>(
        "./liteviz/shaders/draw_texture.vert", 
        "./liteviz/shaders/draw_texture.frag");
    
    triangle_shader = std::make_shared<Shader>(
        "./liteviz/shaders/draw_triangles.vert", 
        "./liteviz/shaders/draw_triangles.frag");

    surfels_shader  = std::make_shared<Shader>(
        "./liteviz/shaders/draw_surfels.vert", 
        "./liteviz/shaders/draw_surfels.frag",
        "./liteviz/shaders/draw_surfels.geom"
        );
}


void ViewerDetail::unload() {
    grid_shader.reset();
    points_shader.reset();
    texture_shader.reset();
    triangle_shader.reset();
    surfels_shader.reset();
}

void ViewerDetail::draw_camera(
    Eigen::Matrix4f pose,
    Eigen::Vector4f intrinsics,
    Eigen::Vector4f color,
    const size_t linewidth,
    const float scale){

    Eigen::Matrix4f transform = viewport.getProjectionMatrix() * viewport.getViewMatrix();

    glLineWidth(linewidth);

    frustum->setup(intrinsics, scale);
    frustum->setColor(color);
    frustum->transform(pose);
    frustum->draw(points_shader, transform, viewport);

    Eigen::Vector3f x = Eigen::Vector3f(1, 0, 0);
    Eigen::Vector3f y = Eigen::Vector3f(0, -1, 0);
    Eigen::Vector3f z = Eigen::Vector3f(0, 0, -1);

    world_frame->setup(scale,x, y, z);
    world_frame->transform(pose);
    world_frame->draw(points_shader, transform, viewport);  

    glLineWidth(1.0f);
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
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(viewer->viewport.window_size.x(), viewer->viewport.window_size.y(), viewer->title.c_str(), NULL, NULL);

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
    const char* glsl_version = "#version 330";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Set Fonts
    io.Fonts->AddFontFromFileTTF("./liteviz/assets/JetBrainsMono-Regular.ttf", 14.0f);

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
        std::cerr << "Failed to init the SLAM Viewer window" << std::endl;
        return;
    }
 
    load_shader();

    image_window = std::make_shared<ImageTexture>();
    
    while (!glfwWindowShouldClose(window))
    {
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_DEPTH_BUFFER_BIT);

        GLfloat clearColor[4] = {clear_color.x, clear_color.y, clear_color.z, clear_color.w};
        glClearBufferfv(GL_COLOR, 0, clearColor);

        updateWindowSize();

        predraw();

        draw();

        gui();

        viewport.reset();
        
        glfwPollEvents();

        glfwSwapBuffers(window);
    }
}

void ViewerDetail::setNotifier(std::shared_ptr<ViewerNotifier> notifier){
    viewer->notifier = notifier;
}

void ViewerDetail::predraw(){

    Eigen::Matrix4f transform = viewport.getProjectionMatrix() * viewport.getViewMatrix();
    
    grid->draw(grid_shader, transform, viewport);

    // cube->draw(triangle_shader, transform, viewport);

    // point_cloud->draw(points_shader, transform, viewport);

    // surfels->draw(surfels_shader, transform, viewport);

    draw_camera(
        Eigen::Matrix4f::Identity(), 
        Eigen::Vector4f(600, 600, 320, 240), 
        Eigen::Vector4f(0.9059f, 0.2980f, 0.2353f, 1.0f), 
        2, 0.001f);

    world_frame->setup();
    world_frame->transform(Eigen::Matrix4f::Identity());
    world_frame->draw(points_shader, transform, viewport);  
}

void ViewerDetail::gui(){
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(ImVec2(panel_size.x, 0), ImGuiCond_Always);

    window_hovered = false;
    any_window_active = ImGui::IsAnyItemActive() ;

    if (show_imgui_demo_window)
        ImGui::ShowDemoWindow(&show_imgui_demo_window);  

    configuration_panel();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void ViewerDetail::configuration_panel(){
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.3f));
    ImGui::SetNextWindowSize(ImVec2(panel_size.x, 0), ImGuiCond_Always);
    ImGui::Begin("Configuration", nullptr,  window_flags); 
    window_hovered |= check_window_hovered();

    ImGui::Checkbox("Show ImGui Demo Window", &show_imgui_demo_window);

    // static ImGuiSliderFlags flags = ImGuiSliderFlags_None;
    // ImGui::SliderInt("Progress", &selected_id, 0, latest_id, "%d", flags);

    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 size = ImGui::GetItemRectSize();
    size.x = windowSize.x;

    render_canvas(size.x);

    if (ImGui::Button("Save", size)){
        save_canvas();
    }

    if(notifier){
        std::lock_guard<std::mutex> lock(notifier->mtx);  
        const char* buttonText = isRunning ? "Running..." : "Stopped";
        if (ImGui::Button(buttonText, size)){
            isRunning = !isRunning;
            notifier->ready = isRunning;
        }
        notifier->cv.notify_one();
    }

    static ImGuiSliderFlags flags1 = ImGuiSliderFlags_None;
    ImGui::SliderInt("Point Size", &pc_size, 1, 10, "%d", flags1);

    ImGui::Checkbox("Show Grid", &show_grid);

    if (ImGui::CollapsingHeader("Virtual World")){
        render_window();
    }

    if (ImGui::CollapsingHeader("Global Map")){
        ImGui::Checkbox("Show KeyFrame", &show_map_keyframe);
        ImGui::Checkbox("Show Image", &show_map_image);
        ImGui::Checkbox("Show PointCloud", &show_map_pointcloud);
    }

    if (ImGui::CollapsingHeader("Colors")){
        ImGui::ColorEdit4("Background", (float*)&clear_color);
        ImGui::ColorEdit4("Keyframe", (float*)&keyframe_color);
        ImGui::ColorEdit4("PointCloud", (float*)&point_cloud_color);
    }

    ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();
    ImGui::PopStyleColor();
}

void ViewerDetail::save_canvas(){

    cv::Mat color;
    cv::Mat depth;

    // color
    cv::Mat color_mat(viewport.framebuffer_size.y(), viewport.framebuffer_size.x(), CV_8UC3);
    glReadPixels(0, 0, color_mat.cols, color_mat.rows, GL_BGR, GL_UNSIGNED_BYTE, color_mat.data);
    cv::flip(color_mat, color, 0);
    std::string filename_color = "snapshot-color-" + get_timestamp() + ".png";
    cv::imwrite(filename_color, color);

    // depth
    cv::Mat depth_mat(viewport.framebuffer_size.y(), viewport.framebuffer_size.x(), CV_32FC1);
    glReadPixels(0, 0, depth_mat.cols, depth_mat.rows, GL_DEPTH_COMPONENT, GL_FLOAT, depth_mat.data);
    cv::flip(depth_mat, depth, 0);
    std::string filename_depth = "snapshot-depth-" + get_timestamp() + ".png";
    cv::imwrite(filename_depth, depth);
}   

std::string ViewerDetail::get_timestamp(){
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
    glfwGetWindowSize(viewer->window, &viewer->viewport.window_size.x(), &viewer->viewport.window_size.y());
    glfwGetFramebufferSize(viewer->window, &viewer->viewport.framebuffer_size.x(), &viewer->viewport.framebuffer_size.y());
    glViewport(0, 0, viewer->viewport.framebuffer_size.x(), viewer->viewport.framebuffer_size.y());
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

    // if (glfwGetKey(viewer->window, GLFW_KEY_W) == GLFW_PRESS){
    //         viewport.viewport_ypr.y() -= 2;
    // }

    // if (glfwGetKey(viewer->window, GLFW_KEY_S) == GLFW_PRESS){
    //     viewport.viewport_ypr.y() += 2;
    // }

    // if (glfwGetKey(viewer->window, GLFW_KEY_A) == GLFW_PRESS){
    //     viewport.viewport_ypr.x() -= 2;
    // }

    // if (glfwGetKey(viewer->window, GLFW_KEY_D) == GLFW_PRESS){
    //     viewport.viewport_ypr.x() += 2;
    // }
    // if (glfwGetKey(viewer->window, GLFW_KEY_UP) == GLFW_PRESS){
    //     viewport.scale = std::clamp(viewport.scale * (1.0 + 2 / 100.0), 1.0e-4, 1.0e4);
    // }
    // if (glfwGetKey(viewer->window, GLFW_KEY_DOWN) == GLFW_PRESS){
    //     viewport.scale = std::clamp(viewport.scale * (1.0 - 2 / 100.0), 1.0e-4, 1.0e4);
    // }
}

bool ViewerDetail::check_window_hovered(){
    return ImGui::IsItemHovered() || ImGui::IsAnyItemHovered()? true: false;
}

void ViewerDetail::destroy(){
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
}