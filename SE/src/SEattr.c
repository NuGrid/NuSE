#include <string.h>
#include <SE.h>
#include "FSE.h"
#include "SEprivatequery.h"
#include "SEprivateopen.h"

static void SE_writeAttr(int ifile_id, int model, char *attrname, void *buf,
			 hid_t type)
{
    char groupname[256];
    hsize_t attrdim = 1;
    hid_t file_id = (hid_t)ifile_id, loc_id, attrspace_id, attr_id;

    if (model != -1) {
	SEnameFromCycle(model, groupname, sizeof(groupname));
	loc_id = SEgroupopen(file_id, groupname);
    }
    else {
	loc_id = file_id;
    }

    attrspace_id = H5Screate_simple(1, &attrdim, NULL);    

     /* Open Attribute if it already exists */
    H5E_BEGIN_TRY { 
	attr_id = H5Aopen_name(loc_id, attrname);
    } H5E_END_TRY; 

    /* Delete attribute if it exists */
    if ( attr_id > 0 ) {
	H5Aclose(attr_id);
	H5Adelete(loc_id, attrname);
    }
    
    /* (Re)Create attribute */
    attr_id = H5Acreate(loc_id, attrname, type, attrspace_id, 
			H5P_DEFAULT);
  
    H5Awrite(attr_id, type, buf);
    H5Aclose(attr_id);
    
    H5Sclose(attrspace_id);

    if (model != -1)
	H5Gclose(loc_id);
}

static void SEwriteAttr(int ifile_id, int model, char *attrname, void *buf,
			hid_t type)
{
    if (strncmp(attrname,"SE_",3) == 0 ) {
	fprintf(stderr,
		"SE_ is a reserved attribute prefix for the SE library.\n");
	return;
    }

    SE_writeAttr(ifile_id, model, attrname, buf, type);
}

/**
 * @ingroup Attr
 * @todo Add Documentation
 *
 * @param 
 *
 * @return 
 */
void SEwriteDAttr(int file_id, int model, char *name, double value)
{
    SEwriteAttr(file_id, model, name, &value, H5T_NATIVE_DOUBLE);
}

/**
 * @ingroup Attr
 * @todo Add Documentation
 *
 * @param 
 *
 * @return 
 */
void SEwriteFAttr(int file_id, int model, char *name, float value)
{
    SEwriteAttr(file_id, model, name, &value, H5T_NATIVE_FLOAT);
}

void SEwriteIAttr(int file_id, int model, char *name, int value)
{
    SEwriteAttr(file_id, model, name, &value, H5T_NATIVE_INT);
}

/**
 * @ingroup Attr
 * @todo Add Documentation
 *
 * @param 
 *
 * @return 
 */
void SEwriteSAttr(int file_id, int model, char *name, char *value)
{
    hid_t str_id = H5Tcopy(H5T_C_S1);

    H5Tset_size(str_id, strlen(value)+1);

    SEwriteAttr(file_id, model, name, value, str_id);

    H5Tclose(str_id);
}

static void FSEwriteAttr(int ifile_id, int model, 
			 char *attrname, int nachars, void *buf, hid_t type)
{
    char *cattrname = (char *)malloc((nachars + 1) * sizeof(char));

    strtrim(cattrname, attrname, nachars);

    SEwriteAttr(ifile_id, model, cattrname, buf, type);

    free(cattrname);
}

void FSEWRITEDATTR(int *file_id, int *model, char *attrname, double *attr,
		   int len)
{
    FSEwriteAttr(*file_id, *model, attrname, len, attr, 
		H5T_NATIVE_DOUBLE);
}

void FSEWRITEFATTR(int *file_id, int *model, char *attrname, float *attr,
		   int len)
{
    FSEwriteAttr(*file_id, *model, attrname, len, attr, 
		H5T_NATIVE_FLOAT);
}

void FSEWRITEIATTR(int *file_id, int *model, char *attrname, int *attr,
		   int len)
{
    FSEwriteAttr(*file_id, *model, attrname, len, attr, 
		H5T_NATIVE_INT);
}

