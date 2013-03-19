/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/os.h>
#include <libcork/helpers/errors.h>

#include "buzzy/action.h"
#include "buzzy/error.h"
#include "buzzy/native.h"
#include "buzzy/os.h"
#include "buzzy/package.h"
#include "buzzy/version.h"
#include "buzzy/distro/arch.h"


#if !defined(BZ_DEBUG_ARCH)
#define BZ_DEBUG_ARCH  0
#endif

#if BZ_DEBUG_ARCH
#include <stdio.h>
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG(...) /* no debug messages */
#endif


/*-----------------------------------------------------------------------
 * Platform detection
 */

int
bz_arch_is_present(bool *dest)
{
    int  rc;
    struct stat  info;
    rc = stat("/etc/arch-release", &info);
    if (rc == 0) {
        *dest = true;
        return 0;
    } else if (errno == ENOENT) {
        *dest = false;
        return 0;
    } else {
        cork_system_error_set();
        return -1;
    }
}

static struct cork_buffer  architecture = CORK_BUFFER_INIT();

static void
done_architecture(void)
{
    cork_buffer_done(&architecture);
}

CORK_INITIALIZER(init_architecture)
{
    cork_cleanup_at_exit(0, done_architecture);
}

const char *
bz_arch_current_architecture(void)
{
    if (architecture.size == 0) {
        char  *buf;
        rpi_check(bz_subprocess_get_output
                  (&architecture, NULL, NULL, "uname", "-m", NULL));
        /* Chomp the trailing newline */
        buf = architecture.buf;
        buf[--architecture.size] = '\0';
    }
    return architecture.buf;
}


/*-----------------------------------------------------------------------
 * Arch version strings
 */

#define cork_buffer_append_copy(dest, src) \
    (cork_buffer_append((dest), (src)->buf, (src)->size))

static bool
handle_rev_tag(struct bz_version_part *part, struct cork_buffer *dest)
{
    size_t  i;
    const char  *part_string = part->string_value.buf;

    /* We already know this is a postrelease part, and that it's at the end of
     * the Arch version.  Check whether it has the form "revXX", where XX
     * consists only of digits. */
    if (part->string_value.size <= 3) {
        return false;
    }
    if (memcmp(part_string, "rev", 3) != 0) {
        return false;
    }
    for (i = 3; i < part->string_value.size; i++) {
        if (!isdigit(part_string[i])) {
            return false;
        }
    }

    /* Is it rev1?  We ignore that. */
    if (part->string_value.size == 4 && part_string[3] == '1') {
        return true;
    }

    cork_buffer_append(dest, "-", 1);
    cork_buffer_append(dest, part_string + 3, part->string_value.size - 3);
    return true;
}

void
bz_version_to_arch(struct bz_version *version, struct cork_buffer *dest)
{
    size_t  i;
    size_t  count;
    struct bz_version_part  *part;

    count = bz_version_part_count(version);
    assert(count > 0);

    part = bz_version_get_part(version, 0);
    assert(part->kind == BZ_VERSION_RELEASE);
    cork_buffer_append_copy(dest, &part->string_value);

    for (i = 1; i < count; i++) {
        const char  *string_value;
        part = bz_version_get_part(version, i);
        string_value = part->string_value.buf;
        switch (part->kind) {
            case BZ_VERSION_RELEASE:
                cork_buffer_append(dest, ".", 1);
                cork_buffer_append_copy(dest, &part->string_value);
                break;

            case BZ_VERSION_PRERELEASE:
                /* Arch considers a tag to be a prerelease tag if it starts with
                 * an alphanumeric, and is not separated from the preceding
                 * numeric release tag. */
                assert(part->string_value.size > 0);
                if (!isalpha(*string_value)) {
                    cork_buffer_append(dest, "pre", 3);
                }
                cork_buffer_append_copy(dest, &part->string_value);
                break;

            case BZ_VERSION_POSTRELEASE:
                /* Arch considers a tag to be a postrelease tag if it starts
                 * with an alphanumeric, and has a "." separating it from the
                 * preceding numeric release tag. */
                assert(part->string_value.size > 0);

                /* Treat a +revXX tag specially, if it occurs at the end of the
                 * Buzzy version.  That kind of tag should become the Arch
                 * version release tag. */
                if (i == count-2 && handle_rev_tag(part, dest)) {
                    break;
                }

                if (isalpha(*string_value)) {
                    cork_buffer_append(dest, ".", 1);
                } else {
                    cork_buffer_append(dest, ".post", 5);
                }
                cork_buffer_append_copy(dest, &part->string_value);
                break;

            default:
                break;
        }
    }
}


