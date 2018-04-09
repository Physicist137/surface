#pragma once
#include <vector>
#include "surfacedata.hpp"

// TODO: Later: To create a SurfaceHD Class.

template <typename Integer>
class Surface {
	std::vector<Integer> _grid;
	
	// Sizes. 2D and 3D Mode.
	unsigned _size;
	unsigned _sx, _sy;
	
public:
	// Constructor functions
	explicit Surface(unsigned size) : _size(size), _grid(size) {}
	
	explicit Surface(const Surface& surface)
	: _grid(surface._grid), _size(surface._size), _sx(surface._sx), _sy(surface._sy) {}

	Surface(unsigned sx, unsigned sy)
	: _sx(sx), _sy(sy)  {_size = _sx * _sy; _grid.resize(_size);}
	
	
	// Accessor functions
	inline const std::vector<Integer>& grid() const {return _grid;}
	inline unsigned size() const {return _size;}
	inline unsigned sizex() const {return _sx;}
	inline unsigned sizey() const {return _sy;}
	
	
	// Accessing the surface
	inline const Integer& operator[](unsigned num) const {return _grid[num];}
	inline Integer& operator[](unsigned num) {return _grid[num];}
	
	inline const Integer& operator()(unsigned x, unsigned y) const {return _grid[_sx * y + x];}
	inline Integer& operator()(unsigned x, unsigned y) {return _grid[_sx * y + x];}
	

	// Modifying the surface
	inline void clear() { _grid.clear(); }
	inline void clear(const Surface<Integer>& surface) {_grid = surface._grid;}

	// Surface calculation data: nth moment
	template <typename FloatingPoint>
	FloatingPoint nthMomentHeight(unsigned order) const;

	// Surface Calculation data: nth central moment
	template <typename FloatingPoint>
	FloatingPoint nthCentralMomentHeight(unsigned order) const;
	
	template <typename FloatingPoint>
	SurfaceData<FloatingPoint> surfaceData() const;
	
	// TODO: To plot the surface somehow.
};


template <typename Integer>
template <typename FloatingPoint>
FloatingPoint Surface<Integer>::nthMomentHeight(unsigned order) const {
	FloatingPoint result = FloatingPoint();

	for (unsigned i = 0; i < _size; ++i) {
		FloatingPoint size = static_cast<FloatingPoint>(_size);
		FloatingPoint height = static_cast<FloatingPoint>(_grid[i]);
		FloatingPoint power = 1;
		
		for (unsigned n = 0; n < order; ++n) {
			power *= height;
		}
		
		result += power / size;
	}
	
	return result;
}


template <typename Integer>
template <typename FloatingPoint>
FloatingPoint Surface<Integer>::nthCentralMomentHeight(unsigned order) const {
	// Trivial Central Moments.
	if (order == 0) return 1;
	if (order == 1) return 0;

	// Calculate the first moment (average around zero)
	FloatingPoint av = nthMomentHeight<FloatingPoint>(1);

	// Compute the nthCentralMoment as requested
	FloatingPoint result = FloatingPoint();
	for (unsigned i = 0; i < _size; ++i) {
		FloatingPoint size = static_cast<FloatingPoint>(_size);
		FloatingPoint height = static_cast<FloatingPoint>(_grid[i]);
		FloatingPoint power = 1;
		
		for (unsigned n = 0; n < order; ++n) {
			power *= (height - av);
		}
		
		result += power / size;
	}

	return result;
}


template <typename Integer>
template <typename FloatingPoint>
SurfaceData<FloatingPoint> Surface<Integer>::surfaceData() const {
	// Define the moments.
	std::array<FloatingPoint, 4> moment = {0, 0, 0, 0};
	std::array<FloatingPoint, 4> central = {0, 0, 0, 0};

	// Calculate the moments around zero.
	for (unsigned i = 0; i < _size; ++i) {
		FloatingPoint size = static_cast<FloatingPoint>(_size);
		FloatingPoint height = static_cast<FloatingPoint>(_grid[i]);
		FloatingPoint power[4];
		
		power[0] = height;
		for (unsigned n = 1; n < 4; ++n) power[n] = power[0] * power[n-1];
		for (unsigned n = 0; n < 4; ++n) moment[n] += power[n] / size; 
	}

	// Calculate the central moments
	FloatingPoint av = moment[0];
	for (unsigned i = 0; i < _size; ++i) {
		FloatingPoint size = static_cast<FloatingPoint>(_size);
		FloatingPoint height = static_cast<FloatingPoint>(_grid[i]);
		FloatingPoint power[4];
		
		power[0] = height - av;
		for (unsigned n = 1; n < 4; ++n) power[n] = power[0] * power[n-1];
		for (unsigned n = 0; n < 4; ++n) central[n] += power[n] / size; 
	}

	// Return the data
	return SurfaceData<FloatingPoint>(moment, central);
}
