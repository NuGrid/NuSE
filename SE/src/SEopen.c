#include <stdio.h>
#include <string.h>
#include <hdf5.h>
#include <SE.h>
#include "SEprivatequery.h"
#include "FSE.h"
#include "config.h"

static herr_t moveAttr(hid_t loc_id, const char *name, void *data);
void _SE_update_internal_layout(hid_t file_id);

/**
 * @mainpage Documentation for the Stellar Evolution HDF5 Wrapper Library (SE).
 *
 * @ref OpenClose
 *
 * @ref ReadWrite
 *
 * @ref Attr
 *
 * @ref Comments
 *
 * @ref Query
 **/ 


/**
 @defgroup OpenClose Opening/Closing an SE File
 @defgroup ReadWrite Reading/Writing SE datasets
 @defgroup Query The SE Query Interface
 @defgroup Attr The SE Attribute Interface
 @defgroup Comments The SE Comments Interface
 */


/**
 * @ingroup OpenClose
 * Opens an existing SE file, or creates a new if it does not exist already.
 *
 * @param filename          Name of the file to open
 *
 * @return An integer containing the file id.
 */
int SEopen(char *filename)
{
    hid_t file_id;
    unsigned int majnum, minnum, relnum;
    char hdf5version[32];
    char se_version[32];

    /* Attempt to create new file. */
    H5E_BEGIN_TRY {
	file_id = H5Fcreate(filename, H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);
    } H5E_END_TRY;

    /* If that fails, it probably means that the file already exists;
       open it for read and write.  If that succeeds, a new file has
       been created; write HDF5 version as an attribute. */
    if (file_id < 0) {
	H5E_BEGIN_TRY {
	    file_id = H5Fopen(filename, H5F_ACC_RDWR, H5P_DEFAULT);
	} H5E_END_TRY;
    }
    else {
	H5get_libversion(&majnum, &minnum, &relnum);
	snprintf(hdf5version, sizeof(hdf5version), "%u.%u.%u",
		 majnum, minnum, relnum);
	SEwriteSAttr(file_id, -1, "HDF5_version", hdf5version);	
    }

    /* If opening for read-write access fails, try opening read-only.
       If opening for read-write succeeded, make sure that the
       internal file structure conforms to the latest version. */
    if (file_id < 0) {
	file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    }
    else {
	_SE_update_internal_layout(file_id);
    }

    /* If the file still uses an older internal layout, fail.  The
       file is apparently read-only, so we can't update the internal
       layout, but our "backwards compatibility" is really "quietly
       update to latest version".  We don't actually support reading
       old files directly... */
    SEreadSAttr(file_id, -1, "SE_version", se_version, sizeof(se_version));
    if (strncmp(se_version, PACKAGE_VERSION,
		strlen(se_version) < strlen(PACKAGE_VERSION) ?
		strlen(se_version) : strlen(PACKAGE_VERSION))) {
	fprintf(stderr, "This read-only SDF file uses an old internal format.");
	exit(1);
    }

    return (int)file_id;
}

static herr_t moveAttr(hid_t loc_id, const char *name, void *data)
{
    hid_t attr_id, type_id, space_id;
    void *buf = malloc(sizeof(double));

    attr_id = H5Aopen_name(loc_id, name);
    type_id = H5Aget_type(attr_id);
    space_id = H5Aget_space(attr_id);

    H5Aread(attr_id, type_id, buf);
    H5Aclose(attr_id);
    attr_id = H5Acreate(*((hid_t *)data), name, type_id, space_id, H5P_DEFAULT);
    H5Awrite(attr_id, type_id, buf);
    H5Aclose(attr_id);

    H5Tclose(type_id);
    H5Sclose(space_id);
    free(buf);

    return 0;
}

void _SE_update_internal_layout(hid_t file_id)
{
    hid_t dset_id, group_id, attr_id;
    int i, type, target_type, cycle;
    hsize_t nobjects;
    char **name, newname[32], se_version[32];

    H5E_BEGIN_TRY {
	attr_id = H5Aopen_name(file_id, "SE_version");
    } H5E_END_TRY;

    /* Get SE version; absence of the SE_version attribute means that
       the file is from version 1.0. */
    if (attr_id < 0) {
	snprintf(se_version, sizeof(se_version), "1.0");
	target_type = H5G_DATASET;
    }
    else {
	H5Aclose(attr_id);
	SEreadSAttr(file_id, -1, "SE_version", se_version, sizeof(se_version));
	target_type = H5G_GROUP;
    }

    /* Do nothing if the file is already up-to-date. */
    if (strncmp(se_version, PACKAGE_VERSION, 3) == 0) return;

    H5Gget_num_objs(file_id, &nobjects);

    name = (char **)malloc(nobjects * sizeof(char *));

    /* Get the names of relevant objects (datasets for v1.0, groups
       for later versions). */
    for (i = 0; i < nobjects; ++i) {
	name[i] = NULL;
	type = H5Gget_objtype_by_idx(file_id, i);

	if (type == target_type) {
	    name[i] = (char *)malloc(32 * sizeof(char));
	    H5Gget_objname_by_idx(file_id, i, name[i], 32);
	}
    }

    /* Update the objects; move, rename, etc. */
    if (strncmp(se_version, "1.0", 3) == 0) {
	for (i = 0; i < nobjects; ++i) {
	    if (name[i]) {
		H5Gmove(file_id, name[i], "SE_DATASET");
		sscanf(name[i], "cycle-%d", &cycle);
		SEnameFromCycle(cycle, name[i], 32);
		group_id = H5Gcreate(file_id, name[i], 0);
		H5Gmove2(file_id, "SE_DATASET", group_id, "SE_DATASET");

		dset_id = H5Dopen(group_id, "SE_DATASET");
		H5Aiterate(dset_id, NULL, moveAttr, &group_id);
		H5Dclose(dset_id);

		H5Gclose(group_id);

		free(name[i]);
	    }
	}
    }
    else {
	for (i = 0; i < nobjects; ++i) {
	    if (name[i]) {
		sscanf(name[i], "cycle-%d", &cycle);
		SEnameFromCycle(cycle, newname, sizeof(newname));
		H5Gmove(file_id, name[i], newname);

		free(name[i]);
	    }
	}
    }

    free(name);

    SEwriteVersion(file_id, PACKAGE_VERSION);
}

void FSEOPEN(char *filename, int *file_id, int len)
{
    char *cfilename = (char *)malloc((len + 1) * sizeof(filename));

    strtrim(cfilename, filename, len);

    *file_id = SEopen(cfilename);

    free(cfilename);
}

/**
 * @ingroup OpenClose
 * Closes the SE file associated with file_id
 *
 * @param file_id          Integer file id of an open SE file. 
 *
 * @return Nothing.
 */
void SEclose(int file_id)
{
    H5Fclose((hid_t) file_id); 
    return;
}

void FSECLOSE(int *file_id)
{
    SEclose(*file_id);
}
