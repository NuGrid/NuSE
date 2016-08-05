#include <stdlib.h>
#include <assert.h>
#include <SE.h>
#include <SEevol.h>

int main(int argc, char *argv[])
{
    int file_id, cycle = 1;
    int i, j, orignshells = 1000, nshells = 1000;
    evol_t *shells;
    double mtot;

    shells = (evol_t *)malloc(nshells * sizeof(evol_t));

    /* Generate fake dataset */
    for (mtot = 0.0, i = 0; i < nshells; ++i) {
	shells[i].mass = 2.0*i;
	shells[i].radius = 3.0*i;
	shells[i].rho = 4.0*i;
	shells[i].temperature = 5.0*i;
	shells[i].pressure = 6.0*i;
	shells[i].velocity = 7.0*i;
	shells[i].dcoeff = 8.0*i;

	for (j = 0; j < NSP; ++j) {
	    shells[i].yps[j] = 9.0*i + j;
	}
    }

    /* Write to SE file */
    file_id = SEopen("test.h5");
    SEwrite_evol(file_id, cycle, shells, nshells);
    SEclose(file_id);
    free(shells);

    /* Reread; SEread probably shouldn't malloc */
    file_id = SEopen("test.h5");
    SEread_evol(file_id, cycle, &shells, &nshells);

    assert(nshells = orignshells);

    /* Write back to same file */
    SEwrite_evol(file_id, cycle, shells, nshells);

    free(shells);

    return 0;
}
