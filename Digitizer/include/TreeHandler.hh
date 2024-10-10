#include <TTree.h>
#include <TFile.h>
#include <TROOT.h>
#include <Eigen/Dense>
#include "globals.hh"

#ifndef TH_DEFINE
#define TH_DEFINE

class TreeHandler{ 
public:
	//PUT ALL INPUT AND OUTPUT BRANCHES HERE
	TTree* OutputTree;
	TTree* InputTree;
	TFile* OutputFile;
	TFile* InputFile;
	int index = -1;
	int NumEntries;
	bool _Null = false;
	bool IsNull(){return _Null;}

	int Next(){
    index++;
		if (index >= NumEntries) return -1;
		return index;
	}

	int LoadEvent(){
		Flush();
		if (InputTree == nullptr) return -1;
		InputTree->GetEvent(index);
		AddEvent();
		return 0;
	}

	void Fill(){
		//OutputFile->cd();
		gROOT->cd();
		InputTree->GetEvent(index);
		OutputTree->Fill();
		Flush();
	}

	void Write(){
    InputFile->Close();
		OutputFile->cd();
		OutputFile->Write();
		OutputFile->Close();
	}


	template<class digi_hit>
	void ExportDigis(std::vector<digi_hit*>, long long int digi_seed);

	TreeHandler(TString input_tree_name, TString input_file_name, TreeHandler* TH) {
		InputFile = TFile::Open(input_file_name);
		if (! InputFile) {
      		_Null = true;
      		return;
    	}

		InputTree = (TTree*) InputFile->Get(input_tree_name);

    	if (! InputTree){
      		_Null = true;
      		return;
    	}

		InputTree->SetBranchAddress("NumHits", &sim_numhits);
 		InputTree->SetBranchAddress("Hit_energy", &sim_hit_e);
 		InputTree->SetBranchAddress("Hit_time", &sim_hit_t);
 		InputTree->SetBranchAddress("Hit_Center1", &sim_hit_center1);
 		InputTree->SetBranchAddress("Hit_Center2", &sim_hit_center2);
 		InputTree->SetBranchAddress("Hit_bar_direction", &sim_hit_bar_direction);
 		InputTree->SetBranchAddress("Hit_layer_direction", &sim_hit_layer_direction);
 		InputTree->SetBranchAddress("Hit_LayerID", &sim_hit_layerID);

 		InputTree->SetBranchAddress("Hit_particlePdgId", &sim_hit_particlePdgId);
 		InputTree->SetBranchAddress("Hit_G4TrackId", &sim_hit_G4TrackId);
 		InputTree->SetBranchAddress("Hit_G4ParentTrackId", &sim_hit_G4ParentTrackId);
 		InputTree->SetBranchAddress("Hit_x", &sim_hit_x);
 		InputTree->SetBranchAddress("Hit_y", &sim_hit_y);
 		InputTree->SetBranchAddress("Hit_z", &sim_hit_z);
 		InputTree->SetBranchAddress("Hit_particleEnergy", &sim_hit_particleEnergy);
 		InputTree->SetBranchAddress("Hit_particlePx", &sim_hit_px);
 		InputTree->SetBranchAddress("Hit_particlePy", &sim_hit_py);
		InputTree->SetBranchAddress("Hit_particlePz", &sim_hit_pz);
		InputTree->SetBranchAddress("GenParticle_energy", &sim_GenParticle_energy);
		InputTree->SetBranchAddress("GenParticle_pdgid", &sim_GenParticle_pdgid);
		InputTree->SetBranchAddress("GenParticle_index", &sim_GenParticle_index);
		InputTree->SetBranchAddress("GenParticle_G4index", &sim_GenParticle_G4index);
	 	InputTree->SetBranchAddress("GenParticle_time", &sim_GenParticle_time);
	 	InputTree->SetBranchAddress("GenParticle_x", &sim_GenParticle_x);
	 	InputTree->SetBranchAddress("GenParticle_y", &sim_GenParticle_y);
	 	InputTree->SetBranchAddress("GenParticle_z", &sim_GenParticle_z);
	 	InputTree->SetBranchAddress("GenParticle_px", &sim_GenParticle_px);
	 	InputTree->SetBranchAddress("GenParticle_py", &sim_GenParticle_py);
	 	InputTree->SetBranchAddress("GenParticle_pz", &sim_GenParticle_pz);
	 	InputTree->SetBranchAddress("GenParticle_mass", &sim_GenParticle_mass);


		InputTree->SetBranchStatus("Hit_weight", 0);
   		InputTree->SetBranchStatus("NumGenParticles", 0);
   		InputTree->SetBranchStatus("GenParticle_status", 0);
   		InputTree->SetBranchStatus("GenParticle_pt", 0);
   		InputTree->SetBranchStatus("GenParticle_eta", 0);
   		InputTree->SetBranchStatus("GenParticle_phi", 0);

   		InputTree->SetBranchStatus("GenParticle_mo1", 0);
   		InputTree->SetBranchStatus("GenParticle_mo2", 0);
   		InputTree->SetBranchStatus("GenParticle_dau1", 0);
   		InputTree->SetBranchStatus("GenParticle_dau2", 0);
		InputTree->SetBranchStatus("COSMIC_EVENT_ID", 0);
 		InputTree->SetBranchStatus("COSMIC_CORE_X", 0);
 		InputTree->SetBranchStatus("COSMIC_CORE_Y", 0);
 		InputTree->SetBranchStatus("COSMIC_GEN_PRIMARY_ENERGY", 0);
 		InputTree->SetBranchStatus("COSMIC_GEN_THETA", 0);
 		InputTree->SetBranchStatus("COSMIC_GEN_PHI", 0);
 		InputTree->SetBranchStatus("COSMIC_GEN_FIRST_HEIGHT", 0);
 		InputTree->SetBranchStatus("COSMIC_GEN_ELECTRON_COUNT", 0);
 		InputTree->SetBranchStatus("COSMIC_GEN_MUON_COUNT", 0);
 		InputTree->SetBranchStatus("COSMIC_GEN_HADRON_COUNT", 0);
 		InputTree->SetBranchStatus("COSMIC_GEN_PRIMARY_ID", 0);
 		InputTree->SetBranchStatus("EXTRA_12", 0);
 		InputTree->SetBranchStatus("EXTRA_13", 0);
 		InputTree->SetBranchStatus("EXTRA_14", 0);
 		InputTree->SetBranchStatus("EXTRA_15", 0);

 		NumEntries = InputTree->GetEntries();

 		Double_t sim_numhits_buf = 0;
 		sim_hit_e_buf 					= TH->sim_hit_e_buf;					
 		sim_hit_t_buf 					= TH->sim_hit_t_buf;
 		sim_hit_center1_buf 			= TH->sim_hit_center1_buf; 
 		sim_hit_center2_buf 			= TH->sim_hit_center2_buf; 
 		sim_hit_bar_direction_buf 		= TH->sim_hit_bar_direction_buf; 
 		sim_hit_layer_direction_buf 	= TH->sim_hit_layer_direction_buf;
 		sim_hit_layerID_buf 			= TH->sim_hit_layerID_buf;
 		sim_hit_particlePdgId_buf 		= TH->sim_hit_particlePdgId_buf;
 		sim_hit_G4TrackId_buf 			= TH->sim_hit_G4TrackId_buf;
 		sim_hit_G4ParentTrackId_buf 	= TH->sim_hit_G4ParentTrackId_buf;
 		sim_hit_x_buf 					= TH->sim_hit_x_buf;
 		sim_hit_y_buf 					= TH->sim_hit_y_buf;
 		sim_hit_z_buf 					= TH->sim_hit_z_buf;
 		sim_hit_particleEnergy_buf 		= TH->sim_hit_particleEnergy_buf;
 		sim_hit_px_buf 					= TH->sim_hit_px_buf;
 		sim_hit_py_buf 					= TH->sim_hit_py_buf;
 		sim_hit_pz_buf 					= TH->sim_hit_pz_buf;

		sim_hit_type_buf				= TH->sim_hit_type_buf;
		sim_cosmic_e_buf				= TH->sim_cosmic_e_buf;
		sim_cosmic_pid_buf				= TH->sim_cosmic_pid_buf;

 		sim_GenParticle_index_buf 		= TH->sim_GenParticle_index_buf;
 		sim_GenParticle_G4index_buf 	= TH->sim_GenParticle_G4index_buf;
 		sim_GenParticle_pdgid_buf 		= TH->sim_GenParticle_pdgid_buf;
 		sim_GenParticle_time_buf 		= TH->sim_GenParticle_time_buf;
 		sim_GenParticle_x_buf 			= TH->sim_GenParticle_x_buf;
 		sim_GenParticle_y_buf 			= TH->sim_GenParticle_y_buf;
 		sim_GenParticle_z_buf 			= TH->sim_GenParticle_z_buf;
 		sim_GenParticle_energy_buf 		= TH->sim_GenParticle_energy_buf;
 		sim_GenParticle_px_buf 			= TH->sim_GenParticle_px_buf;
 		sim_GenParticle_py_buf 			= TH->sim_GenParticle_py_buf;
 		sim_GenParticle_pz_buf 			= TH->sim_GenParticle_pz_buf;
 		sim_GenParticle_mass_buf 		= TH->sim_GenParticle_mass_buf;
	}

