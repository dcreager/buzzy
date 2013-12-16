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
#include <libcork/ds.h>
#include <libcork/os.h>
#include <libcork/helpers/errors.h>
#include <yaml.h>

#include "buzzy/error.h"
#include "buzzy/repo.h"
#include "buzzy/yaml.h"


/*-----------------------------------------------------------------------
 * URL repository cache
 */

static struct cork_hash_table  *url_repos = NULL;

static void
url_repos_done(void)
{
    cork_hash_table_free(url_repos);
}

static void
url_repos_init(void)
{
    if (CORK_UNLIKELY(url_repos == NULL)) {
        url_repos = cork_string_hash_table_new(0, 0);
        cork_hash_table_set_free_key(url_repos, (cork_free_f) cork_strfree);
        cork_cleanup_at_exit(0, url_repos_done);
    }
}


/*-----------------------------------------------------------------------
 * URL repositories
 */

static struct bz_repo *
bz_url_repo_create(const char *url)
{
    /* If the string doesn't look like an actual URL (by containing "://"), then
     * we assume the URL is a local filesystem path. */
    if (strstr(url, "://") == NULL) {
        return bz_local_filesystem_repo_new(url);
    }

    /* Otherwise we look at the URL scheme.  Each of these memcmp calls are
     * valid, even if we compare against a "short" url string — they can't
     * possibly read past the end of the string, because the NUL terminator
     * would cause the comparison to return. */

    if (memcmp(url, "file://", 7) == 0) {
        /* For file URLs, we just use the content of the URL as a local
         * filesystem path. */
        return bz_local_filesystem_repo_new(url + 7);
    }

    if ((memcmp(url, "git://", 6) == 0) || (memcmp(url, "git+", 4) == 0)) {
        /* For git URLs, we assume the user wants the "master" branch.  (If not,
         * they need to use the !git YAML tag so that they can provide a more
         * complex configuration.) */
        return bz_git_repo_new(url, "master");
    }

    /* Not a URL that we recognize. */
    bz_bad_config("Unknown repository URL %s", url);
    return NULL;
}

struct bz_repo *
bz_url_repo_new(const char *url)
{
    struct cork_hash_table_entry  *entry;
    bool  is_new;

    url_repos_init();
    entry = cork_hash_table_get_or_create(url_repos, (void *) url, &is_new);

    if (is_new) {
        struct bz_repo  *repo;
        entry->key = (void *) cork_strdup(url);
        rpp_check(repo = bz_url_repo_create(url));
        bz_repo_register(repo);
        entry->value = repo;
    }

    return entry->value;
}


/*-----------------------------------------------------------------------
 * YAML repo links
 */

static struct bz_repo *
bz_yaml_git_repo_new(yaml_document_t *doc, int node_id)
{
    struct bz_yaml_mapping_element  elements[] = {
        { "url", -1, true },
        { "commit", -1, true },
        { NULL }
    };
    int  *url_id = &elements[0].value_id;
    int  *commit_id = &elements[1].value_id;
    const char  *url;
    const char  *commit;
    struct cork_hash_table_entry  *entry;
    bool  is_new;

    rpi_check(bz_yaml_get_mapping_elements
              (doc, node_id, elements, true, "!git repo link"));
    rpp_check(url = bz_yaml_get_string(doc, *url_id, "url"));
    rpp_check(commit = bz_yaml_get_string(doc, *commit_id, "commit"));

    entry = cork_hash_table_get_or_create(url_repos, (void *) url, &is_new);

    if (is_new) {
        struct bz_repo  *repo;
        entry->key = (void *) cork_strdup(url);
        rpp_check(repo = bz_git_repo_new(url, commit));
        bz_repo_register(repo);
        entry->value = repo;
    }

    return entry->value;
}

struct bz_repo *
bz_yaml_repo_new(yaml_document_t *doc, int node_id)
{
    yaml_node_t  *node = yaml_document_get_node(doc, node_id);
    const char  *tag = (const char *) node->tag;

    url_repos_init();

    /* Simple strings are treated as URLs. */
    if (strcmp(tag, YAML_STR_TAG) == 0) {
        const char  *url = (const char *) node->data.scalar.value;
        return bz_url_repo_new(url);
    }

    /* !git and !git-env are git repositories */
    if ((strcmp(tag, "!git") == 0) || (strcmp(tag, "!git-env") == 0)) {
        return bz_yaml_git_repo_new(doc, node_id);
    }

    /* Otherwise we don't know what to do. */
    bz_bad_config("Unknown repository type %s", tag);
    return NULL;
}

int
bz_repo_parse_yaml_links(struct bz_repo *repo, const char *path)
{
    yaml_document_t  doc;
    yaml_node_t  *root;
    yaml_node_item_t  *item;

    rii_check(bz_load_yaml_file(&doc, path));
    root = yaml_document_get_root_node(&doc);
    if (CORK_UNLIKELY(root->type != YAML_SEQUENCE_NODE)) {
        bz_bad_config("Repo links file must contain a sequence");
        goto error;
    }

    for (item = root->data.sequence.items.start;
         item < root->data.sequence.items.top; item++) {
        struct bz_repo  *link;
        ep_check(link = bz_yaml_repo_new(&doc, *item));
        bz_repo_add_link(repo, link);
    }

    yaml_document_delete(&doc);
    return 0;

error:
    yaml_document_delete(&doc);
    return -1;
}
