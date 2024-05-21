# MATHUSLA-Digitizer
Tracking software for MATHUSLA Experiment

## Introduction
This repository, including this documentation, is a modification of the following code from: https://github.com/seg188/MATHUSLA-MLTracker 

This repository containts an algorithm which creates realistic signals by combining truth hits from the first simulation, then adding noise at a user-specified rate.

## Installation
The digitizer may be installed with the equivalent script `install_digitizer`. When installing for the first time, be sure to use the option `--cmake`.

## Running the Digitizer

The digitizer requires two command line arguments, the path to an input file, and the path to which the output file should be written. The input file should be the output from a MATHUSLA Mu-Simulation run, a Geant4 based simulation of particle passage through the MATHUSLA detector. 

An example command to run the digitizer:

```bash
$ ./tracker path_to_input_file path_to_write_output 
```
A script for automating series runs of the tracker, and further documentation about it, is located in the /run/ directory. 

Job submission scripts for parallelising the digitizer or analysis code for large datasets is found in parallel.

### Digitizer configuration

Tracker configuration parameters are stored in a txt file located at `Digitizer/run/par_card.txt`
The parameters are explained in the table below:

| Parameter name | Usage | Default value [unit]|
|:--------------|:-------------------------|:---|
|branch              | 0 or 1, 0 for nomal mode and 1 for COSMIC mode | 0|
|debug               | 0 of 1, 1 to turn on debug information| |
|seed                | initial seed value for random generator. Set to -1 to use arbiturary seed|1.0 |
|seed_interval               |Maximum interval for track seeding. Interval defined as ds^2 = dr^2-(c*dt)^2| 7|
|scint_efficiency                | 0.001                    | |
|start_ev                | 0.0                  | |
|end_ev              | 200000.0                 | |
|noise_hz                | 0.0                  | |