	TreeHandler(TString input_tree_name, TString input_file_name, TString output_tree_name, TString outfile_name) {
		InputFile = TFile::Open(input_file_name);
		if (! InputFile) {
      		_Null = true;
      		return;
    	}

		InputTree = (TTree*) InputFile->Get(input_tree_name);

    	if (! InputTree){
      		_Null = true;
      		return;
    	}

		InputTree->SetBranchAddress("NumHits", &sim_numhits);
 		InputTree->SetBranchAddress("Hit_energy", &sim_hit_e);
 		InputTree->SetBranchAddress("Hit_time", &sim_hit_t);
 		InputTree->SetBranchAddress("Hit_Center1", &sim_hit_center1);
 		InputTree->SetBranchAddress("Hit_Center2", &sim_hit_center2);
 		InputTree->SetBranchAddress("Hit_bar_direction", &sim_hit_bar_direction);
 		InputTree->SetBranchAddress("Hit_layer_direction", &sim_hit_layer_direction);
 		InputTree->SetBranchAddress("Hit_LayerID", &sim_hit_layerID);

 		InputTree->SetBranchAddress("Hit_particlePdgId", &sim_hit_particlePdgId);
 		InputTree->SetBranchAddress("Hit_G4TrackId", &sim_hit_G4TrackId);
 		InputTree->SetBranchAddress("Hit_G4ParentTrackId", &sim_hit_G4ParentTrackId);
 		InputTree->SetBranchAddress("Hit_x", &sim_hit_x);
 		InputTree->SetBranchAddress("Hit_y", &sim_hit_y);
 		InputTree->SetBranchAddress("Hit_z", &sim_hit_z);
 		InputTree->SetBranchAddress("Hit_particleEnergy", &sim_hit_particleEnergy);
 		InputTree->SetBranchAddress("Hit_particlePx", &sim_hit_px);
 		InputTree->SetBranchAddress("Hit_particlePy", &sim_hit_py);
		InputTree->SetBranchAddress("Hit_particlePz", &sim_hit_pz);
		InputTree->SetBranchAddress("GenParticle_energy", &sim_GenParticle_energy);
		InputTree->SetBranchAddress("GenParticle_pdgid", &sim_GenParticle_pdgid);
		InputTree->SetBranchAddress("GenParticle_index", &sim_GenParticle_index);
		InputTree->SetBranchAddress("GenParticle_G4index", &sim_GenParticle_G4index);
	 	InputTree->SetBranchAddress("GenParticle_time", &sim_GenParticle_time);
	 	InputTree->SetBranchAddress("GenParticle_x", &sim_GenParticle_x);
	 	InputTree->SetBranchAddress("GenParticle_y", &sim_GenParticle_y);
	 	InputTree->SetBranchAddress("GenParticle_z", &sim_GenParticle_z);
	 	InputTree->SetBranchAddress("GenParticle_px", &sim_GenParticle_px);
	 	InputTree->SetBranchAddress("GenParticle_py", &sim_GenParticle_py);
	 	InputTree->SetBranchAddress("GenParticle_pz", &sim_GenParticle_pz);
	 	InputTree->SetBranchAddress("GenParticle_mass", &sim_GenParticle_mass);


		InputTree->SetBranchStatus("Hit_weight", 0);
   		InputTree->SetBranchStatus("NumGenParticles", 0);
   		InputTree->SetBranchStatus("GenParticle_status", 0);
   		InputTree->SetBranchStatus("GenParticle_pt", 0);
   		InputTree->SetBranchStatus("GenParticle_eta", 0);
   		InputTree->SetBranchStatus("GenParticle_phi", 0);

   		InputTree->SetBranchStatus("GenParticle_mo1", 0);
   		InputTree->SetBranchStatus("GenParticle_mo2", 0);
   		InputTree->SetBranchStatus("GenParticle_dau1", 0);
   		InputTree->SetBranchStatus("GenParticle_dau2", 0);
		InputTree->SetBranchStatus("COSMIC_EVENT_ID", 0);
 		InputTree->SetBranchStatus("COSMIC_CORE_X", 0);
 		InputTree->SetBranchStatus("COSMIC_CORE_Y", 0);
 		InputTree->SetBranchStatus("COSMIC_GEN_PRIMARY_ENERGY", 0);
 		InputTree->SetBranchStatus("COSMIC_GEN_THETA", 0);
 		InputTree->SetBranchStatus("COSMIC_GEN_PHI", 0);
 		InputTree->SetBranchStatus("COSMIC_GEN_FIRST_HEIGHT", 0);
 		InputTree->SetBranchStatus("COSMIC_GEN_ELECTRON_COUNT", 0);
 		InputTree->SetBranchStatus("COSMIC_GEN_MUON_COUNT", 0);
 		InputTree->SetBranchStatus("COSMIC_GEN_HADRON_COUNT", 0);
 		InputTree->SetBranchStatus("COSMIC_GEN_PRIMARY_ID", 0);
 		InputTree->SetBranchStatus("EXTRA_12", 0);
 		InputTree->SetBranchStatus("EXTRA_13", 0);
 		InputTree->SetBranchStatus("EXTRA_14", 0);
 		InputTree->SetBranchStatus("EXTRA_15", 0);

 		NumEntries = InputTree->GetEntries();

 		OutputFile = new TFile(outfile_name, "RECREATE");
		OutputTree = new TTree(output_tree_name, "MATHUSLA Tree");

		OutputTree->Branch("NumHits", &sim_numhits_buf);
		OutputTree->Branch("Hit_energy", "std::vector<double>", &sim_hit_e_buf);
 		OutputTree->Branch("Hit_time", "std::vector<double>", &sim_hit_t_buf);
 		OutputTree->Branch("Hit_Center1", "std::vector<double>", &sim_hit_center1_buf);
 		OutputTree->Branch("Hit_Center2", "std::vector<double>", &sim_hit_center2_buf);
 		OutputTree->Branch("Hit_bar_direction", "std::vector<double>", &sim_hit_bar_direction_buf);
 		OutputTree->Branch("Hit_layer_direction", "std::vector<double>", &sim_hit_layer_direction_buf);
 		OutputTree->Branch("Hit_LayerID", "std::vector<double>", &sim_hit_layerID_buf);
 		OutputTree->Branch("Hit_particlePdgId", "std::vector<double>", &sim_hit_particlePdgId_buf);
 		OutputTree->Branch("Hit_G4TrackId", "std::vector<double>", &sim_hit_G4TrackId_buf);
 		OutputTree->Branch("Hit_G4ParentTrackId", "std::vector<double>", &sim_hit_G4ParentTrackId_buf);
 		OutputTree->Branch("Hit_x", "std::vector<double>", &sim_hit_x_buf);
 		OutputTree->Branch("Hit_y", "std::vector<double>", &sim_hit_y_buf);
 		OutputTree->Branch("Hit_z", "std::vector<double>", &sim_hit_z_buf);
 		OutputTree->Branch("Hit_particleEnergy", "std::vector<double>", &sim_hit_particleEnergy_buf);
 		OutputTree->Branch("Hit_particlePx", "std::vector<double>", &sim_hit_px_buf);
 		OutputTree->Branch("Hit_particlePy", "std::vector<double>", &sim_hit_py_buf);
		OutputTree->Branch("Hit_particlePz", "std::vector<double>", &sim_hit_pz_buf);

		OutputTree->Branch("Hit_type", "std::vector<int>", &sim_hit_type_buf);
		OutputTree->Branch("Cosmic_energy", "std::vector<double>", &sim_cosmic_e_buf);
		OutputTree->Branch("Cosmic_pdgid", "std::vector<int>", &sim_cosmic_pid_buf);
 
		OutputTree->Branch("Digi_numHits", &Digi_numHits);
		OutputTree->Branch("Digi_time", &digi_hit_t);
        OutputTree->Branch("Digi_x", &digi_hit_x);
        OutputTree->Branch("Digi_y", &digi_hit_y);
        OutputTree->Branch("Digi_z", &digi_hit_z);
        OutputTree->Branch("Digi_energy", &digi_hit_e);
        OutputTree->Branch("Digi_px", &digi_hit_px);
        OutputTree->Branch("Digi_py", &digi_hit_py);
        OutputTree->Branch("Digi_pz", &digi_hit_pz);
        OutputTree->Branch("Digi_particle_energy", &digi_particle_energy);
        OutputTree->Branch("Digi_pdg_id", &digi_pdg);
        OutputTree->Branch("Digi_track_id", &digi_track_id);

		OutputTree->Branch("Digi_type", &digi_hit_type);

		OutputTree->Branch("Digi_center1", &digi_center1);
		OutputTree->Branch("Digi_center2", &digi_center2);
		OutputTree->Branch("Digi_bar_direction", &digi_bar_direction);
		OutputTree->Branch("Digi_layer_direction", &digi_layer_direction);
		OutputTree->Branch("Digi_layer_id", &digi_LayerID);
		OutputTree->Branch("Digi_det_id", &digi_detID);
        OutputTree->Branch("Digi_seed", &digi_seed, "Digi_seed/L");
        OutputTree->Branch("Digi_hitIndices", &digi_hit_indices);

 		OutputTree->Branch("GenParticle_index", "std::vector<double>", &sim_GenParticle_index_buf);
 		OutputTree->Branch("GenParticle_G4index", "std::vector<double>", &sim_GenParticle_G4index_buf);
 		OutputTree->Branch("GenParticle_pdgid", "std::vector<double>", &sim_GenParticle_pdgid_buf);
 		OutputTree->Branch("GenParticle_time", "std::vector<double>", &sim_GenParticle_time_buf);
 		OutputTree->Branch("GenParticle_x", "std::vector<double>", &sim_GenParticle_x_buf);
 		OutputTree->Branch("GenParticle_y", "std::vector<double>", &sim_GenParticle_y_buf);
 		OutputTree->Branch("GenParticle_z", "std::vector<double>", &sim_GenParticle_z_buf);
 		OutputTree->Branch("GenParticle_energy", "std::vector<double>", &sim_GenParticle_energy_buf);
 		OutputTree->Branch("GenParticle_px", "std::vector<double>", &sim_GenParticle_px_buf);
 		OutputTree->Branch("GenParticle_py", "std::vector<double>", &sim_GenParticle_py_buf);
 		OutputTree->Branch("GenParticle_pz", "std::vector<double>", &sim_GenParticle_pz_buf);
 		OutputTree->Branch("GenParticle_mass", "std::vector<double>", &sim_GenParticle_mass_buf);
	}
//____________________________________________________________________________________________

//___Make Sim Branches_________________________________________________________________________
 	Double_t sim_numhits;
 	std::vector<double> *sim_hit_e = nullptr;
 	std::vector<double> *sim_hit_t = nullptr;
 	std::vector<double> *sim_hit_center1 = nullptr; 
 	std::vector<double> *sim_hit_center2 = nullptr; 
 	std::vector<double> *sim_hit_bar_direction = nullptr; 
 	std::vector<double> *sim_hit_layer_direction = nullptr;
 	std::vector<double> *sim_hit_layerID = nullptr;
 	std::vector<double> *sim_hit_particlePdgId = nullptr;
 	std::vector<double> *sim_hit_G4TrackId = nullptr;
 	std::vector<double> *sim_hit_G4ParentTrackId = nullptr;
 	std::vector<double> *sim_hit_x = nullptr;
 	std::vector<double> *sim_hit_y = nullptr;
 	std::vector<double> *sim_hit_z = nullptr;
 	std::vector<double> *sim_hit_particleEnergy = nullptr;
 	std::vector<double> *sim_hit_px = nullptr;
 	std::vector<double> *sim_hit_py = nullptr;
 	std::vector<double> *sim_hit_pz = nullptr;
	std::vector<int>	*sim_hit_type = nullptr;
 	std::vector<double> *sim_GenParticle_index = nullptr;
 	std::vector<double> *sim_GenParticle_G4index = nullptr;
 	std::vector<double> *sim_GenParticle_pdgid = nullptr;
 	std::vector<double> *sim_GenParticle_time = nullptr;
 	std::vector<double> *sim_GenParticle_x = nullptr;
 	std::vector<double> *sim_GenParticle_y = nullptr;
 	std::vector<double> *sim_GenParticle_z = nullptr;
 	std::vector<double> *sim_GenParticle_energy = nullptr;
 	std::vector<double> *sim_GenParticle_px = nullptr;
 	std::vector<double> *sim_GenParticle_py = nullptr;
 	std::vector<double> *sim_GenParticle_pz = nullptr;
 	std::vector<double> *sim_GenParticle_mass = nullptr;
 	std::vector<double> *sim_EXTRA_12 = nullptr;
 	std::vector<double> *sim_EXTRA_13 = nullptr;
 	std::vector<double> *sim_EXTRA_14 = nullptr;
 	std::vector<double> *sim_EXTRA_15 = nullptr;

