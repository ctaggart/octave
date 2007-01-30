## Copyright (C) 1996, 1997 John W. Eaton
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, write to the Free
## Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
## 02110-1301, USA.

## -*- texinfo -*-
## @deftypefn {Function File} {} semilogx (@var{args})
## Make a two-dimensional plot using a log scale for the @var{x} axis.  See
## the description of @code{plot} for a description of the arguments
## that @code{semilogx} will accept.
## @seealso{plot, semilogy, loglog, polar, mesh, contour, bar, stairs,
## replot, xlabel, ylabel, title}
## @end deftypefn

## Author: jwe

function semilogx (varargin)

  newplot ();

  ## [h, varargin] = __plt_get_axis_arg__ ("semilogx", varargin{:});
  h = gca ();

  set (h, "xscale", "log");

  __plt__ ("semilogx", h, varargin{:});

endfunction
