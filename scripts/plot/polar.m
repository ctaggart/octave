# Copyright (C) 1993 John W. Eaton
# 
# This file is part of Octave.
# 
# Octave is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any
# later version.
# 
# Octave is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Octave; see the file COPYING.  If not, write to the Free
# Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

function polar (x1, x2)

# usage: polar (theta, rho)
#
# Make a 2D plot given polar the coordinates theta and rho.
#
# See also: plot, semilogx, semilogy, loglog, mesh, contour, bar,
#           stairs, gplot, gsplot, replot, xlabel, ylabel, title 

  set nologscale;
  set nopolar;

  if (nargin == 1)
    polar_int (x1);
  elseif (nargin == 2)
    polar_int (x1, x2);
  else
    usage = sprintf ("usage: polar (x)\n");
    usage = sprintf ("%s       polar (x, y)", usage);
    error (usage);
  endif

endfunction
