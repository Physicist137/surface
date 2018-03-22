#pragma once
#include "statdata.hpp"
#include <functional>
#include <cmath>

template<typename FloatingPoint>
class SurfaceData {
	std::array<FloatingPoint, 4> _moments;
	FloatingPoint _width;
	FloatingPoint _skewness;
	FloatingPoint _kurtosis;

protected:
	void compute();

public:
	// Constructur functions
	SurfaceData() : _moments(), _width(), _skewness(), _kurtosis() {}
	explicit SurfaceData(const std::array<FloatingPoint, 4>& data) : _moments(data) { compute(); }
	
	// Surface Data
	inline FloatingPoint moment(unsigned i) const {return _moments[i-1];}
	inline FloatingPoint height() const {return _moments[0];}
	inline FloatingPoint variance() const {return _moments[0] * _moments[0] - _moments[1];}
	inline FloatingPoint width() const {return _width;}
	inline FloatingPoint skewness() const {return _skewness;}
	inline FloatingPoint kurtosis() const {return _kurtosis;}
	
	// Quick Access function.
	FloatingPoint operator[](unsigned i) const;
	FloatingPoint& operator[](unsigned i);
	FloatingPoint operator[](const std::string& str) const;
	
	// Overloading operators
	SurfaceData& operator+= (const SurfaceData<FloatingPoint>& other);
	SurfaceData& operator-= (const SurfaceData<FloatingPoint>& other);
	SurfaceData& operator*= (const SurfaceData<FloatingPoint>& other);
	SurfaceData& operator/= (const SurfaceData<FloatingPoint>& other);
	
	// Run function
	SurfaceData runFunction(const std::function<FloatingPoint(FloatingPoint)>& f) const;
};

// Declaration of the operator functions.
template <typename FloatingPoint>
SurfaceData<FloatingPoint> operator-(const SurfaceData<FloatingPoint>& a);

template <typename FloatingPoint>
SurfaceData<FloatingPoint> operator+(const SurfaceData<FloatingPoint>& a, const SurfaceData<FloatingPoint>& b);

template <typename FloatingPoint>
SurfaceData<FloatingPoint> operator-(const SurfaceData<FloatingPoint>& a, const SurfaceData<FloatingPoint>& b);

template <typename FloatingPoint>
SurfaceData<FloatingPoint> operator*(const SurfaceData<FloatingPoint>& a, const SurfaceData<FloatingPoint>& b);

template <typename FloatingPoint>
SurfaceData<FloatingPoint> operator/(const SurfaceData<FloatingPoint>& a, const SurfaceData<FloatingPoint>& b);

template <typename FloatingPoint>
SurfaceData<FloatingPoint> operator*(const SurfaceData<FloatingPoint>& a, const FloatingPoint& b);

template <typename FloatingPoint>
SurfaceData<FloatingPoint> operator*(const FloatingPoint& b, const SurfaceData<FloatingPoint>& a);

template <typename FloatingPoint>
SurfaceData<FloatingPoint> operator/(const SurfaceData<FloatingPoint>& a, const FloatingPoint& b);


// Definition of member class functions
template <typename FloatingPoint>
void SurfaceData<FloatingPoint>::compute() {
	// FIXME: This is simply plain wrong!! PLAIN WRONG.
	// FIXME: Fix skewness and kurtosis calculations.

	FloatingPoint var = _moments[1] - _moments[0] * _moments[0];
	FloatingPoint dev = std::sqrt(var);
	FloatingPoint ske = _moments[2] / (dev * dev * dev);
	FloatingPoint kur = _moments[3] / (var * var);
	
	_width = dev;
	_skewness = ske;
	_kurtosis = kur;
}

template <typename FloatingPoint>
FloatingPoint SurfaceData<FloatingPoint>::operator[](unsigned i) const {
	switch (i) {
		case 0: return _moments[0];
		case 1: return _moments[1];
		case 2: return _moments[2];
		case 3: return _moments[3];
		case 4: return _width;
		case 5: return _skewness;
		case 6: return _kurtosis;
	}
	
	throw "Invalid number in the quick accessing function";
}

template <typename FloatingPoint>
FloatingPoint& SurfaceData<FloatingPoint>::operator[](unsigned i) {
	switch (i) {
		case 0: return _moments[0];
		case 1: return _moments[1];
		case 2: return _moments[2];
		case 3: return _moments[3];
		case 4: return _width;
		case 5: return _skewness;
		case 6: return _kurtosis;
	}
	
	throw "Invalid number in the quick accessing function";
}

