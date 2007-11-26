## Copyright (C) 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2004, 2005
##                2006, 2007 John W. Eaton
## Copyright (C) 2007 Ben Abbott
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
## @deftypefn {Function File} {[@var{r}, @var{p}, @var{k}, @var{e}] =} residue (@var{b}, @var{a})
## Compute the partial fraction expansion for the quotient of the
## polynomials, @var{b} and @var{a}.
##
## @iftex
## @tex
## $$
## {B(s)\over A(s)} = \sum_{m=1}^M {r_m\over (s-p_m)^e_m}
##   + \sum_{i=1}^N k_i s^{N-i}.
## $$
## @end tex
## @end iftex
## @ifinfo
##
## @example
##  B(s)    M       r(m)         N
##  ---- = SUM -------------  + SUM k(i)*s^(N-i)
##  A(s)   m=1 (s-p(m))^e(m)    i=1
## @end example
## @end ifinfo
##
## @noindent
## where @math{M} is the number of poles (the length of the @var{r},
## @var{p}, and @var{e}), the @var{k} vector is a polynomial of order @math{N-1}
## representing the direct contribution, and the @var{e} vector specifies
## the multiplicity of the mth residue's pole.
##
## For example,
##
## @example
## @group
## b = [1, 1, 1];
## a = [1, -5, 8, -4];
## [r, p, k, e] = residue (b, a);
##      @result{} r = [-2; 7; 3]
##      @result{} p = [2; 2; 1]
##      @result{} k = [](0x0)
##      @result{} e = [1; 2; 1]
## @end group
## @end example
##
## @noindent
## which represents the following partial fraction expansion
## @iftex
## @tex
## $$
## {s^2+s+1\over s^3-5s^2+8s-4} = {-2\over s-2} + {7\over (s-2)^2} + {3\over s-1}
## $$
## @end tex
## @end iftex
## @ifinfo
##
## @example
##         s^2 + s + 1       -2        7        3
##    ------------------- = ----- + ------- + -----
##    s^3 - 5s^2 + 8s - 4   (s-2)   (s-2)^2   (s-1)
## @end example
##
## @end ifinfo
##
## @deftypefnx {Function File} {[@var{b}, @var{a}] =} residue (@var{r}, @var{p}, @var{k})
## @deftypefnx {Function File} {[@var{b}, @var{a}] =} residue (@var{r}, @var{p}, @var{k}, @var{e})
## Compute the reconstituted quotient of polynomials,
## @var{b}(s)/@var{a}(s), from the partial fraction expansion
## represented by the residues, poles, and a direct polynomial specified
## by @var{r}, @var{p} and @var{k}, and the pole multiplicity @var{e}.
##
## If the multiplicity, @var{e}, is not explicitly specified the multiplicity is
## determined by the script mpoles.m.
##
## For example,
##
## @example
## @group
## r = [-2; 7; 3];
## p = [2; 2; 1];
## k = [1, 0];
## [b, a] = residue (r, p, k);
##      @result{} b = [1, -5, 9, -3, 1]
##      @result{} a = [1, -5, 8, -4]
##
## where mpoles.m is used to determine e = [1; 2; 1]
##
## @end group
## @end example
##
## Alternatively the multiplicity may be defined explicitly, for example,
##
## @example
## @group
## r = [7; 3; -2];
## p = [2; 1; 2];
## k = [1, 0];
## e = [2; 1; 1];
## [b, a] = residue (r, p, k, e);
##      @result{} b = [1, -5, 9, -3, 1]
##      @result{} a = [1, -5, 8, -4]
## @end group
## @end example
##
## @noindent
## which represents the following partial fraction expansion
## @iftex
## @tex
## $$
## {-2\over s-2} + {7\over (s-2)^2} + {3\over s-1} + s = {s^4-5s^3+9s^2-3s+1\over s^3-5s^2+8s-4}
## $$
## @end tex
## @end iftex
## @ifinfo
##
## @example
##     -2        7        3         s^4 - 5s^3 + 9s^2 - 3s + 1
##    ----- + ------- + ----- + s = --------------------------
##    (s-2)   (s-2)^2   (s-1)          s^3 - 5s^2 + 8s - 4
## @end example
## @end ifinfo
## @seealso{poly, roots, conv, deconv, mpoles, polyval, polyderiv, polyinteg}
## @end deftypefn

## Author: Tony Richardson <arichard@stark.cc.oh.us>
## Author: Ben Abbott <bpabbott@mac.com>
## Created: June 1994
## Adapted-By: jwe

function [r, p, k, e] = residue (b, a, varargin)

  if (nargin < 2 || nargin > 4)
    print_usage ();
  endif

  toler = .001;

  if (nargin >= 3)
    if (nargin >= 4)
      e = varargin{2};
    else
      e = [];
    endif
    ## The inputs are the residue, pole, and direct part. Solve for the
    ## corresponding numerator and denominator polynomials
    [r, p] = rresidue (b, a, varargin{1}, toler, e);
    return
  endif

  ## Make sure both polynomials are in reduced form.

  a = polyreduce (a);
  b = polyreduce (b);

  b = b / a(1);
  a = a / a(1);

  la = length (a);
  lb = length (b);

  ## Handle special cases here.

  if (la == 0 || lb == 0)
    k = r = p = e = [];
    return;
  elseif (la == 1)
    k = b / a;
    r = p = e = [];
    return;
  endif

  ## Find the poles.

  p = roots (a);
  lp = length (p);

  ## Determine if the poles are (effectively) zero.

  small = max (abs (p));
  small = max ([small, 1] ) * 1e-8 * (1 + numel (p))^2;
  p(abs (p) < small) = 0;

  ## Determine if the poles are (effectively) real, or imaginary.

  index = (abs (imag (p)) < small);
  p(index) = real (p(index));
  index = (abs (real (p)) < small);
  p(index) = 1i * imag (p(index));

  ## Sort poles so that multiplicity loop will work.

  [e, indx] = mpoles (p, toler, 1);
  p = p (indx);

  ## Find the direct term if there is one.

  if (lb >= la)
    ## Also return the reduced numerator.
    [k, b] = deconv (b, a);
    lb = length (b);
  else
    k = [];
  endif

  if (lp == 1)
    r = polyval (b, p);
    return;
  endif

  ## Determine the order of the denominator and remaining numerator.
  ## With the direct term removed the potential order of the numerator
  ## is one less than the order of the denominator.

  aorder = numel (a) - 1;
  border = aorder - 1;

  ## Construct a system of equations relating the individual
  ## contributions from each residue to the complete numerator.

  A = zeros (border+1, border+1);
  B = prepad (reshape (b, [numel(b), 1]), border+1, 0);
  for ip = 1:numel(p)
    ri = zeros (size (p));
    ri(ip) = 1;
    A(:,ip) = prepad (rresidue (ri, p, [], toler), border+1, 0).';
  endfor

  ## Solve for the residues.

  r = A \ B;