struct bz_version *
bz_version_from_arch(const char *arch_version)
{
    int  cs;
    const char  *p = arch_version;
    const char  *pe = strchr(arch_version, '\0');
    const char  *eof = pe;
    struct bz_version  *version;
    enum bz_version_part_kind  kind;
    const char  *start;
    struct cork_buffer  buf = CORK_BUFFER_INIT();

    DEBUG("---\nParse Arch version \"%s\"\n", arch_version);
    version = bz_version_new();

    %%{
        machine arch_version;

        action start_release {
            kind = BZ_VERSION_RELEASE;
            start = fpc;
            DEBUG("  Create new release version part\n");
        }

        action start_prerelease {
            kind = BZ_VERSION_PRERELEASE;
            start = fpc;
            DEBUG("  Create new prerelease version part\n");
        }

        action start_postrelease {
            kind = BZ_VERSION_POSTRELEASE;
            start = fpc;
            DEBUG("  Create new postrelease version part\n");
        }

        action add_part {
            size_t  size = fpc - start;
            DEBUG("    String value: %.*s\n", (int) size, start);
            bz_version_add_part(version, kind, start, size);
        }

        action start_revision {
            start = fpc + 1;
            DEBUG("  Create new postrelease version part for Arch revision\n");
        }

        action add_revision {
            cork_buffer_set(&buf, "rev", 3);
            cork_buffer_append(&buf, start, fpc - start);
            bz_version_add_part
                (version, BZ_VERSION_POSTRELEASE, buf.buf, buf.size);
        }

        first_release = digit+
                      >start_release %add_part;

        release = '.' digit+
                  >start_release %add_part;

        pre_prerelease = "pre" (alnum+ >start_prerelease)
                       %add_part;

        other_prerelease = ((alpha alnum+) - pre_prerelease)
                         >start_prerelease %add_part;

        prerelease = pre_prerelease | other_prerelease;

        post_postrelease = ".post" (alnum+ >start_postrelease)
                         %add_part;

        other_postrelease = ('.' alpha >start_postrelease alnum+)
                          %add_part;

        postrelease = post_postrelease | other_postrelease;

        skip_rev_1 = "-1";

        real_rev = (('-' (digit+)) - skip_rev_1)
                 >start_revision %add_revision;

        rev = skip_rev_1 | real_rev;

        part = release | prerelease | postrelease;

        main := first_release part** rev?;

        write data noerror nofinal;
        write init;
        write exec;
    }%%

    /* A hack to suppress some unused variable warnings */
    (void) arch_version_en_main;

    if (CORK_UNLIKELY(cs < %%{ write first_final; }%%)) {
        bz_invalid_version("Invalid Arch version \"%s\"", arch_version);
        cork_buffer_done(&buf);
        bz_version_free(version);
        return NULL;
    }

    bz_version_finalize(version);
    cork_buffer_done(&buf);
    return version;
}


/*-----------------------------------------------------------------------
 * Native package database
 */

struct bz_version *
bz_arch_native_version_available(const char *native_package_name)
{
    int  cs;
    char  *p;
    char  *pe;
    char  *start;
    char  *end;
    bool  successful;
    struct cork_buffer  out = CORK_BUFFER_INIT();
    struct bz_version  *result;

    rpi_check(bz_subprocess_get_output
              (&out, NULL, &successful,
               "pacman", "-Sdp", "--print-format", "%v",
               native_package_name, NULL));
    if (!successful) {
        cork_buffer_done(&out);
        return NULL;
    }

    p = out.buf;
    pe = out.buf + out.size;

    %%{
        machine arch_version_available;

        version = (alnum | '.' | '-')+;
        main := (version >{ start = fpc; } %{ end = fpc; }) '\n';

        write data noerror nofinal;
        write init;
        write exec;
    }%%

    /* A hack to suppress some unused variable warnings */
    (void) arch_version_available_en_main;

    if (CORK_UNLIKELY(cs < %%{ write first_final; }%%)) {
        bz_invalid_version("Unexpected output from pacman");
        cork_buffer_done(&out);
        return NULL;
    }

    *end = '\0';
    result = bz_version_from_arch(start);
    cork_buffer_done(&out);
    return result;
}

