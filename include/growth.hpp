#pragma once
#include "surface.hpp"
#include <functional>
#include <fstream>
#include <string>
#include <cmath>


template <typename Integer, typename FloatingPoint>
class SurfaceGrowth : public Surface<Integer> {
protected:
	std::vector<SurfaceData<FloatingPoint>> _data;
	std::vector<FloatingPoint> _nl;

public:
	// Constructor Functions
	explicit SurfaceGrowth(unsigned size) : Surface<Integer>(size) {}
	explicit SurfaceGrowth(const Surface<Integer>& surface) : Surface<Integer>(surface) {}
	SurfaceGrowth(unsigned sx, unsigned sy) : Surface<Integer>(sx, sy) {}
	
	// Inline Functions
	inline unsigned nlSize() const {return _nl.size();}
	inline unsigned dataSize() const {return _data.size();}
	inline auto& dataValue(unsigned i) const {return _data[i];}

	// Growth Functions
	void deposition(unsigned deposition_per_iteration, const FloatingPoint& nltotal, 
		std::function<void(Surface<Integer>& surface,int)> depositionMethod);

	// Modifying the surface
	void clear();
	void clear(const Surface<Integer>& surface);
	
	// Data Analysis Function.
	std::array<SurfaceData<FloatingPoint>, 2> loglogfit() const;
	std::array<SurfaceData<FloatingPoint>, 2> loglogfit(int from, int to) const;
	std::array<SurfaceData<FloatingPoint>, 2> loglogfit(const FloatingPoint& nlfrom, const FloatingPoint& nlto) const;
	
	// Save at file
	void saveFile(const std::string& str) const;
};


template <typename Integer, typename FloatingPoint>
void SurfaceGrowth<Integer, FloatingPoint>::deposition(
unsigned deposition_per_iteration, const FloatingPoint& nltotal, 
std::function<void(Surface<Integer>& surface,int)> depositionMethod) {
	
	// Find the current nl value to begin with.
	FloatingPoint nlcurrent;
	if (_nl.empty()) nlcurrent = FloatingPoint();
	else nlcurrent = _nl.back();
	
	// Peform the Surface Growth.
	while (nlcurrent < nltotal) {
		// Peform the deposition
		depositionMethod(*this, deposition_per_iteration);
		
		// Save the data
		_nl.push_back(nlcurrent);
		_data.push_back(this->template surfaceData<FloatingPoint>());
		// https://stackoverflow.com/questions/3505713/c-template-compilation-error-expected-primary-expression-before-token
		
		// Upgrade time passage
		FloatingPoint deppi = static_cast<FloatingPoint>(deposition_per_iteration);
		FloatingPoint szz = static_cast<FloatingPoint>(this->size());
		nlcurrent += deppi / szz;
	}
}

template <typename Integer, typename FloatingPoint>
void SurfaceGrowth<Integer, FloatingPoint>::clear() {
	Surface<Integer>::clear();
	_nl.clear();
	_data.clear();
}

template <typename Integer, typename FloatingPoint>
void SurfaceGrowth<Integer, FloatingPoint>::clear(const Surface<Integer>& surface) {
	Surface<Integer>::clear(surface);
	_nl.clear();
	_data.clear();
}

// Including from. Excluding to.
template <typename Integer, typename FloatingPoint>
std::array<SurfaceData<FloatingPoint>, 2> SurfaceGrowth<Integer, FloatingPoint>::loglogfit(int from, int to) const {
	int size = to - from + 1;
	FloatingPoint avnl = FloatingPoint();
	SurfaceData<FloatingPoint> avda = SurfaceData<FloatingPoint>();
	FloatingPoint fsize = static_cast<FloatingPoint>(size);

	// Logarithm disambiguation
	typedef FloatingPoint(*type)(FloatingPoint);
	std::function<FloatingPoint(FloatingPoint)> log = static_cast<type>(std::log);
	
	// Compute the averages
	for (int i = from; i < to; ++i) {
		avnl += std::log(_nl[i]) / fsize;
		avda += _data[i].runFunction(log) / fsize;
	}

	// Compute the inclination parameter
	SurfaceData<FloatingPoint> num = SurfaceData<FloatingPoint>();
	FloatingPoint den = FloatingPoint();
	for (int i = from; i < to; ++i) {
		num += std::log(_nl[i]) * (_data[i].runFunction(log) - avda);
		den += std::log(_nl[i]) * (std::log(_nl[i]) - avnl);
	}

	// The coeficients
	SurfaceData<FloatingPoint> a = num / den;
	SurfaceData<FloatingPoint> err = a * avnl;
	SurfaceData<FloatingPoint> b = avda - a * avnl;
	
	// Return result.
	return std::array<SurfaceData<FloatingPoint>, 2>({a, b});
}

template <typename Integer, typename FloatingPoint>
std::array<SurfaceData<FloatingPoint>, 2> SurfaceGrowth<Integer, FloatingPoint>::loglogfit() const {
	if (std::abs(_nl[0]) < 0.01) return loglogfit(1, _nl.size());
	else return loglogfit(0, _nl.size());
}


template <typename Integer, typename FloatingPoint>
std::array<SurfaceData<FloatingPoint>, 2> SurfaceGrowth<Integer, FloatingPoint>::loglogfit(
const FloatingPoint& nlfrom, const FloatingPoint& nlto) const {
	// You can do better! Have a O(log n) search please! FIXME.
	int size = _nl.size();
	int from, to;
	for (int i = 0; i < size; ++i) {
		if (_nl[i] < nlfrom) from = i;
		if (_nl[i] > nlto) to = i;
	}

	return loglogfit(from, to);
}

template <typename Integer, typename FloatingPoint>
void SurfaceGrowth<Integer, FloatingPoint>::saveFile(const std::string& str) const {
	std::ofstream file(str);
	int size = _data.size();

	file << "nl = [" << _nl[0];
	for (int i = 1; i < size; ++i) file << ", " << _nl[i];
	file << "];" << std::endl << std::endl;
	
	file << "height = [" << _data[0].height();
	for (int i = 1; i < size; ++i) file << ", " << _data[i].height();
	file << "];" << std::endl << std::endl;
	
	file << "width = [" << _data[0].width();
	for (int i = 1; i < size; ++i) file << ", " << _data[i].width();
	file << "];" << std::endl << std::endl;
	
	file.close();
}

