#ifndef MU__cry_hh
#define MU__cry_hh

// Geant4
#include "G4ParticleGun.hh"

// CRY
#include "CRYSetup.h"
#include "CRYGenerator.h"
#include "CRYParticle.h"
#include "CRYUtils.h"


// Project
#include "physics/Generator.hh"

#include "physics/Particle.hh"
#include "ui.hh"

#include <string>
#include <iostream>
#include <cstddef>
#include <vector>

namespace MATHUSLA { namespace MU { namespace Physics {

class MuCRYGenerator : public Generator {

    public:
        MuCRYGenerator(const std::string &name,
              const std::string &description);
        virtual ~MuCRYGenerator() = default;

        // Core function 1: GeneratePrimaryVertex()
        // This will be called by GeneratorAction::GeneratePrimaries()
        void GeneratePrimaryVertex(G4Event *event) override;

        // Core function 2: GeneratePrimaryVertex()
        // This is used to set generator parameters
        void SetNewValue(G4UIcommand *command,
                         G4String value) override;

        // Other helper functions
        virtual std::ostream &Print(std::ostream &os = std::cout) const;
        virtual const Analysis::SimSettingList GetSpecification() const;
        virtual GenParticleVector GetLastEvent() const;

    private:
        G4ParticleGun *fParticleGun;
        G4ParticleTable *fparticleTable;
        
        CRYGenerator *fCRYgenerator;
        std::vector<CRYParticle *> *cry_generated;
        std::map<std::string, float> fCRY_additional_setup;
        
        std::string PROJECT_SOURCE_DIR;

        // Keep track of generated particles in the last events
        std::vector<Particle> gen_particles_prev;
        
        Command::StringArg *_ui_pathname;
        G4UIcmdWith3VectorAndUnit *_ui_offset;
        G4UIcmdWithADoubleAndUnit *_ui_offset_t_low;
        G4UIcmdWithADoubleAndUnit *_ui_offset_t_high;
        G4UIcmdWithADoubleAndUnit *_ui_ekin_low;
        G4UIcmdWithADoubleAndUnit *_ui_ekin_high;        

    };


} } } // namespace MATHUSLA::MU::Physics

#endif
