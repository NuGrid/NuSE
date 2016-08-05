#include <stdlib.h>
#include <string.h>
#include <hdf5.h>
#include <SE.h>
#include "FSE.h"
#include "SEprivatequery.h"
#include "SEprivateopen.h"

int do_backup = 0;

typedef enum {
    C,
    FORTRAN
} listflavor_t;

typedef struct {
    char *name, *inname;
    SEtype_t type;
    size_t size, member_size;
    size_t srcoffset, destoffset, dim, arraylen;
    void *array;
} typearray_t;

static void SEbackupcycles(hid_t file_id, char *dsetname);

void SEdobackup(int switch_backup)
{
    do_backup = switch_backup;
}

static void SE_write_group_name(int ifile_id, char *groupname, char *dsetname, 
		     int nshells, listflavor_t flavor, va_list ap)
{
    char *varname;
    hid_t file_id = (hid_t)ifile_id, space_id, dset_id, properties,
	type_id = 0, memtype_id = 0, group_id;
    hsize_t space_dim[1], chunk_size[1], arraydim[1];
    herr_t status;
    int gziplevel = 0, stringlen, i, j, k, tarray_len = 128, nt;
    size_t stride = 0, destoffset, memtype_size = 0;
    void *inbuf = NULL, *outbuf = NULL;
    typearray_t *tarray;
    char groupplusname[256];

    if (nshells == 0) return;  /* Also, nshells should be an unsigned int */

    /* First of all, check whether a data set already exists with that name */
    group_id=SEgroupopen(file_id, groupname);
    if (isDataset(group_id, dsetname, NULL)) {
	dset_id = H5Dopen(group_id, dsetname);   
 	if (dset_id > 0) {
	    sprintf(groupplusname, "/%s/%s", groupname, dsetname);
 	    if (!do_backup) H5Gunlink(dset_id, groupplusname);  
 	    else SEbackupcycles(group_id, dsetname);  
	}
    }

    /* Create the data space. */
    space_dim[0]=nshells;
    space_id = H5Screate_simple(1, space_dim, NULL);

    if (flavor == C) {
	tarray = (typearray_t *)malloc(tarray_len * sizeof(typearray_t));

	inbuf = va_arg(ap, void *);
	stride = va_arg(ap, int);

	for (nt = 0; (varname = va_arg(ap, char *)) != NULL; ++nt ) {
	    if (nt == tarray_len) {
		tarray_len *= 2;
		tarray = (typearray_t *)realloc(tarray,
						tarray_len*sizeof(typearray_t));
	    }

	    tarray[nt].inname = varname;
	    stringlen = strlen(tarray[nt].inname);
	    tarray[nt].name = (char *)malloc((stringlen + 1) * sizeof(char));
	    strncpy(tarray[nt].name, tarray[nt].inname, stringlen + 1);
	    tarray[nt].type = va_arg(ap, SEtype_t);
	    if (tarray[nt].type == SE_INT_2D ||
		tarray[nt].type == SE_FLOAT_2D ||
		tarray[nt].type == SE_DOUBLE_2D) 
		tarray[nt].dim = va_arg(ap, int); /* 2D arrays: dim argument*/
	    else tarray[nt].dim = -1; /* Default entry for scalar variables */ 
	    tarray[nt].srcoffset = va_arg(ap, int);
	}
    }
    else {
	nt = *(va_arg(ap, int *));
	tarray = (typearray_t *)malloc(nt * sizeof(typearray_t));

	for (i = 0; i < nt; ++i) {
	    tarray[i].array = va_arg(ap, void *);
	    tarray[i].inname = va_arg(ap, char *);
	    tarray[i].type = *(va_arg(ap, SEtype_t *));
	    if (tarray[i].type == SE_INT_2D ||
		tarray[i].type == SE_FLOAT_2D ||
		tarray[i].type == SE_DOUBLE_2D) {
		/* 2D in Fortran, need length of allocated array as well! */
		tarray[i].arraylen = *(va_arg(ap, int *)); /* NOT nrows! */
		tarray[i].dim = *(va_arg(ap, int *)); /* 2D arrays: dim arg */
	    } else tarray[i].dim = -1; /* Default entry for scalar vars */ 
	}

	for (i = 0; i < nt; ++i) {
	    stringlen = va_arg(ap, int);
	    tarray[i].name = (char *)malloc((stringlen + 1) * sizeof(char));
	    strtrim(tarray[i].name, tarray[i].inname, stringlen);
	}
    }

    for (destoffset = 0, i = 0; i < nt; ++i) {
	if (flavor == FORTRAN) tarray[i].srcoffset = destoffset;
	tarray[i].destoffset = destoffset;

	switch (tarray[i].type) {
	case SE_INT:
	    tarray[i].size = sizeof(int);
	    type_id = H5T_NATIVE_INT;
	    break;
	case SE_FLOAT:
	    tarray[i].size = sizeof(float);
	    type_id = H5T_NATIVE_FLOAT;
	    break;
	case SE_DOUBLE:
	    tarray[i].size = sizeof(double);
	    type_id = H5T_NATIVE_DOUBLE;
	    break;
	case SE_INT_2D:
	    arraydim[0]=tarray[i].dim;
	    tarray[i].size = sizeof(int)*tarray[i].dim;
	    tarray[i].member_size = sizeof(int);
	    type_id=H5Tarray_create(H5T_NATIVE_INT, 1, arraydim, NULL);
	    break;
	case SE_FLOAT_2D:
	    arraydim[0]=tarray[i].dim;
	    tarray[i].size = sizeof(float)*tarray[i].dim;
	    tarray[i].member_size = sizeof(float);
	    type_id=H5Tarray_create(H5T_NATIVE_FLOAT, 1, arraydim, NULL);
	    break;
	case SE_DOUBLE_2D:
	    arraydim[0]=tarray[i].dim;
	    tarray[i].size = sizeof(double)*tarray[i].dim;
	    tarray[i].member_size = sizeof(double);
	    type_id=H5Tarray_create(H5T_NATIVE_DOUBLE, 1, arraydim, NULL);
	    break;
	default:
	    break;
	}

	destoffset += tarray[i].size;  /* Careful!  Memory alignment
					  will get you here! */
	memtype_size += tarray[i].size;

	if (memtype_size == tarray[i].size) {
	    memtype_id = H5Tcreate(H5T_COMPOUND, memtype_size);
	}
	else {
	    H5Tset_size(memtype_id, memtype_size);
	}

	H5Tinsert(memtype_id, tarray[i].name, tarray[i].destoffset, type_id);
    }

    outbuf = (void *)malloc(nshells * memtype_size);

    for (i = 0; i < nt; ++i) {
	for (j = 0; j < nshells; ++j) {
	    if (flavor == C) {
		memcpy((char *)outbuf + j*memtype_size + tarray[i].destoffset,
		       (char *)inbuf + j*stride + tarray[i].srcoffset,
		       tarray[i].size);
	    }
	    else {
		if (tarray[i].type == SE_INT_2D ||
		    tarray[i].type == SE_FLOAT_2D ||
		    tarray[i].type == SE_DOUBLE_2D) {	    
		    for (k = 0; k < tarray[i].dim; ++k)
			memcpy((char *)outbuf + j*memtype_size + 
			       tarray[i].destoffset+k*tarray[i].member_size,
			       (char *)(tarray[i].array) 
			       + k*tarray[i].arraylen*tarray[i].member_size +
			       j*tarray[i].member_size,
			       tarray[i].member_size);		    
		} else {
		    memcpy((char *)outbuf + j*memtype_size + 
			   tarray[i].destoffset,
			   (char *)(tarray[i].array) + j*tarray[i].size,
			   tarray[i].size);
		}
	    }
	}
	
	free(tarray[i].name);
    }

    free(tarray);

    if (gziplevel > 0) {
	/*
	  Change properties of the data set in order to enable
	  compression. Note that you can only compress "chunked" data.
	*/
	chunk_size[0] = 512*1024/memtype_size;
	if (chunk_size[0] > nshells) chunk_size[0]=nshells;
	/* 
	  Yes, we did think about this. On an ideal data set, time
	  penalty was negligible to 128k, but 4% smaller file
	  size. And within 1% of best case compression with
	  chunksize=dataset size. Note that this best compression case
	  results in extremely large read times!
	*/
	properties = H5Pcreate (H5P_DATASET_CREATE);
	H5Pset_chunk (properties, 1, chunk_size);

	/* 
	   GZIP compression enabled. The last value gives the amount
	   of "effort" that is put into compression, with 9 being the
	   best compression, but slowest performance, and 1 being the
	   best write speed, but worst compression ratio.
	*/
	status = H5Pset_deflate( properties, gziplevel); 
	status = H5Pset_shuffle(properties);
	
    } else {
	properties=H5P_DEFAULT;
    }

    dset_id = H5Dcreate(group_id, dsetname, memtype_id, space_id, properties);

    /* Write data to the dataset */
    status = H5Dwrite(dset_id, memtype_id, H5S_ALL, H5S_ALL, H5P_DEFAULT,
		      outbuf);

    /* Clean up */
    free(outbuf);
    if (memtype_size > 0) H5Tclose(memtype_id);
    H5Pclose(properties);
    H5Dclose(dset_id);
    H5Sclose(space_id);
    SEgroupclose(group_id);
}

