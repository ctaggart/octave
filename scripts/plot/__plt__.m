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
## Software Foundation, 59 Temple Place - Suite 330, Boston, MA
## 02111-1307, USA.

## -*- texinfo -*-
## @deftypefn {Function File} {} __plt__ (@code{caller}, @dots{})
## @end deftypefn

## Author: jwe

function __plt__ (caller, varargin)

  nargs = nargin ();

  if (nargs >= 2)

    k = 1;
    j = 1;
    x = varargin{k++};
    nargs -= 2;
    x_set = 1;
    y_set = 0;
    gp_cmd = "gplot";
    have_gp_cmd = false;

    ## Gather arguments, decode format, gather plot strings, and plot lines.

    while (nargs-- > 0)

      fmt = "";
      new = varargin{k++};

      if (j > 1)
	sep = ",\\\n";
      else
	sep = "";
      endif

      if (isstr (new))
	if (! x_set)
	  error ("plot: no data to plot");
	endif
	fmt = __pltopt__ (caller, new);
	if (! y_set)
	  [data{j}, fmtstr] = __plt1__ (x, fmt);
	else
	  [data{j}, fmtstr] = __plt2__ (x, y, fmt);
	endif
	if (iscell (data{j}))
	  for i = 1:length (data{j})
	    gp_cmd = sprintf ("%s%s data{%d}{%d} %s", gp_cmd, sep,
			      j, i, fmtstr{i});
	    sep = ",\\\n";
	    have_gp_cmd = true;
	  endfor
	else
	  gp_cmd = sprintf ("%s%s data{%d} %s", gp_cmd, sep, j++, fmtstr);
	  have_gp_cmd = true;
	endif
	x_set = 0;
	y_set = 0;
      elseif (x_set)
	if (y_set)
	  [data{j}, fmtstr] = __plt2__ (x, y, fmt);
	  if (iscell (data{j}))
	    for i = 1:length (data{j})
	      gp_cmd = sprintf ("%s%s data{%d}{%d} %s", gp_cmd, sep,
				j, i, fmtstr{i});
	      sep = ",\\\n";
	      have_gp_cmd = true;
	    endfor
	  else
	    gp_cmd = sprintf ("%s%s data{%d} %s", gp_cmd, sep, j++, fmtstr);
	    have_gp_cmd = true;
	  endif
	  x = new;
	  y_set = 0;
	else
	  y = new;
	  y_set = 1;
	endif
      else
	x = new;
	x_set = 1;
      endif

    endwhile

    ## Handle last plot.

    if (x_set)
      if (y_set)
	[data{j}, fmtstr] = __plt2__ (x, y, fmt);
      else
	[data{j}, fmtstr] = __plt1__ (x, fmt);
      endif
      if (iscell (data{j}))
	for i = 1:length (data{j})
	  gp_cmd = sprintf ("%s%s data{%d}{%d} %s", gp_cmd, sep,
			    j, i, fmtstr{i});
	  sep = ",\\\n";
	  have_gp_cmd = true;
	endfor
      else
	gp_cmd = sprintf ("%s%s data{%d} %s", gp_cmd, sep, j++, fmtstr);
	have_gp_cmd = true;
      endif
    endif

    if (have_gp_cmd)
      eval (gp_cmd);
    endif
  else
    msg = sprintf ("%s (x)\n", caller);
    msg = sprintf ("%s       %s (x, y)\n", msg, caller);
    msg = sprintf ("%s       %s (x2, y1, x2, y2)\n", msg, caller);
    msg = sprintf ("%s       %s (x, y, fmt)", msg, caller);
    usage (msg);
  endif

endfunction
