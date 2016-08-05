#include "SEprivateopen.h"

hid_t SEgroupopen(hid_t file_id, char *groupname)
{
    hid_t group_id;

    H5E_BEGIN_TRY {
	group_id = H5Gopen(file_id, groupname);
    } H5E_END_TRY;
    
    if ( group_id < 0 ) {
	group_id = H5Gcreate(file_id, groupname, 0);
    }
    
    return group_id;
}

void SEgroupclose(hid_t group_id)
{
    H5Gclose(group_id);
}
