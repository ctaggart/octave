## Copyright (C) 1998-2011 Walter Gautschi
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
## @deftypefn  {Function File} {@var{q} =} quadl (@var{f}, @var{a}, @var{b})
## @deftypefnx {Function File} {@var{q} =} quadl (@var{f}, @var{a}, @var{b}, @var{tol})
## @deftypefnx {Function File} {@var{q} =} quadl (@var{f}, @var{a}, @var{b}, @var{tol}, @var{trace})
## @deftypefnx {Function File} {@var{q} =} quadl (@var{f}, @var{a}, @var{b}, @var{tol}, @var{trace}, @var{p1}, @var{p2}, @dots{})
##
## Numerically evaluate integral using adaptive Lobatto rule.
## @code{quadl (@var{f}, @var{a}, @var{b})} approximates the integral of
## @code{@var{f}(@var{x})} to machine precision.  @var{f} is either a
## function handle, inline function or string containing the name of
## the function to evaluate.  The function @var{f} must return a vector
## of output values if given a vector of input values.
##
## If defined, @var{tol} defines the relative tolerance to which to
## which to integrate @code{@var{f}(@var{x})}.  While if @var{trace} is
## defined, displays the left end point of the current interval, the 
## interval length, and the partial integral.
##
## Additional arguments @var{p1}, etc., are passed directly to @var{f}.
## To use default values for @var{tol} and @var{trace}, one may pass
## empty matrices.
##
## Reference: W. Gander and W. Gautschi, @cite{Adaptive Quadrature - 
## Revisited}, BIT Vol. 40, No. 1, March 2000, pp. 84--101.
## @url{http://www.inf.ethz.ch/personal/gander/}
##
## @end deftypefn

##   Author: Walter Gautschi
##   Date: 08/03/98
##   Reference: Gander, Computermathematik, Birkhaeuser, 1992.

## 2003-08-05 Shai Ayal
##   * permission from author to release as GPL
## 2004-02-10 Paul Kienzle
##   * renamed to quadl for compatibility
##   * replace global variable terminate2 with local function need_warning
##   * add paper ref to docs

function Q = quadl (f, a, b, tol, trace, varargin)
  need_warning (1);
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
    tol = myeps; 
  endif
  if (isempty (trace))
    trace = 0; 
  endif
  if (tol < myeps)
    tol = myeps;
  endif

  m = (a+b)/2; 
  h = (b-a)/2;
  alpha = sqrt(2/3); 
  beta = 1/sqrt(5);

  x1 = .942882415695480; 
  x2 = .641853342345781;
  x3 = .236383199662150;

  x = [a, m-x1*h, m-alpha*h, m-x2*h, m-beta*h, m-x3*h, m, m+x3*h, ...
       m+beta*h, m+x2*h, m+alpha*h, m+x1*h, b];

  y = feval (f, x, varargin{:});

  fa = y(1); 
  fb = y(13);

  i2 = (h/6)*(y(1) + y(13) + 5*(y(5)+y(9)));

  i1 = (h/1470)*(77*(y(1)+y(13))
                 + 432*(y(3)+y(11))
                 + 625*(y(5)+y(9))
                 + 672*y(7));

  is = h*(.0158271919734802*(y(1)+y(13))
          +.0942738402188500*(y(2)+y(12))
          + .155071987336585*(y(3)+y(11))
          + .188821573960182*(y(4)+y(10))
          + .199773405226859*(y(5)+y(9))
          + .224926465333340*(y(6)+y(8))
          + .242611071901408*y(7));

  s = sign(is); 

  if (s == 0)
    s = 1;
  endif
  erri1 = abs(i1-is);
  erri2 = abs(i2-is);
  R = 1; 
  if (erri2 != 0)
    R = erri1/erri2; 
  endif
  if (R > 0 && R < 1)
    tol = tol/R; 
  endif
  is = s*abs(is)*tol/myeps;
  if (is == 0)
    is = b-a;
  endif
  Q = adaptlobstp (f, a, b, fa, fb, is, trace, varargin{:});
endfunction

## ADAPTLOBSTP  Recursive function used by QUADL.
##
##   Q = ADAPTLOBSTP('F', A, B, FA, FB, IS, TRACE) tries to
##   approximate the integral of F(X) from A to B to
##   an appropriate relative error. The argument 'F' is
##   a string containing the name of f.  The remaining
##   arguments are generated by ADAPTLOB or by recursion.
##
##   Walter Gautschi, 08/03/98

function Q = adaptlobstp (f, a, b, fa, fb, is, trace, varargin)
  h = (b-a)/2; 
  m = (a+b)/2;
  alpha = sqrt(2/3); 
  beta = 1/sqrt(5);
  mll = m-alpha*h; 
  ml = m-beta*h; 
  mr = m+beta*h; 
  mrr = m+alpha*h;
  x = [mll, ml, m, mr, mrr];
  y = feval(f, x, varargin{:});
  fmll = y(1); 
  fml = y(2); 
  fm = y(3); 
  fmr = y(4); 
  fmrr = y(5);
  i2 = (h/6)*(fa + fb + 5*(fml+fmr));
  i1 = (h/1470)*(77*(fa+fb) + 432*(fmll+fmrr) + 625*(fml+fmr) + 672*fm);
  if (is+(i1-i2) == is || mll <= a || b <= mrr)
    if ((m <= a || b <= m) && need_warning ())
      warning ("quadl: interval contains no more machine number");
      warning ("quadl: required tolerance may not be met");
      need_warning (0);
    endif
    Q = i1;
    if (trace)
      disp ([a, b-a, Q]);
    endif
  else
    Q = (adaptlobstp (f, a, mll, fa, fmll, is, trace, varargin{:})
         + adaptlobstp (f, mll, ml, fmll, fml, is, trace, varargin{:})
         + adaptlobstp (f, ml, m, fml, fm, is, trace, varargin{:})
         + adaptlobstp (f, m, mr, fm, fmr, is, trace, varargin{:})
         + adaptlobstp (f, mr, mrr, fmr, fmrr, is, trace, varargin{:})
         + adaptlobstp (f, mrr, b, fmrr, fb, is, trace, varargin{:}));
  endif
endfunction

function r = need_warning (v)
  persistent w = [];
  if (nargin == 0)
    r = w;
  else 
    w = v; 
  endif
endfunction
