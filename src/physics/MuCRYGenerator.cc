// GEANT4
#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4RandomTools.hh" // For G4UniformRand


// CRY
#include "CRYSetup.h"
#include "CRYGenerator.h"
#include "CRYParticle.h"
#include "CRYUtils.h"

// Project
#include "physics/MuCRYGenerator.hh"

namespace MATHUSLA { namespace MU { namespace Physics { 

    //--------------------------------------------------------------------------
    // Wrapper for random number generator
    template <class T>
    class RNGWrapper
    {
    public:
        static void set(T *object, double (T::*func)(void));
        static double rng(void);

    private:
        static T *m_obj;
        static double (T::*m_func)(void);
    };

    template <class T>
    T *RNGWrapper<T>::m_obj;

    template <class T>
    double (T::*RNGWrapper<T>::m_func)(void);

    template <class T>
    void RNGWrapper<T>::set(T *object, double (T::*func)(void))
    {
        m_obj = object;
        m_func = func;
    }

    template <class T>
    double RNGWrapper<T>::rng(void) { return (m_obj->*m_func)(); }

    double GenerateRandomInRange(double min, double max) {
        if (min >= max) {
            throw std::invalid_argument("Invalid range: min must be less than max");
        }
        // Generate a random number in the range [min, max)
        return min + (max - min) * G4UniformRand();
    }   

    std::string readFileToString_CRY(const std::string &filename)
    {
        // Read the cry input file
        std::ifstream inputFile(filename);
        char buffer[1000];

        std::string setupString("");

        if (inputFile.fail())
        {
            throw std::runtime_error("Could not open file: " + filename);
        }
        else
        {
            while (!inputFile.getline(buffer, 1000).eof())
            {
                setupString.append(buffer);
                setupString.append(" ");
            }
        }

        return setupString;
    }    

    //--------------------------------------------------------------------------

    MuCRYGenerator::MuCRYGenerator(const std::string &name,
                 const std::string &description) : Generator(name, description)
    {
        // A particle gun to use with CRY
        G4int nofParticles = 1;
        fParticleGun = new G4ParticleGun(nofParticles);
        auto particleDefinition = G4ParticleTable::GetParticleTable()->FindParticle("e-");
        fParticleGun->SetParticleDefinition(particleDefinition);
        fParticleGun->SetParticlePosition(G4ThreeVector(0., 0., 20 * m));
        fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., -1.));
        fParticleGun->SetParticleEnergy(50. * MeV);


        // CRY initialization
        auto cry_setupString = readFileToString_CRY("scripts/generators/cry_default.file");

        G4cout<<cry_setupString;

        CRYSetup *cry_setup = new CRYSetup(cry_setupString, "cry_v1.7/data");
        // Set random number generator to use GEANT4 engine
        // RNGWrapper<CLHEP::HepRandomEngine>::set(CLHEP::HepRandom::getTheEngine(), &CLHEP::HepRandomEngine::flat);
        // cry_setup->setRandomFunction(RNGWrapper<CLHEP::HepRandomEngine>::rng);        
        // Make the CRY generator
        this->fCRYgenerator = new CRYGenerator(cry_setup);
        this->cry_generated = new std::vector<CRYParticle*>; // vector to hold generated particles
        // this->gen_particles_prev = new std::vector<Particle>; // vector to hold generated particles in mathusla particle format.


        // Create the table containing all particle names
        this->fparticleTable = G4ParticleTable::GetParticleTable();

        // Initialize Other parameters that is not part of CRY, but are needed to generate particle at correct points
        fCRY_additional_setup["offset_x"] = 89 * m;
        fCRY_additional_setup["offset_y"] = 0 * m;
        fCRY_additional_setup["offset_z"] = -18 * m;
        fCRY_additional_setup["offset_t_low"] = -1000 * ns;
        fCRY_additional_setup["offset_t_high"] = 1000 * ns;
    }