  //___Make Digi Branches_____________________________________________________________________
  	std::vector<double> digi_hit_t;
  	std::vector<double> digi_hit_x;
  	std::vector<double> digi_hit_y;
  	std::vector<double> digi_hit_z;
  	std::vector<double> digi_hit_e;
  	std::vector<double> digi_hit_px;
  	std::vector<double> digi_hit_py;
  	std::vector<double> digi_hit_pz;
    std::vector<double> digi_particle_energy;
    std::vector<int> digi_pdg;
    std::vector<int> digi_track_id;

	std::vector<int> digi_hit_type;

	std::vector<double> digi_center1;
	std::vector<double> digi_center2;
	std::vector<int> digi_bar_direction;
	std::vector<int> digi_layer_direction;
	std::vector<int> digi_LayerID;
	std::vector<int> digi_detID;
  	std::vector<int> digi_hit_indices;
  	std::vector<int> Digi_numHits;
    long long int digi_seed;

//___These are the buffers for each branch so that they can be combined
//___Make Sim Branches_________________________________________________________________________
 	Double_t sim_numhits_buf = 0;
 	std::vector<double> *sim_hit_e_buf = nullptr;					
 	std::vector<double> *sim_hit_t_buf = nullptr;
 	std::vector<double> *sim_hit_center1_buf = nullptr; 
 	std::vector<double> *sim_hit_center2_buf = nullptr; 
 	std::vector<double> *sim_hit_bar_direction_buf = nullptr; 
 	std::vector<double> *sim_hit_layer_direction_buf = nullptr;
 	std::vector<double> *sim_hit_layerID_buf = nullptr;
 	std::vector<double> *sim_hit_particlePdgId_buf = nullptr;
 	std::vector<double> *sim_hit_G4TrackId_buf = nullptr;
 	std::vector<double> *sim_hit_G4ParentTrackId_buf = nullptr;
 	std::vector<double> *sim_hit_x_buf = nullptr;
 	std::vector<double> *sim_hit_y_buf = nullptr;
 	std::vector<double> *sim_hit_z_buf = nullptr;
 	std::vector<double> *sim_hit_particleEnergy_buf = nullptr;
 	std::vector<double> *sim_hit_px_buf = nullptr;
 	std::vector<double> *sim_hit_py_buf = nullptr;
 	std::vector<double> *sim_hit_pz_buf = nullptr;

