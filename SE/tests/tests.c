#include <string.h>
#include <hdf5.h>
#include <SE.h>
#include <FSE.h>
#include <check.h>
#include "tests.h"
#include "config.h"

static hid_t file_id;

typedef struct {
    int index;
    double mass;
    double radius;
    float rho;
    int intarr[17];
    float fltarr[9];
    double dblarr[13];
} row_t;

static int nint=17, nflt=9, ndbl=13;

typedef struct {
    double radius;
    float rho;
} shortrow_t;

typedef struct {
    double rho;
    float mass, radius;
} difftype_t;

/* From SEopen.c */
void _SE_update_internal_layout(hid_t file_id);

static void setup(void)
{
    hid_t fapl_id;
    row_t *row;
    int i, j;

    /* Create file in memory, using CORE driver */
    fapl_id = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fapl_core(fapl_id, 256*1024, 0);
    file_id = H5Fcreate("dummy", H5F_ACC_EXCL, H5P_DEFAULT, fapl_id);
    H5Pclose(fapl_id);

    /* Write cycle 0 */
    row = (row_t *)malloc(9 * sizeof(row_t));

    for (i = 0; i < 9; ++i) {
	row[i].index = i+1;
	row[i].mass = 11.0*i;
	row[i].radius = 19.0*i;
	row[i].rho = (float)31.0*i;
	for (j = 0; j < nint; ++j) row[i].intarr[j] = i*100+j;
	for (j = 0; j < nflt; ++j) row[i].fltarr[j] = (float) i*100+j+0.01;
	for (j = 0; j < ndbl; ++j) row[i].dblarr[j] = (double) i*100+j+0.001;
    }

    SEwrite(file_id, 0, 9, row, sizeof(row_t),
	    "index", SE_INT, offsetof(row_t, index),
	    "mass", SE_DOUBLE, offsetof(row_t, mass),
	    "radius", SE_DOUBLE, offsetof(row_t, radius),
	    "rho", SE_FLOAT, offsetof(row_t, rho),
	    "intarr", SE_INT_2D, nint, offsetof(row_t, intarr),
	    "fltarr", SE_FLOAT_2D, nflt, offsetof(row_t, fltarr),
	    "dblarr", SE_DOUBLE_2D, ndbl, offsetof(row_t, dblarr),
	    NULL);

    free(row);

    /* Write cycle 10 */
    row = (row_t *)malloc(32 * sizeof(row_t));

    for (i = 0; i < 32; ++i) {
	row[i].index = -(i+1);
	row[i].mass = -101.0*i;
	row[i].radius = 73.0*i;
	row[i].rho = (float)-13.0*i;
	for (j = 0; j < nint; ++j) row[i].intarr[j] = -(i*4)+3*j;
	for (j = 0; j < nflt; ++j) row[i].fltarr[j] = (float) i*13.+14.*j;
	for (j = 0; j < ndbl; ++j) row[i].dblarr[j] = (double) i*9.-j*11.;
    }

    SEwrite(file_id, 10, 32, row, sizeof(row_t),
	    "index", SE_INT, offsetof(row_t, index),
	    "mass", SE_DOUBLE, offsetof(row_t, mass),
	    "radius", SE_DOUBLE, offsetof(row_t, radius),
	    "rho", SE_FLOAT, offsetof(row_t, rho),
	    "intarr", SE_INT_2D, nint, offsetof(row_t, intarr),
	    "fltarr", SE_FLOAT_2D, nflt, offsetof(row_t, fltarr),
	    "dblarr", SE_DOUBLE_2D, ndbl, offsetof(row_t, dblarr),
	    NULL);

    free(row);

    /* Write some extra short cycles */
    i = 42;
    SEwrite(file_id, 1, 1, &i, sizeof(int), "index", SE_INT, 0, NULL);
    SEwrite(file_id, 2, 1, &i, sizeof(int), "index", SE_INT, 0, NULL);
    SEwrite(file_id, 4, 1, &i, sizeof(int), "index", SE_INT, 0, NULL);
    SEwrite(file_id, 130, 1, &i, sizeof(int), "index", SE_INT, 0, NULL);
    SEwrite(file_id, 9, 1, &i, sizeof(int), "index", SE_INT, 0, NULL);
    SEwrite(file_id, 14, 1, &i, sizeof(int), "index", SE_INT, 0, NULL);
    SEwrite(file_id, 8, 1, &i, sizeof(int), "index", SE_INT, 0, NULL);

    /* Write an empty cycle; this currently doesn't write a cycle at all */
    SEwrite(file_id, 12, 0, &i, sizeof(int), NULL);

    /* Write some duplicate cycles */
    i = -3;
    SEwrite(file_id, 8, 1, &i, sizeof(int), "index", SE_INT, 0, NULL);

    i = 100000;
    SEwrite(file_id, 4, 1, &i, sizeof(int), "index", SE_INT, 0, NULL);

    i = -1;
    SEwrite(file_id, 8, 1, &i, sizeof(int), "index", SE_INT, 0, NULL);
}

static void teardown(void)
{
    H5Fclose(file_id);
    H5close();
}

START_TEST(ncycles)
{
    int ncycles;

    ncycles = SEncycles(file_id);
    fail_unless (ncycles == 9, "ncycles was %d, should be 9", ncycles);

    ncycles = 0;

    FSENCYCLES(&file_id, &ncycles);
    fail_unless (ncycles == 9, "ncycles was %d, should be 9", ncycles);
}
END_TEST

START_TEST(nshells)
{
    int cycle, nshells;

    cycle = 0;
    nshells = SEnshells(file_id, cycle);
    fail_unless (nshells == 9, "nshells was %d, should be 9", nshells);

    cycle = 10;
    nshells = SEnshells(file_id, cycle);
    fail_unless (nshells == 32, "nshells was %d, should be 32", nshells);

    cycle = 130;
    nshells = SEnshells(file_id, cycle);
    fail_unless (nshells == 1, "nshells was %d, should be 1", nshells);

    cycle = 0;
    FSENSHELLS(&file_id, &cycle, &nshells);
    fail_unless (nshells == 9, "nshells was %d, should be 9", nshells);

    cycle = 10;
    FSENSHELLS(&file_id, &cycle, &nshells);
    fail_unless (nshells == 32, "nshells was %d, should be 32", nshells);

    cycle = 130;
    FSENSHELLS(&file_id, &cycle, &nshells);
    fail_unless (nshells == 1, "nshells was %d, should be 1", nshells);
}
END_TEST

START_TEST(cycles)
{
    int i, *cycles, ncycles = SEncycles(file_id);

    cycles = (int *)malloc(ncycles * sizeof(int));

    SEcycles(file_id, cycles, ncycles);

    fail_unless (cycles[0] == 0, "cycles[0] was %d, should be 0", cycles[0]);
    fail_unless (cycles[1] == 1, "cycles[1] was %d, should be 1", cycles[1]);
    fail_unless (cycles[2] == 2, "cycles[2] was %d, should be 2", cycles[2]);
    fail_unless (cycles[3] == 4, "cycles[3] was %d, should be 4", cycles[3]);
    fail_unless (cycles[4] == 8, "cycles[4] was %d, should be 8", cycles[4]);
    fail_unless (cycles[5] == 9, "cycles[5] was %d, should be 9", cycles[5]);
    fail_unless (cycles[6] == 10, "cycles[6] was %d, should be 10", cycles[6]);
    fail_unless (cycles[7] == 14, "cycles[7] was %d, should be 14", cycles[7]);
    fail_unless (cycles[8] == 130, "cycles[8] was %d, should be 130",
		 cycles[8]);

    for (i = 0; i < ncycles; ++i)
	cycles[i] = -1;

    FSECYCLES(&file_id, cycles, &ncycles);

    fail_unless (cycles[0] == 0, "cycles[0] was %d, should be 0", cycles[0]);
    fail_unless (cycles[1] == 1, "cycles[1] was %d, should be 1", cycles[1]);
    fail_unless (cycles[2] == 2, "cycles[2] was %d, should be 2", cycles[2]);
    fail_unless (cycles[3] == 4, "cycles[3] was %d, should be 4", cycles[3]);
    fail_unless (cycles[4] == 8, "cycles[4] was %d, should be 8", cycles[4]);
    fail_unless (cycles[5] == 9, "cycles[5] was %d, should be 9", cycles[5]);
    fail_unless (cycles[6] == 10, "cycles[6] was %d, should be 10", cycles[6]);
    fail_unless (cycles[7] == 14, "cycles[7] was %d, should be 14", cycles[7]);
    fail_unless (cycles[8] == 130, "cycles[8] was %d, should be 130",
		 cycles[8]);

    free(cycles);
}
END_TEST

