## Copyright (C) 2019 Nicholas R. Jankowski
##
## This file is part of Octave.
##
## Octave is free software: you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <https://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn {} {@var{T} =} rotx (@var{angle})
##
## @code{rotx} returns the 3x3 transformation matrix corresponding to an active
## rotation of the vector about the x-axis by the specified @var{angle}, given
## in degrees, where a positive angle corresponds to a counter-clockwise
## rotation when viewing the y-z plane from the positive x side.
##
## The form of the transformation matrix is:
## @tex
## $$
## T = \left[\matrix{ 1 & 0 & 0 \cr
##                    0 & \cos(angle) & -\sin(angle)\cr
##                    0 & \sin(angle) & \cos(angle)}\right].
## $$
## @end tex
## @ifnottex
##
## @example
## @group
##      | 1      0           0      |
##  T = | 0  cos(@var{angle}) -sin(@var{angle}) |
##      | 0  sin(@var{angle})  cos(@var{angle}) |
## @end group
## @end example
## @end ifnottex
##
## This rotation matrix is intended to be used as a left-multiplyig matrix
## when acting on a column vector, using the notation @var{v} = @var{T}@var{u}.
## For example, a vector, @var{u}, pointing along the positive y-axis, rotated
## 90-degrees about the x-axis, will result in a vector pointing along the
## positive z-axis:
##
## @example
## @group
## >> u = [0 1 0]'
## u =
##    0
##    1
##    0
##
## >> T = rotx (90)
## T =
##    1.00000   0.00000   0.00000
##    0.00000   0.00000  -1.00000
##    0.00000   1.00000   0.00000
##
## >> v = T*u
## v =
##    0.00000
##    0.00000
##    1.00000
## @end group
## @end example
##
## @seealso{roty, rotz}
## @end deftypefn

## Author: Nicholas Jankowski <jankowskin@asme.org>
## Created: 2017-04-09

function retmat = rotx (angle_in_deg)

  if ((nargin != 1) || ! isscalar (angle_in_deg))
    print_usage ();
  endif

  angle_in_rad = angle_in_deg * pi / 180;

  s = sin (angle_in_rad);
  c = cos (angle_in_rad);

  retmat = [1 0 0; 0 c -s; 0 s c];

endfunction

## Function output tests
%!assert (rotx (0), [1 0 0; 0 1 0; 0 0 1]);
%!assert (rotx (45), [1, 0, 0; [0; 0],[(sqrt(2)/2).*[1 -1; 1 1]]], 1e-12);
%!assert (rotx (90), [1 0 0; 0 0 -1; 0 1 0], 1e-12);
%!assert (rotx (180), [1 0 0; 0 -1 0; 0 0 -1], 1e-12);

## Test input validation
%!error rotx ()
%!error rotx (1, 2)
%!error rotx ([1 2 3])
