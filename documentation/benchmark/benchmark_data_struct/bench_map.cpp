#include "benchmark_common.hpp"
#include <map>

int main(int argc, char** argv)
{
    size_t N = (argc > 1) ? std::stoi(argv[1]) : 10000;
    std::map<size_t, Component> mapComp;

    long long t_insert = measure("Insertion", [&]() {
        for(size_t i = 0; i < N; ++i) {
            mapComp[i] = Component();
        }
    });

    volatile float total_x = 0;
    long long t_iter = measure("Iteration", [&]() {
        for(auto& [id, comp] : mapComp) {
            total_x += comp.x;
            comp.x += comp.vx;
        }
    });

    std::cout << "STD_MAP," << N << "," << t_insert << "," << t_iter << std::endl;
    return 0;
}