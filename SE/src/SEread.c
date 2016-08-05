#include <string.h>
#include <assert.h>
#include <hdf5.h>
#include <SE.h>
#include "FSE.h"
#include "SEprivatequery.h"
#include "SEprivateopen.h"

static int lmin = -1, lmax = -1;

/**
 * @ingroup ReadWrite
 * 
 * @todo Add more Documentation
 *
 * @param 
 *
 * @return 
 */
void SEread_group_name(int ifile_id, char *groupname, char *dsetname, 
		       int *nshells, void **buf, size_t stride, va_list ap)
{
    hid_t file_id = (hid_t)ifile_id, space_id, dset_id, type_id = 0;
    hid_t h5shell_type, memtype_id = 0, memspace_id, group_id;
    SEtype_t type;
    size_t offset = 0, dim;
    hsize_t space_dim[1], rank, arraydim[1];
    hsize_t start, count;
    herr_t status;
    char *varname;
    
    group_id=SEgroupopen(file_id, groupname);    
    dset_id = H5Dopen(group_id, dsetname);

    h5shell_type = H5Dget_type(dset_id);

    space_id = H5Dget_space(dset_id);
    rank = H5Sget_simple_extent_ndims(space_id);
    status = H5Sget_simple_extent_dims(space_id, space_dim, NULL);
    *nshells = (int)space_dim[0];

    if ( (lmin < 0) || (lmin > *nshells - 1) ) start = 0;
    else start = lmin;

    if ( (lmax < 0) || (lmax > *nshells - 1) ) count = *nshells - start;
    else count = lmax - start + 1;

    *nshells = count;

    H5Sselect_hyperslab(space_id, H5S_SELECT_SET, &start, NULL, &count, NULL);

    memtype_id = H5Tcreate(H5T_COMPOUND, stride);
    memspace_id = H5Screate_simple(1, &count, NULL);

/*     va_start(ap, stride); */

    while( (varname = va_arg(ap, char *)) != NULL ) {
	/* Check to see if varname exists in h5shell_type */
	type = va_arg(ap, SEtype_t);
	if (type == SE_INT_2D || type == SE_FLOAT_2D || type == SE_DOUBLE_2D) 
	    dim = va_arg(ap, int);
	else dim = -1; 
	offset = va_arg(ap, int);

	switch (type) {
	case SE_INT:
	    type_id = H5T_NATIVE_INT;
	    break;
	case SE_FLOAT:
	    type_id = H5T_NATIVE_FLOAT;
	    break;
	case SE_DOUBLE:
	    type_id = H5T_NATIVE_DOUBLE;
	    break;
	case SE_INT_2D:
	    arraydim[0]=dim;
	    type_id=H5Tarray_create(H5T_NATIVE_INT, 1, arraydim, NULL);
	    break;
	case SE_FLOAT_2D:
	    arraydim[0]=dim;
	    type_id=H5Tarray_create(H5T_NATIVE_FLOAT, 1, arraydim, NULL);
	    break;
	case SE_DOUBLE_2D:
	    arraydim[0]=dim;
	    type_id=H5Tarray_create(H5T_NATIVE_DOUBLE, 1, arraydim, NULL);
	    break;
	default:
	    break;
	}

	H5Tinsert(memtype_id, varname, offset, type_id);

	/* If it doesn't, throw some kind of error */
    }

/*     va_end(ap); */

    /* Do the read */
    *buf = malloc(*nshells * stride);
    if (stride > 0) {
	status = H5Dread(dset_id, memtype_id, memspace_id, space_id,
			 H5P_DEFAULT, *buf);
    }

    /* Clean up */
    H5Tclose(memtype_id);
    H5Sclose(space_id);
    H5Tclose(h5shell_type);
    H5Dclose(dset_id);
    SEgroupclose(group_id);
}

