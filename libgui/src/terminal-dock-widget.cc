/*

Copyright (C) 2013-2019 John W. Eaton
Copyright (C) 2011-2019 Jacob Dawid

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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <QDesktopWidget>

#include "gui-preferences-cs.h"
#include "gui-preferences-global.h"
#include "resource-manager.h"
#include "terminal-dock-widget.h"

#include "quit.h"
#include "signal-wrappers.h"

#include "sighandlers.h"

namespace octave
{
  terminal_dock_widget::terminal_dock_widget (QWidget *p)
    : octave_dock_widget ("TerminalDockWidget", p),
      m_terminal (QTerminal::create (p))
  {
    m_terminal->setObjectName ("OctaveTerminal");
    m_terminal->setFocusPolicy (Qt::StrongFocus);

    setWindowIcon (QIcon (":/actions/icons/logo.png"));
    set_title (tr ("Command Window"));

    setWidget (m_terminal);
    setFocusProxy (m_terminal);

    connect (m_terminal, SIGNAL (interrupt_signal (void)),
             this, SLOT (terminal_interrupt (void)));

    // Connect the visibility signal to the terminal for dis-/enabling timers
    connect (this, SIGNAL (visibilityChanged (bool)),
             m_terminal, SLOT (handle_visibility_changed (bool)));

    // Chose a reasonable size at startup in order to avoid truncated
    // startup messages
    gui_settings *settings = resource_manager::get_settings ();

    QFont font = QFont ();
    font.setStyleHint (QFont::TypeWriter);
    QString default_font = settings->value (global_mono_font.key, global_mono_font.def).toString ();
    font.setFamily
      (settings->value (cs_font.key, default_font).toString ());
    font.setPointSize
      (settings->value (cs_font_size.key, cs_font_size.def).toInt ());

    QFontMetrics metrics(font);

    int win_x =  metrics.maxWidth()*80;
    int win_y =  metrics.height()*25;

    int max_x = QApplication::desktop ()->screenGeometry (this).width ();
    int max_y = QApplication::desktop ()->screenGeometry (this).height ();

    if (win_x > max_x)
      win_x = max_x;
    if (win_y > max_y)
      win_y = max_y;

    setGeometry (0, 0, win_x, win_y);
  }

  terminal_dock_widget::~terminal_dock_widget (void)
  {
    delete m_terminal;
  }

  bool terminal_dock_widget::has_focus (void) const
  {
    QWidget *w = widget ();

    return w->hasFocus ();
  }

  void terminal_dock_widget::terminal_interrupt (void)
  {
    // FIXME: Protect with mutex?

    octave_signal_caught = 1;
    octave_interrupt_state++;

    // Send SIGINT to all other processes in our process group.
    // This is needed to interrupt calls to system (), for example.

    int sigint;
    octave_get_sig_number ("SIGINT", &sigint);

    octave_kill_wrapper (0, sigint);
  }
}