static void SE_write(int ifile_id, int cycle, int nshells, 
		     listflavor_t flavor, va_list ap)
{
    char groupname[256], dsetname[]="SE_DATASET";
    SEnameFromCycle(cycle, groupname, sizeof(groupname)); 

    SE_write_group_name(ifile_id, groupname, dsetname, nshells, flavor, ap);

}

static void SEbackupcycles(hid_t file_id, char *dsetname)
{
    char newname[256];
    hid_t group_id;
    herr_t status;
    int i;

    H5E_BEGIN_TRY {
	group_id = H5Gopen(file_id, "backup");
    } H5E_END_TRY;

    if (group_id < 0) {
	group_id = H5Gcreate(file_id, "backup", -1);
    }

    i = 1;
    do {
	snprintf(newname, sizeof(newname), "%s.%d", dsetname, i++);
    } while (isDataset(group_id, newname, NULL));

    status = H5Gmove2(file_id, dsetname, group_id, newname); 

    H5Gclose(group_id);
}

/**
 * @ingroup ReadWrite
 * 
 * @todo Add more Documentation
 *
 * @param 
 *
 * @return 
 */
void SEwrite(int ifile_id, int cycle, int nshells,
	     /* void *buf, size_t stride, char *name, SEtype_t type,
		size_t offset, */...)
{
    va_list ap;

    va_start(ap, nshells);
    SE_write(ifile_id, cycle, nshells, C, ap);
    va_end(ap);
}

