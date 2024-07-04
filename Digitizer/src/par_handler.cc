#include <sstream>
#include <iostream>
#include <fstream>
#include <map>
#include "par_handler.hh"



void ParHandler::Handle() {
	GetPars();
}


void ParHandler::GetPars() {

	std::string par_path = std::string(__FILE__);

	std::string par_card_str = par_path.substr(0, par_path.size()-14)+"/../run/par_card.txt";

	std::ifstream infile(par_card_str);

	if (infile.is_open()) {
		file_opened = true;

		std::string line;
		while (std::getline(infile, line)) {	
			if (line[0]=='#') continue;

			std::istringstream iss(line);
			std::string par;
			double value;

			if (!(iss >> par >> value)) { continue; } // error

			par_map[par] = value;
		}
	} else {
		file_opened = false;
		std::cout << "Error opening parameter card file" << std::endl;
	}
}