endfunction

function [pnum, pden, e] = rresidue (r, p, k, toler, e)

  ## Reconstitute the numerator and denominator polynomials from the
  ## residues, poles, and direct term.

  if (nargin < 2 || nargin > 5)
    print_usage ();
  endif

  if (nargin < 5)
    e = [];
  endif

  if (nargin < 4)
    toler = [];
  endif

  if (nargin < 3)
    k = [];
  endif
 
  if numel (e)
    indx = 1:numel(p);
  else
    [e, indx] = mpoles (p, toler, 0);
    p = p (indx);
    r = r (indx);
  endif

  indx = 1:numel(p);

  for n = indx
    pn = [1, -p(n)];
    if n == 1
      pden = pn;
    else
      pden = conv (pden, pn);
    endif
  endfor

  ## D is the order of the denominator
  ## K is the order of the direct polynomial
  ## N is the order of the resulting numerator
  ## pnum(1:(N+1)) is the numerator's polynomial
  ## pden(1:(D+1)) is the denominator's polynomial
  ## pm is the multible pole for the nth residue
  ## pn is the numerator contribution for the nth residue

  D = numel (pden) - 1;
  K = numel (k) - 1;
  N = K + D;
  pnum = zeros (1, N+1);
  for n = indx(abs (r) > 0)
    p1 = [1, -p(n)];
    for m = 1:e(n)
      if (m == 1)
        pm = p1;
      else
        pm = conv (pm, p1);
      endif
    endfor
    pn = deconv (pden, pm);
    pn = r(n) * pn;
    pnum = pnum + prepad (pn, N+1, 0, 2);
  endfor

  ## Add the direct term.

  if (numel (k))
    pnum = pnum + conv (pden, k);
  endif

  ## Check for leading zeros and trim the polynomial coefficients.

  small = max ([max(abs(pden)), max(abs(pnum)), 1]) * eps;

  pnum(abs (pnum) < small) = 0;
  pden(abs (pden) < small) = 0;

  pnum = polyreduce (pnum);
  pden = polyreduce (pden);

endfunction

%!test
%! b = [1, 1, 1];
%! a = [1, -5, 8, -4];
%! [r, p, k, e] = residue (b, a);
%! assert (abs (r - [-2; 7; 3]) < 1e-5
%!   && abs (p - [2; 2; 1]) < 1e-7
%!   && isempty (k)
%!   && e == [1; 2; 1]);
%! k = [1 0];
%! b = conv (k, a) + prepad (b, numel (k) + numel (a) - 1, 0);
%! a = a;
%! [br, ar] = residue (r, p, k);
%! assert ((abs (br - b) < 1e-12
%!   && abs (ar - a) < 1e-12));
%! [br, ar] = residue (r, p, k, e);
%! assert ((abs (br - b) < 1e-12
%!   && abs (ar - a) < 1e-12));

%!test
%! b = [1, 0, 1];
%! a = [1, 0, 18, 0, 81];
%! [r, p, k, e] = residue(b, a);
%! r1 = [-5i; 12; +5i; 12]/54;
%! p1 = [+3i; +3i; -3i; -3i];
%! assert (abs (r - r1) < 1e-7 && abs (p - p1) < 1e-7
%!   && isempty (k)
%!   && e == [1; 2; 1; 2]);
%! [br, ar] = residue (r, p, k);
%! assert ((abs (br - b) < 1e-12
%!   && abs (ar - a) < 1e-12));

%!test
%! r = [7; 3; -2];
%! p = [2; 1; 2];
%! k = [1 0];
%! e = [2; 1; 1];
%! [b, a] = residue (r, p, k, e);
%! assert ((abs (b - [1, -5, 9, -3, 1]) < 1e-12
%!   && abs (a - [1, -5, 8, -4]) < 1e-12));
%! [rr, pr, kr, er] = residue (b, a);
%! [jnk, n] = mpoles(p);
%! assert ((abs (rr - r(n)) < 1e-5
%!   && abs (pr - p(n)) < 1e-7
%!   && abs (kr - k) < 1e-12
%!   && abs (er - e(n)) < 1e-12));

%!test
%! b = [1];
%! a = [1, 10, 25];
%! [r, p, k, e] = residue(b, a);
%! r1 = [0; 1];
%! p1 = [-5; -5];
%! assert (abs (r - r1) < 1e-7 && abs (p - p1) < 1e-7
%!   && isempty (k)
%!   && e == [1; 2]);
%! [br, ar] = residue (r, p, k);
%! assert ((abs (br - b) < 1e-12
%!   && abs (ar - a) < 1e-12));
