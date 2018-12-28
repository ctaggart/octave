/*

Copyright (C) 2011-2018 Jacob Dawid

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

// Programming Note: this file has many lines longer than 80 characters
// due to long function, variable, and property names.  Please don't
// break those lines as it tends to make this code even harder to read.

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "resource-manager.h"
#include "shortcut-manager.h"
#include "variable-editor.h"
#include "workspace-model.h"
#include "settings-dialog.h"

#include <QButtonGroup>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QVector>
#include <QHash>
#include <QMessageBox>
#include <QScrollBar>
#include <QStyleFactory>
#include <QTextCodec>

#if defined (HAVE_QSCINTILLA)
#  include "octave-qscintilla.h"
#  include "octave-txt-lexer.h"
#  include <QScrollArea>

#  if defined (HAVE_QSCI_QSCILEXEROCTAVE_H)
#    define HAVE_LEXER_OCTAVE 1
#    include <Qsci/qscilexeroctave.h>
#  elif defined (HAVE_QSCI_QSCILEXERMATLAB_H)
#    define HAVE_LEXER_MATLAB 1
#    include <Qsci/qscilexermatlab.h>
#  endif

#  include <Qsci/qscilexercpp.h>
#  include <Qsci/qscilexerbash.h>
#  include <Qsci/qscilexerperl.h>
#  include <Qsci/qscilexerbatch.h>
#  include <Qsci/qscilexerdiff.h>
#endif

namespace octave
{
#if defined (HAVE_QSCINTILLA)

  static const int MaxLexerStyles = 64;
  static const int MaxStyleNumber = 128;

  static int get_valid_lexer_styles (QsciLexer *lexer, int *styles)
  {
    int max_style = 0;
    int actual_style = 0;
    while (actual_style < MaxStyleNumber && max_style < MaxLexerStyles)
      {
        if ((lexer->description (actual_style)) != "")  // valid style
          styles[max_style++] = actual_style;
        actual_style++;
      }
    return max_style;
  }

#endif

  settings_dialog::settings_dialog (QWidget *p, const QString& desired_tab)
    : QDialog (p), Ui::settings_dialog ()
  {
    setupUi (this);

    QSettings *settings = resource_manager::get_settings ();

    if (! settings)
      {
        QMessageBox msgBox
          (QMessageBox::Warning, tr ("Octave Preferences"),
           tr ("Unable to save preferences.  Missing preferences file or unknown directory."));

        msgBox.exec ();

        return;
      }

    // look for available language files and the actual settings
    QString qm_dir_name = resource_manager::get_gui_translation_dir ();
    QDir qm_dir (qm_dir_name);
    QFileInfoList qm_files = qm_dir.entryInfoList (QStringList ("*.qm"), QDir::Files | QDir::Readable, QDir::Name);

    for (int i = 0; i < qm_files.length (); i++)   // insert available languages
      comboBox_language->addItem (qm_files.at (i).baseName ());
    // System at beginning
    comboBox_language->insertItem (0, tr ("System setting"));
    comboBox_language->insertSeparator (1);    // separator after System
    QString language = settings->value ("language", "SYSTEM").toString ();
    if (language == "SYSTEM")
      language = tr ("System setting");
    int selected = comboBox_language->findText (language);
    if (selected >= 0)
      comboBox_language->setCurrentIndex (selected);
    else
      comboBox_language->setCurrentIndex (0);  // System is default

    // Global style
    QStringList styles = QStyleFactory::keys();
    combo_styles->addItems (styles);
    combo_styles->insertItem (0, global_style.def.toString ());
    combo_styles->insertSeparator (1);
    QString current_style = settings->value (global_style.key, global_style.def).toString ();
    if (current_style == global_style.def.toString ())
      current_style = global_style.def.toString ();
    selected = combo_styles->findText (current_style);
    if (selected >= 0)
      combo_styles->setCurrentIndex (selected);
    else
      combo_styles->setCurrentIndex (0);

    // icon size and theme
    QButtonGroup *icon_size_group = new QButtonGroup (this);
    icon_size_group->addButton (icon_size_small);
    icon_size_group->addButton (icon_size_normal);
    icon_size_group->addButton (icon_size_large);
    int icon_size = settings->value (global_icon_size.key, global_icon_size.def).toInt ();
    icon_size_normal->setChecked (true);  // the default
    icon_size_small->setChecked (icon_size < 0);
    icon_size_large->setChecked (icon_size > 0);
    cb_system_icon_theme->setChecked (settings->value (global_icon_theme.key, global_icon_theme.def).toBool ());

    // which icon has to be selected
    QButtonGroup *icon_group = new QButtonGroup (this);
    icon_group->addButton (general_icon_octave);
    icon_group->addButton (general_icon_graphic);
    icon_group->addButton (general_icon_letter);
    QString widget_icon_set =
      settings->value ("DockWidgets/widget_icon_set", "NONE").toString ();
    general_icon_octave->setChecked (true);  // the default (if invalid set)
    general_icon_octave->setChecked (widget_icon_set == "NONE");
    general_icon_graphic->setChecked (widget_icon_set == "GRAPHIC");
    general_icon_letter->setChecked (widget_icon_set == "LETTER");

    // custom title bar of dock widget
    QVariant default_var = QColor (255, 255, 255);
    QColor bg_color = settings->value ("DockWidgets/title_bg_color",
                                       default_var).value<QColor> ();
    m_widget_title_bg_color = new color_picker (bg_color);
    m_widget_title_bg_color->setEnabled (false);
    layout_widget_bgtitle->addWidget (m_widget_title_bg_color, 0);

    connect (cb_widget_custom_style, SIGNAL (toggled (bool)),
             m_widget_title_bg_color, SLOT (setEnabled (bool)));

    default_var = QColor (192, 192, 192);
    QColor bg_color_active = settings->value ("DockWidgets/title_bg_color_active",
                                              default_var).value<QColor> ();
    m_widget_title_bg_color_active = new color_picker (bg_color_active);
    m_widget_title_bg_color_active->setEnabled (false);
    layout_widget_bgtitle_active->addWidget (m_widget_title_bg_color_active, 0);

    connect (cb_widget_custom_style, SIGNAL (toggled (bool)),
             m_widget_title_bg_color_active, SLOT (setEnabled (bool)));

    default_var = QColor (0, 0, 0);
    QColor fg_color = settings->value ("DockWidgets/title_fg_color",
                                       default_var).value<QColor> ();
    m_widget_title_fg_color = new color_picker (fg_color);
    m_widget_title_fg_color->setEnabled (false);
    layout_widget_fgtitle->addWidget (m_widget_title_fg_color, 0);

    connect (cb_widget_custom_style, SIGNAL (toggled (bool)),
             m_widget_title_fg_color, SLOT (setEnabled (bool)));

    default_var = QColor (0, 0, 0);
    QColor fg_color_active = settings->value ("DockWidgets/title_fg_color_active",
                                              default_var).value<QColor> ();
    m_widget_title_fg_color_active = new color_picker (fg_color_active);
    m_widget_title_fg_color_active->setEnabled (false);
    layout_widget_fgtitle_active->addWidget (m_widget_title_fg_color_active, 0);

    connect (cb_widget_custom_style, SIGNAL (toggled (bool)),
             m_widget_title_fg_color_active, SLOT (setEnabled (bool)));

    sb_3d_title->setValue (settings->value ("DockWidgets/widget_title_3d", 50).toInt ());
    cb_widget_custom_style->setChecked (settings->value ("DockWidgets/widget_title_custom_style", false).toBool ());

    // Native file dialogs.
    // FIXME: This preference can be deprecated / removed if all display
    //       managers, especially KDE, run those dialogs without hangs or
    //       delays from the start (bug #54607).
    cb_use_native_file_dialogs->setChecked (settings->value ("use_native_file_dialogs", true).toBool ());

    // Cursor blinking: consider old terminal related setting if not yet set
    // FIXME: This pref. can be deprecated / removed if Qt adds support for
    //       getting the cursor blink preferences from all OS environments
    if (settings->contains ("cursor_blinking"))
      {
        // Preference exists, read its value
        cb_cursor_blinking->setChecked (settings->value ("cursor_blinking", true).toBool ());
      }
    else
      {
        // Pref. does not exist, so take old terminal related pref.
        cb_cursor_blinking->setChecked (settings->value ("terminal/cursorBlinking", true).toBool ());
      }

    // prompt on exit
    cb_prompt_to_exit->setChecked (settings->value ("prompt_to_exit", false).toBool ());

    // Main status bar
    cb_status_bar->setChecked (settings->value ("show_status_bar", true).toBool ());

    // Octave startup
    cb_restore_octave_dir->setChecked (settings->value ("restore_octave_dir", false).toBool ());
    le_octave_dir->setText (settings->value ("octave_startup_dir").toString ());

    connect (pb_octave_dir, SIGNAL (pressed (void)),
             this, SLOT (get_octave_dir (void)));

    //
    // editor
    //
    useCustomFileEditor->setChecked (settings->value ("useCustomFileEditor", false).toBool ());
    customFileEditor->setText (settings->value ("customFileEditor").toString ());
    editor_showLineNumbers->setChecked (settings->value ("editor/showLineNumbers", true).toBool ());
    editor_linenr_size->setValue (settings->value ("editor/line_numbers_size", 0).toInt ());

    resource_manager::combo_encoding (editor_combo_encoding);

    default_var = QColor (240, 240, 240);
    QColor setting_color = settings->value ("editor/highlight_current_line_color", default_var).value<QColor> ();
    m_editor_current_line_color = new color_picker (setting_color);
    editor_grid_current_line->addWidget (m_editor_current_line_color, 0, 3);
    m_editor_current_line_color->setMinimumSize (20, 10);
    m_editor_current_line_color->setEnabled (false);

    connect (editor_highlightCurrentLine, SIGNAL (toggled (bool)),
             m_editor_current_line_color, SLOT (setEnabled (bool)));

    editor_highlightCurrentLine->setChecked (settings->value ("editor/highlightCurrentLine", true).toBool ());
    editor_long_line_marker->setChecked (settings->value ("editor/long_line_marker", true).toBool ());
    bool long_line =
      settings->value ("editor/long_line_marker_line", true).toBool ();
    editor_long_line_marker_line->setChecked (long_line);
    bool long_back =
      settings->value ("editor/long_line_marker_background", false).toBool ();
    editor_long_line_marker_background->setChecked (long_back);
    if (! (long_line || long_back))
      editor_long_line_marker_line->setChecked (true);
    editor_long_line_column->setValue (settings->value ("editor/long_line_column", 80).toInt ());
    editor_break_checkbox->setChecked (settings->value ("editor/break_lines", false).toBool ());
    editor_break_checkbox->setChecked (settings->value ("editor/break_lines_comments", false).toBool ());
    editor_wrap_checkbox->setChecked (settings->value ("editor/wrap_lines", false).toBool ());
    cb_edit_status_bar->setChecked (settings->value ("editor/show_edit_status_bar", true).toBool ());
    cb_edit_tool_bar->setChecked (settings->value ("editor/show_toolbar", true).toBool ());
    cb_code_folding->setChecked (settings->value ("editor/code_folding", true).toBool ());
    editor_highlight_all_occurrences->setChecked (settings->value ("editor/highlight_all_occurrences", true).toBool ());

    editor_auto_endif->setCurrentIndex (settings->value ("editor/auto_endif", 1).toInt () );
    editor_codeCompletion->setChecked (settings->value ("editor/codeCompletion", true).toBool ());
    editor_spinbox_ac_threshold->setValue (settings->value ("editor/codeCompletion_threshold", 2).toInt ());
    editor_checkbox_ac_keywords->setChecked (settings->value ("editor/codeCompletion_keywords", true).toBool ());
    editor_checkbox_ac_builtins->setEnabled (editor_checkbox_ac_keywords->isChecked ());
    editor_checkbox_ac_functions->setEnabled (editor_checkbox_ac_keywords->isChecked ());
    editor_checkbox_ac_builtins->setChecked (settings->value ("editor/codeCompletion_octave_builtins", true).toBool ());
    editor_checkbox_ac_functions->setChecked (settings->value ("editor/codeCompletion_octave_functions", true).toBool ());
    editor_checkbox_ac_document->setChecked (settings->value ("editor/codeCompletion_document", false).toBool ());
    editor_checkbox_ac_case->setChecked (settings->value ("editor/codeCompletion_case", true).toBool ());
    editor_checkbox_ac_replace->setChecked (settings->value ("editor/codeCompletion_replace", false).toBool ());
    editor_ws_checkbox->setChecked (settings->value ("editor/show_white_space", false).toBool ());
    editor_ws_indent_checkbox->setChecked (settings->value ("editor/show_white_space_indent", false).toBool ());
    cb_show_eol->setChecked (settings->value ("editor/show_eol_chars", false).toBool ());
    cb_show_hscrollbar->setChecked (settings->value ("editor/show_hscroll_bar", true).toBool ());

    int selected_comment_string, selected_uncomment_string;

    if (settings->contains (ed_comment_str.key))   // new version (radio buttons)
      selected_comment_string = settings->value (ed_comment_str.key,
                                                 ed_comment_str.def).toInt ();
    else                                         // old version (combo box)
      selected_comment_string = settings->value (ed_comment_str_old.key,
                                                 ed_comment_str.def).toInt ();

    selected_uncomment_string = settings->value (ed_uncomment_str.key,
                                                 ed_uncomment_str.def).toInt ();

    for (int i = 0; i < ed_comment_strings_count; i++)
      {
        m_rb_comment_strings[i] = new QRadioButton ();
        m_rb_uncomment_strings[i] = new QRadioButton ();

        connect (m_rb_comment_strings[i], SIGNAL (clicked (bool)),
                 m_rb_uncomment_strings[i], SLOT (setChecked (bool)));
        connect (m_rb_comment_strings[i], SIGNAL (toggled (bool)),
                 m_rb_uncomment_strings[i], SLOT (setDisabled (bool)));

        m_rb_comment_strings[i]->setText (ed_comment_strings.at(i));
        m_rb_comment_strings[i]->setChecked (i == selected_comment_string);
        layout_comment_strings->addWidget (m_rb_comment_strings[i]);

        m_rb_uncomment_strings[i]->setText (ed_comment_strings.at(i));
        m_rb_uncomment_strings[i]->setAutoExclusive (false);
        m_rb_uncomment_strings[i]->setChecked ( 1 << i & selected_uncomment_string);
        layout_uncomment_strings->addWidget (m_rb_uncomment_strings[i]);
      }


#if defined (HAVE_QSCINTILLA)
#  if defined (Q_OS_WIN32)
    int eol_mode = QsciScintilla::EolWindows;
#elif defined (Q_OS_MAC)
    int eol_mode = QsciScintilla::EolMac;
#else
    int eol_mode = QsciScintilla::EolUnix;
#endif
#else
    int eol_mode = 2;
#endif
    combo_eol_mode->setCurrentIndex (settings->value ("editor/default_eol_mode", eol_mode).toInt ());
    editor_auto_ind_checkbox->setChecked (settings->value ("editor/auto_indent", true).toBool ());
    editor_tab_ind_checkbox->setChecked (settings->value ("editor/tab_indents_line", false).toBool ());
    editor_bs_unind_checkbox->setChecked (settings->value ("editor/backspace_unindents_line", false).toBool ());
    editor_ind_guides_checkbox->setChecked (settings->value ("editor/show_indent_guides", false).toBool ());
    editor_ind_width_spinbox->setValue (settings->value ("editor/indent_width", 2).toInt ());
    editor_ind_uses_tabs_checkbox->setChecked (settings->value ("editor/indent_uses_tabs", false).toBool ());
    editor_tab_width_spinbox->setValue (settings->value ("editor/tab_width", 2).toInt ());
    editor_longWindowTitle->setChecked (settings->value ("editor/longWindowTitle", false).toBool ());
    editor_notebook_tab_width_min->setValue (settings->value ("editor/notebook_tab_width_min", 160).toInt ());
    editor_notebook_tab_width_max->setValue (settings->value ("editor/notebook_tab_width_max", 300).toInt ());
    editor_restoreSession->setChecked (settings->value ("editor/restoreSession", true).toBool ());
    editor_create_new_file->setChecked (settings->value ("editor/create_new_file", false).toBool ());
    editor_reload_changed_files->setChecked (settings->value ("editor/always_reload_changed_files", false).toBool ());
    editor_hiding_closes_files->setChecked (settings->value ("editor/hiding_closes_files", false).toBool ());
    editor_show_dbg_file->setChecked (settings->value (ed_show_dbg_file.key, ed_show_dbg_file.def).toBool ());

    // terminal
    QString default_font = settings->value (global_mono_font.key, global_mono_font.def).toString ();
    terminal_fontName->setCurrentFont (QFont (settings->value (cs_font.key, default_font).toString ()));
    terminal_fontSize->setValue (settings->value ("terminal/fontSize", 10).toInt ());
    terminal_history_buffer->setValue (settings->value ("terminal/history_buffer", 1000).toInt ());
    terminal_cursorUseForegroundColor->setChecked (settings->value ("terminal/cursorUseForegroundColor", true).toBool ());
    terminal_focus_command->setChecked (settings->value ("terminal/focus_after_command", false).toBool ());
    terminal_print_dbg_location->setChecked (settings->value ("terminal/print_debug_location", false).toBool ());

    QString cursorType
      = settings->value ("terminal/cursorType", "ibeam").toString ();

    QStringList items;
    items << QString ("0") << QString ("1") << QString ("2");
    terminal_cursorType->addItems (items);
    terminal_cursorType->setItemText (0, tr ("IBeam Cursor"));
    terminal_cursorType->setItemText (1, tr ("Block Cursor"));
    terminal_cursorType->setItemText (2, tr ("Underline Cursor"));

    if (cursorType == "ibeam")
      terminal_cursorType->setCurrentIndex (0);
    else if (cursorType == "block")
      terminal_cursorType->setCurrentIndex (1);
    else if (cursorType == "underline")
      terminal_cursorType->setCurrentIndex (2);

    // file browser
    connect (sync_octave_directory, SIGNAL (toggled (bool)),
             this, SLOT (set_disabled_pref_file_browser_dir (bool)));

    sync_octave_directory->setChecked (settings->value (fb_sync_octdir.key, fb_sync_octdir.def).toBool ());
    cb_restore_file_browser_dir->setChecked (settings->value (fb_restore_last_dir.key, fb_restore_last_dir.def).toBool ());
    le_file_browser_dir->setText (settings->value (fb_startup_dir.key).toString ());

    connect (pb_file_browser_dir, SIGNAL (pressed (void)),
             this, SLOT (get_file_browser_dir (void)));

    le_file_browser_extensions->setText (settings->value (fb_txt_file_ext.key, fb_txt_file_ext.def).toString ());

    checkbox_allow_web_connect->setChecked (settings->value ("news/allow_web_connection", false).toBool ());
    useProxyServer->setChecked (settings->value ("useProxyServer", false).toBool ());
    proxyHostName->setText (settings->value ("proxyHostName").toString ());

    int currentIndex = 0;
    QString proxyTypeString = settings->value ("proxyType").toString ();
    while ((currentIndex < proxyType->count ())
           && (proxyType->currentText () != proxyTypeString))
      {
        currentIndex++;
        proxyType->setCurrentIndex (currentIndex);
      }

    proxyPort->setText (settings->value ("proxyPort").toString ());
    proxyUserName->setText (settings->value ("proxyUserName").toString ());
    proxyPassword->setText (settings->value ("proxyPassword").toString ());

    // Workspace
    read_workspace_colors (settings);

    // terminal colors
    read_terminal_colors (settings);

    // variable editor
    varedit_columnWidth->setValue (settings->value ("variable_editor/column_width", 100).toInt ());
    varedit_autoFitColumnWidth->setChecked (settings->value ("variable_editor/autofit_column_width", false).toBool ());
    varedit_autofitType->setCurrentIndex (settings->value ("autofit_type", 0).toInt ());
    varedit_rowHeight->setValue (settings->value ("variable_editor/row_height", 10).toInt ());
    varedit_rowAutofit->setChecked (settings->value ("variable_editor/autofit_row_height", true).toBool ());

    varedit_font->setCurrentFont (QFont (settings->value ("variable_editor/font_name", settings->value (cs_font.key, default_font)).toString ()));
    varedit_fontSize->setValue (settings->value ("variable_editor/font_size", QVariant (10)).toInt ());
    connect (varedit_useTerminalFont, SIGNAL (toggled (bool)),
             varedit_font, SLOT (setDisabled (bool)));
    connect (varedit_useTerminalFont, SIGNAL (toggled (bool)),
             varedit_fontSize, SLOT (setDisabled (bool)));
    varedit_useTerminalFont->setChecked (settings->value ("variable_editor/use_terminal_font", false).toBool ());
    varedit_font->setDisabled (varedit_useTerminalFont->isChecked ());
    varedit_fontSize->setDisabled (varedit_useTerminalFont->isChecked ());

    varedit_alternate->setChecked (settings->value ("variable_editor/alternate_rows", QVariant (false)).toBool ());

    // variable editor colors
    read_varedit_colors (settings);

    // shortcuts

    cb_prevent_readline_conflicts->setChecked (settings->value ("shortcuts/prevent_readline_conflicts", true).toBool ());

    // initialize the tree view with all shortcut data
    shortcut_manager::fill_treewidget (shortcuts_treewidget);

    // connect the buttons for import/export of the shortcut sets
    connect (btn_import_shortcut_set, SIGNAL (clicked (void)),
             this, SLOT (import_shortcut_set (void)));

    connect (btn_export_shortcut_set, SIGNAL (clicked (void)),
             this, SLOT (export_shortcut_set (void)));

    connect (btn_default_shortcut_set, SIGNAL (clicked (void)),
             this, SLOT (default_shortcut_set (void)));

#if defined (HAVE_QSCINTILLA)

    // editor styles: create lexer, read settings, and create dialog elements
    QsciLexer *lexer;

#if defined (HAVE_LEXER_OCTAVE)

    lexer = new QsciLexerOctave ();
    read_lexer_settings (lexer, settings);
    delete lexer;

#elif defined (HAVE_LEXER_MATLAB)

    lexer = new QsciLexerMatlab ();
    read_lexer_settings (lexer, settings);
    delete lexer;

#endif

    lexer = new QsciLexerCPP ();
    read_lexer_settings (lexer, settings);
    delete lexer;

    lexer = new QsciLexerPerl ();
    read_lexer_settings (lexer, settings);
    delete lexer;

    lexer = new QsciLexerBatch ();
    read_lexer_settings (lexer, settings);
    delete lexer;

    lexer = new QsciLexerDiff ();
    read_lexer_settings (lexer, settings);
    delete lexer;

    lexer = new QsciLexerBash ();
    read_lexer_settings (lexer, settings);
    delete lexer;

    lexer = new octave::octave_txt_lexer ();
    read_lexer_settings (lexer, settings);
    delete lexer;

#endif

    // which tab is the desired one?
    show_tab (desired_tab);

    // connect button box signal
    connect (button_box, SIGNAL (clicked (QAbstractButton *)),
             this, SLOT (button_clicked (QAbstractButton *)));

    // restore last geometry
    if (settings->contains ("settings/geometry"))
      restoreGeometry (settings->value ("settings/geometry").toByteArray ());
    else
      setGeometry (QRect (10,50,1000,600));
  }

  void settings_dialog::show_tab (const QString& tab)
  {
    if (tab.isEmpty ())
      {
        QSettings *settings = resource_manager::get_settings ();
        if (settings)
          tabWidget->setCurrentIndex (settings->value ("settings/last_tab", 0).toInt ());
      }
    else
      {
        QHash <QString, QWidget*> tab_hash;
        tab_hash["editor"] = tab_editor;
        tab_hash["editor_styles"] = tab_editor;
        tabWidget->setCurrentIndex (tabWidget->indexOf (tab_hash.value (tab)));
        if (tab == "editor_styles")
          tab_editor_scroll_area->ensureWidgetVisible (group_box_editor_styles);
      }
  }

  void settings_dialog::get_octave_dir (void)
  {
    get_dir (le_octave_dir, tr ("Set Octave Startup Directory"));
  }

  void settings_dialog::get_file_browser_dir (void)
  {
    get_dir (le_file_browser_dir, tr ("Set File Browser Startup Directory"));
  }

  void settings_dialog::get_dir (QLineEdit *line_edit, const QString& title)
  {
    // FIXME: Remove, if for all common KDE versions (bug #54607) is resolved.
    int opts = QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks;
    if (! resource_manager::get_settings ()->value ("use_native_file_dialogs",
                                                    true).toBool ())
      opts |= QFileDialog::DontUseNativeDialog;

    QString dir = QFileDialog::getExistingDirectory
      (this, title, line_edit->text (), QFileDialog::Option (opts));

    line_edit->setText (dir);
  }

  void settings_dialog::button_clicked (QAbstractButton *button)
  {
    QDialogButtonBox::ButtonRole button_role = button_box->buttonRole (button);

    if (button_role == QDialogButtonBox::ApplyRole
        || button_role == QDialogButtonBox::AcceptRole)
      {
        write_changed_settings (button_role == QDialogButtonBox::AcceptRole);
        emit apply_new_settings ();
      }

    if (button_role == QDialogButtonBox::RejectRole
        || button_role == QDialogButtonBox::AcceptRole)
      close ();
  }

  void settings_dialog::set_disabled_pref_file_browser_dir (bool disable)
  {
    cb_restore_file_browser_dir->setDisabled (disable);

    if (! disable)
      {
        le_file_browser_dir->setDisabled (cb_restore_file_browser_dir->isChecked ());
        pb_file_browser_dir->setDisabled (cb_restore_file_browser_dir->isChecked ());
      }
    else
      {
        le_file_browser_dir->setDisabled (disable);
        pb_file_browser_dir->setDisabled (disable);
      }
  }

  // slots for import/export of shortcut sets

  void settings_dialog::import_shortcut_set (void)
  {
    shortcut_manager::import_export (shortcut_manager::OSC_IMPORT);
  }

  void settings_dialog::export_shortcut_set (void)
  {
    shortcut_manager::import_export (shortcut_manager::OSC_EXPORT);
  }

  void settings_dialog::default_shortcut_set (void)
  {
    shortcut_manager::import_export (shortcut_manager::OSC_DEFAULT);
  }

  void settings_dialog::read_lexer_settings (QsciLexer *lexer,
                                             QSettings *settings)
  {
#if defined (HAVE_QSCINTILLA)

    lexer->readSettings (*settings);
    int styles[MaxLexerStyles];  // array for saving valid styles
    // (enum is not continuous)
    int max_style = get_valid_lexer_styles (lexer, styles);
    QGridLayout *style_grid = new QGridLayout ();
    QVector<QLabel*> description (max_style);
    QVector<QFontComboBox*> select_font (max_style);
    QVector<QSpinBox*> font_size (max_style);
    QVector<QCheckBox*> attrib_font (3 * max_style);
    QVector<color_picker*> color (max_style);
    QVector<color_picker*> bg_color (max_style);
    int default_size = 10;
    QFont default_font = QFont ();
    int label_width;
    QColor default_color = QColor ();
    QColor dummy_color = QColor (255, 0, 255);

    for (int i = 0; i < max_style; i++)  // create dialog elements for all styles
      {
        QString actual_name = lexer->description (styles[i]);
        QFont   actual_font = lexer->font (styles[i]);
        description[i] = new QLabel (actual_name);
        description[i]->setWordWrap (true);
        label_width = 24*description[i]->fontMetrics ().averageCharWidth ();
        description[i]->setMaximumSize (label_width, QWIDGETSIZE_MAX);
        description[i]->setMinimumSize (label_width, 1);
        select_font[i] = new QFontComboBox ();
        select_font[i]->setObjectName (actual_name + "_font");
        select_font[i]->setMaximumSize (label_width, QWIDGETSIZE_MAX);
        select_font[i]->setMinimumSize (label_width, 1);
        font_size[i] = new QSpinBox ();
        font_size[i]->setObjectName (actual_name + "_size");
        if (styles[i] == 0) // the default
          {
            select_font[i]->setCurrentFont (actual_font);
            default_font = actual_font;
            font_size[i]->setRange (6, 24);
            default_size = actual_font.pointSize ();
            font_size[i]->setValue (default_size);
            default_color = lexer->defaultPaper ();
            bg_color[i] = new color_picker (default_color);
          }
        else   // other styles
          {
            select_font[i]->setCurrentFont (actual_font);
            if (actual_font.family () == default_font.family ())
              select_font[i]->setEditText (lexer->description (0));
            font_size[i]->setRange (-4, 4);
            font_size[i]->setValue (actual_font.pointSize ()-default_size);
            font_size[i]->setToolTip (QObject::tr ("Difference to the default size"));
            if (lexer->paper (styles[i]) == default_color)
              bg_color[i] = new color_picker (dummy_color);
            else
              bg_color[i] = new color_picker (lexer->paper (styles[i]));
            bg_color[i]->setToolTip
              (QObject::tr ("Background color, pink (255, 0, 255) means default"));
          }
        attrib_font[0+3*i] = new QCheckBox (QObject::tr ("b", "short form for bold"));
        attrib_font[1+3*i] = new QCheckBox (QObject::tr ("i", "short form for italic"));
        attrib_font[2+3*i] = new QCheckBox (QObject::tr ("u", "short form for underlined"));
        attrib_font[0+3*i]->setChecked (actual_font.bold ());
        attrib_font[0+3*i]->setObjectName (actual_name + "_bold");
        attrib_font[1+3*i]->setChecked (actual_font.italic ());
        attrib_font[1+3*i]->setObjectName (actual_name + "_italic");
        attrib_font[2+3*i]->setChecked (actual_font.underline ());
        attrib_font[2+3*i]->setObjectName (actual_name + "_underline");
        color[i] = new color_picker (lexer->color (styles[i]));
        color[i]->setObjectName (actual_name + "_color");
        bg_color[i]->setObjectName (actual_name + "_bg_color");
        int column = 1;
        style_grid->addWidget (description[i], i, column++);
        style_grid->addWidget (select_font[i], i, column++);
        style_grid->addWidget (font_size[i], i, column++);
        style_grid->addWidget (attrib_font[0+3*i], i, column++);
        style_grid->addWidget (attrib_font[1+3*i], i, column++);
        style_grid->addWidget (attrib_font[2+3*i], i, column++);
        style_grid->addWidget (color[i], i, column++);
        style_grid->addWidget (bg_color[i], i, column++);
      }
    // place grid with elements into the tab
    QScrollArea *scroll_area = new QScrollArea ();
    QWidget *scroll_area_contents = new QWidget ();
    scroll_area_contents->setObjectName (QString (lexer->language ()) + "_styles");
    scroll_area_contents->setLayout (style_grid);
    scroll_area->setWidget (scroll_area_contents);
    tabs_editor_lexers->addTab (scroll_area, lexer->language ());

    tabs_editor_lexers->setCurrentIndex (settings->value ("settings/last_editor_styles_tab", 0).toInt ());

#else

    octave_unused_parameter (lexer);
    octave_unused_parameter (settings);

#endif
  }

  void settings_dialog::write_lexer_settings (QsciLexer *lexer,
                                              QSettings *settings)
  {
#if defined (HAVE_QSCINTILLA)

    QWidget *tab = tabs_editor_lexers->
      findChild <QWidget *> (QString (lexer->language ()) + "_styles");
    int styles[MaxLexerStyles];  // array for saving valid styles
    // (enum is not continuous)
    int max_style = get_valid_lexer_styles (lexer, styles);
    QFontComboBox *select_font;
    QSpinBox *font_size;
    QCheckBox *attrib_font[3];
    color_picker *color;
    color_picker *bg_color;
    int default_size = 10;

    QString default_font_name
      = settings->value (global_mono_font.key, global_mono_font.def).toString ();
    QFont default_font = QFont (default_font_name, 10, -1, 0);
    QColor default_color = QColor ();
    QColor dummy_color = QColor (255, 0, 255);

    for (int i = 0; i < max_style; i++)  // get dialog elements and their contents
      {
        QString actual_name = lexer->description (styles[i]);
        select_font = tab->findChild <QFontComboBox *> (actual_name + "_font");
        font_size = tab->findChild <QSpinBox *> (actual_name + "_size");
        attrib_font[0] = tab->findChild <QCheckBox *> (actual_name + "_bold");
        attrib_font[1] = tab->findChild <QCheckBox *> (actual_name + "_italic");
        attrib_font[2] = tab->findChild <QCheckBox *> (actual_name + "_underline");
        color = tab->findChild <color_picker *> (actual_name + "_color");
        bg_color = tab->findChild <color_picker *> (actual_name + "_bg_color");
        QFont new_font = default_font;
        if (select_font)
          {
            new_font = select_font->currentFont ();
            if (styles[i] == 0)
              default_font = new_font;
            else if (select_font->currentText () == lexer->description (0))
              new_font = default_font;
          }
        if (font_size)
          {
            if (styles[i] == 0)
              {
                default_size = font_size->value ();
                new_font.setPointSize (font_size->value ());
              }
            else
              new_font.setPointSize (font_size->value ()+default_size);
          }
        if (attrib_font[0])
          new_font.setBold (attrib_font[0]->isChecked ());
        if (attrib_font[1])
          new_font.setItalic (attrib_font[1]->isChecked ());
        if (attrib_font[2])
          new_font.setUnderline (attrib_font[2]->isChecked ());
        lexer->setFont (new_font, styles[i]);
        if (styles[i] == 0)
          lexer->setDefaultFont (new_font);
        if (color)
          lexer->setColor (color->color (), styles[i]);
        if (bg_color)
          {
            if (styles[i] == 0)
              {
                default_color = bg_color->color ();
                lexer->setPaper (default_color, styles[i]);
                lexer->setDefaultPaper (default_color);
              }
            else
              {
                if (bg_color->color () == dummy_color)
                  lexer->setPaper (default_color, styles[i]);
                else
                  lexer->setPaper (bg_color->color (), styles[i]);
              }
          }
      }

    lexer->writeSettings (*settings);

    settings->setValue ("settings/last_editor_styles_tab",
                        tabs_editor_lexers->currentIndex ());
    settings->sync ();

#else

    octave_unused_parameter (lexer);
    octave_unused_parameter (settings);

#endif
  }

  void settings_dialog::write_changed_settings (bool closing)
  {
    QSettings *settings = resource_manager::get_settings ();

    // the icon set
    QString widget_icon_set = "NONE";
    if (general_icon_letter->isChecked ())
      widget_icon_set = "LETTER";
    else if (general_icon_graphic->isChecked ())
      widget_icon_set = "GRAPHIC";
    settings->setValue ("DockWidgets/widget_icon_set", widget_icon_set);

    // language
    QString language = comboBox_language->currentText ();
    if (language == tr ("System setting"))
      language = "SYSTEM";
    settings->setValue ("language", language);

    // style
    QString selected_style = combo_styles->currentText ();
    if (selected_style == global_style.def.toString ())
      selected_style = global_style.def.toString ();
    settings->setValue (global_style.key, selected_style);

    // dock widget title bar
    settings->setValue ("DockWidgets/widget_title_custom_style", cb_widget_custom_style->isChecked ());
    settings->setValue ("DockWidgets/widget_title_3d", sb_3d_title->value ());
    settings->setValue ("DockWidgets/title_bg_color", m_widget_title_bg_color->color ());
    settings->setValue ("DockWidgets/title_bg_color_active", m_widget_title_bg_color_active->color ());
    settings->setValue ("DockWidgets/title_fg_color", m_widget_title_fg_color->color ());
    settings->setValue ("DockWidgets/title_fg_color_active", m_widget_title_fg_color_active->color ());

    // icon size and theme
    int icon_size = icon_size_large->isChecked () - icon_size_small->isChecked ();
    settings->setValue (global_icon_size.key, icon_size);
    settings->setValue (global_icon_theme.key, cb_system_icon_theme->isChecked ());

    // native file dialogs
    settings->setValue ("use_native_file_dialogs", cb_use_native_file_dialogs->isChecked ());

    // cursor blinking
    settings->setValue ("cursor_blinking", cb_cursor_blinking->isChecked ());

    // promp to exit
    settings->setValue ("prompt_to_exit", cb_prompt_to_exit->isChecked ());

    // status bar
    settings->setValue ("show_status_bar", cb_status_bar->isChecked ());

    // Octave startup
    settings->setValue ("restore_octave_dir", cb_restore_octave_dir->isChecked ());
    settings->setValue ("octave_startup_dir", le_octave_dir->text ());

    //editor
    settings->setValue ("useCustomFileEditor", useCustomFileEditor->isChecked ());
    settings->setValue ("customFileEditor", customFileEditor->text ());
    settings->setValue ("editor/showLineNumbers", editor_showLineNumbers->isChecked ());
    settings->setValue ("editor/line_numbers_size", editor_linenr_size->value ());
    settings->setValue ("editor/highlightCurrentLine", editor_highlightCurrentLine->isChecked ());
    settings->setValue ("editor/highlight_current_line_color", m_editor_current_line_color->color ());
    settings->setValue ("editor/long_line_marker", editor_long_line_marker->isChecked ());
    settings->setValue ("editor/long_line_marker_line", editor_long_line_marker_line->isChecked ());
    settings->setValue ("editor/long_line_marker_background", editor_long_line_marker_background->isChecked ());
    settings->setValue ("editor/long_line_column", editor_long_line_column->value ());
    settings->setValue ("editor/break_lines", editor_break_checkbox->isChecked ());
    settings->setValue ("editor/break_lines_comments", editor_break_comments_checkbox->isChecked ());
    settings->setValue ("editor/wrap_lines", editor_wrap_checkbox->isChecked ());
    settings->setValue ("editor/code_folding", cb_code_folding->isChecked ());
    settings->setValue ("editor/show_edit_status_bar", cb_edit_status_bar->isChecked ());
    settings->setValue ("editor/show_toolbar", cb_edit_tool_bar->isChecked ());
    settings->setValue ("editor/highlight_all_occurrences", editor_highlight_all_occurrences->isChecked ());
    settings->setValue ("editor/codeCompletion", editor_codeCompletion->isChecked ());
    settings->setValue ("editor/codeCompletion_threshold", editor_spinbox_ac_threshold->value ());
    settings->setValue ("editor/codeCompletion_keywords", editor_checkbox_ac_keywords->isChecked ());
    settings->setValue ("editor/codeCompletion_octave_builtins", editor_checkbox_ac_builtins->isChecked ());
    settings->setValue ("editor/codeCompletion_octave_functions", editor_checkbox_ac_functions->isChecked ());
    settings->setValue ("editor/codeCompletion_document", editor_checkbox_ac_document->isChecked ());
    settings->setValue ("editor/codeCompletion_case", editor_checkbox_ac_case->isChecked ());
    settings->setValue ("editor/codeCompletion_replace", editor_checkbox_ac_replace->isChecked ());
    settings->setValue ("editor/auto_endif", editor_auto_endif->currentIndex ());
    settings->setValue ("editor/show_white_space", editor_ws_checkbox->isChecked ());
    settings->setValue ("editor/show_white_space_indent", editor_ws_indent_checkbox->isChecked ());
    settings->setValue ("editor/show_eol_chars", cb_show_eol->isChecked ());
    settings->setValue ("editor/show_hscroll_bar", cb_show_hscrollbar->isChecked ());
    settings->setValue ("editor/default_eol_mode", combo_eol_mode->currentIndex ());

    // Comment strings
    int rb_uncomment = 0;
    for (int i = 0; i < ed_comment_strings_count; i++)
      {
        if (m_rb_comment_strings[i]->isChecked ())
          {
            settings->setValue (ed_comment_str.key, i);
            if (i < 3)
              settings->setValue (ed_comment_str_old.key, i);
            else
              settings->setValue (ed_comment_str_old.key, ed_comment_str.def);
          }
        if (m_rb_uncomment_strings[i]->isChecked ())
          rb_uncomment = rb_uncomment + (1 << i);
      }
    settings->setValue (ed_uncomment_str.key, rb_uncomment);

    settings->setValue ("editor/default_encoding", editor_combo_encoding->currentText ());
    settings->setValue ("editor/auto_indent", editor_auto_ind_checkbox->isChecked ());
    settings->setValue ("editor/tab_indents_line", editor_tab_ind_checkbox->isChecked ());
    settings->setValue ("editor/backspace_unindents_line", editor_bs_unind_checkbox->isChecked ());
    settings->setValue ("editor/show_indent_guides", editor_ind_guides_checkbox->isChecked ());
    settings->setValue ("editor/indent_width", editor_ind_width_spinbox->value ());
    settings->setValue ("editor/indent_uses_tabs", editor_ind_uses_tabs_checkbox->isChecked ());
    settings->setValue ("editor/tab_width", editor_tab_width_spinbox->value ());
    settings->setValue ("editor/longWindowTitle", editor_longWindowTitle->isChecked ());
    settings->setValue ("editor/notebook_tab_width_min", editor_notebook_tab_width_min->value ());
    settings->setValue ("editor/notebook_tab_width_max", editor_notebook_tab_width_max->value ());
    settings->setValue ("editor/restoreSession", editor_restoreSession->isChecked ());
    settings->setValue ("editor/create_new_file", editor_create_new_file->isChecked ());
    settings->setValue ("editor/hiding_closes_files", editor_hiding_closes_files->isChecked ());
    settings->setValue ("editor/always_reload_changed_files", editor_reload_changed_files->isChecked ());
    settings->setValue (ed_show_dbg_file.key, editor_show_dbg_file->isChecked ());

    settings->setValue ("terminal/fontSize", terminal_fontSize->value ());
    settings->setValue ("terminal/fontName", terminal_fontName->currentFont ().family ());

    // file browser
    settings->setValue (fb_sync_octdir.key, sync_octave_directory->isChecked ());
    settings->setValue (fb_restore_last_dir.key, cb_restore_file_browser_dir->isChecked ());
    settings->setValue (fb_startup_dir.key, le_file_browser_dir->text ());
    settings->setValue (fb_txt_file_ext.key, le_file_browser_extensions->text ());

    settings->setValue ("news/allow_web_connection", checkbox_allow_web_connect->isChecked ());
    settings->setValue ("useProxyServer", useProxyServer->isChecked ());
    settings->setValue ("proxyType", proxyType->currentText ());
    settings->setValue ("proxyHostName", proxyHostName->text ());
    settings->setValue ("proxyPort", proxyPort->text ());
    settings->setValue ("proxyUserName", proxyUserName->text ());
    settings->setValue ("proxyPassword", proxyPassword->text ());
    settings->setValue ("terminal/cursorUseForegroundColor", terminal_cursorUseForegroundColor->isChecked ());
    settings->setValue ("terminal/focus_after_command", terminal_focus_command->isChecked ());
    settings->setValue ("terminal/print_debug_location", terminal_print_dbg_location->isChecked ());
    settings->setValue ("terminal/history_buffer", terminal_history_buffer->value ());

    // the cursor
    QString cursorType;
    switch (terminal_cursorType->currentIndex ())
      {
      case 0: cursorType = "ibeam"; break;
      case 1: cursorType = "block"; break;
      case 2: cursorType = "underline";  break;
      }
    settings->setValue ("terminal/cursorType", cursorType);

#if defined (HAVE_QSCINTILLA)
    // editor styles: create lexer, get dialog contents, and write settings
    QsciLexer *lexer;

#if defined (HAVE_LEXER_OCTAVE)

    lexer = new QsciLexerOctave ();
    write_lexer_settings (lexer, settings);
    delete lexer;

#elif defined (HAVE_LEXER_MATLAB)

    lexer = new QsciLexerMatlab ();
    write_lexer_settings (lexer, settings);
    delete lexer;

#endif

    lexer = new QsciLexerCPP ();
    write_lexer_settings (lexer, settings);
    delete lexer;

    lexer = new QsciLexerPerl ();
    write_lexer_settings (lexer, settings);
    delete lexer;

    lexer = new QsciLexerBatch ();
    write_lexer_settings (lexer, settings);
    delete lexer;

    lexer = new QsciLexerDiff ();
    write_lexer_settings (lexer, settings);
    delete lexer;

    lexer = new QsciLexerBash ();
    write_lexer_settings (lexer, settings);
    delete lexer;

    lexer = new octave::octave_txt_lexer ();
    write_lexer_settings (lexer, settings);
    delete lexer;

#endif

    // Workspace
    write_workspace_colors (settings);

    // Terminal
    write_terminal_colors (settings);

    // Variable editor
    settings->setValue ("variable_editor/autofit_column_width", varedit_autoFitColumnWidth->isChecked ());
    settings->setValue ("variable_editor/autofit_type", varedit_autofitType->currentIndex ());
    settings->setValue ("variable_editor/column_width", varedit_columnWidth->value ());
    settings->setValue ("variable_editor/row_height", varedit_rowHeight->value ());
    settings->setValue ("variable_editor/autofit_row_height", varedit_rowAutofit->isChecked ());
    settings->setValue ("variable_editor/use_terminal_font", varedit_useTerminalFont->isChecked ());
    settings->setValue ("variable_editor/alternate_rows", varedit_alternate->isChecked ());
    settings->setValue ("variable_editor/font_name", varedit_font->currentFont ().family ());
    settings->setValue ("variable_editor/font_size", varedit_fontSize->value ());
    write_varedit_colors (settings);

    // shortcuts
    settings->setValue ("shortcuts/prevent_readline_conflicts", cb_prevent_readline_conflicts->isChecked ());
    shortcut_manager::write_shortcuts (settings, closing);

    // settings dialog's geometry
    settings->setValue ("settings/last_tab", tabWidget->currentIndex ());
    settings->setValue ("settings/geometry", saveGeometry ());

    settings->sync ();
  }

  void settings_dialog::read_workspace_colors (QSettings *settings)
  {
    // Construct the grid with all color related settings
    QList<QColor> default_colors =
      resource_manager::storage_class_default_colors ();
    QStringList class_names = resource_manager::storage_class_names ();
    QString class_chars = resource_manager::storage_class_chars ();
    int nr_of_classes = class_chars.length ();

    QGridLayout *style_grid = new QGridLayout ();
    QVector<QLabel*> description (nr_of_classes);
    QVector<color_picker*> color (nr_of_classes);

    int column = 0;
    int row = 0;

    m_ws_enable_colors = new QCheckBox (tr ("Enable attribute colors"));
    style_grid->addWidget (m_ws_enable_colors, row++, column, 1, 4);

    m_ws_hide_tool_tips = new QCheckBox (tr ("Hide tools tips"));
    style_grid->addWidget (m_ws_hide_tool_tips, row++, column, 1, 4);
    connect (m_ws_enable_colors, SIGNAL (toggled (bool)),
             m_ws_hide_tool_tips, SLOT(setEnabled (bool)));
    m_ws_hide_tool_tips->setChecked (
      settings->value (ws_hide_tool_tips.key, ws_hide_tool_tips.def).toBool ());

    for (int i = 0; i < nr_of_classes; i++)
      {
        description[i] = new QLabel ("    " + class_names.at (i));
        description[i]->setAlignment (Qt::AlignRight);
        connect (m_ws_enable_colors, SIGNAL (toggled (bool)),
                 description[i], SLOT(setEnabled (bool)));

        QVariant default_var = default_colors.at (i);
        QColor setting_color = settings->value ("workspaceview/color_" + class_chars.mid (i, 1), default_var).value<QColor> ();
        color[i] = new color_picker (setting_color);
        color[i]->setObjectName ("color_" + class_chars.mid (i, 1));
        color[i]->setMinimumSize (30, 10);
        connect (m_ws_enable_colors, SIGNAL (toggled (bool)),
                 color[i], SLOT(setEnabled (bool)));

        style_grid->addWidget (description[i], row, 3*column);
        style_grid->addWidget (color[i], row, 3*column+1);
        if (++column == 3)
          {
            style_grid->setColumnStretch (4*column, 10);
            row++;
            column = 0;
          }
      }

    // Load enable settings at the end for having signals already connected
    bool colors_enabled =
        settings->value (ws_enable_colors.key, ws_enable_colors.def).toBool ();
    m_ws_enable_colors->setChecked (colors_enabled);
    m_ws_hide_tool_tips->setEnabled (colors_enabled);

    // place grid with elements into the tab
    workspace_colors_box->setLayout (style_grid);
  }

  void settings_dialog::write_workspace_colors (QSettings *settings)
  {
    settings->setValue (ws_enable_colors.key, m_ws_enable_colors->isChecked ());
    settings->setValue (ws_hide_tool_tips.key, m_ws_hide_tool_tips->isChecked ());

    QString class_chars = resource_manager::storage_class_chars ();
    color_picker *color;

    for (int i = 0; i < class_chars.length (); i++)
      {
        color = workspace_colors_box->findChild <color_picker *> ("color_" + class_chars.mid (i, 1));
        if (color)
          settings->setValue ("workspaceview/color_" + class_chars.mid (i, 1), color->color ());
      }
    settings->sync ();
  }

  void settings_dialog::read_terminal_colors (QSettings *settings)
  {

    QList<QColor> default_colors = resource_manager::terminal_default_colors ();
    QStringList class_names = resource_manager::terminal_color_names ();
    QString class_chars = resource_manager::terminal_color_chars ();
    int nr_of_classes = class_chars.length ();

    QGridLayout *style_grid = new QGridLayout ();
    QVector<QLabel*> description (nr_of_classes);
    QVector<color_picker*> color (nr_of_classes);

    int column = 0;
    int row = 0;
    for (int i = 0; i < nr_of_classes; i++)
      {
        description[i] = new QLabel ("    " + class_names.at (i));
        description[i]->setAlignment (Qt::AlignRight);
        QVariant default_var = default_colors.at (i);
        QColor setting_color = settings->value ("terminal/color_" + class_chars.mid (i, 1), default_var).value<QColor> ();
        color[i] = new color_picker (setting_color);
        color[i]->setObjectName ("terminal_color_" + class_chars.mid (i, 1));
        color[i]->setMinimumSize (30, 10);
        style_grid->addWidget (description[i], row, 2*column);
        style_grid->addWidget (color[i], row, 2*column+1);
        if (++column == 2)
          {
            style_grid->setColumnStretch (3*column, 10);
            row++;
            column = 0;
          }
      }

    // place grid with elements into the tab
    terminal_colors_box->setLayout (style_grid);
  }

  void settings_dialog::write_terminal_colors (QSettings *settings)
  {
    QString class_chars = resource_manager::terminal_color_chars ();
    color_picker *color;

    for (int i = 0; i < class_chars.length (); i++)
      {
        color = terminal_colors_box->findChild <color_picker *> ("terminal_color_" + class_chars.mid (i, 1));
        if (color)
          settings->setValue ("terminal/color_" + class_chars.mid (i, 1), color->color ());
      }

    settings->sync ();
  }

  void settings_dialog::read_varedit_colors (QSettings *settings)
  {
    QList<QColor> default_colors = octave::variable_editor::default_colors ();
    QStringList class_names = octave::variable_editor::color_names ();
    QString class_chars = resource_manager::varedit_color_chars ();
    int nr_of_classes = class_chars.length ();

    QGridLayout *style_grid = new QGridLayout ();
    QVector<QLabel*> description (nr_of_classes);
    QVector<color_picker*> color (nr_of_classes);

    int column = 0;
    int row = 0;
    for (int i = 0; i < nr_of_classes; i++)
      {
        description[i] = new QLabel ("    " + class_names.at (i));
        description[i]->setAlignment (Qt::AlignRight);
        QVariant default_var = default_colors.at (i);
        QColor setting_color = settings->value ("variable_editor/color_" + class_chars.mid (i, 1), default_var).value<QColor> ();
        color[i] = new color_picker (setting_color);
        color[i]->setObjectName ("varedit_color_" + class_chars.mid (i, 1));
        color[i]->setMinimumSize (30, 10);
        style_grid->addWidget (description[i], row, 2*column);
        style_grid->addWidget (color[i], row, 2*column+1);
        if (++column == 2)
          {
            style_grid->setColumnStretch (3*column, 10);
            row++;
            column = 0;
          }
      }

    // place grid with elements into the tab
    varedit_colors_box->setLayout (style_grid);
  }

  void settings_dialog::write_varedit_colors (QSettings *settings)
  {
    QString class_chars = resource_manager::varedit_color_chars ();
    color_picker *color;

    for (int i = 0; i < class_chars.length (); i++)
      {
        color = varedit_colors_box->findChild <color_picker *> ("varedit_color_" + class_chars.mid (i, 1));
        if (color)
          settings->setValue ("variable_editor/color_" + class_chars.mid (i, 1), color->color ());
      }

    settings->sync ();
  }
}
