#include "geometry/Box.hh"

#include <G4SubtractionSolid.hh>
#include <tls.hh>

#include "action.hh"
#include "analysis.hh"
#include "geometry/Earth.hh"
#include "physics/Units.hh"
#include "tracking.hh"
#include "geometry/Cavern.hh"
#include <G4IntersectionSolid.hh>
#include <G4UnionSolid.hh>
#include <G4SubtractionSolid.hh>
#include "TROOT.h"
#include "TTree.h"
#include "MuonDataController.hh"

#include <iostream>
#include <string>
#include <fstream>

#define UNUSED(x) (void)(x)
using dimension = double;

namespace MATHUSLA { namespace MU {

namespace Box { //////////////////////////////////////////////////////////////////////////////////////////////////////


namespace Box::CavernConstruction{ ////////////////////////////////////////////////////////////////////////////////////

//__Check Between-Ness__________________________________________________________________________
bool _between(const double min_layer,
              const double max_layer,
              const double target) {
  return min_layer < target && target < max_layer;
}
//----------------------------------------------------------------------------------------------

//__Calculate Subtraction of Volumes____________________________________________________________
G4LogicalVolume* _calculate_modification(const std::string& name,
                                         G4LogicalVolume* earth_component,
                                         const double base_depth,
                                         const double top_depth) {
  return Construction::Volume(new G4SubtractionSolid(name,
    earth_component->GetSolid(),
    Cavern::Volume()->GetSolid(),
    Construction::Transform(0, 1.7 * m, -0.5 * (base_depth - top_depth) + Cavern::CenterDepth() - top_depth)),
    earth_component->GetMaterial());
}

}



namespace { ////////////////////////////////////////////////////////////////////////////////////

//__Box Sensitive Material______________________________________________________________________
std::vector<Scintillator*> _scintillators;
G4LogicalVolume* _steel;
//----------------------------------------------------------------------------------------------

//__Box Hit Collection__________________________________________________________________________
G4ThreadLocal Tracking::HitCollection* _hit_collection;
//----------------------------------------------------------------------------------------------

//__Box Specification Variables_________________________________________________________________

constexpr int scintillators_per_layer{64};
constexpr int NMODULES{16};
constexpr int n_top_layers{4};
constexpr int n_floor_layers{2};
constexpr auto x_edge_length = 39.0*m;
constexpr auto y_edge_length = 39.0*m;
constexpr auto x_displacement = 100.0*m;
constexpr auto y_displacement = -19.5*m;
constexpr auto z_displacement = 8547.0*cm;

constexpr auto layer_x_edge_length = 9.0*m;
constexpr auto layer_y_edge_length = 9.0*m;

constexpr auto scint_x_edge_length = 4.5*m;
constexpr auto scint_y_edge_length = 0.045*m;
constexpr auto scintillator_height = 0.01*m;

constexpr auto shelf_thickness = 0.002*m;
constexpr auto shelf_x_edge_length = 9.0*m;
constexpr auto shelf_y_edge_length = 9.0*m;

constexpr auto steel_height = 0.03*m;

constexpr auto air_gap = 16*m; // Total volume height that has air inside
constexpr auto air_gap_decay = 12.6*m; // height of decay volume

constexpr auto scintillator_casing_thickness = 0.003*m;

constexpr auto layer_spacing = 0.8*m;
constexpr auto layer_spacing_top = 0.8*m; // Top layers have different spacing
constexpr auto layer_count   = 4UL;

constexpr auto module_x_edge_length = 9.0*m;
constexpr auto module_y_edge_length = 9.0*m;
constexpr auto module_case_thickness = 0.02*m;
constexpr auto module_spacing = 1.0*m;

constexpr auto full_layer_height = scintillator_height + 2*scintillator_casing_thickness;
constexpr auto wall_gap = 0.01*m;
constexpr auto wall_gap2 = 1.0*m;
//increase detector volume for walls - symmetric, so its size is not that relevant.
constexpr auto x_edge_increase = 4*full_layer_height + 2*wall_gap2 + 4*wall_gap;

constexpr auto layer_w_case = full_layer_height;

constexpr auto full_module_height =  air_gap_decay + 4.0*layer_w_case + 3.0*layer_spacing_top;

constexpr auto scintillator_z_position = 0.00;

constexpr auto wall_height = (2*full_layer_height + layer_spacing + air_gap_decay);

constexpr int NBEAMLAYERS = 4;
constexpr auto beam_x_edge_length = 0.10*m;
constexpr auto beam_y_edge_length = 0.10*m;
constexpr auto beam_thickness = 0.02*m;

constexpr auto full_detector_height = full_module_height + steel_height + n_floor_layers*layer_w_case + (n_floor_layers - 1)*layer_spacing;
constexpr auto half_detector_height = 0.5L * full_detector_height;

constexpr auto layer_z_displacement = [](){
	std::array<double, n_top_layers> arr{};
	for (int i = 0; i < n_top_layers; i++){
		arr[i] = -0.5*full_module_height + air_gap_decay + i*layer_spacing_top + (i + 0.5)*layer_w_case;
	}
	return arr;
}();

// The z coordinates of the BOTTOM of all layers in the world (from the ground)
constexpr auto layer_z_world = [](){
	std::array<double, n_top_layers + n_floor_layers> arr{};
	for (int i = 0; i < n_floor_layers; i++){
		arr[i] = steel_height + i*(layer_spacing + layer_w_case);
	}
	for (int i = n_floor_layers; i < n_floor_layers + n_top_layers; i++){
		arr[i] = steel_height + n_floor_layers*layer_w_case + (n_floor_layers - 1)*layer_spacing; //The bottom floor part
		arr[i] = arr[i] + air_gap_decay + (i-n_floor_layers)*(layer_spacing + layer_w_case);
	}
	return arr;
}();


constexpr auto module_beam_heights = [](){
	std::array<double, n_top_layers> arr{};
	for (int i=0; i < n_top_layers; i++){
		if (i==0){arr[i] = air_gap_decay;}
		else {arr[i] = layer_spacing_top;}
	}
	return arr;
}();

constexpr auto module_beam_z_pos = [](){
	std::array<double, n_top_layers> arr{};
	for (int i=0; i < n_top_layers; i++){
		arr[i] = -0.5*full_module_height + i*layer_w_case + 0.5*module_beam_heights[i];
		if (i > 0){arr[i] += (i-1)*layer_spacing_top + air_gap_decay;}
	}
	return arr;
}();

//Returns the LOWEST position value of each module
auto get_module_x_displacement(int tag_number){
	int step = int(sqrt(NMODULES));
	return -0.5*x_edge_length + 0.5*module_x_edge_length + double(tag_number % step)*(module_x_edge_length + module_spacing);
}


auto get_module_y_displacement(int tag_number){
	int step = int(sqrt(NMODULES));
  return double(tag_number % step)*(module_y_edge_length + module_spacing) -0.5 * y_edge_length + 0.5*module_y_edge_length;
}

auto get_layer_z_displacement(int layer_number){
  return -1.0*layer_z_displacement[layer_number];
}

//----------------------------------------------------------------------------------------------

} /* anonymous namespace */ ////////////////////////////////////////////////////////////////////

//__Box Data Variables__________________________________________________________________________
const std::string& Detector::DataName = "box_run";
const Analysis::ROOT::DataKeyList Detector::DataKeys = Analysis::ROOT::DefaultDataKeyList;
const Analysis::ROOT::DataKeyTypeList Detector::DataKeyTypes = Analysis::ROOT::DefaultDataKeyTypeList;
bool Detector::SaveAll = false;
//----------------------------------------------------------------------------------------------

//__Detector Constructor________________________________________________________________________
Detector::Detector() : G4VSensitiveDetector("MATHUSLA/MU/Box") {
  collectionName.insert("Box_HC");
  for (auto& scintillator : _scintillators)
    scintillator->Register(this);

    // Print out the detector dislacement in z direction
  std::cout<<"Layer z displacement [cm]:\n   world coord.---- CMS coord.\n"<<std::endl;
  for (auto& layer_y : layer_z_world)
      std::cout<< "y =  "<<layer_y/Units::Length<<",\t\t"<<  (-layer_y+Box_IP_Depth)/Units::Length <<std::endl;
  std::cout<<"----------------------\n \n"<<std::endl;

  // std::string prefix = ".";
  // Detector::SaveInfo(prefix);

}
//----------------------------------------------------------------------------------------------

//__Detector Constructor________________________________________________________________________
void Detector::SaveInfo(std::string & prefix){
  //open file for writing
  std::cout<<"Save infomration at: "<< prefix+"_SimInfo.txt" <<std::endl;
  std::ofstream fw(prefix+"_SimInfo.txt", std::ofstream::out);

  //check if file was successfully opened for writing
  if (fw.is_open())
  {
    //store array contents to text fill
    for (auto& layer_y : layer_z_world)
      fw<< '[' << (-layer_y+Box_IP_Depth+scintillator_casing_thickness)/Units::Length << ", "<<  (-layer_y+Box_IP_Depth+scintillator_casing_thickness+scintillator_height)/Units::Length<< "]\n";
    fw.close();
  }
  else std::cout << "Problem with opening file";
  fw.close();  
}

//----------------------------------------------------------------------------------------------



//__Initalize Event_____________________________________________________________________________
void Detector::Initialize(G4HCofThisEvent* event) {
  _hit_collection = Tracking::GenerateHitCollection(this, event);
}
//----------------------------------------------------------------------------------------------

//__Hit Processing______________________________________________________________________________
G4bool Detector::ProcessHits(G4Step* step, G4TouchableHistory*) {
  const auto deposit = step->GetTotalEnergyDeposit();


  if (deposit == 0.0L){
	  //    pre_data->Fill();
    return false;
  }


////////////////////////
  const auto track      = step->GetTrack();
  const auto step_point = step->GetPostStepPoint();
  const auto particle   = track->GetParticleDefinition();
  const auto trackID    = track->GetTrackID();
  const auto parentID   = track->GetParentID();
  const auto position   = G4LorentzVector(step_point->GetGlobalTime(), step_point->GetPosition());
  const auto momentum   = G4LorentzVector(step_point->GetTotalEnergy(), step_point->GetMomentum());

  //______Tranfomation to CMS Coordinates_____________________________________________________
  const auto transformed_z = -(position.z() - Box_IP_Depth);
  const auto position_transformed = G4ThreeVector(position.y(), transformed_z, position.x());
  const auto new_position = G4LorentzVector(step_point->GetGlobalTime(), position_transformed);

  const auto transformed_pz = -(momentum.pz());
  const auto momentum_transformed = G4ThreeVector(momentum.py(), transformed_pz, momentum.px());
  const auto new_momentum = G4LorentzVector(step_point->GetTotalEnergy(), momentum_transformed);
  //__________________________________________________________________________________________

  const auto local_position = new_position.vect() - G4ThreeVector(y_displacement, z_displacement, x_displacement);

  size_t y_index = 0;


	double layer_y_global[n_top_layers + n_floor_layers];
	for (int i = 0; i < n_top_layers + n_floor_layers; i++){
		layer_y_global[i] = layer_z_world[i] + Box_IP_Depth;
	}
	
	for (int i = 0; i < n_top_layers + n_floor_layers; i++){
		if (new_position.y() >= layer_y_global[i] && new_position.y() <= layer_y_global[i] + layer_w_case){
			y_index = i;
			break;
		}
	}


	size_t x_index = 0;
	size_t z_index = 0;

	double layer_x_global[int(sqrt(NMODULES))];//we assume detector is a square
	for (int i = 0; i < int(sqrt(NMODULES)); i++) {
		layer_x_global[i] = get_module_y_displacement(i); //CMS coordinates
	}

	double layer_z_global[int(sqrt(NMODULES))];
	for (int i = 0; i < int(sqrt(NMODULES)); i++) {
		layer_z_global[i] = get_module_x_displacement(i) + x_displacement;//CMS coordinates
	}

	bool x_long = y_index % 2; // Even -> x-direction is long
	if (x_long) {
		for (int i = 0; i < int(sqrt(NMODULES)); i++){
			if (new_position.x() >= layer_x_global[i] && new_position.x() <= layer_x_global[i] + module_x_edge_length){
				x_index = std::floor(i*(module_y_edge_length/scint_x_edge_length)); // i * scintillator bars per module
		    	x_index = x_index + (std::floor((+local_position.x() - i*(module_spacing + module_x_edge_length))/(scint_x_edge_length)));
			}
			if (new_position.z() >=layer_z_global[i] && new_position.z() <= layer_z_global[i] + module_y_edge_length){
				z_index = std::floor(i*(module_x_edge_length/scint_y_edge_length)); // i * scintillator bars per module
		    	z_index = z_index + (std::floor((+local_position.z() - i*(module_spacing + module_y_edge_length))/(scint_y_edge_length))); // The current module
			}
		}
	} else { // z-direction is long
		for (int i = 0; i < int(sqrt(NMODULES)); i++){
			if (new_position.x() >= layer_x_global[i] && new_position.x() <= layer_x_global[i] + module_y_edge_length){
				x_index = std::floor(i*(module_x_edge_length/scint_y_edge_length)); // i * scintillator bars per module
		    	x_index = x_index + (std::floor((+local_position.x() - i*(module_spacing + module_x_edge_length))/(scint_y_edge_length)));
			}
			if (new_position.z() >=layer_z_global[i] && new_position.z() <= layer_z_global[i] + module_x_edge_length){
				z_index = std::floor(i*(module_x_edge_length/scint_x_edge_length)); // i * scintillator bars per module
		    	z_index = z_index + (std::floor((+local_position.z() - i*(module_spacing + module_y_edge_length))/(scint_x_edge_length))); // The current module
			}
		}
	}

  const auto x_name = std::to_string(x_index);
  const auto z_name = std::to_string(z_index);

  _hit_collection->insert(new Tracking::Hit(
    particle,
    trackID,
    parentID,
	std::to_string(1UL + y_index)
    + (z_index < 10UL ? "000" + z_name : (z_index < 100UL ? "00" + z_name : (z_index < 1000UL ? "0" + z_name : z_name)))
	+ (x_index < 10UL ? "000" + x_name : (x_index < 100UL ? "00" + x_name : (x_index < 1000UL ? "0" + x_name : x_name))),
    deposit / Units::Energy,
    G4LorentzVector(new_position.t() / Units::Time,   new_position.vect() / Units::Length),
    G4LorentzVector(new_momentum.e() / Units::Energy, new_momentum.vect() / Units::Momentum)));

  return true;
}
//----------------------------------------------------------------------------------------------

//__Post-Event Processing_______________________________________________________________________
void Detector::EndOfEvent(G4HCofThisEvent*) {
  if (_hit_collection->GetSize() == 0)
    return;  
 
  MuonDataController* controller = MuonDataController::getMuonDataController();
  if(controller->getOn() ==true){
    if(controller->getDecayInEvent() == false){
      return;
      }
    if(controller->getDecayInZone() == false){
      G4cout<<"Decay In Zone is false"<<G4endl;
      return;
      }
     G4cout<<"Decay in zone is true"<<G4endl;
    }
 
  const auto collection_data = Tracking::ConvertToAnalysis(_hit_collection);

  Analysis::ROOT::DataEntryList root_data;
  root_data.reserve(24UL);
  root_data.push_back(collection_data[0]);
  root_data.push_back(collection_data[1]);
  root_data.push_back(collection_data[2]);
  root_data.push_back(collection_data[3]);
  root_data.push_back(collection_data[4]);
  root_data.push_back(collection_data[5]);
  root_data.push_back(collection_data[6]);
  root_data.push_back(collection_data[7]);
  root_data.push_back(collection_data[8]);
  root_data.push_back(collection_data[9]);
  root_data.push_back(collection_data[10]);
  root_data.push_back(collection_data[11]);
  root_data.push_back(collection_data[12]);
  root_data.push_back(collection_data[13]);

  const auto gen_particle_data = Tracking::ConvertToAnalysis(GeneratorAction::GetLastEvent(), true);
  const auto extra_gen_data = Tracking::ConvertToAnalysis(GeneratorAction::GetGenerator()->ExtraDetails());
  root_data.insert(root_data.cend(), gen_particle_data.cbegin(), gen_particle_data.cend());
  root_data.insert(root_data.cend(), extra_gen_data.cbegin(), extra_gen_data.cend());

  Analysis::ROOT::DataEntry metadata;
  metadata.reserve(2UL);
  metadata.push_back(collection_data[0UL].size());
  metadata.push_back(gen_particle_data[0UL].size());

  Analysis::ROOT::FillNTuple(DataName, Detector::DataKeyTypes, metadata, root_data);
  if (verboseLevel >= 2 && _hit_collection)
    std::cout << *_hit_collection;
}
//----------------------------------------------------------------------------------------------

//Build 1 Module for detector
G4VPhysicalVolume* Detector::ConstructScintillatorLayer(G4LogicalVolume* ModuleVolume, int module_number, int layer_number, dimension module_x_displacement, dimension module_y_displacement, dimension layer_z_displacement){

    auto current = new Scintillator("M" + std::to_string(module_number) + "L" + std::to_string(layer_number),
      layer_x_edge_length,
      layer_y_edge_length,
      full_layer_height,
      scintillator_casing_thickness);

      _scintillators.push_back(current);

      UNUSED(module_x_displacement);
      UNUSED(module_y_displacement);

  return current->PlaceIn(ModuleVolume, G4Translate3D(0.0, 0.0, layer_z_displacement) );
  //G4Translate3D(0.5*layer_x_edge_length, 0.5*layer_y_edge_length, layer_z_displacement)
}

G4VPhysicalVolume* Detector::ConstructShelf(G4LogicalVolume* ModuleVolume, int module_number, int layer_number, dimension module_x_displacement, dimension module_y_displacement, dimension layer_z_displacement){

	auto Shelf = Construction::BoxVolume("M" + std::to_string(module_number) + "SL" + std::to_string(layer_number), shelf_x_edge_length, shelf_y_edge_length, shelf_thickness, Construction::Material::Aluminum, Construction::AlAttributes());

	return Construction::PlaceVolume(Shelf, ModuleVolume, Construction::Transform(-0.50*module_x_edge_length + 0.50*shelf_x_edge_length,
																				-0.50*module_y_edge_length + 0.50*shelf_y_edge_length,
																				 layer_z_displacement + 0.5*layer_w_case +0.5*shelf_thickness));

  //G4Translate3D(0.5*layer_x_edge_length, 0.5*layer_y_edge_length, layer_z_displacement)
}

G4VPhysicalVolume* Detector::ConstructModule(G4LogicalVolume* DetectorVolume, int tag_number, dimension detector_x, dimension detector_y, dimension detector_z){

	auto ModuleVolume = Construction::BoxVolume("Module" + std::to_string(tag_number), module_x_edge_length + module_case_thickness, module_y_edge_length + module_case_thickness, full_module_height);

	// auto ModuleCaseVolume = Construction::OpenBoxVolume("Module" + std::to_string(tag_number) + "_Case", module_x_edge_length, module_y_edge_length, full_module_height,
	//                                                 module_case_thickness, Construction::Material::Iron, *ModuleVisAttr());

	// Construction::PlaceVolume(ModuleCaseVolume, ModuleVolume, Construction::Transform(0.0, 0.0, 0.0));


	for (std::size_t layer{}; layer < layer_count; ++layer) {
		auto current = Detector::ConstructScintillatorLayer(ModuleVolume, tag_number, layer,
															0*m,
															0*m,
															get_layer_z_displacement(layer));
		//Inserting the Shelves below the scintillators
		auto currentShelf = Detector::ConstructShelf(ModuleVolume, tag_number, layer, 0*m, 0*m, get_layer_z_displacement(layer));
    UNUSED(current);
	}

  UNUSED(detector_x);
  UNUSED(detector_y);
  UNUSED(detector_z);


	//CONSTRUCTING AND INSERTING STEEL BEAMS

	for (int beam_layer = 0; beam_layer < NBEAMLAYERS; beam_layer++){
		auto BeamL1 = Construction::OpenBoxVolume("Module" + std::to_string(tag_number) + "BL" + std::to_string(beam_layer) + "PL1", beam_x_edge_length, beam_y_edge_length, module_beam_heights[beam_layer],
												  beam_thickness, Construction::Material::Iron, Construction::CasingAttributes());
		auto BeamL2 = Construction::OpenBoxVolume("Module" + std::to_string(tag_number) + "BL" + std::to_string(beam_layer) + "PL2", beam_x_edge_length, beam_y_edge_length, module_beam_heights[beam_layer],
												  beam_thickness, Construction::Material::Iron, Construction::CasingAttributes());
		auto BeamR1 = Construction::OpenBoxVolume("Module" + std::to_string(tag_number) + "BL" + std::to_string(beam_layer) + "PR1", beam_x_edge_length, beam_y_edge_length, module_beam_heights[beam_layer],
												  beam_thickness, Construction::Material::Iron, Construction::CasingAttributes());
		auto BeamR2 = Construction::OpenBoxVolume("Module" + std::to_string(tag_number) + "BL" + std::to_string(beam_layer) + "PR2", beam_x_edge_length, beam_y_edge_length, module_beam_heights[beam_layer],
												  beam_thickness, Construction::Material::Iron, Construction::CasingAttributes());

		Construction::PlaceVolume(BeamL1, ModuleVolume, Construction::Transform(-0.50*module_x_edge_length + 0.50*beam_x_edge_length,
																				-0.50*module_y_edge_length + 0.50*beam_y_edge_length,
																				-1.0*module_beam_z_pos[beam_layer]));
		Construction::PlaceVolume(BeamL2, ModuleVolume, Construction::Transform(-0.50*module_x_edge_length + 0.50*beam_x_edge_length,
																				0.50*module_y_edge_length - 0.50*beam_y_edge_length,
																				-1.0*module_beam_z_pos[beam_layer]));
		Construction::PlaceVolume(BeamR1, ModuleVolume, Construction::Transform(0.50*module_x_edge_length - 0.50*beam_x_edge_length,
																				-0.50*module_y_edge_length + 0.50*beam_y_edge_length,
																				-1.0* module_beam_z_pos[beam_layer]));
		Construction::PlaceVolume(BeamR2, ModuleVolume, Construction::Transform(0.50*module_x_edge_length - 0.50*beam_x_edge_length,
																				0.50*module_y_edge_length - 0.50*beam_y_edge_length,
																				-1.0* module_beam_z_pos[beam_layer]));
	}


	// if (tag_number == 0) {
	// 	std::cout << "ABOUT TO WRITE GDML FOR MODULE" << std::endl;
	// 	Construction::Export(ModuleVolume, folder, file2, arg4 );
	// }


    return Construction::PlaceVolume(ModuleVolume, DetectorVolume,
									 Construction::Transform(get_module_x_displacement(tag_number),
															 get_module_y_displacement(tag_number),
                                                             half_detector_height - steel_height - n_floor_layers*layer_w_case - (n_floor_layers -1)*layer_spacing  - 0.5*full_module_height,
															 0.0, 0.0, 1.0, 0.0));


}

//__Build Detector______________________________________________________________________________
G4VPhysicalVolume* Detector::Construct(G4LogicalVolume* world) {
	Scintillator::Material::Define();
	_scintillators.clear();
	// pre_data->Branch("X_S", &X_POS_STEP, "X_S/D");
	// pre_data->Branch("Y_S", &Y_POS_STEP, "Y_S/D");
	// pre_data->Branch("X_H", &X_POS_HIT, "X_H/D");
	// pre_data->Branch("Y_H", &Y_POS_HIT, "Y_H/D");

	auto DetectorVolume = Construction::BoxVolume("Box", x_edge_length + x_edge_increase, y_edge_length, full_detector_height,
												  Construction::Material::Air, G4VisAttributes::GetInvisible());

	//DetectorVolume->SetVisAttributes(G4VisAttributes::Invisible);

	for (int module_number = 0; module_number < NMODULES; module_number++){
		Detector::ConstructModule(DetectorVolume, module_number,
					   0.5L, //These arguments are not actually used
					   0.5L,
					   0.0L);
	}

    auto first_hermetic_floor = new Scintillator("HF1",
                                                 x_edge_length,
                                                 y_edge_length,
                                                 full_layer_height,
                                                 scintillator_casing_thickness);
    _scintillators.push_back(first_hermetic_floor);
    first_hermetic_floor->PlaceIn(DetectorVolume, G4Translate3D(0.0, 0.0, half_detector_height - 0.5*layer_w_case - steel_height));

    auto second_hermetic_floor = new Scintillator("HF2",
                                                 x_edge_length,
                                                 y_edge_length,
                                                 full_layer_height,
                                                 scintillator_casing_thickness);
    _scintillators.push_back(second_hermetic_floor);
    second_hermetic_floor->PlaceIn(DetectorVolume, G4Translate3D(0.0, 0.0, half_detector_height - 1.5*layer_w_case - layer_spacing - steel_height));

 //   auto third_hermetic_floor = new Scintillator("HF3",
 //                                               x_edge_length,
 //                                                y_edge_length,
 //                                                full_layer_height,
 //                                                scintillator_casing_thickness);
 //   _scintillators.push_back(third_hermetic_floor);
 //   third_hermetic_floor->PlaceIn(DetectorVolume, G4Translate3D(0.0, 0.0, half_detector_height - 2.5*layer_w_case - 2*layer_spacing - steel_height));

    auto hermetic_wall = new Scintillator("HW1",
                                            full_layer_height,
                                            y_edge_length,
                                            wall_height,
                                            scintillator_casing_thickness);                                                                      
    _scintillators.push_back(hermetic_wall);
    hermetic_wall->PlaceIn(DetectorVolume, G4Translate3D(-0.5L*x_edge_length - 0.5L*full_layer_height - wall_gap, 0.0, half_detector_height -  0.5L*wall_height));

    // Tom: add a second layer of wall
    auto hermetic_wall2 = new Scintillator("HW2",
                                            full_layer_height,
                                            y_edge_length,
                                            wall_height,
                                            scintillator_casing_thickness);                                                                      
    _scintillators.push_back(hermetic_wall2);
    hermetic_wall2->PlaceIn(DetectorVolume, G4Translate3D(-0.5L*x_edge_length - 1.5L*full_layer_height - wall_gap - wall_gap2, 0.0, half_detector_height -  0.5L*wall_height));
     
    _steel = Construction::BoxVolume("SteelPlate",
			 x_edge_length, y_edge_length, steel_height,
			 Construction::Material::Iron,
			 Construction::CasingAttributes());
	Construction::PlaceVolume(_steel, DetectorVolume, Construction::Transform(0.0, 0.0, half_detector_height - 0.5*steel_height));

	//	Construction::Export(DetectorVolume, folder, file, arg4 );

	return Construction::PlaceVolume(DetectorVolume, world,
		   Construction::Transform(0.5L*x_edge_length + x_displacement, 0.5L*y_edge_length + y_displacement, -0.50*full_detector_height));

}


//----------------------------------------------------------------------------------------------

namespace CMS{

