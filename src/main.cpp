#include <winctl/Controller.hpp>

#include <nlohmann/json.hpp>

#include <chrono>
#include <iostream>
#include <thread>
#include <set>
#include <sstream>

static std::string toString(BYTE* buttons, int count) {
    std::stringstream str;
    for (int i = 0; i < count; i++) {
        str << (int)buttons[i] << ", ";
    }
    return str.str();
}

struct Input {
    std::string name;
    int value;

    bool operator<(const Input& other) const {
        if (name < other.name) {
            return true;
        } else if (name == other.name) {
            return value < other.value;
        } else {
            return false;
        }
    }

    /*bool operator==(const Input& other) const {
        return name == other.name && value == other.value;
    }*/

    operator nlohmann::json() const {
        return {
            { "name", name },
            { "value", value }
        };
    }
};
struct InputEvent {
    std::set<Input> inputs;
    std::chrono::milliseconds time;

    operator nlohmann::json() const {
        return {
            { "inputs", inputs },
            { "timeMS", time.count() }
        };
    }
};

std::set<Input> getInputs(const DIJOYSTATE& state) {
    std::set<Input> rv;

    rv.insert({"AxisX", state.lX});
    rv.insert({"AxisY", state.lY});
    rv.insert({"AxisZ", state.lZ});

    rv.insert({"AxisRX", state.lRx});
    rv.insert({"AxisRY", state.lRy});
    rv.insert({"AxisRZ", state.lRz});

    for (auto i = 0; i < sizeof(state.rgbButtons) / sizeof(state.rgbButtons[0]); i++) {
        std::stringstream ss; 
        ss << "Btn" << i;
        rv.insert({ss.str(), state.rgbButtons[i]});
    }

    return rv;
}

int main(int argc, char** argv) {
    auto controllerIds = winctl::enumerateControllers();

    std::vector<winctl::Controller> controllers;
    for (auto& id : controllerIds) {
        controllers.emplace_back(id);
    }

    controllers.erase(controllers.begin() + 1, controllers.end());

    std::set<Input> lastInputs;

    using clock_t = std::chrono::steady_clock;
    auto startTime = clock_t::now();

    while (true) {
        int i = 0;
        for (auto& c : controllers) {
            try {
                auto state = c.getState();
                auto time = clock_t::now() - startTime;

                auto inputs = getInputs(state);

                std::set<Input> difference;

                std::set_difference(inputs.begin(), inputs.end(), lastInputs.begin(), lastInputs.end(), std::inserter(difference, difference.end()));

                if (!difference.empty()) {
                    InputEvent event {
                        .inputs = std::move(difference),
                        .time = std::chrono::duration_cast<std::chrono::milliseconds>(time)
                    };

                    std::cout << event.operator nlohmann::json().dump(4) << std::endl << std::endl;

                    lastInputs = std::move(inputs);
                }
                
                //std::cout << "X = " << state.lX << ", Y = " << state.lY << ", Z =" << state.lZ << std::endl;
                //std::cout << "Buttons: " << toString(state.rgbButtons, sizeof(state.rgbButtons) / sizeof(state.rgbButtons[0])) << std::endl;
            } catch (const std::exception& ex) {
                std::cout << "Error fetching state: " << ex.what() << std::endl << std::endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        //std::this_thread::yield();
    }

    return 0;
}
