#include <iostream>
#include <fstream>
#include <chrono>
#include <yaml-cpp/yaml.h>

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    YAML::Node data = YAML::LoadFile("config_test.yml");

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "YAML parse time: " << ms << " ms\n";
    return 0;
}
