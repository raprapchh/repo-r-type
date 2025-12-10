#ifndef BENCHMARK_COMMON_HPP
#define BENCHMARK_COMMON_HPP

#include <iostream>
#include <chrono>
#include <string>
#include <vector>

struct Component {
    float x = 0.0f, y = 0.0f;
    float vx = 1.0f, vy = 1.0f;
    int sprite_id = 0;
    char padding[32];
};

template <typename Func> long long measure(const std::string& name, Func f) {
    auto start = std::chrono::high_resolution_clock::now();
    f();
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    return duration;
}

#endif