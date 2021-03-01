#ifndef ADDER_OPTIONS_H
#define ADDER_OPTIONS_H

#include <vector>
#include <initializer_list>
#include <string>
#include <algorithm>

class Options {
private:

    std::vector<std::string> unhandledOptions;

public:

    Options(int, char **);

    bool checkAndHandleOption(std::initializer_list<std::string>);

    std::vector<std::string> getOtherOptions();

};

#endif //ADDER_OPTIONS_H