struct bz_version *
bz_arch_native_version_installed(const char *native_package_name)
{
    int  cs;
    char  *p;
    char  *pe;
    char  *start;
    char  *end;
    bool  successful;
    struct cork_buffer  out = CORK_BUFFER_INIT();
    struct bz_version  *result;

    rpi_check(bz_subprocess_get_output
              (&out, NULL, &successful,
               "pacman", "-Q", native_package_name, NULL));
    if (!successful) {
        cork_buffer_done(&out);
        return NULL;
    }

    p = out.buf;
    pe = out.buf + out.size;

    %%{
        machine arch_version_installed;

        package_name = (alnum | '-')+;
        version = (alnum | '.' | '-')+;
        main := package_name ' '
                (version >{ start = fpc; } %{ end = fpc; })
                '\n';

        write data noerror nofinal;
        write init;
        write exec;
    }%%

    /* A hack to suppress some unused variable warnings */
    (void) arch_version_installed_en_main;

    if (CORK_UNLIKELY(cs < %%{ write first_final; }%%)) {
        bz_invalid_version("Unexpected output from pacman");
        cork_buffer_done(&out);
        return NULL;
    }

    *end = '\0';
    result = bz_version_from_arch(start);
    cork_buffer_done(&out);
    return result;
}


static int
bz_arch_native_install(const char *native_package_name,
                       struct bz_version *version)
{
    /* We don't pass the --needed flag to pacman since our is_needed method
     * should have already verified that the desired version isn't installed
     * yet. */
    return bz_subprocess_run
        (false, NULL,
         "sudo", "pacman", "-S", "--noconfirm", native_package_name,
         NULL);
}

struct bz_pdb *
bz_arch_native_pdb(void)
{
    return bz_native_pdb_new
        ("Arch",
         bz_arch_native_version_available,
         bz_arch_native_version_installed,
         bz_arch_native_install,
         "%s", "lib%s", NULL);
}


/*-----------------------------------------------------------------------
 * Creating Arch packages
 */

struct bz_pacman_packager {
    struct bz_package_spec  *spec;
    struct cork_path  *package_path;
    struct cork_path  *staging_path;
    bool  force;
    bool  verbose;
};

static void
bz_pacman_packager__free(void *user_data)
{
    struct bz_pacman_packager  *packager = user_data;
    cork_path_free(packager->package_path);
    cork_path_free(packager->staging_path);
    free(packager);
}

static void
bz_pacman_packager__message(void *user_data, struct cork_buffer *dest)
{
    struct bz_pacman_packager  *packager = user_data;
    cork_buffer_append_printf
        (dest, "Package %s %s",
         bz_package_spec_name(packager->spec),
         bz_package_spec_version_string(packager->spec));
}

static int
bz_pacman_packager__is_needed(void *user_data, bool *is_needed)
{
    struct bz_pacman_packager  *packager = user_data;

    if (packager->force) {
        *is_needed = true;
        return 0;
    } else {
        const char  *package_name = bz_package_spec_name(packager->spec);
        struct bz_version  *version = bz_package_spec_version(packager->spec);
        const char  *architecture = bz_arch_current_architecture();
        struct cork_buffer  package_file = CORK_BUFFER_INIT();
        struct cork_path  *package_path;

        cork_buffer_printf(&package_file, "%s-", package_name);
        bz_version_to_arch(version, &package_file);
        cork_buffer_append_printf
            (&package_file, "-%s-%s.pkg.tar.xz", BZ_ARCH_RELEASE, architecture);
        package_path = cork_path_join(packager->package_path, package_file.buf);
        cork_buffer_done(&package_file);
        rii_check(bz_file_exists(package_path, is_needed));
        *is_needed = !*is_needed;
        return 0;
    }
}

