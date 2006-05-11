## Copyright (C) 2000  Etienne Grossmann
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

## -*- texinfo -*-
## @deftypefn {Function File} {} rmpath (@var{dir1}, @dots{})
## Remove @var{dir1}, @dots{} from the current function search path.
##
## @seealso{path, addpath, genpath, pathdef, savepath, pathsep}
## @end deftypefn

## Author: Etienne Grossmann <etienne@cs.uky.edu>

## PKGADD: mark_as_command rmpath

function ret = rmpath (varargin)

  if (nargout > 0)
    ret = path ();
  endif

  psep = pathsep ();

  xpath = cellstr (split (path (), psep));
  n_path_elts = length (xpath);

  for i = 1:nargin
    dir_elts = cellstr (split (varargin{i}, psep));
    n_dir_elts = length (dir_elts);
    for j = 1:n_dir_elts
      dir = regexprep (dir_elts{j}, "//+", "/");
      dir = regexprep (dir, "/$", "");
      elt_found = false;
      for k = n_path_elts:-1:1
	if (strcmp (dir, "."))
	  warning ("rmpath: can't remove \".\" from path");
	elseif (strcmp (dir, xpath{k}))
	  xpath(k) = [];
	  n_path_elts--;
	  elt_found = true;
	endif
      endfor
      if (! elt_found)
	warning ("rmpath: %s: not found", dir);
      endif
    endfor
  endfor

  ## Ensure a 1xN cell array.
  xpath = xpath(:)';

  xpath{2,:} = psep;
  xpath{2,end} = "";

  tmp = strcat (xpath{:});

  path (tmp);
  
endfunction  
