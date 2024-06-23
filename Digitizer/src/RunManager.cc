#include <iostream>
#include "RunManager.hh"
#include "TreeHandler.hh"
#include "NoiseMaker.hh"
#include "Digitizer.hh"
#include "globals.hh"
#include <iostream>
#include <fstream>
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include "par_handler.hh"

int RunManager::StartTracking()
{

	TH = new TreeHandler(_InputTree_Name, _InputFile_Name, _OutputTree_Name, OutFileName());
	if (TH->IsNull()) {
		std::cout << "Sorry, I couldn't open that file" << std::endl;
		return 0;
	} 
	
	TreeHandler *CH;
	if (cosmic) {
		CH = new TreeHandler(_InputTree_Name, _CosmicFile_Name, TH);
		if (CH->IsNull()) {
			std::cout << "Sorry, I couldn't open that cosmic file" << std::endl;
			return 0;
		}
	} 

	GeometryHandler _geometry(_geomTree_Name, _InputFile_Name);
	_geometry.LoadGeometry();

	int events_handled = 0;

	int dropped_hits = 0;
	int floor_wall_hits = 0;

	//For debugging only
	int trueNumber = 0;

	ParHandler hndlr;
	hndlr.Handle();

	if (hndlr.par_map["branch"] == 1.0) std::cout << "Running in Cosmic Mode" << std::endl;

	_digitizer->par_handler = &hndlr;

	NoiseMaker::preDigitizer(&_geometry);
	
	_digitizer->InitGenerators();

	std::cout<< "TH OutputTree: " << TH->OutputTree << std::endl;
	if (cosmic)
		std::cout<< "CH OutputTree: " << CH->OutputTree << std::endl; 
	
	while (TH->Next() >= 0)
	{
		if (events_handled >= hndlr.par_map["end_ev"])//cuts::end_ev)
			break;
		if (events_handled >= hndlr.par_map["start_ev"]) //cuts::start_ev)
		{
			if (hndlr.par_map["debug"] == 1) 
				std::cout << "\n"<< std::endl;
			if ((events_handled) % 1000 == 0 || hndlr.par_map["debug"] == 1 || hndlr.par_map["debug_vertex"] == 1 )
				std::cout << "=== Event is " << events_handled <<" ==="<< std::endl;

			TotalEventsProcessed++;
			_digitizer->clear();

			//Doing cosmic (if any)
			int n_cosmic = 0;	
			if (cosmic){
				n_cosmic = _digitizer->generator.Poisson(hndlr.par_map["cosmic_rate"]);
			}
			//adding cosmic events as chosen by poisson distribution
			while (n_cosmic > 0) {
				std::cout << "Going to set the index" << std::endl;
				CH->index =(int)_digitizer->generator.Uniform(CH->NumEntries);
				if (CH->Next() < 0) {
					CH->index = 0;// for now, just loop back to start	
				}
				std::cout << "Going to load event | ";
				CH->LoadEvent();
				std::cout << "Loaded Event Properly | ";
				for (int n_hit = 0; n_hit < CH->sim_numhits; n_hit++){
					physics::sim_hit *current = new physics::sim_hit(CH, &_geometry, n_hit);
					CH->sim_hit_type_buf->push_back(1);
					std::cout << "Pushed back value for cosmic" <<std::endl;
					if (hndlr.par_map["branch"] == 1.0) {
						current->x += detector::COSMIC_SHIFT[0];
						current->y += detector::COSMIC_SHIFT[1];
						current->z += detector::COSMIC_SHIFT[2];
					}
					_digitizer->AddHit(current);
				}
				n_cosmic -= 1;
			}
			// copying the data to the new tree, and loading all the variables, incrementing index
			TH->LoadEvent();
			//adding all hits of the tree into the digitizer
			for (int n_hit = 0; n_hit < TH->sim_numhits; n_hit++){
				physics::sim_hit *current = new physics::sim_hit(TH, &_geometry, n_hit);
				TH->sim_hit_type_buf->push_back(0);
				if (hndlr.par_map["branch"] == 1.0) {
					current->x += detector::COSMIC_SHIFT[0];
					current->y += detector::COSMIC_SHIFT[1];
					current->z += detector::COSMIC_SHIFT[2];
				}
				_digitizer->AddHit(current);
			}
			std::vector<physics::digi_hit *> digi_list = _digitizer->Digitize();
			trueNumber += digi_list.size();

			std::vector<physics::digi_hit*> noise_digis;
			if(NoiseMaker::run){
                NoiseMaker* noise = new NoiseMaker(digi_list);
                noise_digis = noise->return_digis();
                for(auto digi:noise_digis){
                        digi_list.push_back(digi);
                }
        	}
			//std::cout << "events handled: " << digi_list.size() << " | ";
			TH->ExportDigis(digi_list, _digitizer->seed);
			std::cout << TH->sim_hit_type_buf->size() << std::endl;

			TH->Fill();

			dropped_hits += _digitizer->dropped_hits;
			floor_wall_hits += _digitizer->floor_wall_hits;
		}
		events_handled++;
	}
	std::cout << "Number of events total: " << trueNumber << std::endl;
	if (hndlr.file_opened) {
		std::cout << "writing to file" << std::endl;
		TH->Write();
	}
	return 0;
}
