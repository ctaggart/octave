## Copyright (C) 2008-2011 David Bateman
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or (at
## your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn  {Function File} {@var{q} =} quadv (@var{f}, @var{a}, @var{b})
## @deftypefnx {Function File} {@var{q} =} quadv (@var{f}, @var{a}, @var{b}, @var{tol})
## @deftypefnx {Function File} {@var{q} =} quadv (@var{f}, @var{a}, @var{b}, @var{tol}, @var{trace})
## @deftypefnx {Function File} {@var{q} =} quadv (@var{f}, @var{a}, @var{b}, @var{tol}, @var{trace}, @var{p1}, @var{p2}, @dots{})
## @deftypefnx {Function File} {[@var{q}, @var{fcnt}] =} quadv (@dots{})
##
## Numerically evaluate the integral of @var{f} from @var{a} to @var{b}
## using adaptive Simpson's rule.
## @var{f} is either a function handle, inline function or string
## containing the name of the function to evaluate.
## The function defined by @var{f} may be a scalar, vector or array-valued.
##
## If a value for @var{tol} is given, it defines the tolerance used to stop
## the adaptation procedure, otherwise the default value of 1e-6 is used.
##
## The algorithm used by @code{quadv}, involves recursively subdividing the
## integration interval and applying Simpson's rule on each sub-interval.
## If @var{trace} is @var{true}, after computing each of these partial
## integrals, display the total number of function evaluations, the left end
## of the sub-interval, the length of the sub-interval and the approximation
## of the integral over the sub-interval.
##
## Additional arguments @var{p1}, etc., are passed directly to @var{f}.
## To use default values for @var{tol} and @var{trace}, one may pass
## empty matrices.
##
##@seealso{quad, quadl, quadgk, quadcc, trapz, dblquad, triplequad}
## @end deftypefn

function [q, fcnt] = quadv (f, a, b, tol, trace, varargin)
  if (nargin < 3)
    print_usage ();
  endif
  if (nargin < 4)
    tol = [];
  endif
  if (nargin < 5)
    trace = [];
  endif
  if (isa (a, "single") || isa (b, "single"))
    myeps = eps ("single");
  else
    myeps = eps;
  endif
  if (isempty (tol))
    tol = 1e-6;
  endif
  if (isempty (trace))
    trace = 0;
  endif

  ## Split the interval into 3 abscissa, and apply a 3 point Simpson's rule
  c = (a + b) / 2;
  fa = feval (f, a, varargin{:});
  fc = feval (f, c, varargin{:});
  fb = feval (f, b, varargin{:});
  fcnt = 3;

  ## If have edge singularities, move edge point by eps*(b-a) as
  ## discussed in Shampine paper used to implement quadgk
  if (isinf (fa))
    fa = feval (f, a + myeps * (b-a), varargin{:});
  endif
  if (isinf (fb))
    fb = feval (f, b - myeps * (b-a), varargin{:});
  endif

  h = (b - a);
  q = (b - a) / 6 * (fa + 4 * fc + fb);

  [q, fcnt, hmin] = simpsonstp (f, a, b, c, fa, fb, fc, q, fcnt, abs (h),
                                tol, trace, varargin{:});

  if (fcnt > 10000)
    warning ("maximum iteration count reached");
  elseif (isnan (q) || isinf (q))
    warning ("infinite or NaN function evaluations were returned");
  elseif (hmin < (b - a) * myeps)
    warning ("minimum step size reached -- possibly singular integral");
  endif
endfunction

function [q, fcnt, hmin] = simpsonstp (f, a, b, c, fa, fb, fc, q0,
                                       fcnt, hmin, tol, trace, varargin)
  if (fcnt > 10000)
    q = q0;
  else
    d = (a + c) / 2;
    e = (c + b) / 2;
    fd = feval (f, d, varargin{:});
    fe = feval (f, e, varargin{:});
    fcnt += 2;
    q1 = (c - a) / 6 * (fa + 4 * fd + fc);
    q2 = (b - c) / 6 * (fc + 4 * fe + fb);
    q = q1 + q2;

    if (abs(a -  c) < hmin)
      hmin = abs (a - c);
    endif

    if (trace)
      disp ([fcnt, a, b-a, q]);
    endif

    ## Force at least one adpative step.
    if (fcnt == 5 || abs (q - q0) > tol)
      [q1, fcnt, hmin] = simpsonstp (f, a, c, d, fa, fc, fd, q1, fcnt, hmin,
                                    tol, trace, varargin{:});
      [q2, fcnt, hmin] = simpsonstp (f, c, b, e, fc, fb, fe, q2, fcnt, hmin,
                                     tol, trace, varargin{:});
      q = q1 + q2;
    endif
  endif
endfunction

%!assert (quadv (@sin, 0, 2 * pi), 0, 1e-5)
%!assert (quadv (@sin, 0, pi), 2, 1e-5)

%% Handles weak singularities at the edge
%!assert (quadv (@(x) 1 ./ sqrt(x), 0, 1), 2, 1e-5)

%% Handles vector-valued functions
%!assert (quadv (@(x) [(sin (x)), (sin (2 * x))], 0, pi), [2, 0], 1e-5)