	std::vector<int> 	*sim_hit_type_buf = nullptr;
	std::vector<double> 	*sim_cosmic_e_buf = nullptr;
	std::vector<int> 	*sim_cosmic_pid_buf = nullptr;

 	std::vector<double> *sim_GenParticle_index_buf = nullptr;
 	std::vector<double> *sim_GenParticle_G4index_buf = nullptr;
 	std::vector<double> *sim_GenParticle_pdgid_buf = nullptr;
 	std::vector<double> *sim_GenParticle_time_buf = nullptr;
 	std::vector<double> *sim_GenParticle_x_buf = nullptr;
 	std::vector<double> *sim_GenParticle_y_buf = nullptr;
 	std::vector<double> *sim_GenParticle_z_buf = nullptr;
 	std::vector<double> *sim_GenParticle_energy_buf = nullptr;
 	std::vector<double> *sim_GenParticle_px_buf = nullptr;
 	std::vector<double> *sim_GenParticle_py_buf = nullptr;
 	std::vector<double> *sim_GenParticle_pz_buf = nullptr;
 	std::vector<double> *sim_GenParticle_mass_buf = nullptr;
 	std::vector<double> *sim_EXTRA_12_buf = nullptr;
 	std::vector<double> *sim_EXTRA_13_buf = nullptr;
 	std::vector<double> *sim_EXTRA_14_buf = nullptr;
 	std::vector<double> *sim_EXTRA_15_buf = nullptr;

