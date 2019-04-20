#pragma once
#include <vector>
#include <random>

inline std::mt19937& get_random_engine() {
    static std::mt19937 mt;
    return mt;
}

template<typename T>
T& random_choice(std::vector<T>& from) {
    auto dist = std::uniform_int_distribution<size_t>(0, from.size() - 1);
    return from[dist(get_random_engine())];
}

template<typename T>
const T& random_choice(const std::vector<T>& from) {
    auto dist = std::uniform_int_distribution<size_t>(0, from.size() - 1);
    return from[dist(get_random_engine())];
}

inline int random_int(int max_ex) {
    auto dist = std::uniform_int_distribution<size_t>(0, max_ex - 1);
    return dist(get_random_engine());
}

inline double random_float(double min, double max) {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(get_random_engine());
}
