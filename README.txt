./BatteryModel directory contains the following files:


README.txt          - this file

currentModel.m      - MATLAB function used by paramEst.m
paramEst.m          - MATLAB function that estimates battery parameters

lifePredictor.h     - C header file used by lifePredictor.c
lifePredictor.c     - C code that computes battery life
lifePredictor       - SPARC executable (Solaris v5.8)

ILtable.dat         - example of constant currents and battery lives*
configData.dat      - example of battery configuration data**
currentProfile.dat  - example of current profile***


NOTE: See paramEst.m and lifePredictor.c for usage details.
_____________________________________________________
*   Constant load simulation results from [1].
**  Model settings for simulated battery from [1]. 
*** Specification of variable load case C12 from [1].

[1] D.Rakhmatov, S.Vrudhula, and D.Wallach; 
"A Model for Battery Lifetime Analysis for Organizing Applications
 on a Pocket Computer", IEEE Trans. VLSI, vol.11, no.6, 2003.
 
_____________________________________________________
Created by Daler N. Rakhmatov on November 12, 2003.
Contact: daler@ece.uvic.ca.
