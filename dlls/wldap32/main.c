/*
 * WLDAP32 - LDAP support for Wine
 *
 * Copyright 2005 Hans Leidekker
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

#include <stdarg.h>
#include "windef.h"
#include "winternl.h"
#include "winbase.h"

#include "wine/debug.h"
#include "libldap.h"

HINSTANCE hwldap32;

WINE_DEFAULT_DEBUG_CHANNEL(wldap32);

unixlib_handle_t ldap_handle = 0;

BOOL WINAPI DllMain( HINSTANCE hinst, DWORD reason, LPVOID reserved )
{
    TRACE( "(%p, %#lx, %p)\n", hinst, reason, reserved );

    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        hwldap32 = hinst;
        DisableThreadLibraryCalls( hinst );
        if (NtQueryVirtualMemory( GetCurrentProcess(), hinst, MemoryWineUnixFuncs,
                                  &ldap_handle, sizeof(ldap_handle), NULL ) || LDAP_CALL( process_attach, NULL ))
	{
            ldap_handle = 0;
            ERR( "No libldap support, expect problems\n" );
        }
        break;
    }
    return TRUE;
}
