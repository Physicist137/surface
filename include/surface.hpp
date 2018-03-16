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

	// Surface calculation data
	template <typename FloatingPoint>
	FloatingPoint nthMomentHeight(unsigned order) const;
	
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
SurfaceData<FloatingPoint> Surface<Integer>::surfaceData() const {
	
	// FIXME: DO something faster!
	return SurfaceData<FloatingPoint>({
		nthMomentHeight<FloatingPoint>(1),
		nthMomentHeight<FloatingPoint>(2),
		nthMomentHeight<FloatingPoint>(3),
		nthMomentHeight<FloatingPoint>(4)
	});
}