START_TEST(lastcycle)
{
    int lastcycle;

    lastcycle = SElastcycle(file_id);
    fail_unless (lastcycle == 130, 
		 "lastcycle was %d should be 130", lastcycle);

    lastcycle = 0;

    FSELASTCYCLE(&file_id, &lastcycle);
    fail_unless (lastcycle == 130, 
		 "lastcycle was %d should be 130", lastcycle);
}
END_TEST

START_TEST(members)
{
    int cycle = 10, member_id = 2;
    int nmembers, nelements;
    char *mname, fname[32];

    nmembers = SEget_nmembers(file_id, cycle);
    fail_unless (nmembers == 7, "nmembers was %d, should be 7", nmembers);

    nmembers = 0;
    FSEGETNMEMBERS(&file_id, &cycle, &nmembers);
    fail_unless (nmembers == 7, "nmembers was %d, should be 7", nmembers);

    mname = SEget_member_name(file_id, cycle, member_id);
    fail_unless (strcmp(mname, "radius") == 0,
		 "mname was \"%s\", should be \"radius\"", mname);

    free(mname);
    mname = (char *)malloc(32 * sizeof(char));

    FSEGETMEMBERNAME(&file_id, &cycle, &member_id, fname, sizeof(fname));
    strtrim(mname, fname, sizeof(fname));
    fail_unless (strcmp(mname, "radius") == 0,
		 "mname was \"%s\", should be \"radius\"", mname);

    free(mname);

    nelements = SEnentries(file_id, cycle, "intarr");
    fail_unless (nelements == nint, "nelements was %d, should be %d",
		 nelements, nint);

    nelements = SEnentries(file_id, cycle, "dblarr");
    fail_unless (nelements == ndbl, "nelements was %d, should be %d",
		 nelements, ndbl);

    nelements = SEnentries(file_id, cycle, "fltarr");
    fail_unless (nelements == nflt, "nelements was %d, should be %d",
		 nelements, nflt);

    FSENENTRIES(&file_id, &cycle, "intarr", &nelements, 6);
    fail_unless (nelements == nint, "nelements was %d, should be %d",
		 nelements, nint);

    FSENENTRIES(&file_id, &cycle, "dblarr", &nelements, 6);
    fail_unless (nelements == ndbl, "nelements was %d, should be %d",
		 nelements, ndbl);

    FSENENTRIES(&file_id, &cycle, "fltarr", &nelements, 6);
    fail_unless (nelements == nflt, "nelements was %d, should be %d",
		 nelements, nflt);
}
END_TEST

START_TEST(read)
{
    /* What happens if you try to read a column that doesn't exist?
       That should trigger an error (and no, a big HDF5 stack trace is
       not a reasonable alternative). */

    void *buf;
    row_t *row;
    difftype_t *drow;
    int i, nrows, cycle, nchars;
    char colname[128];
    double *massarray, *radarray;
    float *rhoarray;
    int *indexarray;

    /* Read cycle 0 */
    cycle = 0;
    SEread(file_id, cycle, &nrows, &buf, sizeof(row_t),
	   "index", SE_INT, offsetof(row_t, index),
	   "mass", SE_DOUBLE, offsetof(row_t, mass),
	   "radius", SE_DOUBLE, offsetof(row_t, radius),
	   "rho", SE_FLOAT, offsetof(row_t, rho),
	   "intarr", SE_INT_2D, nint, offsetof(row_t, intarr),
	   "fltarr", SE_FLOAT_2D, nflt, offsetof(row_t, fltarr),
	   "dblarr", SE_DOUBLE_2D, ndbl, offsetof(row_t, dblarr),
	   NULL);

    fail_unless (nrows == 9, "nrows was %d, should be 9", nrows);

    for (row = (row_t *)buf, i = 0; i < nrows; ++i) {
	fail_unless (row[i].index == i+1,
		     "row[%d].index was %d, should be %d",
		     i, row[i].index, i+1);
	fail_unless (row[i].mass ==11.0*i,
		     "row[%d].mass was %g, should be %g",
		     i, row[i].mass, 11.0*i);
	fail_unless (row[i].radius == 19.0*i,
		     "row[%d].radius was %g, should be %g",
		     i, row[i].radius, 19.0*i);
	fail_unless (row[i].rho == 31.0*i,
		     "row[%d].rho was %g, should be %g",
		     i, row[i].rho, 31.0*i);
    }

    free(buf);

    /* Read only part of cycle 0, in a weird order, and convert types */
    SEread(file_id, cycle, &nrows, &buf, sizeof(difftype_t),
	   "radius", SE_FLOAT, offsetof(difftype_t, radius),
	   "rho", SE_DOUBLE, offsetof(difftype_t, rho),
	   "mass", SE_FLOAT, offsetof(difftype_t, mass),
	   NULL);

    fail_unless (nrows == 9, "nrows was %d, should be 9", nrows);

    for (drow = (difftype_t *)buf, i = 0; i < nrows; ++i) {
	fail_unless (drow[i].mass ==11.0*i,
		     "drow[%d].mass was %g, should be %g",
		     i, drow[i].mass, 11.0*i);
	fail_unless (drow[i].radius == 19.0*i,
		     "drow[%d].radius was %g, should be %g",
		     i, drow[i].radius, 19.0*i);
	fail_unless (drow[i].rho == 31.0*i,
		     "drow[%d].rho was %g, should be %g",
		     i, drow[i].rho, 31.0*i);
    }

    free(buf);

    /* Read only some shells using SEsetlines */
    SEsetlines(3, 8);
    SEread(file_id, cycle, &nrows, &buf, sizeof(difftype_t),
	   "radius", SE_FLOAT, offsetof(difftype_t, radius),
	   "rho", SE_DOUBLE, offsetof(difftype_t, rho),
	   "mass", SE_FLOAT, offsetof(difftype_t, mass),
	   NULL);
    SEsetlines(-1, -1);

    fail_unless (nrows == 6, "nrows was %d, should be 6", nrows);

    for (drow = (difftype_t *)buf, i = 0; i < nrows; ++i) {
	fail_unless (drow[i].mass ==11.0*(i+3),
		     "drow[%d].mass was %g, should be %g",
		     i+3, drow[i].mass, 11.0*(i+3));
	fail_unless (drow[i].radius == 19.0*(i+3),
		     "drow[%d].radius was %g, should be %g",
		     i+3, drow[i].radius, 19.0*(i+3));
	fail_unless (drow[i].rho == 31.0*(i+3),
		     "drow[%d].rho was %g, should be %g",
		     i+3, drow[i].rho, 31.0*(i+3));
    }

    free(buf);

    /* Read each row via array-at-a-time Fortran calls */
    cycle = 10;
    FSENSHELLS(&file_id, &cycle, &nrows);
    fail_unless (nrows == 32, "nrows was %d, should be 32", nrows);

    indexarray = (int *)malloc(nrows * sizeof(int));
    rhoarray = (float *)malloc(nrows * sizeof(float));
    massarray = (double *)malloc(nrows * sizeof(double));
    radarray = (double *)malloc(nrows * sizeof(double));

    strncpy(colname, "index", sizeof(colname));
    nchars = strlen(colname);
    FSEREADI(&file_id, &cycle, &nrows, colname, indexarray, nchars);

    strncpy(colname, "rho", sizeof(colname));
    nchars = strlen(colname);
    FSEREADF(&file_id, &cycle, &nrows, colname, rhoarray, nchars);

    strncpy(colname, "mass", sizeof(colname));
    nchars = strlen(colname);
    FSEREADD(&file_id, &cycle, &nrows, colname, massarray, nchars);

    strncpy(colname, "radius", sizeof(colname));
    nchars = strlen(colname);
    FSEREADD(&file_id, &cycle, &nrows, colname, radarray, nchars);

    for (i = 0; i < nrows; ++i) {
	fail_unless (indexarray[i] == -(i+1),
		     "indexarray[%d] was %d, should be %d",
		     i, indexarray[i], -(i+1));
	fail_unless (rhoarray[i] == -13.0*i,
		     "rhoarray[%d] was %g, should be %g",
		     i, rhoarray[i], -13.0*i);
	fail_unless (massarray[i] == -101.0*i,
		     "massarray[%d] was %g, should be %g",
		     i, massarray[i], -101.0*i);
	fail_unless (radarray[i] == 73.0*i,
		     "radarray[i] was %g, should be %g",
		     i, radarray[i], 73.0*i);
    }

    free(indexarray);
    free(rhoarray);
    free(massarray);
    free(radarray);
}
END_TEST

