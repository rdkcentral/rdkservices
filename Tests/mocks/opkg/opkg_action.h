/* vi: set expandtab sw=4 sts=4: */
/* opkg_action.c - the opkg package management system

   Copyright (C) 2015 National Instruments Corp.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
*/

#ifndef OPKG_ACTION_H
#define OPKG_ACTION_H

#ifdef __cplusplus
extern "C" {
#endif

    int opkg_install(int argc, char **argv);
    int opkg_remove(int argc, char **argv);
    int opkg_upgrade(int argc, char **argv);

#ifdef __cplusplus
}
#endif
#endif    /* OPKG_ACTION_H */
