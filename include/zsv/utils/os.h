/*
 * Copyright (C) 2021 Liquidaty and the zsv/lib contributors
 * All rights reserved
 *
 * This file is part of zsv/lib, distributed under the license defined at
 * https://opensource.org/licenses/MIT
 */

#ifndef ZSV_OS_H
#define ZSV_OS_H

#ifndef _WIN32
# define zsv_replace_file(src, dest) (rename((const char *)src, (const char *)dest))


#else

# include <windows.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#endif

void to_unicode(const void *path, wchar_t *wbuf, size_t wbuf_len);

int zsv_replace_file(const void *src, const void *dest);
#endif

#endif