START_TEST(readwrite)
{
    void *buf;
    row_t *row;
    shortrow_t *srow;
    int i, j, nrows, narrays, cycle = 1, index[16];
    float rho[16];
    double mass[16], radius[16];
    int int2d[nint][16];
    float flt2d[nflt][16];
    double dbl2d[ndbl][16];
    int itype = 0;
    int ftype = 1;
    int dtype = 2;
    int i2dtype = 10;
    int f2dtype = 11;
    int d2dtype = 12;

    char indexname[6];
    char massname[5];
    char radiusname[7];
    char rhoname[4];
    char intarrname[7];
    char fltarrname[7];
    char dblarrname[7];
    int ilen, mlen, ralen, rholen, intarrlen, fltarrlen, dblarrlen;

    row = (row_t *)malloc(16 * sizeof(row_t));

    for (i = 0; i < 16; ++i) {
	row[i].index = index[i] = i+1;
	row[i].mass = mass[i] = 7.0*i;
	row[i].radius = radius[i] = 17.0*i;
	row[i].rho = rho[i] = (float)23.0*i;
	for (j = 0; j < nint; ++j)
	    row[i].intarr[j] = int2d[j][i] = i*13+j*9;
	for (j = 0; j < nflt; ++j)
	    row[i].fltarr[j] = flt2d[j][i] = i*7.+j*39.;
	for (j = 0; j < ndbl; ++j)
	    row[i].dblarr[j] = dbl2d[j][i] = i*37.+j*7.;
   }

    /* Write cycle 1 */
    SEwrite(file_id, 1, 16, row, sizeof(row_t),
	    "index", SE_INT, offsetof(row_t, index),
	    "mass", SE_DOUBLE, offsetof(row_t, mass),
	    "radius", SE_DOUBLE, offsetof(row_t, radius),
	    "rho", SE_FLOAT, offsetof(row_t, rho),
	    "intarr", SE_INT_2D, nint, offsetof(row_t, intarr),
	    "fltarr", SE_FLOAT_2D, nflt, offsetof(row_t, fltarr),
	    "dblarr", SE_DOUBLE_2D, ndbl, offsetof(row_t, dblarr),
	    NULL);

    /* Write only some of the row_t structure members in cycle 2*/
    SEwrite(file_id, 2, 16, row, sizeof(row_t),
	    "rho", SE_FLOAT, offsetof(row_t, rho),
	    "index", SE_INT, offsetof(row_t, index),
	    "radius", SE_DOUBLE, offsetof(row_t, radius),
	    "dblarr", SE_DOUBLE_2D, ndbl, offsetof(row_t, dblarr),
	    "intarr", SE_INT_2D, nint, offsetof(row_t, intarr),
	    NULL);

    free(row);

    /* Read cycle 1 */
    SEread(file_id, cycle, &nrows, &buf, sizeof(row_t),
	   "index", SE_INT, offsetof(row_t, index),
	   "mass", SE_DOUBLE, offsetof(row_t, mass),
	   "radius", SE_DOUBLE, offsetof(row_t, radius),
	   "rho", SE_FLOAT, offsetof(row_t, rho),
	   "intarr", SE_INT_2D, nint, offsetof(row_t, intarr),
	   "fltarr", SE_FLOAT_2D, nflt, offsetof(row_t, fltarr),
	   "dblarr", SE_DOUBLE_2D, ndbl, offsetof(row_t, dblarr),
	   NULL);

    fail_unless (nrows == 16, "nrows was %d, should be 16", nrows);

    for (row = (row_t *)buf, i = 0; i < nrows; ++i) {
	fail_unless (row[i].index == i+1,
		     "row[%d].index was %d, should be %d",
		     i, row[i].index, i+1);
	fail_unless (row[i].mass == 7.0*i,
		     "row[%d].mass was %g, should be %g",
		     i, row[i].mass, 7.0*i);
	fail_unless (row[i].radius == 17.0*i,
		     "row[%d].radius was %g, should be %g",
		     i, row[i].radius, 17.0*i);
	fail_unless (row[i].rho == 23.0*i,
		     "row[%d].rho was %g, should be %g",
		     i, row[i].rho, 23.0*i);
 	for (j = 0; j < nint; ++j) 
	    fail_unless (row[i].intarr[j] == i*13+j*9,
			 "row[%d].intarr[%d] was %d, should be %d",
			 i, j, row[i].intarr[j], i*13+j*9);
	for (j = 0; j < nflt; ++j) 
	    fail_unless (row[i].fltarr[j] == (float) i*7.+j*39.,
			 "row[%d].fltarr[%d] was %g, should be %g",
			 i, j, row[i].fltarr[j],(float) i*7.+j*39.);	 
	for (j = 0; j < ndbl; ++j) 
	    fail_unless (row[i].dblarr[j] == (double) i*37.+j*7.,
			 "row[%d].dblarr[%d] was %g, should be %g",
			 i, j, row[i].dblarr[j],(double) i*37.+j*7.);
    }

    free(buf);

    /* Write (different) cycle 1 */
    nrows = 16;
    narrays = 7;

    strncpy(indexname, "index", sizeof(indexname));
    ilen = strlen(indexname);

    strncpy(massname, "mass", sizeof(massname));
    mlen = strlen(massname);

    strncpy(radiusname, "radius", sizeof(radiusname));
    ralen = strlen(radiusname);

    strncpy(rhoname, "rho", sizeof(rhoname));
    rholen = strlen(rhoname);

    strncpy(intarrname, "intarr", sizeof(intarrname));
    intarrlen = strlen(intarrname);

    strncpy(fltarrname, "fltarr", sizeof(fltarrname));
    fltarrlen = strlen(fltarrname);

    strncpy(dblarrname, "dblarr", sizeof(dblarrname));
    dblarrlen = strlen(dblarrname);

    FSEWRITE(&file_id, &cycle, &nrows, &narrays,
	     index, indexname, &itype,
	     mass, massname, &dtype,
	     radius, radiusname, &dtype,
	     rho, rhoname, &ftype, 
	     dbl2d, dblarrname, &d2dtype, &nrows, &ndbl,
	     int2d, intarrname, &i2dtype, &nrows, &nint,
	     flt2d, fltarrname, &f2dtype, &nrows, &nflt,
	     ilen, mlen, ralen, rholen, dblarrlen, intarrlen, fltarrlen);

    /* Re-read cycle 1 */
    SEread(file_id, cycle, &nrows, &buf, sizeof(row_t),
	   "index", SE_INT, offsetof(row_t, index),
	   "mass", SE_DOUBLE, offsetof(row_t, mass),
	   "radius", SE_DOUBLE, offsetof(row_t, radius),
	   "rho", SE_FLOAT, offsetof(row_t, rho),
	   "intarr", SE_INT_2D, nint, offsetof(row_t, intarr),
	   "fltarr", SE_FLOAT_2D, nflt, offsetof(row_t, fltarr),
	   "dblarr", SE_DOUBLE_2D, ndbl, offsetof(row_t, dblarr),
	   NULL);

    fail_unless (nrows == 16, "nrows was %d, should be 16", nrows);

    for (row = (row_t *)buf, i = 0; i < nrows; ++i) {
	fail_unless (row[i].index == i+1,
		     "row[%d].index was %d, should be %d",
		     i, row[i].index, i+1);
	fail_unless (row[i].mass == 7.0*i,
		     "row[%d].mass was %g, should be %g",
		     i, row[i].mass, 7.0*i);
	fail_unless (row[i].radius == 17.0*i,
		     "row[%d].radius was %g, should be %g",
		     i, row[i].radius, 17.0*i);
	fail_unless (row[i].rho == 23.0*i,
		     "row[%d].rho was %g, should be %g",
		     i, row[i].rho, 23.0*i);
	for (j = 0; j < nint; ++j)
	    fail_unless (row[i].intarr[j] == i*13+j*9,
			 "row[%d].intarr[%d] was %d, should be %d",
			 i, j, row[i].intarr[j], i*13+j*9);
	for (j = 0; j < nflt; ++j)
	    fail_unless (row[i].fltarr[j] == (float) i*7.+j*39.,
			 "row[%d].fltarr[%d] was %g, should be %g",
			 i, j, row[i].fltarr[j],(float) i*7.+j*39.);
	for (j = 0; j < ndbl; ++j)
	    fail_unless (row[i].dblarr[j] == (double) i*37.+j*7.,
			 "row[%d].dblarr[%d] was %g, should be %g",
			 i, j, row[i].dblarr[j],(double) i*37.+j*7.);
    }

    free(buf);

    /* Read only some members from cycle 2 */
    SEread(file_id, 2, &nrows, &buf, sizeof(shortrow_t),
	   "radius", SE_DOUBLE, offsetof(shortrow_t, radius),
	   "rho", SE_FLOAT, offsetof(shortrow_t, rho),
	   NULL);

    fail_unless (nrows == 16, "nrows was %d, should be 16", nrows);

    for (srow = (shortrow_t *)buf, i = 0; i < nrows; ++i) {
	fail_unless (srow[i].radius == 17.0*i,
		     "srow[%d].radius was %g, should be %g",
		     i, srow[i].radius, 17.0*i);
	fail_unless (srow[i].rho == 23.0*i,
		     "srow[%d].rho was %g, should be %g",
		     i, srow[i].rho, 23.0*i);
    }

    free(buf);
}
END_TEST


