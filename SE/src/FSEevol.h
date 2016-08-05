#include <config.h>

#define FSEREADEVOL F77_FUNC_(fse_read_evol, FSE_READ_EVOL)
void FSEREADEVOL(int *file_id, int *model, int *nshells, 
		 int *arraylen, double *mass, double *radius, double *rho, 
		 double *temperature, double *pressure, double *velocity, 
		 double *dcoeff, double *yps);

#define FSEWRITEEVOL F77_FUNC_(fse_write_evol, FSE_WRITE_EVOL)
void FSEWRITEEVOL(int *file_id, int *model, int *nshells, 
		  int *arraylen, double *mass, double *radius, double *rho, 
		  double *temperature, double *pressure, double *velocity, 
		  double *dcoeff, double *yps);