	void AddEvent() {
		sim_numhits_buf += sim_numhits;
		sim_hit_e_buf->insert(sim_hit_e_buf->end(), sim_hit_e->begin(), sim_hit_e->end());
		sim_hit_t_buf->insert(sim_hit_t_buf->end(), sim_hit_t->begin(), sim_hit_t->end());
		sim_hit_center1_buf->insert(sim_hit_center1_buf->end(), sim_hit_center1->begin(), sim_hit_center1->end());
		sim_hit_center2_buf->insert(sim_hit_center2_buf->end(), sim_hit_center2->begin(), sim_hit_center2->end());
		sim_hit_bar_direction_buf->insert(sim_hit_bar_direction_buf->end(), sim_hit_bar_direction->begin(), sim_hit_bar_direction->end());
		sim_hit_layer_direction_buf->insert(sim_hit_layer_direction_buf->end(), sim_hit_layer_direction->begin(), sim_hit_layer_direction->end());
		sim_hit_layerID_buf->insert(sim_hit_layerID_buf->end(), sim_hit_layerID->begin(), sim_hit_layerID->end());
		sim_hit_particlePdgId_buf->insert(sim_hit_particlePdgId_buf->end(), sim_hit_particlePdgId->begin(), sim_hit_particlePdgId->end());
		sim_hit_G4TrackId_buf->insert(sim_hit_G4TrackId_buf->end(), sim_hit_G4TrackId->begin(), sim_hit_G4TrackId->end());
		sim_hit_G4ParentTrackId_buf->insert(sim_hit_G4ParentTrackId_buf->end(), sim_hit_G4ParentTrackId->begin(), sim_hit_G4ParentTrackId->end());
		sim_hit_x_buf->insert(sim_hit_x_buf->end(), sim_hit_x->begin(), sim_hit_x->end());
		sim_hit_y_buf->insert(sim_hit_y_buf->end(), sim_hit_y->begin(), sim_hit_y->end());
		sim_hit_z_buf->insert(sim_hit_z_buf->end(), sim_hit_z->begin(), sim_hit_z->end());
		sim_hit_particleEnergy_buf->insert(sim_hit_particleEnergy_buf->end(), sim_hit_particleEnergy->begin(), sim_hit_particleEnergy->end());
		sim_hit_px_buf->insert(sim_hit_px_buf->end(), sim_hit_px->begin(), sim_hit_px->end());
		sim_hit_py_buf->insert(sim_hit_py_buf->end(), sim_hit_py->begin(), sim_hit_py->end());
		sim_hit_pz_buf->insert(sim_hit_pz_buf->end(), sim_hit_pz->begin(), sim_hit_pz->end());
		sim_GenParticle_index_buf->insert(sim_GenParticle_index_buf->end(), sim_GenParticle_index->begin(), sim_GenParticle_index->end());
		sim_GenParticle_G4index_buf->insert(sim_GenParticle_G4index_buf->end(), sim_GenParticle_G4index->begin(), sim_GenParticle_G4index->end());
		sim_GenParticle_pdgid_buf->insert(sim_GenParticle_pdgid_buf->end(), sim_GenParticle_pdgid->begin(), sim_GenParticle_pdgid->end());
		sim_GenParticle_time_buf->insert(sim_GenParticle_time_buf->end(), sim_GenParticle_time->begin(), sim_GenParticle_time->end());
		sim_GenParticle_x_buf->insert(sim_GenParticle_x_buf->end(), sim_GenParticle_x->begin(), sim_GenParticle_x->end());
		sim_GenParticle_y_buf->insert(sim_GenParticle_y_buf->end(), sim_GenParticle_y->begin(), sim_GenParticle_y->end());
		sim_GenParticle_z_buf->insert(sim_GenParticle_z_buf->end(), sim_GenParticle_z->begin(), sim_GenParticle_z->end());
		sim_GenParticle_energy_buf->insert(sim_GenParticle_energy_buf->end(), sim_GenParticle_energy->begin(), sim_GenParticle_energy->end());
		sim_GenParticle_px_buf->insert(sim_GenParticle_px_buf->end(), sim_GenParticle_px->begin(), sim_GenParticle_px->end());
		sim_GenParticle_py_buf->insert(sim_GenParticle_py_buf->end(), sim_GenParticle_py->begin(), sim_GenParticle_py->end());
		sim_GenParticle_pz_buf->insert(sim_GenParticle_pz_buf->end(), sim_GenParticle_pz->begin(), sim_GenParticle_pz->end());
		sim_GenParticle_mass_buf->insert(sim_GenParticle_mass_buf->end(), sim_GenParticle_mass->begin(), sim_GenParticle_mass->end());
	}

