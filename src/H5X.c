/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * Purpose:	Index routines.
 */

/****************/
/* Module Setup */
/****************/

#define H5X_PACKAGE /* Suppress error about including H5Xpkg */

/* Interface initialization */
#define H5_INTERFACE_INIT_FUNC	H5X_init_interface

/***********/
/* Headers */
/***********/
#include "H5private.h"		/* Generic Functions */
#include "H5Eprivate.h"		/* Error handling */
#include "H5Iprivate.h"		/* IDs */
#include "H5MMprivate.h"	/* Memory management */
#include "H5Pprivate.h"     /* Property lists */
#include "H5Xpkg.h"         /* Index plugins */
#include "H5Dprivate.h"     /* Datasets */

/****************/
/* Local Macros */
/****************/

/******************/
/* Local Typedefs */
/******************/

/********************/
/* Local Prototypes */
/********************/
H5_DLL int H5X_term_interface(void);

/*********************/
/* Package Variables */
/*********************/

/*****************************/
/* Library Private Variables */
/*****************************/

/*******************/
/* Local Variables */
/*******************/
static size_t       H5X_table_alloc_g = 0;
static size_t       H5X_table_used_g = 0;
static H5X_class_t *H5X_table_g = NULL;


static H5_inline hbool_t
H5X__registered(unsigned plugin_id, size_t *index)
{
    size_t plugin_index;
    hbool_t registered = FALSE;

    /* Is the filter already registered? */
    for (plugin_index = 0; plugin_index < H5X_table_used_g; plugin_index++) {
        if (H5X_table_g[plugin_index].id == plugin_id) {
            registered = TRUE;
            if (index) *index = plugin_index;
            break;
        }
    }

    return registered;
}

/*-------------------------------------------------------------------------
 * Function:    H5X_init
 *
 * Purpose:	Initialize the interface from some other package.
 *
 * Return:  Success:    non-negative
 *          Failure:    negative
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5X_init(void)
{
    herr_t ret_value = SUCCEED;   /* Return value */

    FUNC_ENTER_NOAPI(FAIL)
    /* FUNC_ENTER() does all the work */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5X_init() */

/*--------------------------------------------------------------------------
NAME
   H5X_init_interface -- Initialize interface-specific information
USAGE
    herr_t H5X_init_interface()

RETURNS
    Non-negative on success/Negative on failure
DESCRIPTION
    Initializes any interface-specific data or routines.

--------------------------------------------------------------------------*/
static herr_t
H5X_init_interface(void)
{
    herr_t ret_value = SUCCEED;   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    if (H5X_register(H5X_DUMMY) < 0)
        HGOTO_ERROR (H5E_PLINE, H5E_CANTINIT, FAIL, "unable to register dummy index plugin");
#ifdef H5_HAVE_ALACRITY
    if (H5X_register(H5X_ALACRITY) < 0)
        HGOTO_ERROR (H5E_PLINE, H5E_CANTINIT, FAIL, "unable to register ALACRITY index plugin");
#endif
#ifdef H5_HAVE_FASTBIT
    if (H5X_register(H5X_FASTBIT) < 0)
        HGOTO_ERROR (H5E_PLINE, H5E_CANTINIT, FAIL, "unable to register FastBit index plugin");
#endif

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5X_init_interface() */

/*--------------------------------------------------------------------------
 NAME
    H5X_term_interface
 PURPOSE
    Terminate various H5X objects
 USAGE
    void H5X_term_interface()
 RETURNS
    Non-negative on success/Negative on failure
 DESCRIPTION
    Release the atom group and any other resources allocated.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
     Can't report errors...
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int
H5X_term_interface(void)
{
    int	n = 0;

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    if (H5_interface_initialize_g) {
        /* Free the table of filters */
        H5X_table_g = (H5X_class_t *) H5MM_xfree(H5X_table_g);
        H5X_table_used_g = H5X_table_alloc_g = 0;

        /* Shut down interface */
        H5_interface_initialize_g = 0;
    } /* end if */

    FUNC_LEAVE_NOAPI(n)
} /* end H5X_term_interface() */

/*-------------------------------------------------------------------------
 * Function:    H5X_registered
 *
 * Purpose: Get registered plugin index class from plugin ID.
 *
 * Return:  Success:    pointer to registered index class
 *          Failure:    NULL
 *
 *-------------------------------------------------------------------------
 */
H5X_class_t *
H5X_registered(unsigned plugin_id)
{
    H5X_class_t *ret_value = NULL;
    size_t plugin_index;

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    if (H5X__registered(plugin_id, &plugin_index)) {
        ret_value = &H5X_table_g[plugin_index];
    }

    FUNC_LEAVE_NOAPI(ret_value)
}

