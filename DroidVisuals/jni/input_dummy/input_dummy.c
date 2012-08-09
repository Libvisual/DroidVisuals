/* Libvisual-plugins - Standard plugins for libvisual
 * 
 * Copyright (C) 2012
 *
 * Authors: Scott Sibley <sisibley@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <gettext.h>
#include <math.h>
#include <libvisual/libvisual.h>

static int inp_alsa_init (VisPluginData *plugin);
static int inp_alsa_cleanup (VisPluginData *plugin);
static int inp_alsa_upload (VisPluginData *plugin, VisAudio *audio);

VISUAL_PLUGIN_API_VERSION_VALIDATOR

const VisPluginInfo *get_plugin_info ()
{
    static VisInputPlugin input = {
        .upload = inp_alsa_upload
    };

    static VisPluginInfo info = {
        .type = VISUAL_PLUGIN_TYPE_INPUT,

        .plugname = "dummy",
        .name = "dummy",
        .author = "Scott Sibley <sisibley@gmail.com>",
        .version = "0.1",
        .about = ("DUMMY input plugin for libvisual"),
        .help = ("Just a dummy input plugin with no function."),
        .license = VISUAL_PLUGIN_LICENSE_LGPL,

        .init = inp_alsa_init,
        .cleanup = inp_alsa_cleanup,

        .plugin = VISUAL_OBJECT (&input)
    };

    return &info;
}

int inp_alsa_init (VisPluginData *plugin)
{
    return 0;
}

int inp_alsa_cleanup (VisPluginData *plugin)
{
    return 0;
}

int inp_alsa_upload (VisPluginData *plugin, VisAudio *audio)
{
    return 0;
}

