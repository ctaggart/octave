/*

Copyright (C) 2017-2018 Torsten <mttl@mailbox.de>

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#if ! defined (octave_editor_settings_h)
#define octave_editor_settings_h 1

//#if defined (HAVE_CONFIG_H)
//#  include "config.h"
//#endif

#include <QStringList>
#include <QVariant>
#include <QStyle>

// Structure for the definition of pairs: key and default value

struct gui_pref
{
  gui_pref (const QString& key_, QVariant def_): key (key_), def (def_) {}
  QString   key;  // the key name
  QVariant  def;  // the default value
};


// Global preferences

// Get the default monospaced font
#if defined (Q_WS_X11)
const QString global_font_family = "Monospace";
#elif defined (Q_WS_WIN) || defined (Q_WS_MAC)
const QString global_font_family = "Courier";
#else
const QString global_font_family = "Courier";
#endif
const gui_pref global_mono_font ("monospace_font", global_font_family);

// Icon size (in preferences: values -1, 0, 1)
const QStyle::PixelMetric global_icon_sizes[3] =
{
  QStyle::PM_SmallIconSize,
  QStyle::PM_ToolBarIconSize,
  QStyle::PM_LargeIconSize
};
const gui_pref global_icon_size ("toolbar_icon_size", QVariant (0));
const gui_pref global_icon_theme ("use_system_icon_theme", QVariant (true));

// Style
const gui_pref global_style ("style", QVariant ("default"));

// Main window preferences

const unsigned char ba_geometry_data[] = {
  0x01,0xd9,0xd0,0xcb,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x1b,0x00,0x00,0x03,0xaf,0x00,0x00,0x02,0xb8,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x39,0x00,0x00,0x03,0xaf,0x00,0x00,0x02,0xb8,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x00
};
const gui_pref mw_geometry ("MainWindow/geometry",
  QVariant (QByteArray (reinterpret_cast<const char*> (ba_geometry_data), sizeof (ba_geometry_data))));
  // FIXME: usew the following version when Qt4 support is dropped
  // QVariant (QByteArrayLiteral ("\x1\xd9\xd0\xcb\0\x2\0\0\0\0\0\0\0\0\0\x1b\0\0\x3\xaf\0\0\x2\xb8\0\0\0\0\0\0\0\x39\0\0\x3\xaf\0\0\x2\xb8\0\0\0\0\0\0\0\0\x5\0")));

const unsigned char ba_state_data[] = {
  '\0','\0','\0',0xff,'\0','\0','\0','\0',0xfd,'\0','\0','\0',0x2,'\0','\0',
  '\0','\0','\0','\0',0x1,'&','\0','\0',0x2,'+',0xfc,0x2,'\0','\0','\0',0x3,
  0xfb,'\0','\0','\0',0x1e,'\0','F','\0','i','\0','l','\0','e','\0','s','\0',
  'D','\0','o','\0','c','\0','k','\0','W','\0','i','\0','d','\0','g','\0','e',
  '\0','t',0x1,'\0','\0','\0','?','\0','\0','\0',0xac,'\0','\0','\0',0x88,
  '\0',0xff,0xff,0xff,0xfb,'\0','\0','\0',0x1a,'\0','W','\0','o','\0','r',
  '\0','k','\0','s','\0','p','\0','a','\0','c','\0','e','\0','V','\0','i',
  '\0','e','\0','w',0x1,'\0','\0','\0',0xf1,'\0','\0','\0',0xca,'\0','\0',
  '\0',0x82,'\0',0xff,0xff,0xff,0xfb,'\0','\0','\0','\"','\0','H','\0','i',
  '\0','s','\0','t','\0','o','\0','r','\0','y','\0','D','\0','o','\0','c',
  '\0','k','\0','W','\0','i','\0','d','\0','g','\0','e','\0','t',0x1,'\0',
  '\0',0x1,0xc1,'\0','\0','\0',0xa9,'\0','\0','\0',0x82,'\0',0xff,0xff,0xff,
  '\0','\0','\0',0x1,'\0','\0',0x2,0x84,'\0','\0',0x2,'+',0xfc,0x2,'\0','\0',
  '\0',0x1,0xfc,'\0','\0','\0','?','\0','\0',0x2,'+','\0','\0','\0',0xeb,0x1,
  '\0','\0',0x1b,0xfa,'\0','\0','\0','\0',0x2,'\0','\0','\0',0x4,0xfb,'\0',
  '\0','\0','$','\0','T','\0','e','\0','r','\0','m','\0','i','\0','n','\0',
  'a','\0','l','\0','D','\0','o','\0','c','\0','k','\0','W','\0','i','\0',
  'd','\0','g','\0','e','\0','t',0x1,'\0','\0','\0','\0',0xff,0xff,0xff,0xff,
  '\0','\0','\0',0x46,'\0',0xff,0xff,0xff,0xfb,'\0','\0','\0','.','\0','D','\0',
  'o','\0','c','\0','u','\0','m','\0','e','\0','n','\0','t','\0','a','\0','t',
  '\0','i','\0','o','\0','n','\0','D','\0','o','\0','c','\0','k','\0','W',
  '\0','i','\0','d','\0','g','\0','e','\0','t',0x1,'\0','\0','\0','\0',0xff,
  0xff,0xff,0xff,'\0','\0','\0',0xcf,'\0',0xff,0xff,0xff,0xfb,'\0','\0','\0',
  0x14,'\0','F','\0','i','\0','l','\0','e','\0','E','\0','d','\0','i','\0',
  't','\0','o','\0','r',0x1,'\0','\0','\0','\0',0xff,0xff,0xff,0xff,'\0',
  '\0','\0',0x62,'\0',0xff,0xff,0xff,0xfb,'\0','\0','\0',0x1c,'\0','V','\0',
  'a','\0','r','\0','i','\0','a','\0','b','\0','l','\0','e','\0','E','\0',
  'D','\0','i','\0','t','\0','o','\0','r',0x1,'\0','\0','\0','\0',0xff,0xff,
  0xff,0xff,'\0','\0','\0',';','\0',0xff,0xff,0xff,'\0','\0','\0','\0','\0',
  '\0',0x2,'+','\0','\0','\0',0x4,'\0','\0','\0',0x4,'\0','\0','\0','\b',
  '\0','\0','\0','\b',0xfc,'\0','\0','\0',0x1,'\0','\0','\0',0x2,'\0','\0',
  '\0',0x1,'\0','\0','\0',0x16,'\0','M','\0','a','\0','i','\0','n','\0','T',
  '\0','o','\0','o','\0','l','\0','b','\0','a','\0','r',0x1,'\0','\0','\0',
  '\0',0xff,0xff,0xff,0xff,'\0','\0','\0','\0','\0','\0','\0','\0'
};
const gui_pref mw_state ("MainWindow/windowState",
  QVariant (QByteArray (reinterpret_cast<const char*> (ba_state_data), sizeof (ba_state_data))));
  // FIXME: usew the following version when Qt4 support is dropped
  // QVariant (QByteArrayLiteral ("\0\0\0\xff\0\0\0\0\xfd\0\0\0\x2\0\0\0\0\0\0\x1&\0\0\x2+\xfc\x2\0\0\0\x3\xfb\0\0\0\x1e\0\x46\0i\0l\0\x65\0s\0\x44\0o\0\x63\0k\0W\0i\0\x64\0g\0\x65\0t\x1\0\0\0?\0\0\0\xac\0\0\0\x88\0\xff\xff\xff\xfb\0\0\0\x1a\0W\0o\0r\0k\0s\0p\0\x61\0\x63\0\x65\0V\0i\0\x65\0w\x1\0\0\0\xf1\0\0\0\xca\0\0\0\x82\0\xff\xff\xff\xfb\0\0\0\"\0H\0i\0s\0t\0o\0r\0y\0\x44\0o\0\x63\0k\0W\0i\0\x64\0g\0\x65\0t\x1\0\0\x1\xc1\0\0\0\xa9\0\0\0\x82\0\xff\xff\xff\0\0\0\x1\0\0\x2\x84\0\0\x2+\xfc\x2\0\0\0\x1\xfc\0\0\0?\0\0\x2+\0\0\0\xeb\x1\0\0\x1b\xfa\0\0\0\0\x2\0\0\0\x4\xfb\0\0\0$\0T\0\x65\0r\0m\0i\0n\0\x61\0l\0\x44\0o\0\x63\0k\0W\0i\0\x64\0g\0\x65\0t\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\x46\0\xff\xff\xff\xfb\0\0\0.\0\x44\0o\0\x63\0u\0m\0\x65\0n\0t\0\x61\0t\0i\0o\0n\0\x44\0o\0\x63\0k\0W\0i\0\x64\0g\0\x65\0t\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\xcf\0\xff\xff\xff\xfb\0\0\0\x14\0\x46\0i\0l\0\x65\0\x45\0\x64\0i\0t\0o\0r\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\x62\0\xff\xff\xff\xfb\0\0\0\x1c\0V\0\x61\0r\0i\0\x61\0\x62\0l\0\x65\0\x45\0\x64\0i\0t\0o\0r\x1\0\0\0\0\xff\xff\xff\xff\0\0\0;\0\xff\xff\xff\0\0\0\0\0\0\x2+\0\0\0\x4\0\0\0\x4\0\0\0\b\0\0\0\b\xfc\0\0\0\x1\0\0\0\x2\0\0\0\x1\0\0\0\x16\0M\0\x61\0i\0n\0T\0o\0o\0l\0\x42\0\x61\0r\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\0\0\0\0\0")));


// Console preferences

const gui_pref cs_font ("terminal/fontName", QVariant ());


// Variable Editor preferences

const gui_pref ve_font ("variable_editor/font_size", QVariant ());


// Editor preferences

// Octave comment strings
const gui_pref ed_comment_str_old ("editor/octave_comment_string", QVariant (0));
const gui_pref ed_comment_str ("editor/oct_comment_str", QVariant (0));
const gui_pref ed_uncomment_str ("editor/oct_uncomment_str", QVariant (1 + 2 + 4 + 8));
const QString ed_last_comment_str ("editor/oct_last_comment_str");
const QStringList ed_comment_strings (QStringList () << "##" << "#" << "%"<< "%%" << "%!");
const int ed_comment_strings_count = 5;


// Session data
const gui_pref ed_session_names ("editor/savedSessionTabs",
                                  QVariant (QStringList ()));
const gui_pref ed_session_enc ("editor/saved_session_encodings",
                                  QVariant (QStringList ()));
const gui_pref ed_session_ind ("editor/saved_session_tab_index",
                                  QVariant (QStringList ()));
const gui_pref ed_session_lines ("editor/saved_session_lines",
                                  QVariant (QStringList ()));


// File handling
const gui_pref ed_show_dbg_file ("editor/show_dbg_file", QVariant (true));
#if defined (Q_OS_WIN32)
const gui_pref ed_default_enc ("editor/default_encoding", QVariant ("SYSTEM"));
#else
const gui_pref ed_default_enc ("editor/default_encoding", QVariant ("UTF-8"));
#endif


// Files dock widget

const gui_pref fb_column_state ("filesdockwidget/column_state", QVariant ());
const gui_pref fb_show_ ("filesdockwidget/column_state", QVariant ());
const gui_pref fb_mru_list ("filesdockwidget/mru_dir_list", QVariant (QStringList ()));
const gui_pref fb_show_size ("filesdockwidget/showFileSize", QVariant (false));
const gui_pref fb_show_type ("filesdockwidget/showFileType", QVariant (false));
const gui_pref fb_show_date ("filesdockwidget/showLastModified", QVariant (false));
const gui_pref fb_show_hidden ("filesdockwidget/showHiddenFiles", QVariant (false));
const gui_pref fb_show_altcol ("filesdockwidget/useAlternatingRowColors", QVariant (true));
const gui_pref fb_sort_column ("filesdockwidget/sort_files_by_column", QVariant (0));
const gui_pref fb_sort_order ("filesdockwidget/sort_files_by_order", QVariant (Qt::AscendingOrder));
const gui_pref fb_sync_octdir ("filesdockwidget/sync_octave_directory", QVariant (true));
const gui_pref fb_restore_last_dir ("filesdockwidget/restore_last_dir", QVariant (false));
const gui_pref fb_startup_dir ("filesdockwidget/startup_dir", QVariant (QString ()));
const gui_pref fb_txt_file_ext ("filesdockwidget/txt_file_extensions",
                                QVariant ("m;c;cc;cpp;h;txt"));

// Workspace view

const gui_pref ws_enable_colors ("workspaceview/enable_colors", QVariant (false));
const gui_pref ws_hide_tool_tips ("workspaceview/hide_tools_tips", QVariant (false));

#endif
