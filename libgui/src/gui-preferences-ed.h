/*

Copyright (C) 2017-2019 Torsten <mttl@mailbox.de>

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

#if ! defined (octave_gui_preferences_ed_h)
#define octave_gui_preferences_ed_h 1

#include "gui-preferences.h"

// Editor preferences

// Octave comment strings

const gui_pref
ed_comment_str_old ("editor/octave_comment_string", QVariant (0));

const gui_pref
ed_comment_str ("editor/oct_comment_str", QVariant (0));

const gui_pref
ed_uncomment_str ("editor/oct_uncomment_str", QVariant (1 + 2 + 4 + 8));

const QString
ed_last_comment_str ("editor/oct_last_comment_str");

const QStringList
ed_comment_strings (QStringList () << "##" << "#" << "%"<< "%%" << "%!");

const int ed_comment_strings_count = 5;

// Session data

const gui_pref
ed_session_names ("editor/savedSessionTabs", QVariant (QStringList ()));

const gui_pref
ed_session_enc ("editor/saved_session_encodings", QVariant (QStringList ()));

const gui_pref
ed_session_ind ("editor/saved_session_tab_index", QVariant (QStringList ()));

const gui_pref
ed_session_lines ("editor/saved_session_lines", QVariant (QStringList ()));

// Tabs
const QStringList
ed_tab_position_names (QStringList ()
                       << QT_TRANSLATE_NOOP ("file_editor::file_editor", "Top")
                       << QT_TRANSLATE_NOOP ("file_editor::file_editor", "Bottom")
                       << QT_TRANSLATE_NOOP ("file_editor::file_editor", "Left")
                       << QT_TRANSLATE_NOOP ("file_editor::file_editor", "Right"));

const gui_pref
ed_tab_position ("editor/tab_position", QVariant (QTabWidget::North));

// File handling

const gui_pref
ed_show_dbg_file ("editor/show_dbg_file", QVariant (true));

const gui_pref
ed_default_enc ("editor/default_encoding",
                QVariant (QTextCodec::codecForLocale ()->name ().toUpper ().prepend ("SYSTEM (").append (")")));

#endif
