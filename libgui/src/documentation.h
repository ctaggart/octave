/*

Copyright (C) 2018 Torsten <mttl@maibox.org>

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

#if ! defined (octave_documentation_h)
#define octave_documentation_h 1

#include <QComboBox>
#include <QWidget>
#include <QSettings>
#include <QShortcut>
#include <QSplitter>
#include <QTextBrowser>
#include <QtHelp/QHelpEngine>

namespace octave
{
  //! Documentation browser derived from Textbrowser

  class documentation_browser : public QTextBrowser
  {
    Q_OBJECT

  public:

    documentation_browser (QHelpEngine *help_engine, QWidget *parent = nullptr);
    ~documentation_browser (void);

    virtual QVariant loadResource (int type, const QUrl &url);

  public slots:

    void handle_index_clicked (const QUrl& url,
                               const QString& keyword = QString ());
    void notice_settings (const QSettings *settings);

    //! Zooming in and out while taking care of the zoom level
    //!@{
    void zoom_in (void);
    void zoom_out (void);
    void zoom_normal (void);
    //!@}

  private:

    QHelpEngine *m_help_engine;

    //! Store the current zoom level
    int m_zoom_level;

    //! Minimal and maximal zoom level avoiding calling
    //! zoom_in and zoom_out without actually zooming but
    //! with changing the stored zoom level
    enum
    {
      min_zoom_level = -5,
      max_zoom_level = 10
    };
  };


  //! The documentaiton main class derived from QSplitter

  class documentation : public QSplitter
  {
    Q_OBJECT

  public:

    documentation (QWidget *parent = nullptr);
    ~documentation (void);

  public slots:

    void notice_settings (const QSettings *settings);

    void copyClipboard (void);
    void pasteClipboard (void);
    void selectAll (void);

    void load_ref (const QString & name);
    void registerDoc (const QString & name);
    void unregisterDoc (const QString & name);

  private slots:

    void global_search (void);
    void global_search_started (void);
    void global_search_finished (int hits);
    void filter_update (const QString& expression);
    void filter_update_history (void);
    void find_forward (void);
    void find_backward (void);
    void find_forward_from_anchor (const QString& text);
    void record_anchor_position (void);
    void handle_cursor_position_change (void);

  signals:

    void show_single_result (const QUrl);

  private:

    QHelpEngine *m_help_engine;
    QString m_internal_search;
    documentation_browser *m_doc_browser;
    QLineEdit *m_find_line_edit;
    int m_search_anchor_position;
    QComboBox *m_filter;
    QString m_collection;

    QShortcut *m_show_shortcut;
    QShortcut *m_findnext_shortcut;
    QShortcut *m_findprev_shortcut;
    QShortcut *m_zoom_in_shortcut;
    QShortcut *m_zoom_out_shortcut;
    QShortcut *m_zoom_normal_shortcut;
  };
}

#endif
