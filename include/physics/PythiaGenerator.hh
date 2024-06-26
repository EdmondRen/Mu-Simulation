/*
 * include/physics/PythiaGenerator.hh
 *
 * Copyright 2018 Brandon Gomes
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MU__PHYSICS_PYTHIAGENERATOR_HH
#define MU__PHYSICS_PYTHIAGENERATOR_HH
#pragma once

#include <string>
#include <vector>

#include <Pythia8/Pythia.h>
#include <Pythia8/Event.h>

#include "physics/Generator.hh"

namespace MATHUSLA { namespace MU {

    namespace Physics { ////////////////////////////////////////////////////////////////////////////

      class PythiaFilter {
      public:
	PythiaFilter() {}
	virtual ~PythiaFilter() {}

	virtual void GetParticles(const Pythia8::Event& event, std::vector<int>& indexlist);
	virtual std::string GetName(void) { return  "PythiaFilter"; }
	virtual const Analysis::SimSettingList GetSpecification(void) const { Analysis::SimSettingList l; return l; }
      };

      class PythiaPromptMuonFilter : public PythiaFilter {
      public:
	PythiaPromptMuonFilter() {}
	~PythiaPromptMuonFilter() {}
	
	void GetParticles(const Pythia8::Event& event, std::vector<int>& indexlist);
	virtual std::string GetName(void) { return  "PythiaPromptMuonFilter"; }
	virtual const Analysis::SimSettingList GetSpecification(void) const;
	
	static G4ThreadLocal double _pCut;
	static G4ThreadLocal double _ptCut;
	static G4ThreadLocal double _etaLoCut, _etaHiCut;
	static G4ThreadLocal double _phiLoCut, _phiHiCut;
	
      };

      class PythiaDisplacedFilter : public PythiaFilter {
      public:
	PythiaDisplacedFilter() {}
	~PythiaDisplacedFilter() {}

	void GetParticles(const Pythia8::Event& event, std::vector<int>& indexlist);

	virtual std::string GetName(void) { return  "PythiaDisplacedFilter"; }
	virtual const Analysis::SimSettingList GetSpecification(void) const;

	static G4ThreadLocal double _xLoCut, _xHiCut;
	static G4ThreadLocal double _yLoCut, _yHiCut;
	static G4ThreadLocal double _zLoCut, _zHiCut;
      };
      
      
//__Pythia Particle Generator___________________________________________________________________
      class PythiaGenerator : public Generator {
      public:
	PythiaGenerator(Pythia8::Pythia* pythia=nullptr);
	PythiaGenerator(const std::vector<std::string>& settings);
	PythiaGenerator(const std::string& path);
	
	void GeneratePrimaryVertex(G4Event* event);
	virtual GenParticleVector GetLastEvent() const;
	void SetNewValue(G4UIcommand* command, G4String value);
	void SetPythia(Pythia8::Pythia* pythia);
	void SetPythia(const std::vector<std::string>& settings);
	void SetPythia(const std::string& path);
	
	virtual const Analysis::SimSettingList GetSpecification() const;

	void SetFilter(PythiaFilter* filter) { _filter=filter; }
	PythiaFilter* GetFilter(void) const { return _filter; }
	
      protected:
	static G4ThreadLocal Pythia8::Pythia* _pythia;
	static G4ThreadLocal std::vector<std::string>* _pythia_settings;
	static G4ThreadLocal bool _settings_on;

	PythiaFilter* _filter;
	GenParticleVector _last_event;
	std::uint_fast64_t _counter;
	std::string _path;
	Command::StringArg* _read_string;
	Command::StringArg* _read_file;
	Command::StringArg* _add_filter;
	Command::StringArg* _add_filter_cut;
      };
      //----------------------------------------------------------------------------------------------
      
      

    } /* namespace Physics */ //////////////////////////////////////////////////////////////////////
    
  } } /* namespace MATHUSLA::MU */

#endif /* MU__PHYSICS_PYTHIAGENERATOR_HH */
