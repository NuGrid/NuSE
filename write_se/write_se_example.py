
#Example case how to write a se file
#We write 3 cycles in a newly created file example.se.h5


import sewrite

#create the h5 output file   
file_sewrite = sewrite.startfile('example.se.h5') 


cycles=[1,2,3]


#writing global parameters:

#Make sure that you write the right units to ensure that MPPNP compute with the right values.

hattr_name = [	"codev", "modname", "mini", "zini", "rotini", "overini", "age_unit",
		"mass_unit", "radius_unit", "rho_unit", "temperature_unit",
		"dcoeff_unit","firstcycle"]
hattr_data = [	'codev1', 'modname', 1., 0.02, 0., 0., 1., 1.,
		1., 1., 1., 1.,cycles[0]]
file_sewrite.write_hattr(hattr_name, hattr_data)

rhot, tempt, mass, dcoeff, radius, delta_mass = [[1,2,3],[1,2,3],[1,2,3],[1,2,3],[1,2,3],[1,2,3]]
mtot, shellnb, age, deltat = [[1,2,3],[1,2,3],[1,2,3],[1,2,3]]

#write h5 cycle data
for i in range(len(cycles)):
    #write data columns
    dcol_name = ["rho", "temperature", "mass", "dcoeff", "radius", "delta_mass"]
    dcol_data = [rhot, tempt, mass, dcoeff, radius, delta_mass]
    file_sewrite.write_dcol(cycles[i], dcol_name, dcol_data)
    	
    #write data attributes
    cattr_name = ["total_mass", "shellnb", "age", "deltat"]
    cattr_data = [0.5, 1000,1234, 200]
    file_sewrite.write_cattr(cycles[i], cattr_name, cattr_data)

