#pragma once
#include <array>

template <typename DataStructure, typename FloatingPoint, unsigned order=2>
class StatisticalData {
	std::array<DataStructure, order> _moments;
	unsigned _size;
	
public:
	// Constructor
	StatisticalData() : _moments(), _size(0) {}
	
	StatisticalData(const StatisticalData& data)
	: _moments(data._moments), _size(data._size) {}
	
	template <typename Type>
	StatisticalData(const std::vector<Type>& data);
	
	// Accessor Functions
	inline unsigned size() const {return _size;}
	inline FloatingPoint moment(unsigned i) const {return _moments[i-1];}

	// Modification functions
	void newData(const DataStructure& data);
	void newData(const StatisticalData& data);
	void clear();
	
	// Direct from data source
	template <typename Type>
	void overrideData(const std::vector<Type>& array);
	
	template <typename Type>
	void newData(const std::vector<Type>& array);
	
	// Accessing more data
	DataStructure average() const;
	DataStructure variance() const;
};

template <typename DataStructure, typename FloatingPoint, unsigned order>
template <typename Type>
StatisticalData<DataStructure, FloatingPoint, order>::StatisticalData(const std::vector<Type>& data) {
	this->overrideData(data);
}

template <typename DataStructure, typename FloatingPoint, unsigned order>
void StatisticalData<DataStructure, FloatingPoint, order>::newData(const DataStructure& data) {
	FloatingPoint size = static_cast<FloatingPoint>(_size);
	FloatingPoint newsize = static_cast<FloatingPoint>(_size+1);
	
	DataStructure dd = data;
	_moments[0] = size / newsize  * _moments[0] + dd / newsize;
	for (unsigned i = 1; i < order; ++i) {
		dd *= data;
		_moments[i] = size / newsize  * _moments[i] + dd / newsize;
	}
	
	++_size;
}

template <typename DataStructure, typename FloatingPoint, unsigned order>
void StatisticalData<DataStructure, FloatingPoint, order>::newData(const StatisticalData& data) {
	FloatingPoint size1 = static_cast<FloatingPoint>(_size);
	FloatingPoint size2 = static_cast<FloatingPoint>(data._size);
	FloatingPoint newsize = static_cast<FloatingPoint>(_size+data._size);
	
	for (unsigned i = 0; i < order; ++i) {
		_moments[i] = size1 / newsize * _moments[i] + size2 / newsize * data._moments[i];
	}
	
	_size += data._size;
}

template <typename DataStructure, typename FloatingPoint, unsigned order>
template <typename Type>
void StatisticalData<DataStructure, FloatingPoint, order>::overrideData(const std::vector<Type>& array) {
	FloatingPoint size = static_cast<FloatingPoint>(array.size());
	for (const Type& data : array) {
		DataStructure converted_data = static_cast<DataStructure>(data);
		
		DataStructure dd = converted_data;
		_moments[0] += dd / size;
		for (int i = 1; i < order; ++i) {
			dd *= converted_data;
			_moments[i] += dd / size;
		}
	}
	
	_size = array.size();
}

template <typename DataStructure, typename FloatingPoint, unsigned order>
template <typename Type>
void StatisticalData<DataStructure, FloatingPoint, order>::newData(const std::vector<Type>& array) {
	this->newData(StatisticalData<DataStructure, FloatingPoint, order>(array));
}


template <typename DataStructure, typename FloatingPoint, unsigned order>
void StatisticalData<DataStructure, FloatingPoint, order>::clear() {
	_size = 0;
	for (unsigned i = 0; i < order; ++i) {
		_moments[i] = DataStructure();
	}
}

template <typename DataStructure, typename FloatingPoint, unsigned order>
DataStructure StatisticalData<DataStructure, FloatingPoint, order>::average() const {
	return _moments[0];
}

template <typename DataStructure, typename FloatingPoint, unsigned order>
DataStructure StatisticalData<DataStructure, FloatingPoint, order>::variance() const {
	return _moments[1] - _moments[0] * _moments[0];
}