START_TEST(readwrite_arrayattr)
{
    void *buf;
    row_t *row;
    shortrow_t *srow;
    int i, j, nrows, narrays, cycle = 1, index[16];
    float rho[16];
    double mass[16], radius[16];
    int int2d[nint][16];
    float flt2d[nflt][16];
    double dbl2d[ndbl][16];
    int itype = 0;
    int ftype = 1;
    int dtype = 2;
    int i2dtype = 10;
    int f2dtype = 11;
    int d2dtype = 12;

    char indexname[6];
    char massname[5];
    char radiusname[7];
    char rhoname[4];
    char intarrname[7];
    char fltarrname[7];
    char dblarrname[7];
    char dsetname[]="testdata";
    int dsetnchar;
    int ilen, mlen, ralen, rholen, intarrlen, fltarrlen, dblarrlen;

    row = (row_t *)malloc(16 * sizeof(row_t));

    for (i = 0; i < 16; ++i) {
	row[i].index = index[i] = i+1;
	row[i].mass = mass[i] = 7.0*i;
	row[i].radius = radius[i] = 17.0*i;
	row[i].rho = rho[i] = (float)23.0*i;
	for (j = 0; j < nint; ++j)
	    row[i].intarr[j] = int2d[j][i] = i*13+j*9;
	for (j = 0; j < nflt; ++j)
	    row[i].fltarr[j] = flt2d[j][i] = i*7.+j*39.;
	for (j = 0; j < ndbl; ++j)
	    row[i].dblarr[j] = dbl2d[j][i] = i*37.+j*7.;
   }

    /* Write data set "testdata" */
    SEwriteDataset(file_id, 1, "testdata", 16, row, sizeof(row_t),
		   "index", SE_INT, offsetof(row_t, index),
		   "mass", SE_DOUBLE, offsetof(row_t, mass),
		   "radius", SE_DOUBLE, offsetof(row_t, radius),
		   "rho", SE_FLOAT, offsetof(row_t, rho),
		   "intarr", SE_INT_2D, nint, offsetof(row_t, intarr),
		   "fltarr", SE_FLOAT_2D, nflt, offsetof(row_t, fltarr),
		   "dblarr", SE_DOUBLE_2D, ndbl, offsetof(row_t, dblarr),
		   NULL);

    /* Write only some of the row_t structure members in cycle 2*/
    SEwriteDataset(file_id, 2, "testdata", 16, row, sizeof(row_t),
		   "rho", SE_FLOAT, offsetof(row_t, rho),
		   "index", SE_INT, offsetof(row_t, index),
		   "radius", SE_DOUBLE, offsetof(row_t, radius),
		   "dblarr", SE_DOUBLE_2D, ndbl, offsetof(row_t, dblarr),
		   "intarr", SE_INT_2D, nint, offsetof(row_t, intarr),
		   NULL);

    free(row);

    /* Read cycle 1 */
    SEreadDataset(file_id, cycle, "testdata", &nrows, &buf, sizeof(row_t),
		  "index", SE_INT, offsetof(row_t, index),
		  "mass", SE_DOUBLE, offsetof(row_t, mass),
		  "radius", SE_DOUBLE, offsetof(row_t, radius),
		  "rho", SE_FLOAT, offsetof(row_t, rho),
		  "intarr", SE_INT_2D, nint, offsetof(row_t, intarr),
		  "fltarr", SE_FLOAT_2D, nflt, offsetof(row_t, fltarr),
		  "dblarr", SE_DOUBLE_2D, ndbl, offsetof(row_t, dblarr),
		  NULL);

    fail_unless (nrows == 16, "nrows was %d, should be 16", nrows);

    for (row = (row_t *)buf, i = 0; i < nrows; ++i) {
	fail_unless (row[i].index == i+1,
		     "row[%d].index was %d, should be %d",
		     i, row[i].index, i+1);
	fail_unless (row[i].mass == 7.0*i,
		     "row[%d].mass was %g, should be %g",
		     i, row[i].mass, 7.0*i);
	fail_unless (row[i].radius == 17.0*i,
		     "row[%d].radius was %g, should be %g",
		     i, row[i].radius, 17.0*i);
	fail_unless (row[i].rho == 23.0*i,
		     "row[%d].rho was %g, should be %g",
		     i, row[i].rho, 23.0*i);
 	for (j = 0; j < nint; ++j)
	    fail_unless (row[i].intarr[j] == i*13+j*9,
			 "row[%d].intarr[%d] was %d, should be %d",
			 i, j, row[i].intarr[j], i*13+j*9);
	for (j = 0; j < nflt; ++j)
	    fail_unless (row[i].fltarr[j] == (float) i*7.+j*39.,
			 "row[%d].fltarr[%d] was %g, should be %g",
			 i, j, row[i].fltarr[j],(float) i*7.+j*39.);
	for (j = 0; j < ndbl; ++j)
	    fail_unless (row[i].dblarr[j] == (double) i*37.+j*7.,
			 "row[%d].dblarr[%d] was %g, should be %g",
			 i, j, row[i].dblarr[j],(double) i*37.+j*7.);
    }

    free(buf);

    /* Write (different) cycle 1 */
    nrows = 16;
    narrays = 7;

    strncpy(indexname, "index", sizeof(indexname));
    ilen = strlen(indexname);

    strncpy(massname, "mass", sizeof(massname));
    mlen = strlen(massname);

    strncpy(radiusname, "radius", sizeof(radiusname));
    ralen = strlen(radiusname);

    strncpy(rhoname, "rho", sizeof(rhoname));
    rholen = strlen(rhoname);

    strncpy(intarrname, "intarr", sizeof(intarrname));
    intarrlen = strlen(intarrname);

    strncpy(fltarrname, "fltarr", sizeof(fltarrname));
    fltarrlen = strlen(fltarrname);

    strncpy(dblarrname, "dblarr", sizeof(dblarrname));
    dblarrlen = strlen(dblarrname);

    dsetnchar=strlen(dsetname);
    FSEWRITEDATASET(&file_id, &cycle, dsetname, &dsetnchar,
		    &nrows, &narrays,
		    index, indexname, &itype,
		    mass, massname, &dtype,
		    radius, radiusname, &dtype,
		    rho, rhoname, &ftype,
		    dbl2d, dblarrname, &d2dtype, &nrows, &ndbl,
		    int2d, intarrname, &i2dtype, &nrows, &nint,
		    flt2d, fltarrname, &f2dtype, &nrows, &nflt,
		    ilen, mlen, ralen, rholen, dblarrlen, intarrlen,
		    fltarrlen);


    /* Re-read cycle 1 */
    SEreadDataset(file_id, cycle, "testdata", &nrows, &buf, sizeof(row_t),
		  "index", SE_INT, offsetof(row_t, index),
		  "mass", SE_DOUBLE, offsetof(row_t, mass),
		  "radius", SE_DOUBLE, offsetof(row_t, radius),
		  "rho", SE_FLOAT, offsetof(row_t, rho),
		  "intarr", SE_INT_2D, nint, offsetof(row_t, intarr),
		  "fltarr", SE_FLOAT_2D, nflt, offsetof(row_t, fltarr),
		  "dblarr", SE_DOUBLE_2D, ndbl, offsetof(row_t, dblarr),
		  NULL);

    fail_unless (nrows == 16, "nrows was %d, should be 16", nrows);

    for (row = (row_t *)buf, i = 0; i < nrows; ++i) {
	fail_unless (row[i].index == i+1,
		     "row[%d].index was %d, should be %d",
		     i, row[i].index, i+1);
	fail_unless (row[i].mass == 7.0*i,
		     "row[%d].mass was %g, should be %g",
		     i, row[i].mass, 7.0*i);
	fail_unless (row[i].radius == 17.0*i,
		     "row[%d].radius was %g, should be %g",
		     i, row[i].radius, 17.0*i);
	fail_unless (row[i].rho == 23.0*i,
		     "row[%d].rho was %g, should be %g",
		     i, row[i].rho, 23.0*i);
	for (j = 0; j < nint; ++j)
	    fail_unless (row[i].intarr[j] == i*13+j*9,
			 "row[%d].intarr[%d] was %d, should be %d",
			 i, j, row[i].intarr[j], i*13+j*9);
	for (j = 0; j < nflt; ++j)
	    fail_unless (row[i].fltarr[j] == (float) i*7.+j*39.,
			 "row[%d].fltarr[%d] was %g, should be %g",
			 i, j, row[i].fltarr[j],(float) i*7.+j*39.);
	for (j = 0; j < ndbl; ++j)
	    fail_unless (row[i].dblarr[j] == (double) i*37.+j*7.,
			 "row[%d].dblarr[%d] was %g, should be %g",
			 i, j, row[i].dblarr[j],(double) i*37.+j*7.);
    }

    free(buf);

    for (i = 0; i < nrows; ++i) {
	index[i]=0;
	mass[i]=0;
	radius[i]=0;
	rho[i]=0;	
    }


    /* Read only some members from cycle 2 */
    SEreadDataset(file_id, 2, "testdata", &nrows, &buf, sizeof(shortrow_t),
		  "radius", SE_DOUBLE, offsetof(shortrow_t, radius),
		  "rho", SE_FLOAT, offsetof(shortrow_t, rho),
		  NULL);

    fail_unless (nrows == 16, "nrows was %d, should be 16", nrows);

    for (srow = (shortrow_t *)buf, i = 0; i < nrows; ++i) {
	fail_unless (srow[i].radius == 17.0*i,
		     "srow[%d].radius was %g, should be %g",
		     i, srow[i].radius, 17.0*i);
	fail_unless (srow[i].rho == 23.0*i,
		     "srow[%d].rho was %g, should be %g",
		     i, srow[i].rho, 23.0*i);
    }

    free(buf);
}
END_TEST

