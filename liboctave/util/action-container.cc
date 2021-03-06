/*

Copyright (C) 1993-2019 John W. Eaton
Copyright (C) 2009-2010 VZLU Prague

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

#include "action-container.h"
#include "cmd-edit.h"

namespace octave
{
  void action_container::run (size_t num)
  {
    if (num > size ())
      num = size ();

    for (size_t i = 0; i < num; i++)
      {
        run_first ();

        // If event_loop_interrupted is TRUE, a user callback event has
        // requested that we break out of the readline event handler to
        // process a command or other action.

        if (command_editor::event_loop_interrupted ())
          {
            command_editor::interrupt_event_loop (false);
            break;
          }
      }
  }
}
