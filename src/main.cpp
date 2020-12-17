// TEMPORARY!
// winctl will be a lib used by other apps, this is just for testing

#include <winctl/Controller.hpp>

#include <iostream>
#include <thread>
#include <sstream>

static std::string toString(BYTE* buttons, int count) {
    std::stringstream str;
    for (int i = 0; i < count; i++) {
        str << (int)buttons[i] << ", ";
    }
    return str.str();
}

int main(int argc, char** argv) {
    auto controllerIds = winctl::enumerateControllers();

    std::vector<winctl::Controller> controllers;
    for (auto& id : controllerIds) {
        controllers.emplace_back(id);
    }

    controllers.erase(controllers.begin() + 1, controllers.end());

    while (true) {
        int i = 0;
        for (auto& c : controllers) {
            try {
                auto state = c.getState();
                std::cout << "X = " << state.lX << ", Y = " << state.lY << ", Z =" << state.lZ << std::endl;
                std::cout << "Buttons: " << toString(state.rgbButtons, sizeof(state.rgbButtons) / sizeof(state.rgbButtons[0])) << std::endl;
            } catch (const std::exception& ex) {
                std::cout << "Error fetching state: " << ex.what() << std::endl;
            }
        }
        std::cout << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        //std::this_thread::yield();
    }

    return 0;
}