START_TEST(int_attributes)
{
    int i, ti, cycle;

    /* Write file-level attributes */
    cycle = -1;

    ti = 42;
    SEwriteIAttr(file_id, cycle, "int", ti);
    i = SEreadIAttr(file_id, cycle, "int");
    fail_unless (i == ti, "\"int\" was %d, should be %d", i, ti);

    ti = 180;
    FSEWRITEIATTR(&file_id, &cycle, "fortran_int", &ti,
		  strlen("fortran_int"));
    FSEREADIATTR(&file_id, &cycle, "fortran_int", &i,
		 strlen("fortran_int"));
    fail_unless (i == ti, "\"int\" was %d, should be %d", i, ti);

    /* Write cycle-level attributes */
    cycle = 0;

    ti = -50001;
    SEwriteIAttr(file_id, cycle, "int", ti);
    i = SEreadIAttr(file_id, cycle, "int");
    fail_unless (i == ti, "\"int\" was %d, should be %d", i, ti);

    ti = -220566;
    FSEWRITEIATTR(&file_id, &cycle, "fortran_int", &ti,
		  strlen("fortran_int"));
    FSEREADIATTR(&file_id, &cycle, "fortran_int", &i,
		 strlen("fortran_int"));
    fail_unless (i == ti, "\"int\" was %d, should be %d", i, ti);

    /* Overwrite a cycle-level attribute and read the new value */
    ti = 98765;

    SEwriteIAttr(file_id, cycle, "int", ti);
    i = SEreadIAttr(file_id, cycle, "int");
    fail_unless (i == ti, "\"int\" was %d, should be %d", i, ti);

    /* Attach an attribute to a cycle that hasn't been written yet */
    cycle = 123;

    ti = 6152;
    SEwriteIAttr(file_id, cycle, "int", ti);
    i = SEreadIAttr(file_id, cycle, "int");
    fail_unless (i == ti, "\"int\" was %d, should be %d", i, ti);
}
END_TEST


START_TEST(float_attributes)
{
    float f, tf;
    int cycle;

    /* Write file-level attributes */
    cycle = -1;

    tf = 6.5e-4;
    SEwriteFAttr(file_id, cycle, "float", tf);
    f = SEreadFAttr(file_id, cycle, "float");
    fail_unless (f == tf, "\"float\" was %g, should be %g", f, tf);

    tf = 8.101e25;
    FSEWRITEFATTR(&file_id, &cycle, "fortran_float", &tf,
		  strlen("fortran_float"));
    FSEREADFATTR(&file_id, &cycle, "fortran_float", &f,
		 strlen("fortran_float"));
    fail_unless (f == tf, "\"float\" was %g, should be %g", f, tf);

    /* Write cycle-level attributes */
    cycle = 0;

    tf = 0.01;
    SEwriteFAttr(file_id, cycle, "float", tf);
    f = SEreadFAttr(file_id, cycle, "float");
    fail_unless (f == tf, "\"float\" was %g, should be %g", f, tf);

    tf = -7.44603e-12;
    FSEWRITEFATTR(&file_id, &cycle, "fortran_float", &tf,
		  strlen("fortran_float"));
    FSEREADFATTR(&file_id, &cycle, "fortran_float", &f,
		 strlen("fortran_float"));
    fail_unless (f == tf, "\"float\" was %g, should be %g", f, tf);

    /* Overwrite a cycle-level attribute and read the new value */
    tf = -4.2;

    SEwriteFAttr(file_id, cycle, "float", tf);
    f = SEreadFAttr(file_id, cycle, "float");
    fail_unless (f == tf, "\"float\" was %g, should be %g", f, tf);

    /* Attach an attribute to a cycle that hasn't been written yet */
    cycle = 127;

    tf = -8.059e6;
    SEwriteFAttr(file_id, cycle, "float", tf);
    f = SEreadFAttr(file_id, cycle, "float");
    fail_unless (f == tf, "\"float\" was %g, should be %g", f, tf);
}
END_TEST

