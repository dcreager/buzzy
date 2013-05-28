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

int
bz_uninstall_message(struct bz_env *env, const char *packager_name)
{
    const char  *package_name;
    const char  *version;
    rip_check(package_name = bz_env_get_string(env, "name", true));
    rip_check(version = bz_env_get_string(env, "version", true));
    bz_log_action("Uninstall %s %s (%s)", package_name, version, packager_name);
    return 0;
}


/*-----------------------------------------------------------------------
 * Packagers
 */

struct bz_packager {
    struct bz_env  *env;
    const char  *packager_name;
    struct bz_builder  *builder;

    void  *user_data;
    cork_free_f  free_user_data;
    bz_package_is_needed_f  package_needed;
    bz_package_step_f  package;
    bz_package_is_needed_f  install_needed;
    bz_package_step_f  install;
    bz_package_is_needed_f  uninstall_needed;
    bz_package_step_f  uninstall;
    bool  packaged;
    bool  installed;
    bool  uninstalled;
};


struct bz_packager *
bz_packager_new(struct bz_env *env, const char *packager_name,
                void *user_data, cork_free_f free_user_data,
                bz_package_is_needed_f package_needed,
                bz_package_step_f package,
                bz_package_is_needed_f install_needed,
                bz_package_step_f install,
                bz_package_is_needed_f uninstall_needed,
                bz_package_step_f uninstall)
{
    struct bz_packager  *packager = cork_new(struct bz_packager);
    packager->env = env;
    packager->packager_name = cork_strdup(packager_name);
    packager->builder = NULL;
    packager->user_data = user_data;
    packager->free_user_data = free_user_data;
    packager->package_needed = package_needed;
    packager->package = package;
    packager->install_needed = install_needed;
    packager->install = install;
    packager->uninstall_needed = uninstall_needed;
    packager->uninstall = uninstall;
    packager->packaged = false;
    packager->installed = false;
    packager->uninstalled = false;
    return packager;
}

void
bz_packager_free(struct bz_packager *packager)
{
    cork_strfree(packager->packager_name);
    cork_free_user_data(packager);
    free(packager);
}


void
bz_packager_set_builder(struct bz_packager *packager,
                        struct bz_builder *builder)
{
    packager->builder = builder;
}


int
bz_packager_package(struct bz_packager *packager)
{
    if (!packager->packaged) {
        bool  is_needed;
        packager->packaged = true;
        rii_check(packager->package_needed(packager->user_data, &is_needed));
        if (is_needed) {
            if (packager->builder != NULL) {
                rii_check(bz_builder_stage(packager->builder));
            }
            return packager->package(packager->user_data);
        }
    }
    return 0;
}

int
bz_packager_install(struct bz_packager *packager)
{
    rii_check(bz_packager_package(packager));
    if (!packager->installed) {
        bool  is_needed;
        packager->installed = true;
        rii_check(packager->install_needed(packager->user_data, &is_needed));
        if (is_needed) {
            rii_check(bz_packager_package(packager));
            return packager->install(packager->user_data);
        }
    }
    return 0;
}

int
bz_packager_uninstall(struct bz_packager *packager)
{
    rii_check(bz_packager_package(packager));
    if (!packager->uninstalled) {
        bool  is_needed;
        packager->uninstalled = true;
        rii_check(packager->uninstall_needed(packager->user_data, &is_needed));
        if (is_needed) {
            rii_check(bz_packager_package(packager));
            return packager->uninstall(packager->user_data);
        }
    }
    return 0;
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
    { "rpm", bz_rpm_packager_new },
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
