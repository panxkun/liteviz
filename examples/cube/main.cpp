#include <filesystem>
#include <liteviz/core/detail.h>

using namespace liteviz;

class CubeConfig: public BaseConfig {
public:
    CubeConfig() = default;
    ~CubeConfig() = default;
};

class CubeRenderer : public BaseRenderer {
public:
    CubeRenderer() {

        _shader = std::make_shared<Shader>(
            (std::string(RESOURCE_DIR) + "/../liteviz/shaders/draw_point.vert").c_str(), 
            (std::string(RESOURCE_DIR) + "/../liteviz/shaders/draw_point.frag").c_str(),
            true
        );

        _cube = std::make_shared<Cube>();
        _cube->setup(1.0f);
    }

    ~CubeRenderer() override {

    }

    void render(const Viewport& viewport) override {
 
        if (!_shader) return;

        _cube->draw(_shader.get(), viewport);
    }

private:
    std::shared_ptr<Shader> _shader;
    std::shared_ptr<Cube> _cube;
};

class LiteViz: public ViewerDetail {

    std::thread renderThread;
    std::shared_ptr<CubeConfig> cubeConfig;

public:
    LiteViz(std::string title, int width, int height):

        ViewerDetail(title, width, height) {

        cubeConfig = std::make_shared<CubeConfig>();

        std::cout << "Cube Viewer initialized." << std::endl;
    }

    ~LiteViz() {
        if (renderThread.joinable()) {
            renderThread.join();
        }
    }

    bool initResources() override {

        std::shared_ptr<CubeRenderer> renderer = std::make_shared<CubeRenderer>();
        _registeredRenderers.push_back(renderer);

        return true;
    }

    void start() {
        // start draw() in a background thread
        renderThread = std::thread([this]() {
            this->draw();
        });
    }
};


int main(int argc, char** argv) {

    std::shared_ptr<LiteViz> viewer = std::make_shared<LiteViz>("LiteViz Viewer", 640, 480);
    viewer->start();

    return 0;
}