  constexpr auto ____DEFINE_ME____   = 0.0*m;

  constexpr auto earth_total_depth   = 4530.0L*cm + 1825.0L*cm + 3645.0L*cm;

  constexpr auto uxc55_cavern_length = 53.0*m;
  constexpr auto uxc55_inner_radius  = 13.250*m;
  constexpr auto uxc55_outer_radius  = 14.530*m;
  constexpr auto IPDepth             = earth_total_depth - uxc55_outer_radius;
  constexpr auto _base_depth         = earth_total_depth;
  constexpr auto access_shaft_x      = -14.00*m;
  constexpr auto access_shaft_y      = 0.00*m;
  constexpr auto access_shaft_z      = -1.0* uxc55_outer_radius;

  constexpr auto CMSSteelThickness   = 1.48L*m;
  constexpr auto CMSDetectorLength   = 20.00L*m;
  constexpr auto CMSDetectorRadius   = 8.00L*m;

  constexpr auto AS_Depth            = 20.50*m;
  constexpr auto AS_Width            = 20.50*m;
  constexpr auto AS_Height           = earth_total_depth - 2 * uxc55_outer_radius;
  constexpr auto AS_Thickness        = 1.5*m;


  G4LogicalVolume* EarthVolume() {
    using namespace Earth;

	auto earth_box = Construction::Box("", LayerWidthX(), LayerWidthY(), TotalDepth());

	auto modified = new G4SubtractionSolid("",
										   earth_box,
										   Construction::Box("AirBox", x_edge_length + x_edge_increase, y_edge_length, air_gap),
										   Construction::Transform(0.5L*x_edge_length + x_displacement,
																   0.5L*y_edge_length + y_displacement,
																   0.5L*(air_gap-Earth::TotalDepth()) -9.50*m ));

    return Construction::Volume(modified);
  }