	void Flush(){
		sim_numhits_buf = 0;
		sim_hit_e_buf->clear();
		sim_hit_t_buf->clear();
		sim_hit_center1_buf->clear();
		sim_hit_center2_buf->clear();
		sim_hit_bar_direction_buf->clear();
		sim_hit_layer_direction_buf->clear();
		sim_hit_layerID_buf->clear();
		sim_hit_particlePdgId_buf->clear();
		sim_hit_G4TrackId_buf->clear();
		sim_hit_G4ParentTrackId_buf->clear();
		sim_hit_x_buf->clear();
		sim_hit_y_buf->clear();
		sim_hit_z_buf->clear();
		sim_hit_particleEnergy_buf->clear();
		sim_hit_px_buf->clear();
		sim_hit_py_buf->clear();
		sim_hit_pz_buf->clear();
		sim_hit_type_buf->clear();
		sim_cosmic_e_buf->clear();
		sim_cosmic_pid_buf->clear();
		sim_GenParticle_index_buf->clear();
		sim_GenParticle_G4index_buf->clear();
		sim_GenParticle_pdgid_buf->clear();
		sim_GenParticle_time_buf->clear();
		sim_GenParticle_x_buf->clear();
		sim_GenParticle_y_buf->clear();
		sim_GenParticle_z_buf->clear();
		sim_GenParticle_energy_buf->clear();
		sim_GenParticle_px_buf->clear();
		sim_GenParticle_py_buf->clear();
		sim_GenParticle_pz_buf->clear();
		sim_GenParticle_mass_buf->clear();
	 }

}; //class TreeHandler

