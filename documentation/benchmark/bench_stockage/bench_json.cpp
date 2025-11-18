#include <iostream>
#include <fstream>
#include <chrono>
#include <nlohmann/json.hpp>

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    std::ifstream file("config_test.json");
    nlohmann::json j;
    file >> j;

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "JSON parse time: " << ms << " ms\n";
    return 0;
}
