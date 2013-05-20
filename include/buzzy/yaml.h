/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#ifndef BUZZY_YAML_H
#define BUZZY_YAML_H

#include <libcork/core.h>
#include <yaml.h>

#include "buzzy/env.h"


/*-----------------------------------------------------------------------
 * YAML helpers
 */

int
bz_load_yaml_file(yaml_document_t *doc, const char *path);

int
bz_load_yaml_string(yaml_document_t *doc, const char *content);


#endif /* BUZZY_YAML_H */
