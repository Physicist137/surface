#pragma once
#include "growth.hpp"
#include "statdata.hpp"
#include <thread>
#include <mutex>
#include <json/json.h>
#include <json/writer.h>

template <typename Integer, typename FloatingPoint, unsigned systems>
class SurfaceGrowthEnsemble : private SurfaceGrowth<Integer, FloatingPoint> {
	// Surface growth dynamics.
	std::vector<StatisticalData<SurfaceData<FloatingPoint>, FloatingPoint>> _data;
	std::vector<FloatingPoint> _nl;

	// Coeficients of the linear regression over logarithm data points
	StatisticalData<SurfaceData<FloatingPoint>, FloatingPoint> _log_inclination;
	StatisticalData<SurfaceData<FloatingPoint>, FloatingPoint> _log_independent;

	// Initial Surface to begin deposition
	Surface<Integer> _surface;

public:
	// Constructor functions
	explicit SurfaceGrowthEnsemble(unsigned size)
	: SurfaceGrowth<Integer, FloatingPoint>(size), _surface(size) {}
	
	explicit SurfaceGrowthEnsemble(const Surface<Integer>& surface)
	: SurfaceGrowth<Integer, FloatingPoint>(surface), _surface(surface) {}
	
	SurfaceGrowthEnsemble(unsigned sx, unsigned sy)
	: SurfaceGrowth<Integer, FloatingPoint>(sx, sy), _surface(sx, sy) {}


	// Single threaded deposition
	void deposition(unsigned deposition_per_iteration, const FloatingPoint& nltotal,
		std::function<void(Surface<Integer>& surface,int)> depositionMethod);

	// Multithreaded deposition
	void multithreadDeposition(unsigned threads,
		unsigned deposition_per_iteration, const FloatingPoint& nltotal,
		std::function<void(Surface<Integer>& surface,int)> depositionMethod);
	
	// Accessing Functions
	inline const StatisticalData<SurfaceData<FloatingPoint>, FloatingPoint>& logInclination() const {
		return _log_inclination;
	}

	inline const StatisticalData<SurfaceData<FloatingPoint>, FloatingPoint>& logIndependent() const {
		return _log_independent;
	}

	// Save dynamics at file. FIXME: ERASE ME!
	void saveFile(const std::string& str) const;

	// Save dynamic at Json file.
	void saveJson(std::string& str) const;
};

template <typename Integer, typename FloatingPoint, unsigned systems>
void SurfaceGrowthEnsemble<Integer, FloatingPoint, systems>::deposition(
unsigned deposition_per_iteration, const FloatingPoint& nltotal,
std::function<void(Surface<Integer>& surface,int)> depositionMethod) {

	for (int s = 0; s < systems; ++s) {
		// Run the deposition
		SurfaceGrowth<Integer, FloatingPoint>::clear(_surface);
		SurfaceGrowth<Integer, FloatingPoint>::deposition(deposition_per_iteration, nltotal, depositionMethod);
	
		// Size of the dataset
		int sz = SurfaceGrowth<Integer, FloatingPoint>::_data.size();
		if (s == 0) _data.resize(sz);

		// Update the surface ensemble
		for (int i = 0; i < sz; ++i) _data[i].newData(SurfaceGrowth<Integer, FloatingPoint>::_data[i]);
	
		// Compute the loglog linear coeficients.
		auto coeff = SurfaceGrowth<Integer, FloatingPoint>::loglogfit();
		_log_inclination.newData(coeff[0]);
		_log_independent.newData(coeff[1]);
	}

	_nl = SurfaceGrowth<Integer, FloatingPoint>::_nl;
}