/*-------------------------------------------------------------------------
 * Function:    H5Xregister
 *
 * Purpose: This function registers new index classes.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Xregister(const H5X_class_t *index_class)
{
    herr_t ret_value = SUCCEED;      /* Return value */

    FUNC_ENTER_API(FAIL)

    /* Check args */
    if (!index_class)
        HGOTO_ERROR (H5E_ARGS, H5E_BADVALUE, FAIL, "invalid index plugin class");
    if (index_class->version != H5X_CLASS_T_VERS)
        HGOTO_ERROR (H5E_ARGS, H5E_BADVALUE, FAIL, "invalid index plugin version number");
    if (index_class->id > H5X_PLUGIN_MAX)
        HGOTO_ERROR (H5E_ARGS, H5E_BADVALUE, FAIL, "invalid index plugin identification number");
    if (index_class->id < H5X_PLUGIN_RESERVED)
        HGOTO_ERROR (H5E_ARGS, H5E_BADVALUE, FAIL, "unable to modify predefined index plugins");

    /* Do or do not. There is no try. */
    if (H5X_register(index_class) < 0)
        HGOTO_ERROR (H5E_PLINE, H5E_CANTINIT, FAIL, "unable to register index");

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Xregister() */

/*-------------------------------------------------------------------------
 * Function:    H5X_register
 *
 * Purpose: Same as the public version except this one allows plugins
 *      to be set for predefined method numbers < H5X_INDEX_RESERVED
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5X_register(const H5X_class_t *index_class)
{
    size_t i;
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(index_class);
    HDassert(index_class->id >= 0 && index_class->id <= H5X_PLUGIN_MAX);

    /* Is the index class already registered? */
    for (i = 0; i < H5X_table_used_g; i++)
        if (H5X_table_g[i].id == index_class->id)
            break;

    /* Filter not already registered */
    if (i >= H5X_table_used_g) {
        if (H5X_table_used_g >= H5X_table_alloc_g) {
            size_t n = MAX(H5X_MAX_NPLUGINS, 2 * H5X_table_alloc_g);
            H5X_class_t *table = (H5X_class_t *) H5MM_realloc(H5X_table_g,
                    n * sizeof(H5X_class_t));
            if (!table)
                HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL, "unable to extend index plugin table");
            H5X_table_g = table;
            H5X_table_alloc_g = n;
        } /* end if */

        /* Initialize */
        i = H5X_table_used_g++;
        HDmemcpy(H5X_table_g + i, index_class, sizeof(H5X_class_t));
    } /* end if */
    else { /* Filter already registered */
        /* Replace old contents */
        HDmemcpy(H5X_table_g + i, index_class, sizeof(H5X_class_t));
    } /* end else */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5X_register() */

/*-------------------------------------------------------------------------
 * Function:    H5Xunregister
 *
 * Purpose: This function unregisters an index class.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Xunregister(unsigned plugin_id)
{
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE1("e", "Iu", plugin_id);

    /* Check args */
    if (plugin_id > H5X_PLUGIN_MAX)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid filter identification number");
    if (plugin_id < H5X_PLUGIN_RESERVED)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "unable to modify predefined index plugins");

    /* Do it */
    if (H5X_unregister(plugin_id) < 0)
        HGOTO_ERROR(H5E_PLINE, H5E_CANTINIT, FAIL, "unable to unregister index plugin");

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Xunregister() */

/*-------------------------------------------------------------------------
 * Function:    H5X_unregister
 *
 * Purpose: Same as the public version except this one allows plugins
 *      to be unset for predefined method numbers < H5X_PLUGIN_RESERVED
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5X_unregister(unsigned plugin_id)
{
    size_t       plugin_index;        /* Local index variable for filter */
//    H5X_object_t object;
    herr_t       ret_value = SUCCEED;   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    HDassert((plugin_id >= 0) && (plugin_id <= H5X_PLUGIN_MAX));

    /* Is the plugin already registered? */
    if (FALSE == H5X__registered(plugin_id, &plugin_index))
        HGOTO_ERROR(H5E_PLINE, H5E_NOTFOUND, FAIL, "plugin is not registered")

    /* Initialize the structure object for iteration */
//    object.plugin_id = plugin_id;
//    object.found = FALSE;

    /* Iterate through all opened datasets, returns a failure if any of them uses the filter */
//    if(H5I_iterate(H5I_DATASET, H5Z__check_unregister_dset_cb, &object, FALSE) < 0)
//        HGOTO_ERROR(H5E_FILE, H5E_BADITER, FAIL, "iteration failed")

//    if(object.found)
//        HGOTO_ERROR(H5E_PLINE, H5E_CANTRELEASE, FAIL, "can't unregister plugin because a dataset is still using it")

    /* Iterate through all opened groups, returns a failure if any of them uses the filter */
//    if(H5I_iterate(H5I_GROUP, H5Z__check_unregister_group_cb, &object, FALSE) < 0)
//        HGOTO_ERROR(H5E_FILE, H5E_BADITER, FAIL, "iteration failed")

//    if(object.found)
//        HGOTO_ERROR(H5E_PLINE, H5E_CANTRELEASE, FAIL, "can't unregister plugin because a group is still using it")

    /* Iterate through all opened files and flush them */
//    if(H5I_iterate(H5I_FILE, H5Z__flush_file_cb, NULL, FALSE) < 0)
//        HGOTO_ERROR(H5E_FILE, H5E_BADITER, FAIL, "iteration failed")

    /* Remove filter from table */
    /* Don't worry about shrinking table size (for now) */
    HDmemmove(&H5X_table_g[plugin_index], &H5X_table_g[plugin_index + 1],
            sizeof(H5X_class_t) * ((H5X_table_used_g - 1) - plugin_index));

    H5X_table_used_g--;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5X_unregister() */

