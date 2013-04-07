/*

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "documentation-dockwidget.h"

documentation_dock_widget::documentation_dock_widget (QWidget *p)
  : octave_dock_widget (p)
{
  setObjectName ("DocumentationDockWidget");
  setWindowIcon (QIcon (":/actions/icons/logo.png"));
  setWindowTitle (tr ("Documentation"));
  setStatusTip (tr ("See the documentation for help."));

  _webinfo = new webinfo (this);
  setWidget (_webinfo);
}

void
documentation_dock_widget::connect_visibility_changed (void)
{
  connect (this, SIGNAL (visibilityChanged (bool)),
           this, SLOT (handle_visibility (bool)));
}

void
documentation_dock_widget::focus (void)
{
  if (! isVisible ())
    setVisible (true);

  setFocus ();
  activateWindow ();
  raise ();
}

void
documentation_dock_widget::handle_visibility (bool visible)
{
  if (visible && ! isFloating ())
    focus ();
}
