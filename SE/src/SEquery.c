#include <string.h>
#include <assert.h>
#include <hdf5.h>
#include <SE.h>
#include "FSE.h"
#include "SEprivatequery.h"


static int cmpInt(const void *a, const void *b);

/**
 * @ingroup Query
 * SEncycles: return number of cycles in an HDF5 file.
 *
 * @todo Add more Documentation
 *
 * @param 
 *
 * @return 
 */
int SEncycles(int ifile_id)
{
    hid_t file_id=(hid_t)ifile_id;
    int index = 0, ncycles = 0;

    H5E_BEGIN_TRY {
	while ( (H5Giterate(file_id, "/", &index, isCycle, NULL)) > 0 )
	    ++ncycles;
    } H5E_END_TRY;

    return ncycles;
}

void FSENCYCLES(int *file_id, int *ncycles)
{
    *ncycles = SEncycles(*file_id);
}

/**
 * @ingroup Query
 * Get number of shells in cycle. 
 *
 * Right now this assumes that the dataset is one-dimensional;
 * i.e. nshells = dims[0].
 * @todo Add more Documentation
 *
 * @param 
 *
 * @return 
 */
int SEnshells(int ifile_id, int cycle)
{
    hid_t file_id=(hid_t)ifile_id, dset_id, space_id;
    int ndims, nshells;
    hsize_t *dims;

    dset_id = SEdatasetFromCycle(file_id, cycle);
    space_id = H5Dget_space(dset_id);

    ndims = H5Sget_simple_extent_ndims(space_id);
    dims = (hsize_t *)malloc(ndims * sizeof(hsize_t));
    H5Sget_simple_extent_dims(space_id, dims, NULL);

    nshells = (int)dims[0];

    free(dims);
    H5Sclose(space_id);
    H5Dclose(dset_id);

    return nshells;
}

void FSENSHELLS(int *file_id, int *cycle, int *nshells)
{
    *nshells = SEnshells(*file_id, *cycle);
}

/*
 * SEnentries: get number of entries in a dataset member
 *
 * Returns one for scalar dataset members, and the number of elements
 * in an array member.
 */
int SEnentries(int ifile_id, int cycle, char *name)
{
    hid_t file_id = (hid_t)ifile_id, dset_id, type_id, arrtype_id;
    int ndims, dim;
    hsize_t *dims;

    dset_id = SEdatasetFromCycle(file_id, cycle);
    type_id = H5Dget_type(dset_id);
    arrtype_id = H5Tget_member_type(type_id,
				    H5Tget_member_index(type_id, name));

    if (H5Tget_class(arrtype_id) == H5T_ARRAY) {
	ndims = H5Tget_array_ndims(arrtype_id);

	dims = (hsize_t *)malloc(ndims * sizeof(hsize_t));
	H5Tget_array_dims(arrtype_id, dims, NULL);
	dim = dims[0];
	free(dims);
    }
    else {
	dim = 1;
    }

    H5Tclose(arrtype_id);
    H5Tclose(type_id);
    H5Dclose(dset_id);

    return dim;
}

void FSENENTRIES(int *ifile_id, int *cycle, char *name, int *n, int len)
{
    char *cname = (char *)malloc((len + 1) * sizeof(char));

    strtrim(cname, name, len);

    *n = SEnentries(*ifile_id, *cycle, cname);

    free(cname);
}

/**
 * @ingroup Query
 * Fill an integer array with all the cycles in filename
 *
 * @todo Add more Documentation
 *
 * @param 
 *
 * @return 

 * @bug Right now ncycles is just used in an assertion.  What should really
 * happen if i > ncycles?
 */
void SEcycles(int ifile_id, int *cycles, int ncycles)
{
    hid_t file_id=(hid_t)ifile_id;
    int index = 0, i = 0;
    char dsetname[256];

    H5E_BEGIN_TRY {
	while ( (H5Giterate(file_id, "/", &index, isCycle, dsetname)) > 0 )
	    cycles[i++] = SEcycleFromName(dsetname);
    } H5E_END_TRY;

    assert(i == ncycles);

    qsort(cycles, ncycles, sizeof(int), cmpInt);
}