template<class digi_hit>
void TreeHandler::ExportDigis(std::vector<digi_hit*> digi_list, long long int seed){
	digi_hit_indices.clear();
	digi_hit_t.clear();
    digi_hit_x.clear();
    digi_hit_y.clear();
    digi_hit_z.clear();
    digi_hit_e.clear();
    digi_hit_px.clear();
    digi_hit_py.clear();
    digi_hit_pz.clear();
    Digi_numHits.clear();
    digi_particle_energy.clear();
    digi_pdg.clear();
	digi_track_id.clear();
	digi_hit_type.clear();
	digi_center1.clear();
	digi_center2.clear();
	digi_bar_direction.clear();
	digi_layer_direction.clear();
	digi_LayerID.clear();
	digi_detID.clear();
    
    digi_seed = seed;

    for (auto digi : digi_list){
      Digi_numHits.push_back(digi->hits.size());
      digi_hit_t.push_back(digi->t);
      digi_hit_x.push_back(digi->x);
      digi_hit_y.push_back(digi->y);
      digi_hit_z.push_back(digi->z);
      digi_hit_e.push_back(digi->e);
      digi_hit_px.push_back(digi->px);
      digi_hit_py.push_back(digi->py);
      digi_hit_pz.push_back(digi->pz);
      digi_particle_energy.push_back(digi->particle_energy);
      digi_pdg.push_back(digi->pdg);
	  digi_track_id.push_back(digi->min_track_id);

	  digi_hit_type.push_back(digi->type);

	  digi_center1.push_back(digi->det_id.center1);
	  digi_center2.push_back(digi->det_id.center2);
	  digi_bar_direction.push_back(digi->det_id.bar_direction);
	  digi_layer_direction.push_back(digi->det_id.normal);
	  digi_LayerID.push_back(digi->det_id.layerID);
	  digi_detID.push_back(digi->det_id.detectorID);
      for (auto hit : digi->hits){
        digi_hit_indices.push_back(hit->index);
      }
    }
}


