## Copyright (C) 2017 John W. Eaton
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn {} {} disp (@var{player})
## Display the properties of the audioplayer object @var{player}.
## @end deftypefn

function disp (player)

  if (nargin != 1)
    print_usage ();
  endif

  printf ("audioplayer object with properties:\n\n");
  for [val, prop] = __get_properties__ (player)
    printf ("  %s = ", prop), disp (val);
  endfor
  ## FIXME: display in pr-output.cc should add "\n", not this function. 
  printf ("\n");

endfunction