void SEread(int ifile_id, int cycle, int *nshells, void **buf, 
	    size_t stride, /* char *name, size_t offset, */...)
{
    char groupname[256], dsetname[]="SE_DATASET";
    va_list ap;

    SEnameFromCycle(cycle, groupname, sizeof(groupname));
    va_start(ap, stride);
    SEread_group_name(ifile_id, groupname, dsetname, nshells, buf, stride, 
		      ap);
    va_end(ap);

}

void SEsetlines(int l_min, int l_max)
{
    lmin = l_min;
    lmax = l_max;
}

void FSESETLINES(int *l_min, int *l_max)
{
    lmin = *l_min - 1;
    lmax = *l_max - 1;
}

static void FSEread(int file_id, int cycle, char *name, int nchars,
		    int *nshells, size_t stride, SEtype_t type, void **array)
{
    char *cname = (char *)malloc((nchars + 1) * sizeof(char));

    strtrim(cname, name, nchars);

    SEread(file_id, cycle, nshells, array, stride, cname, type, 0, NULL);

    free(cname);
}

void FSEREADD(int *file_id, int *cycle, int *nshells, char *name, double *array,
	      int len)
{
    void *buf;
    int i, n;

    FSEread(*file_id, *cycle, name, len, &n, sizeof(double),
	    SE_DOUBLE, &buf);

    n = (n < *nshells) ? n : *nshells;

    for (i = 0; i < n; ++i) {
	array[i] = ((double *)buf)[i];
    }

    free(buf);
}


void FSEREADF(int *file_id, int *cycle, int *nshells, char *name, float *array,
	      int len)
{
    void *buf;
    int i, n;

    FSEread(*file_id, *cycle, name, len, &n, sizeof(float),
	    SE_FLOAT, &buf);

    n = (n < *nshells) ? n : *nshells;

    for (i = 0; i < n; ++i) {
	array[i] = ((float *)buf)[i];
    }

    free(buf);
}

void FSEREADI(int *file_id, int *cycle, int *nshells, char *name, int *array,
	      int len)
{
    void *buf;
    int i, n;

    FSEread(*file_id, *cycle, name, len, &n, sizeof(int),
	    SE_INT, &buf);

    n = (n < *nshells) ? n : *nshells;

    for (i = 0; i < n; ++i) {
	array[i] = ((int *)buf)[i];
    }

    free(buf);
}

static void FSEread2d(int file_id, int cycle, char *name, int nchars,
		       int *nshells, size_t stride, SEtype_t type, int dim, 
		       void **array)
{
    char *cname = (char *)malloc((nchars + 1) * sizeof(char));

    strtrim(cname, name, nchars);

    SEread(file_id, cycle, nshells, array, stride, cname, type, dim, 0, NULL);

    free(cname);
}

void FSEREADI2D(int *file_id, int *cycle, int *nshells, char *name, 
		 int *array, int *arraylen, int *dim, int len)
{
    void *buf;
    int i, j, n;

    FSEread2d(*file_id, *cycle, name, len, &n, *dim*sizeof(int),
	    SE_INT_2D, *dim, &buf);

    n = (n < *nshells) ? n : *nshells;

    for (i = 0; i < n; ++i) {
	for (j = 0; j < *dim; j++){
	    array[j*(*arraylen)+i]=((int *)buf)[i*(*dim)+j];
	}
    }

    free(buf);
}

void FSEREADF2D(int *file_id, int *cycle, int *nshells, char *name, 
		 float *array, int *arraylen, int *dim, int len)
{
    void *buf;
    int i, j, n;

    FSEread2d(*file_id, *cycle, name, len, &n, *dim*sizeof(float),
	    SE_FLOAT_2D, *dim, &buf);

    n = (n < *nshells) ? n : *nshells;

    for (i = 0; i < n; ++i) {
	for (j = 0; j < *dim; j++){
	    array[j*(*arraylen)+i]=((float *)buf)[i*(*dim)+j];
	}
    }

    free(buf);
}

