/*

Copyright (C) 2013 John W. Eaton
Copyright (C) 2011-2012 Jacob Dawid

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#if !defined (main_window_h)
#define main_window_h 1

// Qt includes
#include <QtGui/QMainWindow>
#include <QThread>
#include <QTabWidget>
#include <QMdiArea>
#include <QStatusBar>
#include <QToolBar>
#include <QQueue>
#include <QMdiSubWindow>
#include <QCloseEvent>
#include <QToolButton>
#include <QComboBox>

// Editor includes
#include "file-editor-interface.h"

// QTerminal includes
#include "QTerminal.h"

// Own includes
#include "resource-manager.h"
#include "workspace-model.h"
#include "workspace-view.h"
#include "history-dock-widget.h"
#include "files-dock-widget.h"
#include "terminal-dock-widget.h"
#include "documentation-dock-widget.h"
#include "octave-qt-link.h"

/**
 * \class MainWindow
 *
 * Represents the main window.
 */
class main_window : public QMainWindow
{
  Q_OBJECT

public:

  main_window (QWidget *parent = 0);

  ~main_window (void);

  void focus_command_window (void);

signals:
  void settings_changed (const QSettings *);
  void relay_command_signal (const QString&);
  void new_file_signal (const QString&);
  void open_file_signal (const QString&);

public slots:
  void report_status_message (const QString& statusMessage);
  void handle_save_workspace_request (void);
  void handle_load_workspace_request (void);
  void handle_clear_workspace_request (void);
  void handle_clear_history_request (void);
  void new_file (const QString& commands = QString ());
  void open_file (const QString& file_name = QString ());
  void open_online_documentation_page (void);
  void open_bug_tracker_page (void);
  void open_octave_forge_page (void);
  void open_agora_page (void);
  void process_settings_dialog_request (void);
  void show_about_octave (void);
  void notice_settings (const QSettings *settings);
  void prepare_to_exit (void);
  void exit (void);
  void reset_windows (void);

  void change_directory (const QString& dir);
  void browse_for_directory (void);
  void set_current_working_directory (const QString& dir);
  void change_directory_up (void);
  void accept_directory_line_edit (void);

  void handle_command_double_clicked (const QString& command);

  void handle_enter_debugger (void);
  void handle_exit_debugger (void);
  void debug_continue (void);
  void debug_step_into (void);
  void debug_step_over (void);
  void debug_step_out (void);
  void debug_quit (void);

  void read_settings (void);
  void write_settings (void);
  void connect_visibility_changed (void);

protected:
  void closeEvent (QCloseEvent * closeEvent);

private:

  void construct (void);

  void construct_octave_qt_link (void);

  void construct_menu_bar (void);
  void construct_file_menu (QMenuBar *p);
  void construct_new_menu (QMenu *p);
  void construct_edit_menu (QMenuBar *p);
  void construct_debug_menu_item (QMenu *p, const QString& item,
                                  const QKeySequence& key);
  QAction *construct_debug_menu_item (const char *icon_file,
                                      const QString& item,
                                      const QKeySequence& key);
  void construct_debug_menu (QMenuBar *p);
  void construct_desktop_menu (QMenuBar *p);
  QAction *construct_window_menu_item (QMenu *p, const QString& item,
                                       bool checkable,
                                       const QKeySequence& key);
  void construct_window_menu (QMenuBar *p);
  void construct_help_menu (QMenuBar *p);
  void construct_documentation_menu (QMenu *p);

  void construct_tool_bar (void);

  void establish_octave_link (void);

  void save_workspace_callback (const std::string& file);

  void load_workspace_callback (const std::string& file);

  void clear_workspace_callback (void);

  void clear_history_callback (void);

  void change_directory_callback (const std::string& directory);

  void debug_continue_callback (void);

  void debug_step_into_callback (void);

  void debug_step_over_callback (void);

  void debug_step_out_callback (void);

  void debug_quit_callback (void);

  void exit_callback (void);

  // Data models.
  workspace_model *_workspace_model;

  // Toolbars.
  QStatusBar *status_bar;

  // Subwindows.
  terminal_dock_widget *command_window;
  history_dock_widget *history_window;
  files_dock_widget *file_browser_window;
  documentation_dock_widget *doc_browser_window;
  file_editor_interface *editor_window;
  workspace_view *workspace_window;

  QMenu *_debug_menu;

  QAction *_debug_continue;
  QAction *_debug_step_into;
  QAction *_debug_step_over;
  QAction *_debug_step_out;
  QAction *_debug_quit;

  QAction *_new_script_action;
  QAction *_open_action;

  QAction *_cut_action;
  QAction *_copy_action;
  QAction *_paste_action;
  QAction *_undo_action;
  QAction *_redo_action;

  // Toolbars.
  QComboBox *_current_directory_combo_box;
  static const int current_directory_width = 300;
  static const int current_directory_max_visible = 16;
  static const int current_directory_max_count = 16;
  QLineEdit *_current_directory_line_edit;

  octave_qt_link *_octave_qt_link;

  // Flag for closing whole application.
  bool _closing;
};

#endif // MAINWINDOW_H
