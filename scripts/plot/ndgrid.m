## Copyright (C) 2006, Alexander Barth
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
## @deftypefn {Function File} {[@var{y1}, @var{y2}, ...,  @var{y}n]} = ndgrid (@var{x1}, @var{x2}, ..., @var{x}n)
## @deftypefnx {Function File} {[@var{y1}, @var{y2}, ...,  @var{y}n]} = ndgrid (@var{x})
## Given n vectors @var{x1}, ... @var{x}n, ndgrid returns n arrays of dimension n. 
## The elements of the ith output argument contains the elements of the vector 
## @var{x}i repeated over all dimensions different from the ith dimension.
## Calling ndgrid with only one input argument @var{x} is equivalent of calling ndgrid with 
## all n input arguments equal to @var{x}:
##
## [@var{y1}, @var{y2}, ...,  @var{y}n] = ndgrid (@var{x}, ..., @var{x})
## @seealso{meshgrid}
## @end deftypefn

## Author: Alexander Barth <abarth@marine.usf.edu>

function varargout = ndgrid (varargin)

  if (nargin == 1)
    n = max ([nargout, 2]);  
    ## If only one input argument is given, repeat it n-times
    varargin{1:n} = varargin{1};
  elseif (nargin >= nargout)
    n = max ([nargin, 2]);  
  else
    error ("ndgrid: wrong number of input arguments");
  endif

  ## Determine the size of the output arguments
  
  shape = zeros (1, n);

  for i = 1:n
    if (! isvector (varargin{i}))
      error ("ndgrid: arguments must be vectors");
    endif

    shape(i) = length (varargin{i});
  endfor

  for i = 1:n
    ## size for reshape
    r = ones (1, n);
    r(i) = shape(i);

    ## size for repmat
    s = shape;
    s(i) = 1;

    varargout{i} = repmat (reshape (varargin{i}, r), s);
  endfor

endfunction
