#include "benchmark_common.hpp"
#include <optional>
#include <vector>

int main(int argc, char** argv)
{
    size_t N = (argc > 1) ? std::stoi(argv[1]) : 10000;
    std::vector<std::optional<Component>> sparseArray;
    sparseArray.resize(N);

    long long t_insert = measure("Insertion", [&]() {
        for(size_t i = 0; i < N; ++i) {
            sparseArray[i] = Component();
        }
    });

    volatile float total_x = 0;
    long long t_iter = measure("Iteration", [&]() {
        for(size_t i = 0; i < N; ++i) {
            if (sparseArray[i].has_value()) {
                total_x += sparseArray[i]->x;
                sparseArray[i]->x += sparseArray[i]->vx;
            }
        }
    });

    std::cout << "SPARSE_ARRAY," << N << "," << t_insert << "," << t_iter << std::endl;
    return 0;
}
