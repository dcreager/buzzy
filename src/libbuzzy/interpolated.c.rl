/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2013, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the COPYING file in this distribution for license details.
 * ----------------------------------------------------------------------
 */

#include <libcork/core.h>
#include <libcork/ds.h>
#include <libcork/helpers/errors.h>

#include "buzzy/env.h"
#include "buzzy/error.h"


/*-----------------------------------------------------------------------
 * Interpolated elements
 */

typedef int
(*bz_render_element_f)(void *user_data, struct bz_env *env,
                       struct cork_buffer *dest);

struct bz_interpolated_element {
    void  *user_data;
    cork_free_f  free_user_data;
    bz_render_element_f  render;
};

static struct bz_interpolated_element *
bz_interpolated_element_new(void *user_data, cork_free_f free_user_data,
                            bz_render_element_f render)
{
    struct bz_interpolated_element  *element =
        cork_new(struct bz_interpolated_element);
    element->user_data = user_data;
    element->free_user_data = free_user_data;
    element->render = render;
    return element;
}

static void
bz_interpolated_element_free(struct bz_interpolated_element *element)
{
    cork_free_user_data(element);
    free(element);
}

static int
bz_interpolated_element_render(struct bz_interpolated_element *element,
                               struct bz_env *env, struct cork_buffer *dest)
{
    return element->render(element->user_data, env, dest);
}


/*-----------------------------------------------------------------------
 * Strings
 */

static void
bz_string_element__free(void *user_data)
{
    const char  *content = user_data;
    cork_strfree(content);
}

static int
bz_string_element__render(void *user_data, struct bz_env *env,
                          struct cork_buffer *dest)
{
    const char  *content = user_data;
    cork_buffer_append_string(dest, content);
    return 0;
}

static struct bz_interpolated_element *
bz_string_element_new(const char *content, size_t length)
{
    const char  *copy = cork_strndup(content, length);
    return bz_interpolated_element_new
        ((void *) copy, bz_string_element__free, bz_string_element__render);
}


/*-----------------------------------------------------------------------
 * Variable references
 */

static void
bz_var_ref_element__free(void *user_data)
{
    const char  *var_name = user_data;
    cork_strfree(var_name);
}

static int
bz_var_ref_element__render(void *user_data, struct bz_env *env,
                           struct cork_buffer *dest)
{
    const char  *var_name = user_data;
    const char  *value = bz_env_get(env, var_name, NULL);
    if (CORK_UNLIKELY(cork_error_occurred())) {
        return -1;
    } else if (CORK_UNLIKELY(value == NULL)) {
        bz_bad_config("No variable named \"%s\"", var_name);
        return -1;
    } else {
        cork_buffer_append_string(dest, value);
        return 0;
    }
}

static struct bz_interpolated_element *
bz_var_ref_element_new(const char *var_name, size_t length)
{
    const char  *copy = cork_strndup(var_name, length);
    return bz_interpolated_element_new
        ((void *) copy, bz_var_ref_element__free, bz_var_ref_element__render);
}


/*-----------------------------------------------------------------------
 * Interpolated values
 */

struct bz_interpolated_value {
    cork_array(struct bz_interpolated_element *)  elements;
    struct cork_buffer  value;
};

static void
bz_interpolated_value__free(void *user_data)
{
    struct bz_interpolated_value  *value = user_data;
    size_t  i;
    for (i = 0; i < cork_array_size(&value->elements); i++) {
        struct bz_interpolated_element  *element =
            cork_array_at(&value->elements, i);
        bz_interpolated_element_free(element);
    }
    cork_array_done(&value->elements);
    cork_buffer_done(&value->value);
    free(value);
}

static const char *
bz_interpolated_value__provide(void *user_data, struct bz_env *env)
{
    size_t  i;
    struct bz_interpolated_value  *value = user_data;
    struct cork_buffer  *dest = &value->value;

    cork_buffer_clear(dest);
    for (i = 0; i < cork_array_size(&value->elements); i++) {
        struct bz_interpolated_element  *element =
            cork_array_at(&value->elements, i);
        rpi_check(bz_interpolated_element_render(element, env, dest));
    }
    return value->value.buf;
}

static void
bz_interpolated_value_add(struct bz_interpolated_value *value,
                          struct bz_interpolated_element *element)
{
    cork_array_append(&value->elements, element);
}

static int
bz_interpolated_value_parse(struct bz_interpolated_value *value,
                            const char *template_value)
{
    int  cs;
    const char  *p = template_value;
    const char  *pe = strchr(template_value, '\0');
    const char  *eof = pe;
    const char  *sstart = NULL;
    const char  *send = NULL;
    const char  *vstart = NULL;
    const char  *vend = NULL;
    struct bz_interpolated_element  *element;

    %%{
        machine interpolated_value;

        var_char = alnum | '.' | '_';
        var_name = var_char+
                   >{ vstart = fpc; }
                   %{ vend = fpc; };

        var_ref = '${' var_name '}'
                  %{
                      element = bz_var_ref_element_new(vstart, vend - vstart);
                      bz_interpolated_value_add(value, element);
                  };

        dollar = "$$"
               %{
                   element = bz_string_element_new("$", 1);
                   bz_interpolated_value_add(value, element);
               };

        string_char    = any - '$';
        string_content = string_char+
                       >{ sstart = fpc; }
                       %{ send = fpc; };

        string = string_content
                 %{
                     element = bz_string_element_new(sstart, send - sstart);
                     bz_interpolated_value_add(value, element);
                 };

        element = string | var_ref | dollar;

        main := element**;

        write data noerror nofinal;
        write init;
        write exec;
    }%%

    /* A hack to suppress some unused variable warnings */
    (void) interpolated_value_en_main;

    if (CORK_UNLIKELY(cs < %%{ write first_final; }%%)) {
        bz_bad_config("Bad interpolated value \"%s\"", template_value);
        return -1;
    }

    return 0;
}

struct bz_value_provider *
bz_interpolated_value_new(const char *template_value)
{
    struct bz_interpolated_value  *value;

    value = cork_new(struct bz_interpolated_value);
    cork_array_init(&value->elements);
    cork_buffer_init(&value->value);
    cork_buffer_append(&value->value, "", 0);
    ei_check(bz_interpolated_value_parse(value, template_value));
    return bz_value_provider_new
        (value, bz_interpolated_value__free, bz_interpolated_value__provide);

error:
    bz_interpolated_value__free(value);
    return NULL;
}