    // Core function 1: GeneratePrimaryVertex()
    // This will be called by GeneratorAction::GeneratePrimaries()
    void MuCRYGenerator::GeneratePrimaryVertex(G4Event *anEvent)
    {

        G4String particleName;
        bool pass_cuts = false;


        int countAttempt = 0;
        do
        {
            cry_generated->clear();
            gen_particles_prev.clear();
            fCRYgenerator->genEvent(cry_generated);

            for (unsigned j = 0; j < cry_generated->size(); j++)
            {

                particleName = CRYUtils::partName((*cry_generated)[j]->id());
                G4ParticleDefinition *particleDefinition = fparticleTable->FindParticle((*cry_generated)[j]->PDGid());

                double kinEnergy = (*cry_generated)[j]->ke() * MeV;
                if (kinEnergy >= 2670)
                    pass_cuts = true; // 2670
                else
                    countAttempt++;
            }

        } while (pass_cuts == false);

        // Sample a time for this event
        G4double t0 = GenerateRandomInRange(fCRY_additional_setup["offset_t_low"],fCRY_additional_setup["offset_t_high"]);        

        //....debug output
        G4cout << "\nEvent=" << anEvent->GetEventID() << " "
               << "CRY generated nparticles=" << cry_generated->size()
               << " pass Ekin threshold: " << pass_cuts << G4endl;

        for (unsigned j = 0; j < cry_generated->size(); j++)
        {
            particleName = CRYUtils::partName((*cry_generated)[j]->id());

            auto particleDefinition = fparticleTable->FindParticle((*cry_generated)[j]->PDGid());

            G4double fParticleEkin = (*cry_generated)[j]->ke() * MeV;
            G4double fParticleMass = particleDefinition->GetPDGMass() * MeV;
            G4double fParticleMomentum = sqrt(fParticleEkin * fParticleEkin + 2 * fParticleEkin * fParticleMass);
            G4double fParticlePosX = (*cry_generated)[j]->x() * m;
            G4double fParticlePosY = (*cry_generated)[j]->y() * m;
            G4double fParticlePosZ = (*cry_generated)[j]->z() * m;
            G4double fParticleMomentumDirectionU = (*cry_generated)[j]->u();
            G4double fParticleMomentumDirectionV = (*cry_generated)[j]->v();
            G4double fParticleMomentumDirectionW = -(*cry_generated)[j]->w();
            G4double fParticleTime = t0;//(*cry_generated)[j]->t();

            // Save this particle to list
            _particle.index = 0;
            _particle.id = (*cry_generated)[j]->PDGid();
            _particle.x = fParticlePosX + fCRY_additional_setup["offset_x"];
            _particle.y = fParticlePosY + fCRY_additional_setup["offset_y"];
            _particle.z = fParticlePosZ + fCRY_additional_setup["offset_z"];
            _particle.px  = fParticleMomentum*fParticleMomentumDirectionU;
            _particle.py  = fParticleMomentum*fParticleMomentumDirectionV;
            _particle.pz  = fParticleMomentum*fParticleMomentumDirectionW;
            _particle.t = fParticleTime;
            gen_particles_prev.push_back(_particle);

            // auto particle = MATHUSLA::MU::Physics::Particle((*cry_generated)[j]->PDGid(), fParticleTime, fParticlePosX, fParticlePosY, fParticlePosX, _particle.px, _particle.py, _particle.pz);
            // auto particle = MATHUSLA::MU::Physics::Particle((*cry_generated)[j]->PDGid(),0., 89000., 0., -18000., 0.,0., 1800.);
            // AddParticle(particle, *anEvent);
            // std::cout << "Particle generated: ";

            
            // std::cout << "fParticleDefinition: " << (*cry_generated)[j]->PDGid() << " " << std::endl;
            // std::cout << "fParticleEkin: " << fParticleEkin << " MeV" << std::endl;
            // std::cout << "fParticleMomentum: " << sqrt(fParticleEkin * fParticleEkin + 2 * fParticleEkin * fParticleMass) << " MeV" << std::endl;
            // std::cout << "fParticlePosX: " << fParticlePosX << " mm" << std::endl;
            // std::cout << "fParticlePosY: " << fParticlePosY << " mm" << std::endl;
            // std::cout << "fParticlePosZ: " << fParticlePosZ << " mm" << std::endl;
            // std::cout << "fParticleMomentumDirectionU: " << fParticleMomentumDirectionU << "" << std::endl;
            // std::cout << "fParticleMomentumDirectionV: " << fParticleMomentumDirectionV << "" << std::endl;
            // std::cout << "fParticleMomentumDirectionW: " << fParticleMomentumDirectionW << "" << std::endl;
            // std::cout << "fParticleTime: " << fParticleTime << "" << std::endl;
            // std::cout << std::endl;
            

            // fParticleGun->SetParticleDefinition(particleDefinition);
            // fParticleGun->SetParticleMomentum(sqrt(fParticleEkin * fParticleEkin + 2 * fParticleEkin * fParticleMass));
            // fParticleGun->SetParticlePosition(G4ThreeVector(fParticlePosX + fCRY_additional_setup["offset_x"],
            //                                                 fParticlePosY + fCRY_additional_setup["offset_y"],
            //                                                 fParticlePosZ + fCRY_additional_setup["offset_z"]));

            // // fParticleGun->SetParticleMomentum(1000);
            // // fParticleGun->SetParticlePosition(G4ThreeVector(89000., 0, -10000));
            // fParticleGun->SetParticleMomentumDirection(G4ThreeVector(fParticleMomentumDirectionU, fParticleMomentumDirectionV, fParticleMomentumDirectionW));
            // fParticleGun->SetParticleTime(fParticleTime);
            // fParticleGun->GeneratePrimaryVertex(anEvent);
        }


        for (std::size_t i{}; i < gen_particles_prev.size(); ++i) {
            auto particle = gen_particles_prev[i];
            AddParticle(particle, *anEvent);
        }          

    }

    // Core function 2: GeneratePrimaryVertex()
    // This is used to set generator parameters
    void MuCRYGenerator::SetNewValue(G4UIcommand *command,
                            G4String value)
    {
    }


    GenParticleVector MuCRYGenerator::GetLastEvent() const {
        GenParticleVector current_event = {};
        for (int i=0; i<gen_particles_prev.size(); i++){
            current_event.push_back(gen_particles_prev[i]);
        }
        return current_event;
    }

    std::ostream &MuCRYGenerator::Print(std::ostream &os) const {
        os << "Generator Info:\n  "
            << "Name:        " << "CRY V1.7"                           << "\n  "
            << "Description: " << "Cosmic generator"                    << "\n  "
            << "Pathname:    " << "" << "\n  ";
        return os;
    }

    const Analysis::SimSettingList MuCRYGenerator::GetSpecification() const {
        return Analysis::Settings(SimSettingPrefix,
            "",         _name,
            "_PATHNAME",  "cry");        
    }    


} } } // namespace MATHUSLA::MU::Physics