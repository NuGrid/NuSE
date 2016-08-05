#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SEprivatequery.h"

static void SE_Error(char *message, int errorcode);
 
/*
 * SEcycleFromName: get cycle number from dataset name
 *
 * Returns -1 if there's any sort of problem.
 *
 * At the moment, this doesn't even touch the HDF5 file; it just
 * strips off "cycle-" from the beginning of dsetname and returns the
 * remaining integer.
 */
int SEcycleFromName(char *dsetname)
{
    int cycle = -1;

    sscanf(dsetname, "cycle%d", &cycle);

    return cycle;
}

/*
 * SEnameFromCycle: get dataset name from cycle number
 *
 * As above, right now this just cheats and generates the string on
 * the spot.
 */
char *SEnameFromCycle(int cycle, char *dsetname, int len)
{
    if (cycle < -1) 
	SE_Error("Negative Cycle numbers not allowed inside the SE.\n",1);
    else if (cycle == -1)
	snprintf(dsetname, len, "/");
    else
	snprintf(dsetname, len, "cycle%010d", cycle);

    return dsetname;
}

/* 
 * SEgroupFromCycle: get hid for group containing cycle number
 *
 * Returns a negative number if group does not exist.
 */
hid_t SEgroupFromCycle(hid_t file_id, int cycle)
{
    char groupname[32];
    hid_t group_id;

    SEnameFromCycle(cycle, groupname, sizeof(groupname));

    group_id = H5Gopen(file_id, groupname);

    return group_id;
}

/* 
 * SEdatasetFromCycle: get hid for dataset from cycle number
 *
 * Returns a negative number if dataset does not exist.
 */
hid_t SEdatasetFromCycle(hid_t file_id, int cycle)
{
    hid_t dset_id, group_id = SEgroupFromCycle(file_id, cycle);

    if (group_id < 0) return -1;

    dset_id = H5Dopen(group_id, "SE_DATASET");

    H5Gclose(group_id);

    return dset_id;
}

/*
 * isDataset: returns 1 if name in loc_id is a dataset
 *
 * If data is not NULL, it better point to a 256-character array.
 */
herr_t isDataset(hid_t loc_id, const char *name, void *data)
{
    H5G_stat_t statbuf;
    herr_t status;

    H5E_BEGIN_TRY {
	status = H5Gget_objinfo(loc_id, name, 0, NULL);
    } H5E_END_TRY;

    if (status < 0) return 0;

    H5E_BEGIN_TRY {
	H5Gget_objinfo(loc_id, name, 0, &statbuf);
    } H5E_END_TRY;

    if (statbuf.type == H5G_DATASET) {
	if (data) strncpy(data, name, 256*sizeof(char)); 
	return 1;
    }  
    else return 0;
}

/*
 * isCycle: returns 1 if name in loc_id is a group containing a cycle
 *
 * If data is not NULL, it better point to a 256-character array.
 */
herr_t isCycle(hid_t loc_id, const char *name, void *data)
{
    H5G_stat_t statbuf;
    herr_t status;
    hid_t group_id;

    H5E_BEGIN_TRY {
	status = H5Gget_objinfo(loc_id, name, 0, NULL);
    } H5E_END_TRY;

    /* name doesn't exist in loc_id */
    if (status < 0) return 0;

    H5E_BEGIN_TRY {
	H5Gget_objinfo(loc_id, name, 0, &statbuf);
    } H5E_END_TRY;

    /* name isn't a group */
    if (statbuf.type != H5G_GROUP) return 0;

    group_id = H5Gopen(loc_id, name);

    H5E_BEGIN_TRY {
	status = H5Gget_objinfo(group_id, "SE_DATASET", 0, NULL);
    } H5E_END_TRY;

    /* SE_DATASET doesn't exist in group_id */
    if (status < 0) {
	H5Gclose(group_id);
	return 0;
    }

    H5E_BEGIN_TRY {
	H5Gget_objinfo(group_id, "SE_DATASET", 0, &statbuf);
    } H5E_END_TRY;

    H5Gclose(group_id);

    if (statbuf.type == H5G_DATASET) {
	if (data) strncpy(data, name, 256*sizeof(char));
	return 1;
    }
    else return 0;
}

/*
 * SEmemberType: returns type if name is a member of the compound
 * type type_id; otherwise returns negative number
 */
hid_t SEmemberType(hid_t type_id, const char *name)
{
    int field_idx = H5Tget_member_index(type_id, name);
    hid_t field_type;

    if (field_idx < 0) return field_idx;  /* error */

    field_type = H5Tget_member_type(type_id, field_idx);

    return field_type;
}


static void SE_Error(char *message, int errorcode)
{
    fprintf(stderr, "%s", message);
    exit(errorcode);
}