  G4LogicalVolume* SandstoneVolume() {
    using namespace Earth;
    auto sandstone_box = Construction::Box("", LayerWidthX(), LayerWidthY(), SandstoneDepth());

    return Construction::Volume(sandstone_box, Material::SiO2, Construction::BorderAttributes());
  }

  long double BaseDepth() {
     return _base_depth - Earth::TotalShift();
  }

  long double TopDepth() {
    return BaseDepth() - uxc55_outer_radius;
  }
  long double CenterDepth() {
      return BaseDepth() - uxc55_outer_radius;
  }
  G4Translate3D IPTransform() {
      return G4Translate3D(0.0, 0.0, IPDepth);
  }

  G4Translate3D Access_Shaft_Transform(){
    return G4Translate3D(access_shaft_x, access_shaft_y, static_cast<long double>(access_shaft_z));
  }

  G4Translate3D Cavern_Transform(){
    return G4Translate3D(0, 0, -0.5 * Earth::TotalDepth() + IPDepth);
       //* Construction::Rotate(0, 1, 0, 90*deg))
  }


  G4LogicalVolume* CMSRingVolume() {
      return Construction::Volume(Construction::Cylinder("DetectorRing",
           CMSDetectorLength, CMSDetectorRadius - CMSSteelThickness, CMSDetectorRadius),
           Construction::Material::Iron,
           Construction::CasingAttributes());
  }

