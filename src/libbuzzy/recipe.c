/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <libcork/core.h>
#include <libcork/os.h>
#include <libcork/helpers/errors.h>

#include "buzzy/os.h"
#include "buzzy/recipe.h"


/*-----------------------------------------------------------------------
 * Builtin recipe-specific variables
 */

bz_define_variables(recipe)
{
    bz_package_variable(
        install_prefix, "install_prefix",
        bz_string_value_new("/usr"),
        "The installation prefix for any built packages",
        ""
    );

    bz_package_variable(
        build_path, "build_path",
        bz_interpolated_value_new("${package_work_path}/build"),
        "Where a package's build artefacts should be placed",
        ""
    );

    bz_package_variable(
        source_path, "source_path",
        bz_interpolated_value_new("${package_work_path}/source"),
        "Where a package's extracted source archive should be placed",
        ""
    );
}


/*-----------------------------------------------------------------------
 * Recipes
 */

struct bz_recipe {
    struct bz_env  *env;
    const char  *builder_name;
    void  *user_data;
    bz_free_f  user_data_free;
    bz_recipe_build_f  build;
    bz_recipe_test_f  test;
    bz_recipe_stage_f  stage;
    struct bz_action  *build_action;
    struct bz_action  *test_action;
    struct bz_action  *stage_action;
};


static void
bz_recipe__build__message(void *user_data, struct cork_buffer *dest)
{
    struct bz_recipe  *recipe = user_data;
    const char  *package_name = "unknown";
    const char  *version = "unknown";
    package_name = bz_env_get_string(recipe->env, "name", false);
    version = bz_env_get_string(recipe->env, "version", false);
    cork_buffer_append_printf
        (dest, "Build %s %s (%s)",
         package_name, version, recipe->builder_name);
}

static int
bz_recipe__build__is_needed(void *user_data, bool *is_needed)
{
    struct bz_recipe  *recipe = user_data;
    struct cork_path  *build_tag;
    rip_check(build_tag = bz_env_get_path
              (recipe->env, "package_work_path", true));
    cork_path_append(build_tag, ".built");
    ei_check(bz_file_exists(build_tag, is_needed));
    *is_needed = !*is_needed;
    cork_path_free(build_tag);
    return 0;

error:
    cork_path_free(build_tag);
    return -1;
}

static int
bz_recipe__build__perform(void *user_data)
{
    struct bz_recipe  *recipe = user_data;
    struct cork_path  *build_path;
    struct cork_file  *build_dir;

    /* Create the build path */
    rip_check(build_path = bz_env_get_path(recipe->env, "build_path", true));
    rip_check(build_dir = bz_create_directory(build_path));
    cork_file_free(build_dir);

    /* Then pass control to the actual builder function */
    return recipe->build(recipe->user_data, recipe->env);
}


static void
bz_recipe__test__message(void *user_data, struct cork_buffer *dest)
{
    struct bz_recipe  *recipe = user_data;
    const char  *package_name = "unknown";
    const char  *version = "unknown";
    package_name = bz_env_get_string(recipe->env, "name", false);
    version = bz_env_get_string(recipe->env, "version", false);
    cork_buffer_append_printf
        (dest, "Test %s %s (%s)",
         package_name, version, recipe->builder_name);
}

static int
bz_recipe__test__is_needed(void *user_data, bool *is_needed)
{
    /* For now, we only perform tests when explicitly asked to, and when asked,
     * we always perform them. */
    *is_needed = true;
    return 0;
}

static int
bz_recipe__test__perform(void *user_data)
{
    struct bz_recipe  *recipe = user_data;
    return recipe->test(recipe->user_data, recipe->env);
}


static void
bz_recipe__stage__message(void *user_data, struct cork_buffer *dest)
{
    struct bz_recipe  *recipe = user_data;
    const char  *package_name = "unknown";
    const char  *version = "unknown";
    package_name = bz_env_get_string(recipe->env, "name", false);
    version = bz_env_get_string(recipe->env, "version", false);
    cork_buffer_append_printf
        (dest, "Stage %s %s (%s)",
         package_name, version, recipe->builder_name);
}

static int
bz_recipe__stage__is_needed(void *user_data, bool *is_needed)
{
    struct bz_recipe  *recipe = user_data;
    struct cork_path  *stage_tag;
    rip_check(stage_tag = bz_env_get_path
              (recipe->env, "package_work_path", true));
    cork_path_append(stage_tag, ".staged");
    ei_check(bz_file_exists(stage_tag, is_needed));
    *is_needed = !*is_needed;
    cork_path_free(stage_tag);
    return 0;

error:
    cork_path_free(stage_tag);
    return -1;
}

static int
bz_recipe__stage__perform(void *user_data)
{
    struct bz_recipe  *recipe = user_data;
    struct cork_path  *staging_path;
    struct cork_file  *staging_dir;

    /* Create the staging path */
    rip_check(staging_path = bz_env_get_path
              (recipe->env, "staging_path", true));
    rip_check(staging_dir = bz_create_directory(staging_path));
    cork_file_free(staging_dir);

    /* Then pass control to the actual staginger function */
    return recipe->stage(recipe->user_data, recipe->env);
}



struct bz_recipe *
bz_recipe_new(struct bz_env *env, const char *builder_name,
              struct bz_action *source_action,
              void *user_data, bz_free_f user_data_free,
              bz_recipe_build_f build,
              bz_recipe_test_f test,
              bz_recipe_stage_f stage)
{
    struct bz_recipe  *recipe = cork_new(struct bz_recipe);
    recipe->env = env;
    recipe->builder_name = cork_strdup(builder_name);
    recipe->user_data = user_data;
    recipe->user_data_free = user_data_free;
    recipe->build = build;
    recipe->test = test;
    recipe->stage = stage;

    recipe->build_action = bz_action_new
        (recipe, NULL,
         bz_recipe__build__message,
         bz_recipe__build__is_needed,
         bz_recipe__build__perform);
    if (source_action != NULL) {
        bz_action_add_pre(recipe->build_action, source_action);
    }

    recipe->test_action = bz_action_new
        (recipe, NULL,
         bz_recipe__test__message,
         bz_recipe__test__is_needed,
         bz_recipe__test__perform);
    bz_action_add_pre(recipe->test_action, recipe->build_action);

    recipe->stage_action = bz_action_new
        (recipe, NULL,
         bz_recipe__stage__message,
         bz_recipe__stage__is_needed,
         bz_recipe__stage__perform);
    bz_action_add_pre(recipe->stage_action, recipe->build_action);

    return recipe;
}

void
bz_recipe_free(struct bz_recipe *recipe)
{
    cork_strfree(recipe->builder_name);
    bz_user_data_free(recipe);
    bz_action_free(recipe->build_action);
    bz_action_free(recipe->test_action);
    bz_action_free(recipe->stage_action);
    free(recipe);
}



struct bz_action *
bz_recipe_build_action(struct bz_recipe *recipe)
{
    return recipe->build_action;
}

struct bz_action *
bz_recipe_test_action(struct bz_recipe *recipe)
{
    return recipe->test_action;
}

struct bz_action *
bz_recipe_stage_action(struct bz_recipe *recipe)
{
    return recipe->stage_action;
}