START_TEST(double_attributes)
{
    double d, td;
    int cycle;

    /* Write file-level attributes */
    cycle = -1;

    td = 1.04e88;
    SEwriteDAttr(file_id, cycle, "double", td);
    d = SEreadDAttr(file_id, cycle, "double");
    fail_unless (d == td, "\"double\" was %g, should be %g", d, td);

    td = -7.7532e112;
    FSEWRITEDATTR(&file_id, &cycle, "fortran_double", &td,
		  strlen("fortran_double"));
    FSEREADDATTR(&file_id, &cycle, "fortran_double", &d,
		 strlen("fortran_double"));
    fail_unless (d == td, "\"double\" was %g, should be %g", d, td);

    /* Write cycle-level attributes */
    cycle = 0;

    td = 4.9e-50;
    SEwriteDAttr(file_id, cycle, "double", td);
    d = SEreadDAttr(file_id, cycle, "double");
    fail_unless (d == td, "\"double\" was %g, should be %g", d, td);

    td = 3.00656e-203;
    FSEWRITEDATTR(&file_id, &cycle, "fortran_double", &td,
		  strlen("fortran_double"));
    FSEREADDATTR(&file_id, &cycle, "fortran_double", &d,
		 strlen("fortran_double"));
    fail_unless (d == td, "\"double\" was %g, should be %g", d, td);

    /* Overwrite a cycle-level attribute and read the new value */
    td = -42.0;

    SEwriteDAttr(file_id, cycle, "double", td);
    d = SEreadDAttr(file_id, cycle, "double");
    fail_unless (d == td, "\"double\" was %g, should be %g", d, td);

    /* Attach an attribute to a cycle that hasn't been written yet */
    cycle = 129;

    td = -1.0091e49;
    SEwriteDAttr(file_id, cycle, "double", td);
    d = SEreadDAttr(file_id, cycle, "double");
    fail_unless (d == td, "\"double\" was %g, should be %g", d, td);
}
END_TEST

START_TEST(string_attributes)
{
    int cycle, len;
    char s[128], ts[128], fs[128];

    /* Write file-level attributes */
    cycle = -1;

    strncpy(ts, "Hello, world!", sizeof(ts));
    SEwriteSAttr(file_id, cycle, "string", ts);
    len = SEgetSAttrLength(file_id, cycle, "string");
    fail_unless (len == strlen(ts), "len was %d, should be %d",
		 len, strlen(ts));
    SEreadSAttr(file_id, cycle, "string", s, sizeof(s));
    fail_unless (strncmp(s, ts, sizeof(s)) == 0,
		 "\"string\" was \"%s\", should be \"%s\"", s, ts);

    FSEWRITESATTR(&file_id, &cycle, "fortran_string", ts,
		  strlen("fortran_string"), strlen(ts));
    FSEGETSATTRLENGTH(&file_id, &cycle, "fortran_string", &len,
		      strlen("fortran_string"));
    fail_unless (len == strlen(ts), "len was %d, should be %d",
		 len, strlen(ts));
    FSEREADSATTR(&file_id, &cycle, "fortran_string", fs,
		 strlen("fortran_string"), sizeof(fs));
    strtrim(s, fs, sizeof(fs));
    fail_unless (strncmp(s, ts, sizeof(s)) == 0,
		 "\"string\" was \"%s\", should be \"%s\"", s, ts);

    /* Overwrite a file-level attribute and read the new value */
    strncpy(ts, "Goodbye world!", sizeof(ts));

    SEwriteSAttr(file_id, cycle, "string", ts);
    SEreadSAttr(file_id, cycle, "string", s, sizeof(s));
    fail_unless (strncmp(s, ts, sizeof(s)) == 0,
		 "\"string\" was \"%s\", should be \"%s\"", s, ts);

    FSEGETSATTRLENGTH(&file_id, &cycle, "string", &len, strlen("string"));
    fail_unless (len == strlen(ts), "len was %d, should be %d",
		 len, strlen(ts));
    FSEREADSATTR(&file_id, &cycle, "string", fs, strlen("string"),
		 sizeof(fs));
    strtrim(s, fs, sizeof(fs));
    fail_unless (strncmp(s, ts, sizeof(s)) == 0,
		 "\"string\" was \"%s\", should be \"%s\"", s, ts);

    /* Write cycle-level attributes */
    cycle = 0;

    strncpy(ts, "Mr. Watson, come here. I need you.", sizeof(ts));
    SEwriteSAttr(file_id, cycle, "string", ts);
    len = SEgetSAttrLength(file_id, cycle, "string");
    fail_unless (len == strlen(ts), "len was %d, should be %d",
		 len, strlen(ts));
    SEreadSAttr(file_id, cycle, "string", s, sizeof(s));
    fail_unless (strncmp(s, ts, sizeof(s)) == 0,
		 "\"string\" was \"%s\", should be \"%s\"", s, ts);

    FSEGETSATTRLENGTH(&file_id, &cycle, "string", &len, strlen("string"));
    fail_unless (len == strlen(ts), "len was %d, should be %d",
		 len, strlen(ts));
    FSEREADSATTR(&file_id, &cycle, "string", fs, strlen("string"),
		 sizeof(fs));
    strtrim(s, fs, sizeof(fs));
    fail_unless (strncmp(s, ts, sizeof(s)) == 0,
		 "\"string\" was \"%s\", should be \"%s\"", s, ts);
}
END_TEST


START_TEST(arrayint_attributes)
{
    int i, *ti, cycle, *to, nto, nti;

    /* Write file-level attributes */
    cycle = -1;
    nti=10;
    ti=malloc(nti*sizeof(int));
    for (i=0; i<nti; i++) ti[i] = i;
    SEwriteIArrayAttr(file_id, cycle, "int", ti, nti);
    SEreadIArrayAttr(file_id, cycle, "int", &to, &nto);
    
    for (i=0; i<nti; i++)
	fail_unless (to[i] == ti[i], "\"int\" was %d, should be %d", 
		     to[i], ti[i]);

    for (i=0; i<nti; i++) ti[i] = 10*i;
    FSEWRITEIARRAYATTR(&file_id, &cycle, "fortran_int", ti, &nti,
		       strlen("fortran_int"));
    FSEREADIARRAYATTR(&file_id, &cycle, "fortran_int", to, &nto,
		      strlen("fortran_int"));
    for (i=0; i<nti; i++)
	fail_unless (to[i] == ti[i], "\"int\" was %d, should be %d", 
		     to[i], ti[i]);

    /* Write cycle-level attributes */
    cycle = 0;
    free(to);

    for (i=0; i<nti; i++) ti[i] = i-50001;
    SEwriteIArrayAttr(file_id, cycle, "int", ti, nti);
    SEreadIArrayAttr(file_id, cycle, "int", &to, &nto);
    for (i=0; i<nti; i++)
	fail_unless (to[i] == ti[i], "\"int\" was %d, should be %d", 
		     to[i], ti[i]);

    for (i=0; i<nti; i++) ti[i] = i-220566;
    FSEWRITEIARRAYATTR(&file_id, &cycle, "fortran_int", ti, &nti,
		  strlen("fortran_int"));
    FSEREADIARRAYATTR(&file_id, &cycle, "fortran_int", to, &nto,
		 strlen("fortran_int"));
    for (i=0; i<nti; i++)
	fail_unless (to[i] == ti[i], "\"int\" was %d, should be %d", 
		     to[i], ti[i]);

    /* Overwrite a cycle-level attribute and read the new value */
    for (i=0; i<nti; i++) ti[i] = i+98765;
    free(to);

    SEwriteIArrayAttr(file_id, cycle, "int", ti, nti);
    SEreadIArrayAttr(file_id, cycle, "int", &to, &nto);
    for (i=0; i<nti; i++)
	fail_unless (to[i] == ti[i], "\"int\" was %d, should be %d", 
		     to[i], ti[i]);
}
END_TEST


START_TEST(arrayfloat_attributes)
{
    int i, cycle, nto, nti;
    float *ti, *to;

    /* Write file-level attributes */
    cycle = -1;
    nti=10;
    ti=malloc(nti*sizeof(float));
    for (i=0; i<nti; i++) ti[i] = i*1.232;
    SEwriteFArrayAttr(file_id, cycle, "float", ti, nti);
    SEreadFArrayAttr(file_id, cycle, "float", &to, &nto);
    
    for (i=0; i<nti; i++)
	fail_unless (to[i] == ti[i], "\"float\" was %g, should be %g", 
		     to[i], ti[i]);

    for (i=0; i<nti; i++) ti[i] = 10*i;
    FSEWRITEFARRAYATTR(&file_id, &cycle, "fortran_float", ti, &nti,
		       strlen("fortran_float"));
    FSEREADFARRAYATTR(&file_id, &cycle, "fortran_float", to, &nto,
		      strlen("fortran_float"));
    for (i=0; i<nti; i++)
	fail_unless (to[i] == ti[i], "\"float\" was %g, should be %g", 
		     to[i], ti[i]);

    /* Write cycle-level attributes */
    cycle = 0;
    free(to);

    for (i=0; i<nti; i++) ti[i] = i-50001;
    SEwriteFArrayAttr(file_id, cycle, "float", ti, nti);
    SEreadFArrayAttr(file_id, cycle, "float", &to, &nto);
    for (i=0; i<nti; i++)
	fail_unless (to[i] == ti[i], "\"float\" was %g, should be %g", 
		     to[i], ti[i]);

    for (i=0; i<nti; i++) ti[i] = i-220566;
    FSEWRITEFARRAYATTR(&file_id, &cycle, "fortran_float", ti, &nti,
		  strlen("fortran_float"));
    FSEREADFARRAYATTR(&file_id, &cycle, "fortran_float", to, &nto,
		 strlen("fortran_float"));
    for (i=0; i<nti; i++)
	fail_unless (to[i] == ti[i], "\"float\" was %g, should be %g", 
		     to[i], ti[i]);

    /* Overwrite a cycle-level attribute and read the new value */
    for (i=0; i<nti; i++) ti[i] = i+98765;
    free(to);

    SEwriteFArrayAttr(file_id, cycle, "float", ti, nti);
    SEreadFArrayAttr(file_id, cycle, "float", &to, &nto);
    for (i=0; i<nti; i++)
	fail_unless (to[i] == ti[i], "\"float\" was %g, should be %g", 
		     to[i], ti[i]);
}
END_TEST


