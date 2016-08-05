#include <hdf5.h>

int SEcycleFromName(char *dsetname);
char *SEnameFromCycle(int cycle, char *dsetname, int len);

hid_t SEgroupFromCycle(hid_t file_id, int cycle);
hid_t SEdatasetFromCycle(hid_t file_id, int cycle);

herr_t isDataset(hid_t loc_id, const char *name, void *data);
herr_t isCycle(hid_t loc_id, const char *name, void *data);

hid_t SEmemberType(hid_t type_id, const char *name);
