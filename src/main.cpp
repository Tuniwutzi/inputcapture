#include <winctl/Controller.hpp>

#include <nlohmann/json.hpp>

#include <future>
#include <filesystem>
#include <fstream>
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
    size_t frame;

    operator nlohmann::json() const {
        return {
            { "inputs", inputs },
            { "frame", frame }
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

std::string getConsoleInput() {
    std::string line;
    std::getline(std::cin, line);
    return line;
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

    auto stdinFuture = std::async(getConsoleInput);
    bool quit = false;

    std::list<InputEvent> events;
    size_t frame = 0;

    while (!quit) {
        const auto now = clock_t::now();
        frame++;

        for (auto& c : controllers) {
            try {
                auto state = c.getState();

                auto inputs = getInputs(state);

                std::set<Input> difference;

                std::set_difference(inputs.begin(), inputs.end(), lastInputs.begin(), lastInputs.end(), std::inserter(difference, difference.end()));

                if (!difference.empty()) {
                    InputEvent event {
                        .inputs = std::move(difference),
                        .frame = frame
                    };

                    events.push_back(std::move(event));

                    lastInputs = std::move(inputs);
                }
            } catch (const std::exception& ex) {
                std::cout << "Error fetching state: " << ex.what() << std::endl << std::endl;
            }
        }
        auto continueAt = now + std::chrono::milliseconds(10);
        while (stdinFuture.wait_until(continueAt) != std::future_status::timeout) {
            auto line = stdinFuture.get();
            if (line == "q") {
                quit = true;
                break;
            }
            stdinFuture = std::async(getConsoleInput);
        }
    }


    std::stringstream filename;
    filename << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) << "_inputevents.json";
    std::filesystem::path filepath(filename.str());
    std::ofstream file(filepath, std::ios_base::trunc);
    file << ((nlohmann::json)events).dump(4) << std::endl;
    file.flush();

    return 0;
}
