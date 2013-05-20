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

#include "buzzy/built.h"
#include "buzzy/env.h"
#include "buzzy/error.h"
#include "buzzy/logging.h"
#include "buzzy/os.h"
#include "buzzy/package.h"


/*-----------------------------------------------------------------------
 * Standard messages
 */

int
bz_package_message(struct bz_env *env, const char *packager_name)
{
    const char  *package_name;
    const char  *version;
    rip_check(package_name = bz_env_get_string(env, "name", true));
    rip_check(version = bz_env_get_string(env, "version", true));
    bz_log_action("Package %s %s (%s)", package_name, version, packager_name);
    return 0;
}

int
bz_install_message(struct bz_env *env, const char *packager_name)
{
    const char  *package_name;
    const char  *version;
    rip_check(package_name = bz_env_get_string(env, "name", true));
    rip_check(version = bz_env_get_string(env, "version", true));
    bz_log_action("Install %s %s (%s)", package_name, version, packager_name);
    return 0;
}


/*-----------------------------------------------------------------------
 * Packagers
 */

struct bz_packager {
    struct bz_env  *env;
    const char  *packager_name;
    void  *user_data;
    cork_free_f  free_user_data;
    bz_package_step_f  package;
    bz_package_step_f  install;
    bool  packaged;
    bool  installed;
};


struct bz_packager *
bz_packager_new(struct bz_env *env, const char *packager_name,
                void *user_data, cork_free_f free_user_data,
                bz_package_step_f package,
                bz_package_step_f install)
{
    struct bz_packager  *packager = cork_new(struct bz_packager);
    packager->env = env;
    packager->packager_name = cork_strdup(packager_name);
    packager->user_data = user_data;
    packager->free_user_data = free_user_data;
    packager->package = package;
    packager->install = install;
    packager->packaged = false;
    packager->installed = false;
    return packager;
}

void
bz_packager_free(struct bz_packager *packager)
{
    cork_strfree(packager->packager_name);
    cork_free_user_data(packager);
    free(packager);
}


int
bz_packager_package(struct bz_packager *packager)
{
    if (!packager->packaged) {
        packager->packaged = true;
        return packager->package(packager->user_data);
    } else {
        return 0;
    }
}

int
bz_packager_install(struct bz_packager *packager)
{
    rii_check(bz_packager_package(packager));
    if (!packager->installed) {
        packager->installed = true;
        return packager->install(packager->user_data);
    } else {
        return 0;
    }
}


/*-----------------------------------------------------------------------
 * Available packagers
 */

struct bz_packager_reg {
    const char  *name;
    struct bz_packager *(*new_packager)(struct bz_env *env);
};

static struct bz_packager_reg  packagers[] = {
    { "noop", bz_noop_packager_new },
    { "pacman", bz_pacman_packager_new },
    { NULL }
};

struct bz_packager *
bz_package_packager_new(struct bz_env *env)
{
    const char  *packager_name;
    struct bz_packager_reg  *packager;

    rpp_check(packager_name = bz_env_get_string(env, "packager", true));
    for (packager = packagers; packager->name != NULL; packager++) {
        if (strcmp(packager_name, packager->name) == 0) {
            return packager->new_packager(env);
        }
    }
    bz_bad_config("Unknown packager \"%s\"", packager_name);
    return NULL;
}
