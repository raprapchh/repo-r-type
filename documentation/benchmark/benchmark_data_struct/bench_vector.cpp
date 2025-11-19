#include "benchmark_common.hpp"
#include <vector>

int main(int argc, char** argv)
{
    size_t N = (argc > 1) ? std::stoi(argv[1]) : 10000;
    std::vector<Component> vecComp;
    vecComp.reserve(N);

    long long t_insert = measure("Insertion", [&]() {
        for(size_t i = 0; i < N; ++i) {
            vecComp.push_back(Component());
        }
    });

    volatile float total_x = 0;
    long long t_iter = measure("Iteration", [&]() {
        for(auto& comp : vecComp) {
            total_x += comp.x;
            comp.x += comp.vx;
        }
    });

    std::cout << "STD_VECTOR," << N << "," << t_insert << "," << t_iter << std::endl;
    return 0;
}