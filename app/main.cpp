#include <iostream>
#include <window.h>

int main(int argc, char* argv[]) {

    Window window("LiteViz", 1280, 720, true);
    window.start_thread();

    return 0;
}