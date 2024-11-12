#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <viewer.h>
#include <thread>

class Viewer: public ViewerDetail{
public:
    Viewer(std::string title, int width=1280, int height=720, bool isRunning=false):
        ViewerDetail(title, width, height){
            ViewerDetail::isRunning = isRunning;
            ViewerDetail::show_grid = true;
            ViewerDetail::global_map_color = colors_table[0];
    }
    ~Viewer(){}

    void draw() override{

    }

    void render_canvas(double width) override{

    }

    void render_window(){

    }

};

class Window{
public:
    std::string title;
    int width;
    int height;
    bool isRunning;
    std::shared_ptr<Viewer> detail = nullptr;

    Window(std::string title, int width, int height, bool isRunning=false):
        title(title), width(width), height(height), isRunning(isRunning){
        }


    void start_thread(){
        detail = std::make_shared<Viewer>(title, this->width, this->height, this->isRunning);
        detail->run();
    }

    void run(){
        std::thread viewer_thread([this]() { this->start_thread(); });
        viewer_thread.detach();
    }
    
};



#endif