template <typename Integer, typename FloatingPoint, unsigned systems>
void SurfaceGrowthEnsemble<Integer, FloatingPoint, systems>::multithreadDeposition(
unsigned threads, unsigned deposition_per_iteration, const FloatingPoint& nltotal,
std::function<void(Surface<Integer>& surface,int)> depositionMethod) {

	// Mutex
	std::mutex mutex;

	// Lambda deposition function
	auto lambda_deposition = [&]() {
		
		// Initialize surface and do deposition
		SurfaceGrowth<Integer, FloatingPoint> growthSurface(_surface);
		growthSurface.deposition(deposition_per_iteration, nltotal, depositionMethod);

		// Lock the resources using the mutex
		std::lock_guard<std::mutex> guard(mutex);

		// Check the size of the data
		int sz = growthSurface.dataSize();
		if (_data.empty()) _data.resize(sz);
		
		// Update the surface ensemble.
		for (int i = 0; i < sz; ++i) _data[i].newData(growthSurface.dataValue(i));
		
		// Compute the loglog linear coeficients
		auto coeff = growthSurface.loglogfit();
		_log_inclination.newData(coeff[0]);
		_log_independent.newData(coeff[1]);
	};

	// Initialize the threads
	int done = 0;
	std::vector<std::thread> thread_vector;
	
	// Handle the threads
	while (done < systems) {
		int total = std::min(threads, systems-done);

		// Create the threads and execute the lambda
		for (int i = 0; i < total; ++i) {
			thread_vector.emplace_back(lambda_deposition);
		}

		// Wait for termination of the threads.
		for (std::thread& th : thread_vector) {
			th.join();
			++done;
		}

		// Clear the thread_vector
		thread_vector.clear();
	}

	// Calculate the nl values.
	// FIXME: Possible solution: To have an bool argument in the lambda, to set _nl from growthSurface.
	int size = _data.size();
	FloatingPoint szz = static_cast<FloatingPoint>(_surface.size());
	FloatingPoint deppi = static_cast<FloatingPoint>(deposition_per_iteration);
	FloatingPoint step = deppi / szz;

	FloatingPoint nlcurrent;
	if (SurfaceGrowth<Integer, FloatingPoint>::_nl.empty()) nlcurrent = FloatingPoint();
	else nlcurrent = SurfaceGrowth<Integer, FloatingPoint>::_nl.back();

	for (int i = 0; i < size; ++i) {
		_nl.push_back(nlcurrent);
		nlcurrent += step;
	}
}

template <typename Integer, typename FloatingPoint, unsigned systems>
void SurfaceGrowthEnsemble<Integer, FloatingPoint, systems>::saveJson(std::string& str) const {
	Json::Value root;
	// Deposition method
	root["deposition-type"] = "";

	// Initial surface data
	root["initial-surface"]["size"]["value"] = _surface.size();
	root["initial-surface"]["size"]["text"] = "";
	
	// Initial Surface
	int szi = _surface.size();
	for (int i = 0; i < szi; ++i) {
		root["initial-surface"]["surface"]["value"][i] = _surface[i];
	}
	
	// TODO: Final Surface
	
	// Register the nl parameter
	int sz = _nl.size();
	for (int i = 0; i < sz; ++i) root["growth"]["nl"][i] = _nl[i];
	
	std::array<std::string, 2> _stat_arg = {"average", "variance"};
	std::array<std::string, 4> _data_arg = {"height", "width", "skewness", "kurtosis"};
	for (const std::string& stat_arg : _stat_arg) {
		for (const std::string& data_arg : _data_arg) {
			// Simulation growth data
			for (int i = 0; i < sz; ++i) 
				root["growth"][stat_arg][data_arg][i] = _data[i][stat_arg][data_arg];

			// Log Linear Regression data
			root["log_regression"]["inclination"][stat_arg][data_arg] = _log_inclination[stat_arg][data_arg];
			root["log_regression"]["independent"][stat_arg][data_arg] = _log_inclination[stat_arg][data_arg];
		}
	}

	// Return
	Json::StreamWriterBuilder writer;
	str = Json::writeString(writer, root);
}

template <typename Integer, typename FloatingPoint, unsigned systems>
void SurfaceGrowthEnsemble<Integer, FloatingPoint, systems>::saveFile(const std::string& str) const {
	std::ofstream file(str);
	int size = _data.size();

	file << "nl = [" << _nl[0];
	for (int i = 0; i < size; ++i) file << ", " << _nl[i];
	file << "];" << std::endl << std::endl;

	file << "height_av = [" << _data[0].average().height();
	for (int i = 0; i < size; ++i) file << ", " << _data[i].average().height();
	file << "];" << std::endl << std::endl;
	
	file << "height_var = [" << _data[0].variance().height();
	for (int i = 0; i < size; ++i) file << ", " << _data[i].variance().height();
	file << "];" << std::endl << std::endl;
	
	file << "width_av = [" << _data[0].average().width();
	for (int i = 0; i < size; ++i) file << ", " << _data[i].average().width();
	file << "];" << std::endl << std::endl;
	
	file << "width_var = [" << _data[0].variance().width();
	for (int i = 0; i < size; ++i) file << ", " << _data[i].variance().width();
	file << "];" << std::endl << std::endl;
	
	file << "log_inclination_av_height = " << _log_inclination.average().height() << std::endl;
	file << "log_inclination_var_height = " << _log_inclination.variance().height() << std::endl;
	file << "log_inclination_av_width = " << _log_inclination.average().width() << std::endl;
	file << "log_inclination_var_width = " << _log_inclination.variance().width() << std::endl;
	
	file << "log_independent_av_height = " << _log_independent.average().height() << std::endl;
	file << "log_independent_var_height = " << _log_independent.variance().height() << std::endl;
	file << "log_independent_av_width = " << _log_independent.average().width() << std::endl;
	file << "log_independent_var_width = " << _log_independent.variance().width() << std::endl;
}
