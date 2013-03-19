/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <string.h>

#include <libcork/core.h>
#include <libcork/os.h>
#include <libcork/helpers/errors.h>

#include "buzzy/action.h"
#include "buzzy/error.h"
#include "buzzy/distro.h"
#include "buzzy/package.h"

#include "buzzy/distro/arch.h"


static bz_create_package_f  create_package = NULL;

int
bz_packager_choose(const char *name)
{
    /* Should we autodetect? */
    if (name == NULL) {
        bool  is_arch;

        rii_check(bz_arch_is_present(&is_arch));
        if (is_arch) {
            create_package = bz_pacman_create_package;
            return 0;
        }

        bz_bad_config("Don't know what packager to use on this platform");
        return -1;
    }

    /* A specific packager has been requested. */
    if (strcmp(name, "pacman") == 0) {
        create_package = bz_pacman_create_package;
        return 0;
    } else {
        bz_bad_config("Unknown packager \"%s\"", name);
        return -1;
    }
}

struct bz_action *
bz_create_package(struct bz_package_spec *spec,
                  struct cork_path *package_path,
                  struct cork_path *staging_path,
                  struct bz_action *stage_action,
                  bool force, bool verbose)
{
    if (CORK_UNLIKELY(create_package == NULL)) {
        /* If we haven't chosen a packager yet, try to autodetect. */
        rpi_check(bz_packager_choose(NULL));
    }

    assert(create_package != NULL);
    return create_package
        (spec, package_path, staging_path, stage_action, force, verbose);
}