  G4LogicalVolume* CMSVolume(){
    using namespace Construction;
    auto cavern = Cylinder("cavern", uxc55_cavern_length, 0.0*m, uxc55_outer_radius);

    auto access_shaft = Construction::Box("shaft", AS_Width, AS_Depth, AS_Height );

    return Construction::Volume(new G4UnionSolid("fake_cms",
                    access_shaft,
                    cavern,
                    Construction::Rotate(1, 0, 0, 90*deg)
                    *G4Translate3D(0.0, -1.0*static_cast<long double>(uxc55_outer_radius + 0.5*AS_Height), -0.5*uxc55_cavern_length + 0.5*AS_Width) ));

  }

  //__Calculate Subtraction of Volumes____________________________________________________________
  G4LogicalVolume* _calculate_modification(const std::string& name,
                                         G4LogicalVolume* earth_component,
                                         const double base_depth,
                                         const double top_depth) {
    return Construction::Volume(new G4SubtractionSolid(name,
      earth_component->GetSolid(),
      CMSVolume()->GetSolid(),
      Construction::Transform(-0.5*uxc55_cavern_length + 0.5*AS_Width, 0.0, 0.0)
      *Construction::Rotate(0, 0, 1, 90*deg)
      *Construction::Transform(0.0, 0.0, -0.5 * (base_depth - top_depth) + CenterDepth() - top_depth - uxc55_outer_radius - 0.5*AS_Height)),
      earth_component->GetMaterial());
  }

} // NAMESPACE CMS


//__Build Earth for Detector____________________________________________________________________
G4VPhysicalVolume* Detector::ConstructEarth(G4LogicalVolume* world){

    //EDIT EARTH TO ACCOMODATE VOLUME
	using namespace Box::CavernConstruction;
	using namespace Construction;
	using namespace CMS;

	Earth::Material::Define();

	auto earth = CMS::EarthVolume();

	const auto mix_top = Earth::TotalDepth() - Earth::MixDepth();
	const auto marl_top = mix_top - Earth::MarlDepth();
	const auto sandstone_top = marl_top - Earth::SandstoneDepth();

   	Construction::PlaceVolume(CMS::_calculate_modification("modified_mix", Earth::MixVolume(),
							  mix_top + Earth::MixDepth(), mix_top),
							  earth, Earth::MixTransform());

  	Construction::PlaceVolume(CMS::_calculate_modification("modified_marl", Earth::MarlVolume(),
							  marl_top + Earth::MarlDepth(), marl_top),
							  earth, Earth::MarlTransform());

	auto sandstone = CMS::_calculate_modification("modified_sandstone", CMS::SandstoneVolume(),
												  sandstone_top + Earth::SandstoneDepth(), sandstone_top);

	/////////////// UXC55 AND CMS DETECTOR CONSTRUCTION ////////////////////////////////////////

	auto UXC_55_cavern_solid = Cylinder("UXC55_outer", uxc55_cavern_length, 0.0*m, uxc55_outer_radius);
	auto UXC55_outer_solid = Cylinder("UXC55_outer", uxc55_cavern_length, uxc55_inner_radius, uxc55_outer_radius);
	auto UXC55_outer_logical = Volume(UXC55_outer_solid, Construction::Material::Concrete, Construction::CasingAttributes());
	auto CMS_Detector_logical = CMSRingVolume();
	auto UXC_55_air_v1 = new G4SubtractionSolid("UXC_55_air_v1", UXC_55_cavern_solid, UXC55_outer_solid);
	auto UXC_55_air_v2 = new G4SubtractionSolid("UXC_55_air_v2", UXC_55_air_v1, CMS_Detector_logical->GetSolid());
	auto UXC55_air_logical = Volume("UXC55_air", UXC_55_air_v2, Construction::Material::Air, G4VisAttributes::GetInvisible());

	Construction::PlaceVolume(UXC55_outer_logical, earth, Cavern_Transform()*Construction::Rotate(0, 1, 0, 90*deg) );
	Construction::PlaceVolume(CMS_Detector_logical, earth, Cavern_Transform()*Construction::Rotate(0, 1, 0, 90*deg) );
	Construction::PlaceVolume(UXC55_air_logical, earth, Cavern_Transform()*Construction::Rotate(0, 1, 0, 90*deg) );


	/////////////// ACCESS SHAFT CONSTRUCTION ////////////////////////////////////////

	//MAKE WHOLE SHAFT HERE

	auto Access_Shaft_outer_logical = OpenBoxVolume("Access_Shaft_outer",
                                                    AS_Width,
													AS_Depth,
													AS_Height,
													AS_Thickness,
													Construction::Material::Concrete,
													Construction::CasingAttributes());

	auto Access_Shaft_Air = BoxVolume("Acess_Shaft_Air",
									  AS_Width - 2* AS_Thickness,
									  AS_Depth - 2* AS_Thickness,
									  AS_Height - 2* AS_Thickness,
									  Construction::Material::Air,
									  G4VisAttributes::GetInvisible());

	Construction::PlaceVolume(Access_Shaft_outer_logical, earth, Access_Shaft_Transform() );
	Construction::PlaceVolume(Access_Shaft_Air, earth, Access_Shaft_Transform());

	auto modified = Construction::Volume(new G4SubtractionSolid("ModifiedSandstone",
																sandstone->GetSolid(),
																Construction::Box("AirBox", x_edge_length + x_edge_increase, y_edge_length, air_gap),
																Construction::Transform(0.5L*x_edge_length + x_displacement,
																0.5L*y_edge_length + y_displacement,
															    0.5L*(air_gap-Earth::SandstoneDepth()) - 9.50*m)),
									        	                Earth::Material::SiO2);

	Construction::PlaceVolume(modified, earth, Earth::SandstoneTransform());

	//PLACE WHOLE THING IN WORLD

	//auto mod = CMS::_calculate_modification("modified_marl", Earth::MarlVolume(),
	//                      marl_top + Earth::MarlDepth(), marl_top);


	////export geometry to gdml files
	//	Construction::Export(CMSVolume(), folder, file5, arg4 );
	//	Construction::Export(earth, folder, file4, arg4 );


	//// Put Range Cuts on earth volume
    // G4Region* cut_region = new G4Region("Earth_Cut_Region");
    // cut_region->AddRootLogicalVolume(earth);
    // G4ProductionCuts* cuts = new G4ProductionCuts;
    // cuts->SetProductionCut(1.0*m,G4ProductionCuts::GetIndex("gamma"));
    // cuts->SetProductionCut(1.0*m,G4ProductionCuts::GetIndex("e-"));
    // cuts->SetProductionCut(1.0*m,G4ProductionCuts::GetIndex("e+"));
    // cuts->SetProductionCut(1.0*m,G4ProductionCuts::GetIndex("proton"));
    // cut_region->SetProductionCuts(cuts);



	return Construction::PlaceVolume(earth, world, Earth::Transform());

}


//----------------------------------------------------------------------------------------------

} /* namespace Box */ //////////////////////////////////////////////////////////////////////////



} } /* namespace MATHUSLA::MU */
