#pragma once
#include "surface.hpp"
#include <random>

// Declaration of fucntions. -----------------------------------------
template <typename Integer>
void randomDeposition(Surface<Integer>& surface, int depositions);


// Definition of functions -----------------------------------------
template <typename Integer>
void randomDeposition(Surface<Integer>& surface, int depositions) {
	std::random_device rd;
	std::mt19937 gen(rd());
    std::uniform_int_distribution<Integer> dist(Integer(), static_cast<Integer>(surface.size()-1));

    for (int i = 0; i < depositions; ++i) {
        ++surface[dist(gen)];
    }
}