void FSEWRITESATTR(int *file_id, int *model, char *attrname, char *attr,
		   int len1, int len2)
{
    hid_t str_id = H5Tcopy(H5T_C_S1);
    char *cattr = (char *)malloc((len2+1) * sizeof(char));

    strtrim(cattr, attr, len2);
    H5Tset_size(str_id, strlen(cattr)+1);

    FSEwriteAttr(*file_id, *model, attrname, len1, cattr, str_id);

    H5Tclose(str_id);
    free(cattr);
}

/**
 * @ingroup Attr
 * @todo Add Documentation
 *
 * @param 
 *
 * @return 
 */
static void SEreadAttr(int ifile_id, int model, char *attrname, void *buf, 
		       hid_t type)
{
    hid_t file_id = (hid_t)ifile_id, loc_id, attr_id;

    if (model != -1) {
	loc_id = SEgroupFromCycle(file_id, model);
    }
    else {
	loc_id = file_id;
    }

    /* Open attribute */
    attr_id = H5Aopen_name(loc_id, attrname);
    H5Aread(attr_id, type, buf);    

    H5Aclose(attr_id);

    if (model != -1)
	H5Gclose(loc_id);
}

/**
 * @ingroup Attr
 * @todo Add Documentation
 *
 * @param 
 *
 * @return 
 */
double SEreadDAttr(int file_id, int model, char *name)
{
    double val;

    SEreadAttr(file_id, model, name, &val, H5T_NATIVE_DOUBLE);

    return val;
}

/**
 * @ingroup Attr
 * @todo Add Documentation
 *
 * @param 
 *
 * @return 
 */
float SEreadFAttr(int file_id, int model, char *name)
{
    float val;

    SEreadAttr(file_id, model, name, &val, H5T_NATIVE_FLOAT);

    return val;
}

/**
 * @ingroup Attr
 * @todo Add Documentation
 *
 * @param 
 *
 * @return 
 */
int SEreadIAttr(int file_id, int model, char *name)
{
    int val;

    SEreadAttr(file_id, model, name, &val, H5T_NATIVE_INT);

    return val;
}

static hid_t SAttr_type(int ifile_id, int model, char *name)
{
    hid_t file_id = (hid_t)ifile_id, loc_id, attr_id, type_id;

    if (model != -1) {
	loc_id = SEgroupFromCycle(file_id, model);
    }
    else {
	loc_id = file_id;
    }

    attr_id = H5Aopen_name(loc_id, name);
    type_id = H5Aget_type(attr_id);

    H5Aclose(attr_id);
    if (model != -1)
	H5Gclose(loc_id);

    return type_id;
}

static int SAttr_is_variable_str(int file_id, int model, char *name)
{
    hid_t type_id = SAttr_type(file_id, model, name);
    int is;

    is = H5Tis_variable_str(type_id);

    H5Tclose(type_id);
    return is;
}

/**
 * @ingroup Attr
 * @todo Add Documentation
 *
 * @param 
 *
 * @return 
 */
char *SEreadSAttr(int file_id, int model, char *name, char *buf, int len)
{
    hid_t str_id = H5Tcopy(H5T_C_S1);
    char *outbuf;

    if (SAttr_is_variable_str(file_id, model, name)) {
	H5Tset_size(str_id, H5T_VARIABLE);
	SEreadAttr(file_id, model, name, &outbuf, str_id);

	strncpy(buf, outbuf, len-1);
	buf[len-1]='\0';

	free(outbuf);
    }
    else {
	H5Tset_size(str_id, len);
	SEreadAttr(file_id, model, name, buf, str_id);
    }

    H5Tclose(str_id);
    return buf;
}

static void FSEreadAttr(int ifile_id, int model, 
		       char *attrname, int nachars, void *buf, hid_t type)
{
    char *cattrname = (char *)malloc((nachars + 1) * sizeof(char));

    strtrim(cattrname, attrname, nachars);

    SEreadAttr(ifile_id, model, cattrname, buf, type);

    free(cattrname);
}

void FSEREADDATTR(int *file_id, int *model, char *attrname, double *attr,
		  int len)
{
    FSEreadAttr(*file_id, *model, attrname, len, attr, 
		H5T_NATIVE_DOUBLE);
}

