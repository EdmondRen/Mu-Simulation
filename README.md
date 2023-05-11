# MATHUSLA Mu-Simulation


_simulation of muons through earth material_

## Build & Run

The simulation comes with a simple build script called `install` which allows for build customization and execution of the muon simulation.

Here is a list of useful commands:

| Action             | Options for `./install` |
|:------------------:|:-----------------------:|
| Build Only         | `(none)`                |
| Build and Auto Run | `--run`                 |
| Clean CMake Build  | `--cmake --clean`       |
| More Options       | `--help`                |

After building, the executable, `simulation`, is moved into the root directory of the project.

The simulation executable itself comes with several configuration parameters:

| Action                | Short Options    | Long Options        |
|:---------------------:|:----------------:|:-------------------:|
| Event Count           | `-e <count>`     | `--events=<count>`  |
| Particle Generator    | `-g <generator>` | `--gen=<generator>` |
| Detector              | `-d <detector>`  | `--det=<detector>`  |
| Custom Script         | `-s <file>`      | `--script=<file>`   |
| Data Output Directory | `-o <dir>`       | `--out=<dir>`       |
| Number of Threads     | `-j <count>`     | `--threads=<count>` |
| Visualization         | `-v`             | `--vis`             |
| Save All Generator Events         | `NA` | `--save_all`        |
| Save Events With Pseudo-Digi Cuts (only for Cosmic geometry) | `NA` | `--cut_save`        |
| Bias Muon Nuclear interaction in Earth (for Cosmic and Box geometry) | `NA` | `--bias`        |
| Turn On Five Body Muon Decays     | `-f` | `--five_muon`       |
| Non-Random Five Body Decays       | `-n` | `--non_random`      |
| Quiet Mode            | `-q`             | `--quiet`           |
| Help                  | `-h`             | `--help`            |

Note: The Five Body Muon Decays option will only save tracks with a five-body decay in a certain zone in the detector. Be sure this is what you want.

Arguments can also be passed through the simulation to a script. Adding key value pairs which correspond to aliased arguments in a script, will be forwarded through. Here's an example:

```
./simulation -s example1.mac ke 100 phi 20
```

This will pass the key value pairs `(ke, 100)` and `(phi, 20)` to the underlying script `example1.mac`. The file can look something like this:

```
# example1.mac

/gen/select basic

/gen/basic/ke {ke} GeV
/gen/basic/phi {phi} deg
```

where `{ }` denotes a key. The script will be evaluated by substituting the key for its value so the last two lines become,

```
/gen/basic/ke 100 GeV
/gen/basic/phi 20 deg
```

All of the simulation configuration parameters can be passed through `./install --run` so the example above could also be written as,

```
./install --run -s example1.mac ke 100 phi 20
```

### Generators

There are three general purpose generators built in, `basic`, `range`, and `polar`. The `basic` generator produces a particle with constant `pT`, `eta`, and `phi` while the `range` generator produces particle within a specified range of values for each of the three variables. Any variable can also be fixed to a constant value. The `polar` generator uses the angles from spherical coordinates, polar and azimuth, along with an energy input to generate particles. The polar angle in `polar` generator can be either a constant or within a specified range, while the azimuth is only given within a range.

There is also a _Pythia8_ generator installed which behaves similiarly to the `range` generator.

There is a `file_reader` generator that produces particles with properties that are specified in an input file. The `file_reader` does not do randomization. Each entry in the input file correspond to exactly one event and may include multiple particles, which means that the physics process to generate the primary partiles needs to be taken care of outside GEANT4. This generator provide the possibility to generate controllable primary vertex at given location with pre-assigned momentum for each secondary particles. 

The generator defaults are specified in `src/action/GeneratorAction.cc` but they can be overwritten by a custom generation script.

### Custom Detector

A custom Detector can be specified at run time from one of the following installed detectors:

| Detector   | Status    | Details                                               |
|:----------:|:---------:|:-----------------------------------------------------:|
| Prototype  | COMPLETED | Test stand for the MATHUSLA project                   |
| Box        | COMPLETED | Large Detector as seen in MATHUSLA Original Schematic |
| Cosmic     | COMPLETED | Identical detector design as in Box but optimized for cosmic studies |
| Flat       | BUILDING  | Cheaper Alternative to Box                            |
| MuonMapper | COMPLETED | Measures Muon Energies after Rock Propagation         |

### Custom Scripts

A custom _Geant4_ script can be specified at run time. The script can contain generator specific commands and settings as well as _Pythia8_ settings in the form of `readString`. The script can also specify the detector to use during the simulation.