class GeometryHandler { 
public:
	//PUT ALL INPUT AND OUTPUT BRANCHES HERE
	TTree* InputTree;
	TFile* InputFile;
	int index = -1;
	int NumEntries;
	bool _Null = false;

//__BRANCHES______________________________________________________________________
	//All in CMS coordinates
	//starts and ends of the layer dimensions
	Double_t X_START;
	Double_t X_END;
	Double_t Z_START;
	Double_t Z_END;
	Double_t Wall_START;
	Double_t Wall_END;
	Double_t BACK_START;
	Double_t BACK_END;
	Double_t LENGTH;
	Double_t WIDTH;
	Double_t THICKNESS;
	//starts/ends of each module in said dimension
	std::vector<double> *MODULE_X = nullptr;
	std::vector<double> *LAYERS_Y = nullptr;
	std::vector<double> *MODULE_Z = nullptr;
	std::vector<double> *FLOOR_Y = nullptr;
	std::vector<double> *WALL_Z = nullptr;
	std::vector<double> *BACK_Z = nullptr;


	bool IsNull(){return _Null;}

	int LoadGeometry(){
		if (InputTree == nullptr) return -1;
		InputTree->GetEntry(0);
		return 0;
	}

	double GetXStart(){return X_START;}
	double GetXEnd(){return X_END;}
	double GetZStart(){return Z_START;}
	double GetZEND(){return Z_END;}
	double GetWallStart(){return Wall_START;}
	double GetWallEnd(){return Wall_END;}
	double GetBackStart(){return BACK_START;}
	double GetBackEnd(){return BACK_END;}
	double GetLength(){return LENGTH;}
	double GetWidth(){return WIDTH;}
	double GetThickness(){return THICKNESS;}
	std::vector<double>& GetXModule(){return *MODULE_X;}
	std::vector<double>& GetYLayers(){return *LAYERS_Y;}
	std::vector<double>& GetZModule(){return *MODULE_Z;}
	std::vector<double>& GetYFloor(){return *FLOOR_Y;}
	std::vector<double>& GetZWalls(){return *WALL_Z;}
	std::vector<double>& GetZBack(){return *BACK_Z;}

	GeometryHandler(TString input_tree_name, TString input_file_name) {
		InputFile = TFile::Open(input_file_name);
		if (! InputFile) {
      		_Null = true;
      		return;
    	}

		InputTree = (TTree*) InputFile->Get(input_tree_name);

    	if (! InputTree){
      		_Null = true;
      		return;
    	} 

		InputTree->SetBranchAddress("X_START", &X_START);
 		InputTree->SetBranchAddress("X_END", &X_END);
 		InputTree->SetBranchAddress("Z_START", &Z_START);
 		InputTree->SetBranchAddress("Z_END", &Z_END);
 		InputTree->SetBranchAddress("Wall_START", &Wall_START);
 		InputTree->SetBranchAddress("Wall_END", &Wall_END);
 		InputTree->SetBranchAddress("BACK_START", &BACK_START);
 		InputTree->SetBranchAddress("BACK_END", &BACK_END);
		InputTree->SetBranchAddress("LENGTH", &LENGTH);
		InputTree->SetBranchAddress("WIDTH", &WIDTH);
		InputTree->SetBranchAddress("THICKNESS", &THICKNESS);

		InputTree->SetBranchAddress("MODULE_X", &MODULE_X);
		InputTree->SetBranchAddress("LAYERS_Y", &LAYERS_Y);
		InputTree->SetBranchAddress("MODULE_Z", &MODULE_Z);
		InputTree->SetBranchAddress("FLOOR_Y", &FLOOR_Y);
		InputTree->SetBranchAddress("WALL_Z", &WALL_Z);
		InputTree->SetBranchAddress("BACK_Z", &BACK_Z);
	}

};

#endif
