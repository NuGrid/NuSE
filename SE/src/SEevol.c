#include <stddef.h>
#include <SE.h>
#include <SEevol.h>
#include "FSEevol.h"

void SEread_evol(int ifile_id, int cycle, evol_t **shells, int *nshells)
{
    SEread(ifile_id, cycle, nshells, (void **)shells, sizeof(evol_t),
	   "mass", SE_DOUBLE, offsetof(evol_t, mass),
	   "radius", SE_DOUBLE, offsetof(evol_t, radius),
	   "rho", SE_DOUBLE, offsetof(evol_t, rho),
	   "temperature", SE_DOUBLE, offsetof(evol_t, temperature),
	   "pressure", SE_DOUBLE, offsetof(evol_t, pressure),
	   "velocity", SE_DOUBLE, offsetof(evol_t, velocity),
	   "dcoeff", SE_DOUBLE, offsetof(evol_t, dcoeff),
	   "yps1", SE_DOUBLE, offsetof(evol_t, yps[0]),
	   "yps2", SE_DOUBLE, offsetof(evol_t, yps[1]),
	   "yps3", SE_DOUBLE, offsetof(evol_t, yps[2]),
	   "yps4", SE_DOUBLE, offsetof(evol_t, yps[3]),
	   "yps5", SE_DOUBLE, offsetof(evol_t, yps[4]),
	   "yps6", SE_DOUBLE, offsetof(evol_t, yps[5]),
	   "yps7", SE_DOUBLE, offsetof(evol_t, yps[6]),
	   NULL);
}

void FSEREADEVOL(int *file_id, int *model, int *nshells, 
		 int *arraylen, double *mass, double *radius, double *rho, 
		 double *temperature, double *pressure, double *velocity, 
		 double *dcoeff, double *yps)
     /* Array have to be allocated beforehand!! */
{
    evol_t *shells;
    int i, j;

    SEread_evol(*file_id, *model, &shells, nshells);

    for (i=0;i<*nshells; i++){
	mass[i] = shells[i].mass;
	radius[i] = shells[i].radius;
	rho[i] = shells[i].rho;
	temperature[i] = shells[i].temperature;
	pressure[i] = shells[i].pressure;
	velocity[i] = shells[i].velocity;
	dcoeff[i] = shells[i].dcoeff;
	for (j = 0; j < NSP; j++){
	    yps[j*(*arraylen)+i] = shells[i].yps[j];
	}
    }

    free(shells);
}

void SEwrite_evol(int ifile_id, int cycle, evol_t *shell, int nshells)
{
    SEwrite(ifile_id, cycle, nshells, shell, sizeof(evol_t),
	    "mass", SE_DOUBLE, offsetof(evol_t, mass),
	    "radius", SE_DOUBLE, offsetof(evol_t, radius),
	    "rho", SE_DOUBLE, offsetof(evol_t, rho),
	    "temperature", SE_DOUBLE, offsetof(evol_t, temperature),
	    "pressure", SE_DOUBLE, offsetof(evol_t, pressure),
	    "velocity", SE_DOUBLE, offsetof(evol_t, velocity),
	    "dcoeff", SE_DOUBLE, offsetof(evol_t, dcoeff),
	    "yps1", SE_DOUBLE, offsetof(evol_t, yps[0]),
	    "yps2", SE_DOUBLE, offsetof(evol_t, yps[1]),
	    "yps3", SE_DOUBLE, offsetof(evol_t, yps[2]),
	    "yps4", SE_DOUBLE, offsetof(evol_t, yps[3]),
	    "yps5", SE_DOUBLE, offsetof(evol_t, yps[4]),
	    "yps6", SE_DOUBLE, offsetof(evol_t, yps[5]),
	    "yps7", SE_DOUBLE, offsetof(evol_t, yps[6]),
	    NULL);
}

void FSEWRITEEVOL(int *file_id, int *model, int *nshells, 
		  int *arraylen, double *mass, double *radius, double *rho, 
		  double *temperature, double *pressure, double *velocity, 
		  double *dcoeff, double *yps)
{
    evol_t *shells;
    int i, j;

    shells = malloc(*nshells * sizeof(evol_t));

    for (i = 0; i < *nshells; i++){
	shells[i].mass = mass[i];
	shells[i].radius = radius[i];
	shells[i].rho = rho[i];
	shells[i].temperature = temperature[i];
	shells[i].pressure = pressure[i];
	shells[i].velocity = velocity[i];
	shells[i].dcoeff = dcoeff[i];
	for (j = 0; j < NSP; j++){
	    shells[i].yps[j]=yps[j*(*arraylen)+i];
	}
    }

    SEwrite_evol(*file_id, *model, shells, *nshells);

    free(shells);
}
