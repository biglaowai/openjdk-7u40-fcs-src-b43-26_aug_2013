/*
 * Copyright (c) 1998, 2012, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

/*
 * Maintains a list of currently loaded DLLs (Dynamic Link Libraries)
 * and their associated handles. Library names are case-insensitive.
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <io.h>

#include "sys.h"

#include "path_md.h"

static void dll_build_name(char* buffer, size_t buflen,
                           const char* pname, const char* fname) {
    // Based on os_windows.cpp

    char *path_sep = PATH_SEPARATOR;
    char *pathname = (char *)pname;
    while (strlen(pathname) > 0) {
        char *p = strchr(pathname, *path_sep);
        if (p == NULL) {
            p = pathname + strlen(pathname);
        }
        /* check for NULL path */
        if (p == pathname) {
            continue;
        }
        if (*(p-1) == ':' || *(p-1) == '\\') {
            (void)_snprintf(buffer, buflen, "%.*s%s.dll", (p - pathname),
                            pathname, fname);
        } else {
            (void)_snprintf(buffer, buflen, "%.*s\\%s.dll", (p - pathname),
                            pathname, fname);
        }
        if (_access(buffer, 0) == 0) {
            break;
        }
        pathname = p + 1;
        *buffer = '\0';
    }
}

/*
 * From system_md.c v1.54
 */
int
dbgsysGetLastErrorString(char *buf, int len)
{
    long errval;

    if ((errval = GetLastError()) != 0) {
        /* DOS error */
        int n = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
                              NULL, errval,
                              0, buf, len, NULL);
        if (n > 3) {
            /* Drop final '.', CR, LF */
            if (buf[n - 1] == '\n') n--;
            if (buf[n - 1] == '\r') n--;
            if (buf[n - 1] == '.') n--;
            buf[n] = '\0';
        }
        return n;
    }

    if (errno != 0) {
        /* C runtime error that has no corresponding DOS error code */
        const char *s = strerror(errno);
        int n = (int)strlen(s);
        if (n >= len) n = len - 1;
        strncpy(buf, s, n);
        buf[n] = '\0';
        return n;
    }

    return 0;
}

/*
 * Build a machine dependent library name out of a path and file name.
 */
void
dbgsysBuildLibName(char *holder, int holderlen, char *pname, char *fname)
{
    const int pnamelen = pname ? (int)strlen(pname) : 0;

    *holder = '\0';
    /* Quietly truncates on buffer overflow. Should be an error. */
    if (pnamelen + (int)strlen(fname) + 10 > holderlen) {
        return;
    }

    if (pnamelen == 0) {
        sprintf(holder, "%s.dll", fname);
    } else {
      dll_build_name(holder, holderlen, pname, fname);
    }
}

void *
dbgsysLoadLibrary(const char * name, char *err_buf, int err_buflen)
{
    void *result = LoadLibrary(name);
    if (result == NULL) {
        /* Error message is pretty lame, try to make a better guess. */
        long errcode = GetLastError();
        if (errcode == ERROR_MOD_NOT_FOUND) {
            strncpy(err_buf, "Can't find dependent libraries", err_buflen-2);
            err_buf[err_buflen-1] = '\0';
        } else {
            dbgsysGetLastErrorString(err_buf, err_buflen);
        }
    }
    return result;
}

void dbgsysUnloadLibrary(void *handle)
{
    FreeLibrary(handle);
}

void * dbgsysFindLibraryEntry(void *handle, const char *name)
{
    return GetProcAddress(handle, name);
}