START_TEST(arraydouble_attributes)
{
    int i, cycle, nto, nti;
    double *ti, *to;

    /* Write file-level attributes */
    cycle = -1;
    nti=10;
    ti=malloc(nti*sizeof(double));
    for (i=0; i<nti; i++) ti[i] = i*2.43423e8;
    SEwriteDArrayAttr(file_id, cycle, "double", ti, nti);
    SEreadDArrayAttr(file_id, cycle, "double", &to, &nto);
    
    for (i=0; i<nti; i++)
	fail_unless (to[i] == ti[i], "\"double\" was %g, should be %g", 
		     to[i], ti[i]);

    for (i=0; i<nti; i++) ti[i] = 10*i;
    FSEWRITEDARRAYATTR(&file_id, &cycle, "fortran_double", ti, &nti,
		       strlen("fortran_double"));
    FSEREADDARRAYATTR(&file_id, &cycle, "fortran_double", to, &nto,
		      strlen("fortran_double"));
    for (i=0; i<nti; i++)
	fail_unless (to[i] == ti[i], "\"double\" was %g, should be %g", 
		     to[i], ti[i]);

    /* Write cycle-level attributes */
    cycle = 0;
    free(to);

    for (i=0; i<nti; i++) ti[i] = i-50001;
    SEwriteDArrayAttr(file_id, cycle, "double", ti, nti);
    SEreadDArrayAttr(file_id, cycle, "double", &to, &nto);
    for (i=0; i<nti; i++)
	fail_unless (to[i] == ti[i], "\"double\" was %g, should be %g", 
		     to[i], ti[i]);

    for (i=0; i<nti; i++) ti[i] = i-220566;
    FSEWRITEDARRAYATTR(&file_id, &cycle, "fortran_double", ti, &nti,
		  strlen("fortran_double"));
    FSEREADDARRAYATTR(&file_id, &cycle, "fortran_double", to, &nto,
		 strlen("fortran_double"));
    for (i=0; i<nti; i++)
	fail_unless (to[i] == ti[i], "\"double\" was %g, should be %g", 
		     to[i], ti[i]);

    /* Overwrite a cycle-level attribute and read the new value */
    for (i=0; i<nti; i++) ti[i] = i+98765;
    free(to);

    SEwriteDArrayAttr(file_id, cycle, "double", ti, nti);
    SEreadDArrayAttr(file_id, cycle, "double", &to, &nto);
    for (i=0; i<nti; i++)
	fail_unless (to[i] == ti[i], "\"double\" was %g, should be %g", 
		     to[i], ti[i]);
}
END_TEST




START_TEST(fixed_strings)
{
    char s[128], ts[128], fs[128];
    int cycle = -1;
    hid_t str_id = H5Tcopy(H5T_C_S1), attr_id, attrspace_id;
    hsize_t attrdim = 1;

    strncpy(ts, "This is a fixed-length string.", sizeof(ts));
    H5Tset_size(str_id, strlen(ts));

    attrspace_id = H5Screate_simple(1, &attrdim, NULL);
    attr_id = H5Acreate(file_id, "string", str_id, attrspace_id, H5P_DEFAULT);
    H5Awrite(attr_id, str_id, ts);

    H5Aclose(attr_id);
    H5Sclose(attrspace_id);
    H5Tclose(str_id);

    SEreadSAttr(file_id, cycle, "string", s, sizeof(s));
    fail_unless (strncmp(s, ts, sizeof(s)) == 0,
		 "\"string\" was \"%s\", should be \"%s\"", s, ts);
    FSEREADSATTR(&file_id, &cycle, "string", fs, strlen("string"),
		 sizeof(fs));
    strtrim(s, fs, sizeof(fs));
    fail_unless (strncmp(s, ts, sizeof(s)) == 0,
		 "\"string\" was \"%s\", should be \"%s\"", s, ts);
}
END_TEST

START_TEST(comments)
{
    char s[128], ts1[128] = "This is my comment.",
	ts2[128] = "There are many like it, but this one is mine.";
    int comment;

    SEaddComment(file_id, ts1);
    FSEADDCOMMENT(&file_id, ts2, strlen(ts2));

    comment = 1;
    SEreadComment(file_id, comment, s, sizeof(s));
    fail_unless (strncmp(s, ts2, sizeof(s)) == 0,
		 "comment %d was \"%s\", should be \"%s\"", comment, s, ts2);

    comment = 0;
    FSEREADCOMMENT(&file_id, &comment, s, sizeof(s));
    fail_unless (strncmp(s, ts1, sizeof(s)) == 0,
		 "comment %d was \"%s\", should be \"%s\"", comment, s, ts1);
}
END_TEST

/*
 * Files from version 1.0 had one top-level dataset per cycle.  Scalar
 * attributes were attached directly to that dataset.  Array
 * attributes were not supported.
 *
 * Moving to v1.1 and later requires renaming each dataset to
 * SE_DATASET, moving that dataset into a group named according to the
 * cycle, and moving each attribute from the dataset to the group.
 *
 * Moving to v1.2 and later requires renaming each group to use the
 * new cycle-naming scheme.
 */
static void setup_v10(void)
{
    hid_t fapl_id, dset_id, space_id, attr_id;
    int val;
    hsize_t space_dim = 1;

    /* Create file in memory, using CORE driver */
    fapl_id = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fapl_core(fapl_id, 256*1024, 0);
    file_id = H5Fcreate("dummy", H5F_ACC_EXCL, H5P_DEFAULT, fapl_id);
    H5Pclose(fapl_id);

    /* Create a v1.0 file: three cycles with the old style "cycle-#"
       names, each with scalar attributes attached. */
    space_id = H5Screate_simple(1, &space_dim, NULL);

    dset_id = H5Dcreate(file_id, "cycle-1", H5T_NATIVE_INT, space_id,
			H5P_DEFAULT);
    attr_id = H5Acreate(dset_id, "attr-1", H5T_NATIVE_INT, space_id,
			H5P_DEFAULT);
    val = 1;
    H5Awrite(attr_id, H5T_NATIVE_INT, &val);
    H5Aclose(attr_id);
    attr_id = H5Acreate(dset_id, "attr-2", H5T_NATIVE_INT, space_id,
			H5P_DEFAULT);
    val = 2;
    H5Awrite(attr_id, H5T_NATIVE_INT, &val);
    H5Aclose(attr_id);
    H5Dclose(dset_id);

    dset_id = H5Dcreate(file_id, "cycle-321", H5T_NATIVE_INT, space_id,
			H5P_DEFAULT);
    attr_id = H5Acreate(dset_id, "attr-1", H5T_NATIVE_INT, space_id,
			H5P_DEFAULT);
    val = 1;
    H5Awrite(attr_id, H5T_NATIVE_INT, &val);
    H5Aclose(attr_id);
    H5Dclose(dset_id);

    dset_id = H5Dcreate(file_id, "cycle-4", H5T_NATIVE_INT, space_id,
			H5P_DEFAULT);
    attr_id = H5Acreate(dset_id, "attr-4", H5T_NATIVE_INT, space_id,
			H5P_DEFAULT);
    val = 4;
    H5Awrite(attr_id, H5T_NATIVE_INT, &val);
    H5Aclose(attr_id);
    attr_id = H5Acreate(dset_id, "attr-7", H5T_NATIVE_INT, space_id,
			H5P_DEFAULT);
    val = 7;
    H5Awrite(attr_id, H5T_NATIVE_INT, &val);
    H5Aclose(attr_id);
    H5Dclose(dset_id);

    /* Add a file-level attribute */
    attr_id = H5Acreate(file_id, "attr-43", H5T_NATIVE_INT, space_id,
			H5P_DEFAULT);
    val = 43;
    H5Awrite(attr_id, H5T_NATIVE_INT, &val);
    H5Aclose(attr_id);

    H5Sclose(space_id);
}