void FSECYCLES(int *file_id, int *cycles, int *ncycles)
{
    SEcycles(*file_id, cycles, *ncycles);
}

/**
 * @ingroup Query
 * Returns the highest cycle number in filename.
 * 
 * Calls SEcycles to get all cycle names in filename, then sorts in
 * ascending order and returns the last one.
 * @todo Add more Documentation
 *
 * @param 
 *
 * @return 
 */
int SElastcycle(int ifile_id)
{
    hid_t file_id=(hid_t)ifile_id;
    int ncycles = SEncycles(file_id);
    int *cycles = (int *)malloc(ncycles * sizeof(int));
    int lastcycle;

    SEcycles(file_id, cycles, ncycles);
    lastcycle = cycles[ncycles-1];

    free(cycles);

    return lastcycle;
}

void FSELASTCYCLE(int *file_id, int *lastcycle)
{
    *lastcycle = SElastcycle(*file_id);
}

/*
 * cmpInt: return (-1, 0, 1) if a is (<, =, >) b
 */
static int cmpInt(const void *a, const void *b)
{
    const int ia = *(const int *)a, ib = *(const int *)b;

    if (ia < ib) return -1;
    else if (ia > ib) return 1;
    else return 0;
}

/**
 * @ingroup Query
 * 
 * @todo Add more Documentation
 *
 * @param 
 *
 * @return 
 */
int SEget_nmembers(int ifile_id, int cycle)
{
    hid_t file_id=(hid_t)ifile_id, dset_id;
    hid_t h5shell_type;
    int nmembers=0;

    dset_id = SEdatasetFromCycle(file_id, cycle);
    h5shell_type = H5Dget_type(dset_id);

    /* Get the number of members in the compound type in cycle */
    nmembers=H5Tget_nmembers(h5shell_type);

    H5Tclose(h5shell_type);
    H5Dclose(dset_id);

    return nmembers;
}

void FSEGETNMEMBERS(int *ifile_id, int *cycle, int *nmembers)
{
    *nmembers = SEget_nmembers(*ifile_id, *cycle);
}

/**
 * @ingroup Query
 * 
 * @todo Add more Documentation
 *
 * @param 
 *
 * @return 
 */
char *SEget_member_name(int ifile_id, int cycle, int member_id)
{
    hid_t file_id=(hid_t)ifile_id, dset_id;
    hid_t h5shell_type;
    unsigned field_idx=(unsigned) member_id;
    char *membername;

    dset_id = SEdatasetFromCycle(file_id, cycle);
    h5shell_type = H5Dget_type(dset_id);

    /* Get the member name in the compound type in cycle */
    membername=H5Tget_member_name(h5shell_type, field_idx); 

    H5Tclose(h5shell_type);
    H5Dclose(dset_id);

    return membername;
}

void FSEGETMEMBERNAME(int *ifile_id, int *cycle, int *member_id, char *name,
		      int len)
{
    int i;
    char *membername = SEget_member_name(*ifile_id, *cycle, *member_id);

    strncpy(name, membername, len);

    for (i = strlen(membername); i < len; ++i)
	name[i] = ' ';

    free(membername);
}

/**
 * @ingroup Query
 * 
 * @todo Add more Documentation
 *
 * @param 
 *
 * @return 
 */
int SEget_ncomments(int ifile_id)
{
    hid_t file_id = (hid_t)ifile_id, group_id, dset_id, space_id;
    hsize_t ncomments;

    H5E_BEGIN_TRY {
	group_id = H5Gopen(file_id, "/SE_COMMENT");
	dset_id = H5Dopen(group_id, "COMMENTS");
    } H5E_END_TRY;

    if ((group_id < 0) || (dset_id < 0)) return 0;

    space_id = H5Dget_space(dset_id);
    H5Sget_simple_extent_dims(space_id, &ncomments, NULL);

    H5Sclose(space_id);
    H5Dclose(dset_id);
    H5Gclose(group_id);

    return ncomments;
}