/*-------------------------------------------------------------------------
 * Function:    H5Xcreate
 *
 * Purpose: Create a new index in a container.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Xcreate(hid_t file_id, unsigned plugin_id, hid_t scope_id, hid_t xcpl_id)
{
    H5F_t *file = NULL;
    H5D_t *dset = NULL;
    H5X_class_t *idx_class = NULL;
    void *idx_handle = NULL; /* pointer to index object created */
    hid_t dataset_id = scope_id; /* TODO for now */
    hid_t xapl_id = H5P_INDEX_ACCESS_DEFAULT; /* TODO for now */
    size_t metadata_size; /* size of metadata created by plugin */
    void *metadata; /* metadata created by plugin that needs to be stored */
    H5O_idxinfo_t idx_info;
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE4("e", "iIuii", file_id, plugin_id, scope_id, xcpl_id);

    /* Check args */
    if (plugin_id > H5X_PLUGIN_MAX)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid plugin identification number");
    if (NULL == (file = (H5F_t *) H5I_object_verify(file_id, H5I_FILE)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "not a file ID");
    if (NULL == (dset = (H5D_t *) H5I_object_verify(scope_id, H5I_DATASET)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataset")

    /* Is the plugin already registered */
    if (NULL == (idx_class = H5X_registered(plugin_id)))
        HGOTO_ERROR(H5E_INDEX, H5E_BADVALUE, FAIL, "plugin is not registered");

    /* Get correct property list */
    if (H5P_DEFAULT == xcpl_id)
        xcpl_id = H5P_INDEX_CREATE_DEFAULT;
    else
        if (TRUE != H5P_isa_class(xcpl_id, H5P_INDEX_CREATE))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not index creation property list");

    /* Call create of the plugin */
    if (NULL == idx_class->create)
        HGOTO_ERROR(H5E_INDEX, H5E_BADVALUE, FAIL, "plugin create callback is not defined");
    if (NULL == (idx_handle = idx_class->create(file_id, dataset_id, xcpl_id,
            xapl_id, &metadata_size, &metadata)))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTCREATE, FAIL, "cannot create new plugin index");

    /* Add idx_handle to dataset */
    idx_info.plugin_id = plugin_id;
    idx_info.metadata_size = metadata_size;
    idx_info.metadata = metadata;
    if (FAIL == H5D_set_index(dset, idx_class, idx_handle, idx_info))
        HGOTO_ERROR(H5E_DATASET, H5E_CANTSET, FAIL, "cannot set index");

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Xcreate() */

/*-------------------------------------------------------------------------
 * Function:    H5Xremove
 *
 * Purpose: Remove an index from objects in a container.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Xremove(hid_t file_id, unsigned plugin_id, hid_t scope_id)
{
    void *file = NULL, *dset = NULL;
    size_t plugin_index;
    hid_t dataset_id = scope_id; /* TODO for now */
    hid_t xapl_id = H5P_INDEX_ACCESS_DEFAULT; /* TODO for now */
    size_t metadata_size; /* size of metadata created by plugin */
    void *metadata; /* metadata created by plugin that needs to be stored */
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE3("e", "iIui", file_id, plugin_id, scope_id);

    /* Check args */
    if (plugin_id > H5X_PLUGIN_MAX)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid plugin identification number");
    if (NULL == (file = (void *) H5I_object_verify(file_id, H5I_FILE)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "not a file ID");
    if (NULL == H5I_object_verify(scope_id, H5I_DATASET))
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "scope_id is restricted to dataset ID");

    /* Is the plugin already registered */
    if (FALSE == H5X__registered(plugin_id, &plugin_index))
        HGOTO_ERROR(H5E_INDEX, H5E_BADVALUE, FAIL, "plugin is not registered");

    /* Get index info */

    /* Call remove of the plugin */
    if (NULL == H5X_table_g[plugin_index].remove)
        HGOTO_ERROR(H5E_INDEX, H5E_BADVALUE, FAIL, "plugin remove callback is not defined");
    if (FAIL == H5X_table_g[plugin_index].remove(file_id, dataset_id,
            metadata_size, metadata))
        HGOTO_ERROR(H5E_INDEX, H5E_CANTCREATE, FAIL, "cannot remove index");

    /* Remove idx_handle from dataset */

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Xremove() */

/*-------------------------------------------------------------------------
 * Function:    H5Xget_count
 *
 * Purpose: Determine the number of index objects on an object.
 *
 * Return:  Non-negative on success/Negative on failure
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Xget_count(hid_t scope_id, hsize_t *idx_count)
{
    void *dset;
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_API(FAIL)
    H5TRACE2("e", "i*h", scope_id, idx_count);

    if (NULL == H5I_object_verify(scope_id, H5I_DATASET))
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "scope_id is restricted to dataset ID");
    if (!idx_count)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "idx_count is NULL");

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Xget_count() */