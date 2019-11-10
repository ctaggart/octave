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

#if ! defined (octave_gui_preferences_ve_h)
#define octave_gui_preferences_ve_h 1

#include "gui-preferences.h"

#include <QApplication>

// Variable Editor preferences

const gui_pref
ve_use_terminal_font ("variable_editor/use_terminal_font", QVariant (true));

const gui_pref
ve_font_name ("variable_editor/font_name", QVariant ());

const gui_pref
ve_font_size ("variable_editor/font_size", QVariant (10));

const gui_pref
ve_column_width ("variable_editor/column_width", QVariant (100));

const gui_pref
ve_row_height ("variable_editor/row_height", QVariant (10));

const gui_pref
ve_alternate_rows ("variable_editor/alternate_rows", QVariant (false));

const int ve_colors_count = 5;

const QString ve_color_chars ("fbsha");

const gui_pref ve_colors[ve_colors_count] =
{
  {"variable_editor/color_f", QVariant (QPalette::WindowText)},
  {"variable_editor/color_b", QVariant (QPalette::Base)},
  {"variable_editor/color_s", QVariant (QPalette::HighlightedText)},
  {"variable_editor/color_h", QVariant (QPalette::Highlight)},
  {"variable_editor/color_a", QVariant (QPalette::AlternateBase)}
};

const QStringList ve_color_names (QStringList ()
    << QT_TRANSLATE_NOOP ("variable_editor::variable_editor", "Foreground")
    << QT_TRANSLATE_NOOP ("workspace_model::workspace_model", "Background")
    << QT_TRANSLATE_NOOP ("workspace_model::workspace_model", "Selected Foreground")
    << QT_TRANSLATE_NOOP ("workspace_model::workspace_model", "Selected Background")
    << QT_TRANSLATE_NOOP ("workspace_model::workspace_model", "Alternate Background"));

#endif
