#include <ensemble.hpp>
#include <deposition.hpp>
#include <string>
#include <fstream>

int main() {
	SurfaceGrowthEnsemble<int, float, 10> surface(10);
	surface.deposition(10, 10, &randomDeposition<int>);
	
	std::string json;
	surface.saveJson(json);

	std::ofstream file("simulation.json");
	file << json << std::flush;
	file.close();
}