template <typename FloatingPoint>
FloatingPoint SurfaceData<FloatingPoint>::operator[](const std::string& str) const {
	if (str == "height") return height();
	else if (str == "width") return width();
	else if (str == "skewness") return skewness();
	else if (str == "kurtosis") return kurtosis();
	else throw "Invalid argument at SurfaceData::operator[](const std::string&) const";
}


template <typename FloatingPoint>
SurfaceData<FloatingPoint>& SurfaceData<FloatingPoint>::operator+= (const SurfaceData<FloatingPoint>& other) {
	for (int i = 0; i < 6; ++i) this->operator[](i) += other[i];
	return *this;
}

template <typename FloatingPoint>
SurfaceData<FloatingPoint>& SurfaceData<FloatingPoint>::operator-= (const SurfaceData<FloatingPoint>& other) {
	for (int i = 0; i < 6; ++i) this->operator[](i) -= other[i];
	return *this;
}

template <typename FloatingPoint>
SurfaceData<FloatingPoint>& SurfaceData<FloatingPoint>::operator*= (const SurfaceData<FloatingPoint>& other) {
	for (int i = 0; i < 6; ++i) this->operator[](i) *= other[i];
	return *this;
}

template <typename FloatingPoint>
SurfaceData<FloatingPoint>& SurfaceData<FloatingPoint>::operator/= (const SurfaceData<FloatingPoint>& other) {
	for (int i = 0; i < 6; ++i) this->operator[](i) /= other[i];
	return *this;
}

template <typename FloatingPoint>
SurfaceData<FloatingPoint> SurfaceData<FloatingPoint>::runFunction(const std::function<FloatingPoint(FloatingPoint)>& f) const {
	SurfaceData<FloatingPoint> result;
	for (int i = 0; i < 6; ++i) result[i] = f(this->operator[](i));
	return result;
}


// Definition of operator functions
template <typename FloatingPoint>
SurfaceData<FloatingPoint> operator-(const SurfaceData<FloatingPoint>& a) {
	SurfaceData<FloatingPoint> result;
	for (int i = 0; i < 6; ++i) result[i] = -a[i];
	return result;
}

template <typename FloatingPoint>
SurfaceData<FloatingPoint> operator+(const SurfaceData<FloatingPoint>& a, const SurfaceData<FloatingPoint>& b) {
	SurfaceData<FloatingPoint> result;
	for (int i = 0; i < 6; ++i) result[i] = a[i] + b[i];
	return result;
}

template <typename FloatingPoint>
SurfaceData<FloatingPoint> operator-(const SurfaceData<FloatingPoint>& a, const SurfaceData<FloatingPoint>& b) {
	SurfaceData<FloatingPoint> result;
	for (int i = 0; i < 6; ++i) result[i] = a[i] - b[i];
	return result;
}

template <typename FloatingPoint>
SurfaceData<FloatingPoint> operator*(const SurfaceData<FloatingPoint>& a, const SurfaceData<FloatingPoint>& b) {
	SurfaceData<FloatingPoint> result;
	for (int i = 0; i < 6; ++i) result[i] = a[i] * b[i];
	return result;
}

template <typename FloatingPoint>
SurfaceData<FloatingPoint> operator/(const SurfaceData<FloatingPoint>& a, const SurfaceData<FloatingPoint>& b) {
	SurfaceData<FloatingPoint> result;
	for (int i = 0; i < 6; ++i) result[i] = a[i] / b[i];
	return result;
}

template <typename FloatingPoint>
SurfaceData<FloatingPoint> operator*(const SurfaceData<FloatingPoint>& a, const FloatingPoint& b) {
	SurfaceData<FloatingPoint> result;
	for (int i = 0; i < 6; ++i) result[i] = a[i] * b;
	return result;
}

template <typename FloatingPoint>
SurfaceData<FloatingPoint> operator*(const FloatingPoint& b, const SurfaceData<FloatingPoint>& a) {
	return a * b;
}

template <typename FloatingPoint>
SurfaceData<FloatingPoint> operator/(const SurfaceData<FloatingPoint>& a, const FloatingPoint& b) {
	SurfaceData<FloatingPoint> result;
	for (int i = 0; i < 6; ++i) result[i] = a[i] / b;
	return result;
}