void FSEREADFATTR(int *file_id, int *model, char *attrname, float *attr,
		  int len)
{
    FSEreadAttr(*file_id, *model, attrname, len, attr, 
		H5T_NATIVE_FLOAT);
}

void FSEREADIATTR(int *file_id, int *model, char *attrname, int *attr,
		  int len)
{
    FSEreadAttr(*file_id, *model, attrname, len, attr, 
		H5T_NATIVE_INT);
}

void FSEREADSATTR(int *file_id, int *model, char *attrname, char *attr,
		  int len1, int len2)
{
    hid_t str_id;
    char *outbuf, *cattrname = (char *)malloc((len1 + 1) * sizeof(char));
    int i, len;

    strtrim(cattrname, attrname, len1);

    if (SAttr_is_variable_str(*file_id, *model, cattrname)) {
	str_id = H5Tcopy(H5T_C_S1);
	H5Tset_size(str_id, H5T_VARIABLE);
	FSEreadAttr(*file_id, *model, attrname, len1, &outbuf, str_id);

	if (len2 < strlen(outbuf)) len = len2;
	else len = strlen(outbuf);

	strncpy(attr, outbuf, len);
	for (i = len; i < len2; ++i) attr[i] = ' ';

	free(outbuf);
    }
    else {
	str_id = H5Tcopy(H5T_FORTRAN_S1);
	H5Tset_size(str_id, len2);
	FSEreadAttr(*file_id, *model, attrname, len1, attr, str_id);
    }

    H5Tclose(str_id);
    free(cattrname);
}

/**
 * @ingroup Comments
 * @todo Add Documentation
 *
 * @param 
 *
 * @return 
 */
void SEaddComment(int ifile_id, char *comment)
{
    hsize_t dim = 1, memdim = 1, maxdim = H5S_UNLIMITED, coord[1],
	chunksize = 2;
    hid_t file_id = (hid_t)ifile_id, group_id, dset_id, space_id, prop_id,
	mem_id, str_id = H5Tcopy(H5T_C_S1);

    H5Tset_size(str_id, H5T_VARIABLE);

     /* Open SE_COMMENT group if it already exists */
    H5E_BEGIN_TRY {
	group_id = H5Gopen(file_id, "/SE_COMMENT");
    } H5E_END_TRY;

    /* Create SE_COMMENT group if it doesn't exist */
    if ( group_id < 0 ) {
	group_id = H5Gcreate(file_id, "/SE_COMMENT", 0);
    }

    /* Open COMMENTS dataset if it already exists */
    H5E_BEGIN_TRY {
	dset_id = H5Dopen(group_id, "COMMENTS");
    } H5E_END_TRY;

    if (dset_id < 0) {
	space_id = H5Screate_simple(1, &dim, &maxdim);

	prop_id = H5Pcreate(H5P_DATASET_CREATE);
	H5Pset_chunk(prop_id, 1, &chunksize);

	dset_id = H5Dcreate(group_id, "COMMENTS", str_id, space_id, prop_id);

	H5Dwrite(dset_id, str_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, &comment);
	H5Pclose(prop_id);
    }
    else {
	space_id = H5Dget_space(dset_id);
	H5Sget_simple_extent_dims(space_id, &dim, NULL);
	H5Sclose(space_id);

	coord[0] = dim++;
	H5Dset_extent(dset_id, &dim);

	space_id = H5Dget_space(dset_id);
	H5Sselect_elements(space_id, H5S_SELECT_SET, 1, coord);

	mem_id = H5Screate_simple(1, &memdim, NULL);

	H5Dwrite(dset_id, str_id, mem_id, space_id, H5P_DEFAULT, &comment);

	H5Sclose(mem_id);
    }

    H5Dclose(dset_id);
    H5Sclose(space_id);
    H5Tclose(str_id);
    H5Gclose(group_id);
}

void FSEADDCOMMENT(int *file_id, char *comment, int len)
{
    char *ccomment = (char *)malloc((len + 1) * sizeof(char));

    strtrim(ccomment, comment, len);
    SEaddComment(*file_id, ccomment);

    free(ccomment);
}

