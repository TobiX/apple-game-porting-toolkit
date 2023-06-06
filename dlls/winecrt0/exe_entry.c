/*
 * Default entry point for an exe
 *
 * Copyright 2005 Alexandre Julliard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#if 0
#pragma makedep unix
#endif

#include <stdarg.h>
#include "windef.h"
#include "winbase.h"
#include "winternl.h"

/* 32on64 FIXME: Is this right? If I see it right this non-cdecl
 * main symbol is used when linking against host libc, so it'd be
 * a host addr space symbol. See commit 2e115ab4420e. */

extern int main( int argc, char *argv[] );

#include <wine/hostaddrspace_enter.h>

static char **build_argv( const char *src, int *ret_argc )
{
    char **argv, *arg, *dst;
    int argc, in_quotes = 0, bcount = 0, len = strlen(src) + 1;

    argc = 2 + len / 2;
    argv = HeapAlloc( GetProcessHeap(), 0, argc * sizeof(*argv) + len );
    arg = dst = (char *)(argv + argc);
    argc = 0;
    while (*src)
    {
        if ((*src == ' ' || *src == '\t') && !in_quotes)
        {
            /* skip the remaining spaces */
            while (*src == ' ' || *src == '\t') src++;
            if (!*src) break;
            /* close the argument and copy it */
            *dst++ = 0;
            argv[argc++] = arg;
            /* start with a new argument */
            arg = dst;
            bcount = 0;
        }
        else if (*src == '\\')
        {
            *dst++ = *src++;
            bcount++;
        }
        else if (*src == '"')
        {
            if ((bcount & 1) == 0)
            {
                /* Preceded by an even number of '\', this is half that
                 * number of '\', plus a '"' which we discard.
                 */
                dst -= bcount / 2;
                src++;
                if (in_quotes && *src == '"') *dst++ = *src++;
                else in_quotes = !in_quotes;
            }
            else
            {
                /* Preceded by an odd number of '\', this is half that
                 * number of '\' followed by a '"'
                 */
                dst -= bcount / 2 + 1;
                *dst++ = *src++;
            }
            bcount = 0;
        }
        else  /* a regular character */
        {
            *dst++ = *src++;
            bcount = 0;
        }
    }
    *dst = 0;
    argv[argc++] = arg;
    argv[argc] = NULL;
    *ret_argc = argc;
    return argv;
}

DWORD WINAPI DECLSPEC_HIDDEN __wine_spec_exe_entry( PEB *peb )
{
    int argc;
    char **argv = build_argv( GetCommandLineA(), &argc );

    /* 32on64 FIXME: argv -> __wine_main_argv ?? */
    ExitProcess( main( argc, argv ));
}

#include <wine/hostaddrspace_exit.h>