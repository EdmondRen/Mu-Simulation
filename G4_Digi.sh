#!/bin/bash
#SBATCH --time=6:00:00
#SBATCH --account=def-mdiamond
#SBATCH --array=1-10
#SBATCH --mem=2G

if [ $# -ne 2 ]; then
	echo "Usage: <MacroDirectory> <DigitizerOutput>"
	exit 1
fi

# G4Macros is the macro or a directory of macros
G4Macros=${1}
# DigiDataDir is the directory where the digitized root files are created
DigiDataDir=${2}

#Location where Geant4 files will be stored temporarily
G4Out="/project/rrg-mdiamond/data/MATHUSLA/SimOutput/temp_${SLURM_ARRAY_TASK_ID}"

#Making temporary directory:
if [ ! -d ${G4Out} ]; then
	mkdir ${G4Out}
fi

#echo "Running initsim"
export PYTHIA8=/project/def-mdiamond/tomren/mathusla/pythia8308
export PYTHIA8DATA=$PYTHIA8/share/Pythia8/xmldoc
PATH=$PATH:/project/def-mdiamond/tomren/mathusla/dlib-19.24/install
module load StdEnv/2020
module load qt/5.12.8
module load gcc/9.3.0
module load root/6.26.06
module load eigen/3.3.7
module load geant4/10.7.3
module load geant4-data/10.7.3


echo "Running Geant4"
if [ -d ${G4Macros} ]; then # This is a directory
	find "${G4Macros}" -type f -name "*.mac" | while read -r file; do
		${simulation} -q -s ${file} -o ${G4Out}
		# Example: Print the content of the file
		cat "$file"
	done
else 
	${simulation} -q -s ${G4Macros} -o ${G4Out}
fi

# Run the Digitizer
echo "Running Digitizer"
# Don't know exactly the name of the G4 output root file (dependent on date)
find "data/tmp_${SLURM_ARRAY_TASK_ID}" -type f -name "*.root" | while read -r file; do
  ${digitizer} $file ${DigiDataDir}
  rm -rf "${G4Out}"
done


