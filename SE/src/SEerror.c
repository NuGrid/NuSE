#include <assert.h>
#include <hdf5.h>
#include <SEerror.h>

static herr_t SE_walk_cb(int n, H5E_error_t *err_desc, void *client_data);

herr_t SEerror(FILE *stream)
{
    herr_t ret_value;

    if (!stream) stream = stderr;

    ret_value = H5Ewalk(H5E_WALK_UPWARD, SE_walk_cb, (void*)stream);

    return ret_value;
}

static herr_t SE_walk_cb(int n, H5E_error_t *err_desc, void *client_data)
{
    FILE                *stream = (FILE *)client_data;
    const char          *maj_str = NULL;
    const char          *min_str = NULL;

    /* Check arguments */
    assert(err_desc);
    if (!client_data) client_data = stderr;

    if (n > 0) {
	H5Eclear();
	return 0;
    }

    /* Get descriptions for the major and minor error numbers */
    maj_str = H5Eget_major(err_desc->maj_num);
    min_str = H5Eget_minor(err_desc->min_num);

    /* Print error message */
    fprintf(stream, "Error in SE library: %s:%u: %s\n",
	    err_desc->func_name, err_desc->line, min_str);

    return 0;
}
