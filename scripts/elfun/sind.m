## Copyright (C) 2006 David Bateman <dbateman@free.fr>
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
## @deftypefn {Function File} {} sind (@var{x})
## Compute the sine of each element of @var{x}.  Returns zero in elements
## for which @code{@var{x}/180} is an integer.
## @seealso{sin, cosd, tand, acosd, asind, atand}
## @end deftypefn

function y = sind (x)
  if (nargin != 1)
    print_usage ();
  endif
  I = x / 180;
  y = sin (I .* pi);
  y(I == round (I) & finite (I)) = 0;
endfunction

%!error(sind())
%!error(sind(1,2))
%!assert(sind(10:10:90),sin(pi*[10:10:90]/180),-10*eps)
%!assert(sind([0,180,360]) == 0)
%!assert(sind([90,270]) != 0)
