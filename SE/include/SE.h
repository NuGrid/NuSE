#ifdef __cplusplus
extern "C" {
#endif

#ifndef SETYPE_T
#define SETYPE_T

#include <stdlib.h>
#include <stdarg.h>

typedef enum {
    SE_INT = 0,
    SE_FLOAT = 1,
    SE_DOUBLE = 2,
    SE_INT_2D = 10,
    SE_FLOAT_2D = 11,
    SE_DOUBLE_2D = 12
} SEtype_t;

/* In SEopen.c: open and close SE files */
int SEopen(char *filename);
void SEclose(int file_id);

/* In SEread.c: read datasets */
void SEread(int ifile_id, int cycle, int *nshells, void **buf, size_t stride,
	    /* char *name, SEtype_t type, size_t offset, */...);
void SEsetlines(int l_min, int l_max);
void SEreadDArrayAttr(int ifile_id, int cycle, char *dsetname, 
		      double **buf, int *nentries);
void SEreadFArrayAttr(int ifile_id, int cycle, char *dsetname, 
		      float **buf, int *nentries);
void SEreadIArrayAttr(int ifile_id, int cycle, char *dsetname, 
		      int **buf, int *nentries);
void SEreadDataset(int ifile_id, int cycle, char *dsetname, int *nshells, 
		   void **buf, size_t stride, 
		   /* char *name, size_t offset, */...);

/* In SEwrite.c: write datasets */
void SEwrite(int ifile_id, int cycle, int nshells,
	     /* void *buf, char *name, SEtype_t type, size_t offset, */...);
void SEdobackup(int switch_backup);
void SEwriteDArrayAttr(int ifile_id, int cycle, char *dsetname, 
		       double *attr, int nentries);
void SEwriteFArrayAttr(int ifile_id, int cycle, char *dsetname, 
		       float *attr, int nentries);
void SEwriteIArrayAttr(int ifile_id, int cycle, char *dsetname, 
		       int *attr, int nentries);
void SEwriteDataset(int ifile_id, int cycle, char *dsetname, int nentries,
		    /* void *buf, size_t stride, char *name, SEtype_t type,
		       size_t offset, */...);

/* In SEattr.c: write and read dataset and file attributes */
void SEwriteDAttr(int file_id, int model, char *name, double value);
void SEwriteFAttr(int file_id, int model, char *name, float value);
void SEwriteIAttr(int file_id, int model, char *name, int value);
void SEwriteSAttr(int file_id, int model, char *name, char *value);

double SEreadDAttr(int file_id, int model, char *name);
float SEreadFAttr(int file_id, int model, char *name);
int SEreadIAttr(int file_id, int model, char *name);
char *SEreadSAttr(int file_id, int model, char *name, char *buf, int len);
int SEgetSAttrLength(int ifile_id, int model, char *name);

void SEaddComment(int ifile_id, char *comment);
char *SEreadComment(int file_id, int commentnumber, char *buf, int len);
char *SEreadAllComments(int file_id, char **buf);

void SEwriteVersion(int file_id, char *value);

/* In SEquery.c: query SE files for structural information */
int SEncycles(int file_id);
int SEnshells(int file_id, int cycle);
int SEnentries(int ifile_id, int cycle, char *name);
void SEcycles(int file_id, int *cycles, int ncycles);
int SElastcycle(int file_id);
int SEget_nmembers(int ifile_id, int cycle);
char *SEget_member_name(int ifile_id, int cycle, int member_id);
int SEget_ncomments(int ifile_id);

#endif

#ifdef __cplusplus
}
#endif