static int
bz_pacman_packager__perform(void *user_data)
{
    struct bz_pacman_packager  *packager = user_data;
    struct cork_path  *pkg_path;
    struct cork_file  *pkg_dir;
    struct cork_exec  *exec;
    struct cork_env  *env;
    struct cork_path  *filename;
    struct cork_file  *pkgbuild = NULL;
    struct cork_buffer  buf = CORK_BUFFER_INIT();
    bool  staging_exists;

    const char  *package_name = bz_package_spec_name(packager->spec);
    struct bz_version  *version = bz_package_spec_version(packager->spec);
    const char  *version_string = bz_version_to_string(version);
    const char  *architecture = bz_arch_current_architecture();
    const char  *license = bz_package_spec_license(packager->spec);

    if (license == NULL) {
        license = "unknown";
    }

    rii_check(bz_file_exists(packager->staging_path, &staging_exists));
    if (CORK_UNLIKELY(!staging_exists)) {
        cork_error_set
            (CORK_BUILTIN_ERROR, CORK_SYSTEM_ERROR,
             "Staging directory %s does not exist",
             cork_path_get(packager->staging_path));
        return -1;
    }

    /* Create a temporary directory */
    pkg_path = cork_path_user_cache_path();
    cork_path_append(pkg_path, "buzzy/arch/package");
    cork_path_append(pkg_path, package_name);
    cork_path_append(pkg_path, version_string);
    rip_check(pkg_dir = bz_create_directory(pkg_path));

    /* Create a PKGBUILD file for this package */
    cork_buffer_append_printf(&buf, "pkgname='%s'\n", package_name);
    cork_buffer_append_string(&buf, "pkgver='");
    bz_version_to_arch(version, &buf);
    cork_buffer_append_string(&buf, "'\n");
    cork_buffer_append_printf(&buf, "pkgrel='%s'\n", BZ_ARCH_RELEASE);
    cork_buffer_append_printf(&buf, "arch=('%s')\n", architecture);
    cork_buffer_append_printf(&buf, "license=('%s')\n", license);
    /* TODO: dependencies */
    cork_buffer_append_printf(&buf,
        "package () {\n"
        "    rm -rf \"${pkgdir}\"\n"
        "    cp -a '%s' \"${pkgdir}\"\n"
        "}\n",
        cork_path_get(packager->staging_path)
    );

    filename = cork_path_join(pkg_path, "PKGBUILD");
    ep_check(pkgbuild = bz_create_file(filename, &buf));
    cork_buffer_done(&buf);

    env = cork_env_clone_current();
    cork_env_add(env, "PKGDEST", cork_path_get(packager->package_path));
    cork_env_add(env, "PKGEXT", ".pkg.tar.xz");

    exec = cork_exec_new_with_params("makepkg", "-sf", NULL);
    cork_exec_set_cwd(exec, cork_path_get(pkg_path));
    cork_exec_set_env(exec, env);

    cork_file_free(pkg_dir);
    cork_file_free(pkgbuild);
    return bz_subprocess_run_exec(packager->verbose, NULL, exec);

error:
    cork_file_free(pkg_dir);
    if (pkgbuild != NULL) {
        cork_file_free(pkgbuild);
    }
    cork_buffer_done(&buf);
    return -1;
}

struct bz_action *
bz_pacman_create_package(struct bz_package_spec *spec,
                         struct cork_path *package_path,
                         struct cork_path *staging_path,
                         struct bz_action *stage_action,
                         bool force, bool verbose)
{
    struct bz_action  *action;
    struct bz_action  *prereq;
    struct bz_pacman_packager  *packager;

    packager = cork_new(struct bz_pacman_packager);
    packager->spec = spec;
    packager->package_path = package_path;
    packager->staging_path = staging_path;
    packager->force = force;
    packager->verbose = verbose;

    action = bz_action_new
        (packager, bz_pacman_packager__free,
         bz_pacman_packager__message,
         bz_pacman_packager__is_needed,
         bz_pacman_packager__perform);

    ep_check(prereq = bz_install_dependency_string("pacman"));
    bz_action_add_pre(action, prereq);

    if (stage_action != NULL) {
        bz_action_add_pre(action, stage_action);
    }

    return action;

error:
    bz_action_free(action);
    return NULL;
}
