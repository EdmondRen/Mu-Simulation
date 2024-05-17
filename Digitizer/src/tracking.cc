#include <iostream>
#include "RunManager.hh"
#include "TString.h"
#include "util.hh"


int main(int argc, char *argv[]){
	std::cout <<argc << std::endl;

	if (argc < 5) {
		std::cout << "Usage:" <<std::endl;
		std::cout << "-l LHC_directory -c cosmic_directory -o output_dir_name" <<std::endl;
		return 0;
	}

	std::vector<TString> LHCfiles;
	std::vector<TString> CosmicFiles;
	TString outdir;

	int i = 1;
	bool input = false;
	bool cosmic = false;
	while (i < argc) {
		if (strcmp("-l", argv[i]) == 0) {
			LHCfiles = io::ProcessDirectory(argv[i+1], "");
			input = true;
		} else if (strcmp("-c", argv[i]) == 0) {
			CosmicFiles = io::ProcessDirectory(argv[i+1], "");
			cosmic = true;
		} else if (strcmp("-o", argv[i]) == 0) {
			outdir = TString(argv[i+1]);
		} else if (!input) {
			std::cout << "Usage:" <<std::endl;
			std::cout << "-l LHC_directory -c cosmic_directory -o output_dir_name" <<std::endl;
			return 0;
		}
		i++;
	}
	
	RunManager RM;
	RM.SetOutputFile(outdir);

	for (int i = 0; i < LHCfiles.size(); i++) {
		auto f = LHCfiles[i];
		std::cout << f << std::endl;
		if (cosmic) {
			//For now, this assumes 1-1 and if not, re-loop through
			int cosmic_index = i % CosmicFiles.size();
			RM.SetCosmicFile(CosmicFiles[cosmic_index]);
		}
		RM.SetInputFile(f);
		RM.StartTracking();
	} 

	return 0;

} //main
