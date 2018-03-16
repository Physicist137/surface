#pragma once
#include "growth.hpp"
#include "statdata.hpp"
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
	: SurfaceGrowth<Integer, FloatingPoint>(size),
	 _data(size), _nl(size),  _surface(size) {}
	
	explicit SurfaceGrowthEnsemble(const Surface<Integer>& surface)
	: SurfaceGrowth<Integer, FloatingPoint>(surface),
	 _data(surface.size()), _nl(surface.size()), _surface(surface) {}
	
	SurfaceGrowthEnsemble(unsigned sx, unsigned sy)
	: SurfaceGrowth<Integer, FloatingPoint>(sx, sy), 
	_data(sx*sy), _nl(sx*sy), _surface(sx, sy) {}


	// Growth Functions
	void deposition(unsigned deposition_per_iteration, const FloatingPoint& nltotal,
		std::function<void(Surface<Integer>& surface,int)> depositionMethod);

	// TODO: Multithreaded deposition
	
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

	int sz = SurfaceGrowth<Integer, FloatingPoint>::size();
	
	for (int s = 0; s < systems; ++s) {
		// Run the deposition
		SurfaceGrowth<Integer, FloatingPoint>::clear(_surface);
		SurfaceGrowth<Integer, FloatingPoint>::deposition(deposition_per_iteration, nltotal, depositionMethod);
		
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
	
	// Simulation growth
	int sz = _nl.size();
	for (int i = 0; i < sz; ++i) root["growth"]["nl"][i] = _nl[i];
	for (int i = 0; i < sz; ++i) root["growth"]["average"]["height"][i] = _data[i].average().height();
	for (int i = 0; i < sz; ++i) root["growth"]["variance"]["height"][i] = _data[i].variance().height();
	for (int i = 0; i < sz; ++i) root["growth"]["average"]["width"][i] = _data[i].average().width();
	for (int i = 0; i < sz; ++i) root["growth"]["variance"]["width"][i] = _data[i].variance().width();
	for (int i = 0; i < sz; ++i) root["growth"]["average"]["skewness"][i] = _data[i].average().skewness();
	for (int i = 0; i < sz; ++i) root["growth"]["variance"]["skewness"][i] = _data[i].variance().skewness();
	for (int i = 0; i < sz; ++i) root["growth"]["average"]["kurtosis"][i] = _data[i].average().kurtosis();
	for (int i = 0; i < sz; ++i) root["growth"]["variance"]["kurtosis"][i] = _data[i].variance().kurtosis();
	
	// Log Linear regression data
	root["log_regression"]["inclination"]["average"]["height"] = _log_inclination.average().height();
	root["log_regression"]["inclination"]["variance"]["height"] = _log_inclination.variance().height();
	root["log_regression"]["inclination"]["average"]["width"] = _log_inclination.average().width();
	root["log_regression"]["inclination"]["variance"]["width"] = _log_inclination.variance().width();
	root["log_regression"]["inclination"]["average"]["skewness"] = _log_inclination.average().skewness();
	root["log_regression"]["inclination"]["variance"]["skewness"] = _log_inclination.variance().skewness();
	root["log_regression"]["inclination"]["average"]["kurtosis"] = _log_inclination.average().kurtosis();
	root["log_regression"]["inclination"]["variance"]["kurtosis"] = _log_inclination.variance().kurtosis();

	root["log_regression"]["independent"]["average"]["height"] = _log_independent.average().height();
	root["log_regression"]["independent"]["variance"]["height"] = _log_independent.variance().height();
	root["log_regression"]["independent"]["average"]["width"] = _log_independent.average().width();
	root["log_regression"]["independent"]["variance"]["width"] = _log_independent.variance().width();
	root["log_regression"]["independent"]["average"]["skewness"] = _log_independent.average().skewness();
	root["log_regression"]["independent"]["variance"]["skewness"] = _log_independent.variance().skewness();
	root["log_regression"]["independent"]["average"]["kurtosis"] = _log_independent.average().kurtosis();
	root["log_regression"]["independent"]["variance"]["kurtosis"] = _log_independent.variance().kurtosis();	
	
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
