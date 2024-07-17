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
#include <map>

int RunManager::StartTracking()
{
// ---------- Declaring Tree Handlers and setting up geometry ---------------------------------------
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

	//Total LHC muons
	int nLHC = 0;
	//Number of total noise events
	int nNoise = 0;
	//Number of cosmic events
	int totalCosmic = 0;
	//Full Total
	int FullNumber = 0;

	ParHandler hndlr;
	hndlr.Handle();

// ---------- Finding excluded layers (if any) ------------------------------------------------------
	//Getting the layers that we will not be including
	std::vector<int> excludedLayers;
	int diff = (int)(_geometry.GetYLayers().size()/2) - hndlr.par_map["n_layers"];
	for (int i = 0; i < diff; i++){
		// Remove the horizontal tracking layers
		int layerNum = (int)(_geometry.GetZWalls().size()/2) 
		+ (int)(_geometry.GetYFloor().size()/2) 
		+ (int)(_geometry.GetYLayers().size()/2) 
		- i - 1;
		excludedLayers.push_back(layerNum);
		// Remove the back wall tracking layers
		layerNum = (int)(_geometry.GetZWalls().size()/2) 
		+ (int)(_geometry.GetYFloor().size()/2) 
		+ (int)(_geometry.GetYLayers().size()/2) 
		+ (int)(_geometry.GetZBack().size()/2)
		- i - 1;
		excludedLayers.push_back(layerNum);
	}
		
// ---------- Loop through main tree ----------------------------------------------------------------
	if (hndlr.par_map["branch"] == 1.0) std::cout << "Running in Cosmic Mode" << std::endl;

	_digitizer->par_handler = &hndlr;

	NoiseMaker::preDigitizer(&_geometry);
	
	_digitizer->InitGenerators();
	
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

			TH->LoadEvent();

			int n_added = 0; //This tracks the number of sim_hits added, needed for setting proper index
			//adding all hits of the tree into the digitizer
			for (int n_hit = 0; n_hit < TH->sim_numhits; n_hit++){
				physics::sim_hit *current = new physics::sim_hit(TH, &_geometry, n_added);
				//Check if sim hit is in an excluded layer
				std::cout << excludedLayers.size() << std::endl;
				//if (excludedLayers.size() != 0 && std::find(excludedLayers.begin(), excludedLayers.end(), current->det_id.layerID) 
				//!= excludedLayers.end()) { 
			//		continue;
			//	}
				TH->sim_hit_type_buf->push_back(0);
				current->SetType(0);
				if (hndlr.par_map["branch"] == 1.0) {
					current->x += detector::COSMIC_SHIFT[0];
					current->y += detector::COSMIC_SHIFT[1];
					current->z += detector::COSMIC_SHIFT[2];
				}
				nLHC++;
				_digitizer->AddHit(current);
				//n_added++;
			}
			n_added = TH->sim_numhits;

			//Doing cosmic (if any)
			//The following is used to assign unique track IDs that would otherwise be duplicated
			int n_cosmic = 0;	
			std::map<int, int> trackIDChanger;
			int maxID;
			int curID;
			if (cosmic){
				n_cosmic = _digitizer->generator.Poisson(hndlr.par_map["cosmic_rate"]);
				maxID = 0; // For setting new track IDs so there are no duplicates
				for (int n_hit = 0; n_hit < n_added; n_hit++){
					if (_digitizer->hits[n_hit]->track_id > maxID) {
						maxID = _digitizer->hits[n_hit]->track_id;
					}
				}
				maxID++;
			}
			std::cout << "Got Max ID" << std::endl;
			//adding cosmic events as chosen by poisson distribution
			while (n_cosmic > 0) {
				CH->index =(int)_digitizer->generator.Uniform(CH->NumEntries);
				if (CH->Next() < 0) {
					CH->index = 0;// for now, just loop back to start	
				}
				CH->LoadEvent();
				for (int n_hit = 0; n_hit < CH->sim_numhits; n_hit++){
					physics::sim_hit *current = new physics::sim_hit(CH, &_geometry, n_hit);
					//if (excludedLayers.size() != 0 && std::find(excludedLayers.begin(), excludedLayers.end(), current->det_id.layerID) 
					//!= excludedLayers.end()) { 
					//	continue;
					//}
					current->index = n_hit + n_added;
					//current->index = nLHC + n_added;
					//Reassigning track IDs to new ones
					curID = current->track_id;
					for (int oldHit = 0; oldHit < n_added; oldHit++) {
						if (curID == _digitizer->hits[oldHit]->track_id) {
							//This one not added yet
							if (trackIDChanger.find(curID) == trackIDChanger.end()){
								trackIDChanger[curID] = maxID;
								maxID++;
							}
							current->track_id = maxID;
						}
					}
					CH->sim_hit_type_buf->push_back(1);
					current->SetType(1);
					if (hndlr.par_map["branch"] == 1.0) {
						current->x += detector::COSMIC_SHIFT[0];
						current->y += detector::COSMIC_SHIFT[1];
						current->z += detector::COSMIC_SHIFT[2];
					}
					_digitizer->AddHit(current);
					//n_added++;
				}
				n_added += CH->sim_numhits;
				totalCosmic++;
				n_cosmic -= 1;
			}
			std::cout << "Checked CH Loading" <<std::endl;

			std::vector<physics::digi_hit *> digi_list = _digitizer->Digitize();
			if (digi_list.size() == 0) {
				std::cout << "No digis" << std::endl;
			}
			if (_digitizer->hits.size() == 0) {
				std::cout << "No sim hits" << std::endl;
			}

			std::vector<physics::digi_hit*> noise_digis;
			if(NoiseMaker::run){
				NoiseMaker* noise = new NoiseMaker(digi_list);
				noise_digis = noise->return_digis();
				for(auto digi:noise_digis){
					digi->SetType(2);
					digi_list.push_back(digi);
					nNoise++;
                		}
        		}
			TH->ExportDigis(digi_list, _digitizer->seed);

			TH->Fill();

			dropped_hits += _digitizer->dropped_hits;
			floor_wall_hits += _digitizer->floor_wall_hits;
		}
		events_handled++;
	}
	FullNumber = nLHC + nNoise + totalCosmic;
	std::cout << "Number of LHC events: " << nLHC << std::endl;
	std::cout << "Number of noise events: " << nNoise <<std::endl;
	std::cout << "Number of cosmic events: " << totalCosmic << std::endl;
	std::cout << "Number of events total: " << FullNumber << std::endl;
	if (hndlr.file_opened) {
		std::cout << "writing to file" << std::endl;
		TH->Write();
	}
	return 0;
}
