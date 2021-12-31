/*
 * Copyright (C) 2021 Liquidaty and the zsv/lib contributors
 * All rights reserved
 *
 * This file is part of zsv/lib, distributed under the license defined at
 * https://opensource.org/licenses/MIT
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zsv.h>
#include <zsv/utils/string.h>
#include <zsv/utils/arg.h>

/* havearg(): case-insensitive partial arg matching */
char havearg(const char *arg,
             const char *form1, size_t min_len1,
             const char *form2, size_t min_len2) {
  size_t len = strlen(arg);
  if(!min_len1)
    min_len1 = strlen(form1);
  if(len > min_len1)
    min_len1 = len;

  if(!zsv_strincmp((const unsigned char *)arg, min_len1,
                     (const unsigned char *)form1, min_len1))
    return 1;

  if(form2) {
    if(!min_len2)
      min_len2 = strlen(form2);
    if(len > min_len2)
      min_len2 = len;
    if(!zsv_strincmp((const unsigned char *)arg, min_len2,
                       (const unsigned char *)form2, min_len2))
      return 1;
  }
  return 0;
}

static struct zsv_opts zsv_default_opts = { 0 };
char zsv_default_opts_initd = 0;

ZSV_EXPORT
struct zsv_opts zsv_get_default_opts() {
  if(!zsv_default_opts_initd) {
    zsv_default_opts_initd = 1;
    zsv_default_opts.max_row_size = ZSV_ROW_MAX_SIZE_DEFAULT;
    zsv_default_opts.max_columns = ZSV_MAX_COLS_DEFAULT;
  }
  return zsv_default_opts;
}

ZSV_EXPORT
void zsv_set_default_opts(struct zsv_opts opts) {
  zsv_default_opts = opts;
}

/**
 * str_array_index_of: return index in list, or size of list if not found
 */
static inline int str_array_index_of(const char *list[], const char *s) {
  int i;
  for(i = 0; list[i] && strcmp(list[i], s); i++) ;
  return i;
}

ZSV_EXPORT
int zsv_args_to_opts(int argc, const char *argv[],
                     int *argc_out, const char **argv_out,
                     struct zsv_opts *opts_out
                     ) {
  *opts_out = zsv_get_default_opts();
  int options_start = 1; // skip this many args before we start looking for options
  int err = 0;
  int new_argc = 0;
  for(; new_argc < options_start && new_argc < argc; new_argc++)
    argv_out[new_argc] = argv[new_argc];

  static const char *short_args = "BcrtOqv";
  static const char *long_args[] = {
    "buff-size",
    "max-column-count",
    "max-row-size",
    "tab-delim",
    "other-delim",
    "no-quote",
    "verbose",
    NULL
  };
  for(int i = options_start; !err && i < argc; i++) {
    char arg = 0;
    if(*argv[i] != '-') { /* pass this option through */
      argv_out[new_argc++] = argv[i];
      continue;
    }
    if(argv[i][1] != '-') {
      if(!argv[i][2] && strchr(short_args, argv[i][1]))
        arg = argv[i][1];
    } else
      arg = short_args[str_array_index_of(long_args, argv[i] + 2)];

    switch(arg) {
    case 't':
      opts_out->delimiter = '\t';
      break;
    case 'q':
      opts_out->no_quotes = 1;
      break;
    case 'v':
      opts_out->verbose = 1;
      break;
    case 'B':
    case 'c':
    case 'r':
    case 'O':
      if(++i >= argc)
        err = fprintf(stderr, "Error: option %s requires a value\n", argv[i-1]);
      else if(arg == 'O') {
        const char *val = argv[i];
        if(strlen(val) != 1 || *val == 0)
          err = fprintf(stderr, "Error: delimiter '%s' may only be a single ascii character", val);
        else if(strchr("\n\r\"", *val))
          err = fprintf(stderr, "Error: column delimiter may not be '\\n', '\\r' or '\"'\n");
        else
          opts_out->delimiter = *val;
      } else {
        const char *val = argv[i];
        /* arg = 'B', 'c' or 'r' */
        long n = atol(val);
        if(arg == 'B' && n < ZSV_MIN_SCANNER_BUFFSIZE)
          err = fprintf(stderr, "Error: buff size may not be less than %u (got %s)\n",
                        ZSV_MIN_SCANNER_BUFFSIZE, val);
        else if(arg == 'c' && n < 8)
          err = fprintf(stderr, "Error: max column count may not be less than 8 (got %s)\n", val);
        else if(arg == 'r' && n < ZSV_ROW_MAX_SIZE_MIN)
          err = fprintf(stderr, "Error: max row size size may not be less than %u (got %s)\n",
                        ZSV_ROW_MAX_SIZE_MIN, val);
        else if(arg == 'B')
          opts_out->buffsize = n;
        else if(arg == 'c')
          opts_out->max_columns = n;
        else if(arg == 'r')
          opts_out->max_row_size = n;
      }
      break;
    default: /* pass this option through */
      argv_out[new_argc++] = argv[i];
      break;
    }
  }

  *argc_out = new_argc;
  return err;
}
