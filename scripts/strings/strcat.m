## Copyright (C) 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2002, 2003,
##               2005, 2006, 2007 John W. Eaton
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
## @deftypefn {Function File} {} strcat (@var{s1}, @var{s2}, @dots{})
## Return a string containing all the arguments concatenated.  For example,
##
## @example
## @group
## s = [ "ab"; "cde" ];
## strcat (s, s, s)
##      @result{} "ab ab ab "
##         "cdecdecde"
## @end group
## @end example
## @end deftypefn

## Author: jwe

function st = strcat (varargin)

  if (nargin > 0)

    if (iscellstr (varargin))
      ## All arguments are character strings.
      unwind_protect
	tmp = warning ("query", "Octave:empty-list-elements");
	warning ("off", "Octave:empty-list-elements");
	st = [varargin{:}];
      unwind_protect_cleanup
	warning (tmp.state, "Octave:empty-list-elements");
      end_unwind_protect
    else
      for i = 1:nargin
	tmp = varargin{i};
	if (! (iscellstr (tmp) || ischar (tmp)))
	  error ("strcat: all arguments must be strings or cell arrays of strings");
	endif
      endfor
      st = strcat_cell (varargin);
    endif
  else
    print_usage ();
  endif

endfunction

function st = strcat_cell (varargin)
  ## All args must be same size or scalars.
  ## See the xtest below for expected behavior.
  error ("strcat: concatenating cell arrays of strings not implemented");
endfunction

## test the dimensionality
## 1d
%!assert(strcat("ab ", "ab "), "ab ab ")
## 2d
%!assert(strcat(["ab ";"cde"], ["ab ";"cde"]), ["ab ab ";"cdecde"])

%!assert((strcmp (strcat ("foo", "bar"), "foobar")
%! && strcmp (strcat (["a"; "bb"], ["foo"; "bar"]), ["a foo"; "bbbar"])));

%!xtest
%! assert(all (strcmp (strcat ("a", {"bc", "de"}, "f"), {"abcf", "adef"})))

%!error strcat ();

%!error strcat (1, 2);