static char *SE_readComment(int ifile_id, int commentnumber, char **buf)
{
    hid_t file_id = (hid_t)ifile_id, group_id, dset_id, space_id, mem_id,
	str_id;
    hsize_t dim, memdim = 1, coord[1];
    int ncomments = SEget_ncomments(ifile_id);

    if (ncomments <= commentnumber) goto oops;

    H5E_BEGIN_TRY {
	group_id = H5Gopen(file_id, "/SE_COMMENT");
	dset_id = H5Dopen(group_id, "COMMENTS");
    } H5E_END_TRY;

    if ((group_id < 0) || (dset_id < 0)) goto oops;

    space_id = H5Dget_space(dset_id);
    H5Sget_simple_extent_dims(space_id, &dim, NULL);

    coord[0] = commentnumber;
    H5Sselect_elements(space_id, H5S_SELECT_SET, 1, coord);

    str_id = H5Tcopy(H5T_C_S1);
    H5Tset_size(str_id, H5T_VARIABLE);
    mem_id = H5Screate_simple(1, &memdim, NULL);

    H5Dread(dset_id, str_id, mem_id, space_id, H5P_DEFAULT, buf);

    H5Sclose(mem_id);
    H5Sclose(space_id);
    H5Dclose(dset_id);
    H5Gclose(group_id);
    H5Tclose(str_id);

    return *buf;

 oops:
    *buf = (char *)malloc(sizeof(char));
    (*buf)[0] = '\0';
    return *buf;
}

/**
 * @ingroup Comments
 * @todo Add Documentation
 *
 * @param 
 *
 * @return 
 */
char *SEreadComment(int ifile_id, int commentnumber, char *buf, int len)
{
    char *comment;

    SE_readComment(ifile_id, commentnumber, &comment);
    strncpy(buf, comment, len-1);
    buf[len-1] = '\0';

    free(comment);
    return buf;
}

void FSEREADCOMMENT(int *file_id, int *commentnumber, char *buf, int len)
{
    SEreadComment(*file_id, *commentnumber, buf, len);
}

/**
 * @ingroup Comments
 * @todo Add Documentation
 *
 * @param 
 *
 * @return 
 */
char *SEreadAllComments(int ifile_id, char **buf)
{
    int i, len, ncomments = SEget_ncomments(ifile_id);
    char **comments, *comment;

    comments = (char **)malloc(ncomments * sizeof(char *));

    for (len = 1, i = 0; i < ncomments; ++i) {
	comments[i] = SE_readComment(ifile_id, i, &comment);
	len += strlen(comments[i]);
    }

    *buf = (char *)malloc(len * sizeof(char));

    for (i = 0; i < ncomments; ++i) {
	strncat(*buf, comments[i], strlen(comments[i]));
	free(comments[i]);
    }

    free(comments);

    return *buf;
}

/**
 * @ingroup Attr
 * @todo Add Documentation
 *
 * @param 
 *
 * @return 
 */
int SEgetSAttrLength(int ifile_id, int model, char *name)
{
    hid_t str_id = H5Tcopy(H5T_C_S1);
    hid_t type_id;
    char *outbuf;
    int len;

    if (SAttr_is_variable_str(ifile_id, model, name)) {
	H5Tset_size(str_id, H5T_VARIABLE);
	SEreadAttr(ifile_id, model, name, &outbuf, str_id);

	len = strlen(outbuf);

	free(outbuf);
    }
    else {
	type_id = SAttr_type(ifile_id, model, name);

	len = H5Tget_size(type_id) - 1;

	H5Tclose(type_id);
    }

    H5Tclose(str_id);
    return len;
}

void FSEGETSATTRLENGTH(int *file_id, int *model, char *attrname, int *attrlen,
		       int len)
{
    char *cattrname = (char *)malloc((len + 1) * sizeof(char));

    strtrim(cattrname, attrname, len);

    *attrlen = SEgetSAttrLength(*file_id, *model, cattrname);

    free(cattrname);
}

void SEwriteVersion(int file_id, char *value)
{
    hid_t str_id = H5Tcopy(H5T_C_S1);

    H5Tset_size(str_id, strlen(value)+1);

    SE_writeAttr(file_id, -1, "SE_version", value, str_id);

    H5Tclose(str_id);
}
