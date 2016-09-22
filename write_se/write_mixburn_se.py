
#Mimic rho,T, D 5Msun of set1.1 17001 during HBB burnign, envelope
#Notes:
#cycle start at 1, 2 cycles only
#age start at 0
#take 2 cells relevant for HBB? No, just center values
#initiallize with solar abu..
#conv. happens in both cells dcoeff=2e16
#1Msun only, 

import sewrite

#create the h5 output file   
file_sewrite = sewrite.startfile('mixburn.se.h5') 


cycles=[1,2]
ages=[2e7,4e7]

#writing global parameters:

#Make sure that you write the right units to ensure that MPPNP compute with the right values.

hattr_name = [	"codev", "modname", "mini", "zini", "rotini", "overini", "age_unit",
		"mass_unit", "radius_unit", "rho_unit", "temperature_unit",
		"dcoeff_unit","firstcycle"]
hattr_data = [	'codev1', 'modname', 1., 0.0001, 0., 0., 1, 1.9892e33,
		1., 1., 1., 1.,cycles[0]]
file_sewrite.write_hattr(hattr_name, hattr_data)

rhot, tempt, mass, dcoeff, radius, delta_mass = [[1.3e7,1.3e7],[1.29e8,1.29e8],[0.25,0.75],[2e16,2e16],[2.4e6,2.4e6],[0.5,0.5]]
mtot, shellnb, age, deltat = [[1,2,3],[1,2,3],[1,2,3],[1,2,3]]

#write h5 cycle data
for i in range(len(cycles)):
    #write data columns
    dcol_name = ["rho", "temperature", "mass", "dcoeff", "radius", "delta_mass"]
    dcol_data = [rhot, tempt, mass, dcoeff, radius, delta_mass]
    file_sewrite.write_dcol(cycles[i], dcol_name, dcol_data)
    	
    #write data attributes
    cattr_name = ["total_mass", "shellnb", "age", "deltat"]
    cattr_data = [9.303e33, 2,ages[i], 2e7]
    file_sewrite.write_cattr(cycles[i], cattr_name, cattr_data)