/*
 * Files from version 1.1 had one top-level group per cycle,
 * containing a dataset named SE_DATASET.  Scalar attributes were
 * attached to the group; array attributes were datasets under the
 * same group, named according to the desired name of the attribute.
 *
 * Moving to v1.2 and later requires renaming each group to use the
 * new cycle-naming scheme.
 */
static void setup_v11(void)
{
    hid_t fapl_id, group_id, dset_id, space_id, attr_id;
    int val;
    hsize_t space_dim = 1;

    /* Create file in memory, using CORE driver */
    fapl_id = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fapl_core(fapl_id, 256*1024, 0);
    file_id = H5Fcreate("dummy", H5F_ACC_EXCL, H5P_DEFAULT, fapl_id);
    H5Pclose(fapl_id);

    /* Create a v1.1 file: three cycles, each in separate groups, with
       some scalar and array attributes. */
    SEwriteVersion(file_id, "1.1");

    space_id = H5Screate_simple(1, &space_dim, NULL);

    group_id = H5Gcreate(file_id, "cycle-1", 0);
    dset_id = H5Dcreate(group_id, "SE_DATASET", H5T_NATIVE_INT, space_id,
			H5P_DEFAULT);
    attr_id = H5Acreate(group_id, "attr-1", H5T_NATIVE_INT, space_id,
			H5P_DEFAULT);
    val = 1;
    H5Awrite(attr_id, H5T_NATIVE_INT, &val);
    H5Aclose(attr_id);
    attr_id = H5Acreate(group_id, "attr-2", H5T_NATIVE_INT, space_id,
			H5P_DEFAULT);
    val = 2;
    H5Awrite(attr_id, H5T_NATIVE_INT, &val);
    H5Aclose(attr_id);
    H5Dclose(dset_id);
    H5Gclose(group_id);

    group_id = H5Gcreate(file_id, "cycle-321", 0);
    dset_id = H5Dcreate(group_id, "SE_DATASET", H5T_NATIVE_INT, space_id,
			H5P_DEFAULT);
    attr_id = H5Acreate(group_id, "attr-1", H5T_NATIVE_INT, space_id,
			H5P_DEFAULT);
    val = 1;
    H5Awrite(attr_id, H5T_NATIVE_INT, &val);
    H5Aclose(attr_id);
    H5Dclose(dset_id);
    H5Gclose(group_id);

    group_id = H5Gcreate(file_id, "cycle-4", 0);
    dset_id = H5Dcreate(group_id, "SE_DATASET", H5T_NATIVE_INT, space_id,
			H5P_DEFAULT);
    attr_id = H5Acreate(group_id, "attr-4", H5T_NATIVE_INT, space_id,
			H5P_DEFAULT);
    val = 4;
    H5Awrite(attr_id, H5T_NATIVE_INT, &val);
    H5Aclose(attr_id);
    attr_id = H5Acreate(group_id, "attr-7", H5T_NATIVE_INT, space_id,
			H5P_DEFAULT);
    val = 7;
    H5Awrite(attr_id, H5T_NATIVE_INT, &val);
    H5Aclose(attr_id);
    H5Dclose(dset_id);
    H5Gclose(group_id);

    /* Add a file-level attribute */
    attr_id = H5Acreate(file_id, "attr-43", H5T_NATIVE_INT, space_id,
			H5P_DEFAULT);
    val = 43;
    H5Awrite(attr_id, H5T_NATIVE_INT, &val);
    H5Aclose(attr_id);

    H5Sclose(space_id);
}

START_TEST(update)
{
    int iattr, ncycles, *cycles;
    char se_version[32];

    _SE_update_internal_layout(file_id);

    SEreadSAttr(file_id, -1, "SE_version", se_version, sizeof(se_version));

    fail_unless (strncmp(se_version, PACKAGE_VERSION,
			 strlen(se_version) < strlen(PACKAGE_VERSION) ?
			 strlen(se_version) : strlen(PACKAGE_VERSION)) == 0,
		 "SE_version is %s, should be %s", se_version,
		 PACKAGE_VERSION);

    ncycles = SEncycles(file_id);
    fail_unless (ncycles == 3, "ncycles was %d, should be 3", ncycles);

    cycles = (int *)malloc(ncycles * sizeof(int));

    SEcycles(file_id, cycles, ncycles);

    fail_unless (cycles[0] == 1, "cycles[0] was %d, should be 1", cycles[0]);
    fail_unless (cycles[1] == 4, "cycles[1] was %d, should be 4", cycles[1]);
    fail_unless (cycles[2] == 321, "cycles[2] was %d, should be 321",
		 cycles[2]);

    free(cycles);

    iattr = SEreadIAttr(file_id, 1, "attr-1");
    fail_unless (iattr == 1, "iattr was %d, should be 1", iattr);
    iattr = SEreadIAttr(file_id, 1, "attr-2");
    fail_unless (iattr == 2, "iattr was %d, should be 2", iattr);
    iattr = SEreadIAttr(file_id, 321, "attr-1");
    fail_unless (iattr == 1, "iattr was %d, should be 1", iattr);
    iattr = SEreadIAttr(file_id, 4, "attr-4");
    fail_unless (iattr == 4, "iattr was %d, should be 4", iattr);
    iattr = SEreadIAttr(file_id, 4, "attr-7");
    fail_unless (iattr == 7, "iattr was %d, should be 7", iattr);
    iattr = SEreadIAttr(file_id, -1, "attr-43");
    fail_unless (iattr == 43, "iattr was %d, should be 43", iattr);
}
END_TEST

Suite *create_suite(void)
{
    Suite *s = suite_create("tests");
    TCase *tcquery = tcase_create("query");
    TCase *tcdata = tcase_create("datasets");
    TCase *tcattr = tcase_create("attributes");
    TCase *tccompat_v10 = tcase_create("compat_v1.0");
    TCase *tccompat_v11 = tcase_create("compat_v1.1");

    tcase_add_checked_fixture(tcquery, setup, teardown);
    tcase_add_test(tcquery, ncycles);
    tcase_add_test(tcquery, nshells);
    tcase_add_test(tcquery, cycles);
    tcase_add_test(tcquery, lastcycle);
    tcase_add_test(tcquery, members);
    suite_add_tcase(s, tcquery);

    tcase_add_checked_fixture(tcdata, setup, teardown);
    tcase_add_test(tcdata, read);
    tcase_add_test(tcdata, readwrite); 
    tcase_add_test(tcdata, readwrite_arrayattr); 
    suite_add_tcase(s, tcdata);

    tcase_add_checked_fixture(tcattr, setup, teardown);
    tcase_add_test(tcattr, int_attributes);
    tcase_add_test(tcattr, float_attributes);
    tcase_add_test(tcattr, double_attributes);
    tcase_add_test(tcattr, string_attributes);
    tcase_add_test(tcattr, fixed_strings);
    tcase_add_test(tcattr, comments);
    tcase_add_test(tcattr, arrayint_attributes);
    tcase_add_test(tcattr, arrayfloat_attributes);
    tcase_add_test(tcattr, arraydouble_attributes);
    suite_add_tcase(s, tcattr);

    tcase_add_checked_fixture(tccompat_v10, setup_v10, teardown);
    tcase_add_test(tccompat_v10, update);
    suite_add_tcase(s, tccompat_v10);

    tcase_add_checked_fixture(tccompat_v11, setup_v11, teardown);
    tcase_add_test(tccompat_v11, update);
    suite_add_tcase(s, tccompat_v11);

    return s;
}
