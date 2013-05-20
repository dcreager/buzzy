/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <libcork/core.h>

#include "buzzy/env.h"


bz_define_variables(global)
{
    bz_global_variable(
        cache_dir, "cache_dir",
        bz_path_value_new(cork_path_user_cache_path()),
        "A directory for user-specific nonessential data files",
        "On POSIX systems, this defaults to the value of the $XDG_CACHE_HOME "
        "environment variable, or $HOME/.cache if that's not defined.  Note "
        "that this is not a Buzzy-specific directory; this should refer to the "
        "root of the current user's cache directory."
    );

    bz_global_variable(
        work_dir, "work_dir",
        bz_interpolated_value_new("${cache_dir}/buzzy"),
        "A directory for Buzzy's intermediate build products",
        ""
    );

    bz_global_variable(
        binary_package_dir, "binary_package_dir",
        bz_interpolated_value_new("${work_dir}/packages"),
        "Where new binary packages should be placed",
        ""
    );
}