void FSEWRITE(int *file_id, int *cycle, int *nshells,
	      /* int *narrays, void *array, char *name, SEtype_t *type, */...)
{
    va_list ap;

    va_start(ap, nshells);
    SE_write(*file_id, *cycle, *nshells, FORTRAN, ap);
    va_end(ap);
}

static void SE_writeDataset(int ifile_id, int cycle, char *dsetname, 
			      int nentries, listflavor_t flavor, va_list ap)
{
    char groupname[256];

    SEnameFromCycle(cycle, groupname, sizeof(groupname)); 

    SE_write_group_name(ifile_id, groupname, dsetname, nentries, flavor, ap);

}

void SEwriteDataset(int ifile_id, int cycle, char *dsetname, int nentries,
		      /* void *buf, size_t stride, char *name, SEtype_t type,
			 size_t offset, */...)
{
    va_list ap;

    va_start(ap, nentries);
    SE_writeDataset(ifile_id, cycle, dsetname, nentries, C, ap);
    va_end(ap);
}

void SEwriteDArrayAttr(int ifile_id, int cycle, char *dsetname, 
		       double *attr, int nentries)
{
    SEwriteDataset(ifile_id, cycle, dsetname, nentries, (void *) attr, 
		   sizeof(double), "data", SE_DOUBLE, 0, NULL);
}

void SEwriteFArrayAttr(int ifile_id, int cycle, char *dsetname, 
		       float *attr, int nentries)
{
    SEwriteDataset(ifile_id, cycle, dsetname, nentries, (void *) attr, 
		   sizeof(float), "data", SE_FLOAT, 0, NULL);
}

void SEwriteIArrayAttr(int ifile_id, int cycle, char *dsetname, 
		       int *attr, int nentries)
{
    SEwriteDataset(ifile_id, cycle, dsetname, nentries, (void *) attr, 
		   sizeof(int), "data", SE_INT, 0, NULL);
}

void SEwriteSArrayAttr(int ifile_id, int cycle, char *dsetname, 
			      char *attr, int nentries)
{
    /* Not implemented yet, rewrite addcomments to do this
       functionality, since it is already essentially there. */
}

void FSEWRITEDATASET(int *file_id, int *cycle, char *dsetname, int *nchars, 
		     int *nentries, /* int *narrays, void *array, char *name, 
				       SEtype_t *type, */...)
{
    /* Ugh, this is ugly. I don't want the user to have to supply
       nchars. Since it is already automatically supplied at the end
       of the argument list. But then, how do I read it, since the
       va_arg list is parsed inside SE_writeDataset? I can do the same
       thing again here, that may be doable, but maybe there is an
       easier way.*/
    char *cdsetname = (char *)malloc((*nchars + 1) * sizeof(char));
    va_list ap;

    strtrim(cdsetname, dsetname, *nchars);

    va_start(ap, nentries);
    SE_writeDataset(*file_id, *cycle, cdsetname, *nentries, FORTRAN, ap);
    va_end(ap);
}

void FSEWRITEDARRAYATTR(int *file_id, int *model, char *attrname, double *attr,
			int *arraylen, int nchars)
{
    char *cattrname = (char *)malloc((nchars + 1) * sizeof(char));

    strtrim(cattrname, attrname, nchars);

    SEwriteDataset(*file_id, *model, cattrname, *arraylen, (void *) attr, 
		   sizeof(double), "data", SE_DOUBLE, 0, NULL);
}

void FSEWRITEFARRAYATTR(int *file_id, int *model, char *attrname, float *attr,
			int *arraylen, int nchars)
{
    char *cattrname = (char *)malloc((nchars + 1) * sizeof(char));

    strtrim(cattrname, attrname, nchars);

    SEwriteDataset(*file_id, *model, cattrname, *arraylen, (void *) attr, 
		   sizeof(float), "data", SE_FLOAT, 0, NULL);
}

void FSEWRITEIARRAYATTR(int *file_id, int *model, char *attrname, int *attr,
			int *arraylen, int nchars)
{
    char *cattrname = (char *)malloc((nchars + 1) * sizeof(char));

    strtrim(cattrname, attrname, nchars);

    SEwriteDataset(*file_id, *model, cattrname, *arraylen, (void *) attr, 
		   sizeof(int), "data", SE_INT, 0, NULL);
}

void FSEWRITESARRAYATTR(int *file_id, int *model, char *attrname, 
			       char *attr, int *arraylen, int len)
{
    /* Not implemented yet, think about rewriting the addcomments
       functionality in these terms */
}
