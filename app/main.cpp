#include <filesystem>
#include <liteviz/viewer.h>


class LiteViz: public ViewerDetail {

public:
    LiteViz(std::string title, int width, int height):
        ViewerDetail(title, width, height) {
        std::cout << "LiteViz SLAM Viewer initialized." << std::endl;
    }

    void drawData() override {
        grid->draw(gridShader, viewport);
    }
};


int main(int argc, char** argv) {

    std::shared_ptr<LiteViz> viewer = std::make_shared<LiteViz>("LiteViz Viewer", 1280, 720);
    viewer->run();

    return 0;
}
