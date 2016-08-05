#include <stdarg.h>
#include <config.h>

/* In SEopen.c: open and close SE files */
#define FSEOPEN F77_FUNC_(fse_open, FSE_OPEN)
#define FSECLOSE F77_FUNC_(fse_close, FSE_CLOSE)

void FSEOPEN(char *filename, int *file_id, int len);
void FSECLOSE(int *file_id);

/* In SEread.c: read datasets */
#define FSEREADD F77_FUNC_(fse_read_d, FSE_READ_D)
#define FSEREADF F77_FUNC_(fse_read_f, FSE_READ_F)
#define FSEREADI F77_FUNC_(fse_read_i, FSE_READ_I)
#define FSEREADD2D F77_FUNC_(fse_read_d_2d, FSE_READ_D_2D)
#define FSEREADF2D F77_FUNC_(fse_read_f_2d, FSE_READ_F_2D)
#define FSEREADI2D F77_FUNC_(fse_read_i_2d, FSE_READ_I_2D)
#define FSESETLINES F77_FUNC_(fse_set_lines, FSE_SET_LINES)
#define FSEREADDARRAYATTR F77_FUNC_(fse_read_darrayattr, FSE_READ_DARRAYATTR)
#define FSEREADFARRAYATTR F77_FUNC_(fse_read_farrayattr, FSE_READ_FARRAYATTR)
#define FSEREADIARRAYATTR F77_FUNC_(fse_read_iarrayattr, FSE_READ_IARRAYATTR)

void FSEREADD(int *file_id, int *cycle, int *nshells, char *name, 
	      double *array, int len);
void FSEREADF(int *file_id, int *cycle, int *nshells, char *name, float *array,
	      int len);
void FSEREADI(int *file_id, int *cycle, int *nshells, char *name, int *array,
	      int len);
void FSEREADD2D(int *file_id, int *cycle, int *nshells, char *name, 
		double *array, int *arraylen, int *dim, int len);
void FSEREADF2D(int *file_id, int *cycle, int *nshells, char *name, 
		float *array, int *arraylen, int *dim, int len);
void FSEREADI2D(int *file_id, int *cycle, int *nshells, char *name, 
		int *array, int *arraylen, int *dim, int len);
void FSESETLINES(int *l_min, int *l_max);
void FSEREADDARRAYATTR(int *ifile_id, int *cycle, char *dsetname, 
		       double *attr, int *nentries, int len);
void FSEREADFARRAYATTR(int *ifile_id, int *cycle, char *dsetname, 
		       float *attr, int *nentries, int len);
void FSEREADIARRAYATTR(int *ifile_id, int *cycle, char *dsetname, 
		       int *attr, int *nentries, int len);

/* In SEwrite.c: write datasets */
#define FSEWRITE F77_FUNC_(fse_write, FSE_WRITE)
#define FSEWRITEDATASET F77_FUNC_(fse_write_dataset, FSE_WRITE_DATASET)
#define FSEWRITEDARRAYATTR F77_FUNC_(fse_write_darrayattr, FSE_WRITE_DARRAYATTR)
#define FSEWRITEFARRAYATTR F77_FUNC_(fse_write_farrayattr, FSE_WRITE_FARRAYATTR)
#define FSEWRITEIARRAYATTR F77_FUNC_(fse_write_iarrayattr, FSE_WRITE_IARRAYATTR)

void FSEWRITE(int *file_id, int *cycle, int *nshells,
	      /* int *narrays, void *array, char *name, SEtype_t type, */...);
void FSEWRITEDATASET(int *file_id, int *cycle, char *dsetname, int *nchars, 
		     int *nentries,
	      /* int *narrays, void *array, char *name, SEtype_t *type, */...);
void FSEWRITEDARRAYATTR(int *file_id, int *model, char *attrname, double *attr,
			int *arraylen, int len);
void FSEWRITEFARRAYATTR(int *file_id, int *model, char *attrname, float *attr,
			int *arraylen, int nchars);
