/*
 * wpa_supplicant D-Bus control interface - internal definitions
 * Copyright (c) 2006, Dan Williams <dcbw@redhat.com> and Red Hat, Inc.
 * Copyright (c) 2009, Witold Sowa <witold.sowa@gmail.com>
 * Copyright (c) 2009, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 *
 * Qualcomm Atheros, Inc. chooses to take this file subject only to the 
 * terms of the BSD license.
 */

#ifndef DBUS_COMMON_I_H
#define DBUS_COMMON_I_H

#include <dbus/dbus.h>

struct wpas_dbus_priv {
	DBusConnection *con;
	int should_dispatch;
	struct wpa_global *global;
	u32 next_objid;
	int dbus_new_initialized;

#if defined(CONFIG_CTRL_IFACE_DBUS_NEW) && defined(CONFIG_AP)
	int dbus_noc_refcnt;
#endif /* CONFIG_CTRL_IFACE_DBUS_NEW && CONFIG_AP */
};

#endif /* DBUS_COMMON_I_H */
