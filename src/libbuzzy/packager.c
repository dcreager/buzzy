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
#include "buzzy/built.h"
#include "buzzy/env.h"
#include "buzzy/error.h"
#include "buzzy/os.h"
#include "buzzy/package.h"


/*-----------------------------------------------------------------------
 * Standard messages
 */

int
bz_package_message(struct cork_buffer *dest, struct bz_env *env,
                   const char *packager_name)
{
    const char  *package_name;
    const char  *version;
    rip_check(package_name = bz_env_get_string(env, "name", true));
    rip_check(version = bz_env_get_string(env, "version", true));
    cork_buffer_append_printf
        (dest, "Package %s %s (%s)", package_name, version, packager_name);
    return 0;
}

int
bz_install_message(struct cork_buffer *dest, struct bz_env *env,
                   const char *packager_name)
{
    const char  *package_name;
    const char  *version;
    rip_check(package_name = bz_env_get_string(env, "name", true));
    rip_check(version = bz_env_get_string(env, "version", true));
    cork_buffer_append_printf
        (dest, "Install %s %s (%s)", package_name, version, packager_name);
    return 0;
}


/*-----------------------------------------------------------------------
 * Packagers
 */

struct bz_packager {
    struct bz_env  *env;
    const char  *packager_name;
    struct bz_action  *package;
    struct bz_action  *install;
};


struct bz_packager *
bz_packager_new(struct bz_env *env, const char *packager_name,
                struct bz_action *package,
                struct bz_action *install)
{
    struct bz_packager  *packager = cork_new(struct bz_packager);
    packager->env = env;
    packager->packager_name = cork_strdup(packager_name);
    packager->package = package;
    packager->install = install;
    bz_action_add_pre(packager->install, packager->package);
    return packager;
}

void
bz_packager_free(struct bz_packager *packager)
{
    cork_strfree(packager->packager_name);
    bz_action_free(packager->package);
    bz_action_free(packager->install);
    free(packager);
}

void
bz_packager_add_prereq(struct bz_packager *packager, struct bz_action *action)
{
    bz_action_add_pre(packager->package, action);
    bz_action_add_pre(packager->install, action);
}

int
bz_packager_add_prereq_package(struct bz_packager *packager, const char *dep_string)
{
    struct bz_action  *prereq;
    rip_check(prereq = bz_install_dependency_string(dep_string));
    bz_packager_add_prereq(packager, prereq);
    return 0;
}


struct bz_action *
bz_packager_package_action(struct bz_packager *packager)
{
    return packager->package;
}

struct bz_action *
bz_packager_install_action(struct bz_packager *packager)
{
    return packager->install;
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