void FSEWRITEIARRAYATTR(int *file_id, int *model, char *attrname, int *attr,
			int *arraylen, int nchars);

/* In SEattr.c: write and read dataset and file attributes */
#define FSEWRITEDATTR F77_FUNC_(fse_write_dattr, FSE_WRITE_DATTR)
#define FSEWRITEFATTR F77_FUNC_(fse_write_fattr, FSE_WRITE_FATTR)
#define FSEWRITEIATTR F77_FUNC_(fse_write_iattr, FSE_WRITE_IATTR)
#define FSEWRITESATTR F77_FUNC_(fse_write_sattr, FSE_WRITE_SATTR)

void FSEWRITEDATTR(int *file_id, int *model, char *attrname, double *attr,
		   int len);
void FSEWRITEFATTR(int *file_id, int *model, char *attrname, float *attr,
		   int len);
void FSEWRITEIATTR(int *file_id, int *model, char *attrname, int *attr,
		   int len);
void FSEWRITESATTR(int *file_id, int *model, char *attrname, char *attr,
		   int len1, int len2);

#define FSEREADDATTR F77_FUNC_(fse_read_dattr, FSE_READ_DATTR)
#define FSEREADFATTR F77_FUNC_(fse_read_fattr, FSE_READ_FATTR)
#define FSEREADIATTR F77_FUNC_(fse_read_iattr, FSE_READ_IATTR)
#define FSEREADSATTR F77_FUNC_(fse_read_sattr, FSE_READ_SATTR)

void FSEREADDATTR(int *file_id, int *model, char *attrname, double *attr,
		  int len);
void FSEREADFATTR(int *file_id, int *model, char *attrname, float *attr,
		  int len);
void FSEREADIATTR(int *file_id, int *model, char *attrname, int *attr,
		  int len);
void FSEREADSATTR(int *file_id, int *model, char *attrname, char *attr,
		  int len1, int len2);

#define FSEADDCOMMENT F77_FUNC_(fse_add_comment, FSE_ADD_COMMENT)
#define FSEREADCOMMENT F77_FUNC_(fse_read_comment, FSE_READ_COMMENT)

void FSEADDCOMMENT(int *file_id, char *comment, int len);
void FSEREADCOMMENT(int *file_id, int *commentnumber, char *buf, int len);

#define FSEGETSATTRLENGTH F77_FUNC_(fse_get_sattr_length, FSE_GET_SATTR_LENGTH)

void FSEGETSATTRLENGTH(int *file_id, int *model, char *attrname, int *attrlen,
		       int len);

/* In SEquery.c: query SE files for structural information */
#define FSENCYCLES F77_FUNC_(fse_ncycles, FSE_NCYCLES)
#define FSENSHELLS F77_FUNC_(fse_nshells, FSE_NSHELLS)
#define FSENENTRIES F77_FUNC_(fse_nentries, FSE_NENTRIES)
#define FSECYCLES F77_FUNC_(fse_cycles, FSE_CYCLES)
#define FSELASTCYCLE F77_FUNC_(fse_lastcycle, FSE_LASTCYCLE)
#define FSEGETNMEMBERS F77_FUNC_(fse_get_nmembers, FSE_GET_NMEMBERS)
#define FSEGETMEMBERNAME F77_FUNC_(fse_get_member_name, FSE_GET_MEMBER_NAME)

void FSENCYCLES(int *file_id, int *ncycles);
void FSENSHELLS(int *file_id, int *cycle, int *nshells);
void FSENENTRIES(int *ifile_id, int *cycle, char *name, int *n, int len);
void FSECYCLES(int *file_id, int *cycles, int *ncycles);
void FSELASTCYCLE(int *file_id, int *lastcycle);
void FSEGETNMEMBERS(int *ifile_id, int *cycle, int *nmembers);
void FSEGETMEMBERNAME(int *ifile_id, int *cycle, int *member_id, char *name,
		      int len);

/* In FSE.c: trim Fortran strings */
void strtrim(char *dest, const char *src, size_t len);
