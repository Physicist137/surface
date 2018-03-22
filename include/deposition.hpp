#pragma once
#include "surface.hpp"
#include <random>

// Declaration of fucntions. -----------------------------------------
template <typename Integer>
void randomDeposition(Surface<Integer>& surface, int depositions);

template <typename Integer>
void ballisticDeposition2D(Surface<Integer>& surface, int depositions);

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

template <typename Integer>
void ballisticDeposition2D(Surface<Integer>& surface, int depositions) {
	std::random_device rd;
	std::mt19937 gen(rd());
	
	Integer size = surface.size();
	std::uniform_int_distribution<Integer> dist(Integer(), static_cast<Integer>(size-1));

	for (int i = 0; i < depositions; ++i) {
		Integer site = dist(gen);

		if (site == size -1) surface[site] = std::max<Integer>(surface[site-1], 1+surface[site]);
		else if (site == 0) surface[site] = std::max<Integer>(surface[site+1], 1+surface[site]);
		else surface[site] = std::max<Integer>(
			std::max<Integer>(surface[site-1], 1+surface[site]), 
			surface[site+1]);
	}
}
