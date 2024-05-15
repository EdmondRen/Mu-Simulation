#include<cstdlib>
#include<vector>
#include <fstream>
#include <string>
#include <iostream>


#ifndef UNITS_HH
#define UNITS_HH

// A python-like print function
template<typename T, typename ...TAIL>
void print(const T &t, TAIL... tail);

namespace units{
	const double mm = 10.0;
	const double cm = 1.00;
	const double ns = 1.00;
	const double MeV = 1.00;
};

namespace detector{ 
	using namespace units;

	const std::vector<double> COSMIC_SHIFT = {0.0, 547*cm, 11950.0*cm}; // shift of sim cosmic -> main coordinates
	const double time_resolution = 1.0*units::ns;

};

namespace constants{

	const double c = 29.97*units::cm/units::ns;
	const double optic_fiber_n = 1.580*units::cm; //2023.5.3 Tom: change from 1.5->1.58 according to sim. estimate for the optical fiber index of refraction

};

namespace cuts{

	//digi cuts and constants
	const double digi_spacing = 20.0*units::ns;
	const double SiPM_energy_threshold = 0.65*units::MeV;

	//run options
        const int start_ev = 0;
        const int end_ev = 50;

};





#endif