void FSEREADD2D(int *file_id, int *cycle, int *nshells, char *name, 
		 double *array, int *arraylen, int *dim, int len)
{
    void *buf;
    int i, j, n;

    FSEread2d(*file_id, *cycle, name, len, &n, *dim*sizeof(double),
	    SE_DOUBLE_2D, *dim, &buf);

    n = (n < *nshells) ? n : *nshells;

    for (i = 0; i < n; ++i) {
	for (j = 0; j < *dim; j++){
	    array[j*(*arraylen)+i]=((double *)buf)[i*(*dim)+j];
	}
    }

    free(buf);
}

void SEreadDataset(int ifile_id, int cycle, char *dsetname, int *nshells, 
		   void **buf, size_t stride, 
		   /* char *name, size_t offset, */...)
{
    char groupname[256];
    va_list ap;

    SEnameFromCycle(cycle, groupname, sizeof(groupname));
    va_start(ap, stride);
    SEread_group_name(ifile_id, groupname, dsetname, nshells, buf, stride, 
		      ap);
    va_end(ap);
}

void SEreadDArrayAttr(int ifile_id, int cycle, char *dsetname, 
		      double **buf, int *nentries)
{
    SEreadDataset(ifile_id, cycle, dsetname, nentries, (void **) buf, 
		  sizeof(double), "data", SE_DOUBLE, 0, NULL); 
}

void SEreadFArrayAttr(int ifile_id, int cycle, char *dsetname, 
		      float **buf, int *nentries)
{
    SEreadDataset(ifile_id, cycle, dsetname, nentries, (void **) buf, 
		  sizeof(float), "data", SE_FLOAT, 0, NULL); 
}

void SEreadIArrayAttr(int ifile_id, int cycle, char *dsetname, 
		      int **buf, int *nentries)
{
    SEreadDataset(ifile_id, cycle, dsetname, nentries, (void **) buf, 
		  sizeof(int), "data", SE_INT, 0, NULL); 
}

void SEreadSArrayAttr(int ifile_id, int cycle, char *dsetname, 
		      char **buf, int *nentries)
{
    /* Not implemented yet, modify the comments routines */
}

void FSEREADDARRAYATTR(int *file_id, int *cycle, char *dsetname, 
		       double *array, int *nshells, int nchars)
{
    void *buf;
    char *cdsetname = (char *)malloc((nchars + 1) * sizeof(char));
    int i, n;

    strtrim(cdsetname, dsetname, nchars);

    SEreadDataset(*file_id, *cycle, cdsetname, &n,  
		  &buf, sizeof(double),
 		  "data", SE_DOUBLE, 0, NULL); 

    if ( n > *nshells) n=*nshells; 

    for (i = 0; i < n; ++i) {
	array[i] = ((double *)buf)[i];
    }

    free(cdsetname);
}

void FSEREADFARRAYATTR(int *file_id, int *cycle, char *dsetname, 
		       float *array, int *nshells, int nchars)
{
    void *buf;
    char *cdsetname = (char *)malloc((nchars + 1) * sizeof(char));
    int i, n;

    strtrim(cdsetname, dsetname, nchars);

    SEreadDataset(*file_id, *cycle, cdsetname, &n,  
		  &buf, sizeof(float),
 		  "data", SE_FLOAT, 0, NULL); 

    if ( n > *nshells) n=*nshells; 

    for (i = 0; i < n; ++i) {
	array[i] = ((float *)buf)[i];
    }

    free(cdsetname);
}

void FSEREADIARRAYATTR(int *file_id, int *cycle, char *dsetname, 
		       int *array, int *nshells, int nchars)
{
    void *buf;
    char *cdsetname = (char *)malloc((nchars + 1) * sizeof(char));
    int i, n;

    strtrim(cdsetname, dsetname, nchars);

    SEreadDataset(*file_id, *cycle, cdsetname, &n,  
		  &buf, sizeof(int),
 		  "data", SE_INT, 0, NULL); 

    if ( n > *nshells) n=*nshells; 

    for (i = 0; i < n; ++i) {
	array[i] = ((int *)buf)[i];
    }

    free(cdsetname);
}
