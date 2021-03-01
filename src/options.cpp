#include "options/options.h"

Options::Options(int argc, char **argv) : unhandledOptions(argv + 1, argv + argc) {}

bool Options::checkAndHandleOption(std::initializer_list<std::string> searchedOptions) {

    for (const auto& option: searchedOptions) {
        auto el = find(unhandledOptions.begin(), unhandledOptions.end(), option);
        if (el != unhandledOptions.end()) {
            unhandledOptions.erase(el);
            return true;
        }
    }

    return false;
}

std::vector<std::string> Options::getOtherOptions() {
    return unhandledOptions;
}

