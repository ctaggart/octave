## Copyright (C) 2019 Philip Nienhuis
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <https://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn {} {@var{pkg_list} =} make_rel_paths (@var{pkg_list})
## Internal undocumented function.
## @end deftypefn

## Author: Philip Nienhuis <prnienhuis@users.sf.net>
## Created: 2019-10-28

function pkg_list = make_rel_paths (pkg_list)

  ptn = ["^" strrep(canonicalize_file_name (OCTAVE_HOME), '\', '\\')];

  ## Strip pkg install directories from OCTAVE_HOME
  for i = 1:numel (pkg_list)
    pkg_list{i}.dir = canonicalize_file_name (pkg_list{i}.dir);
    pkg_list{i}.dir = regexprep (pkg_list{i}.dir, ptn, "__OH__");
    pkg_list{i}.archprefix = canonicalize_file_name (pkg_list{i}.archprefix);
    pkg_list{i}.archprefix = regexprep (pkg_list{i}.archprefix, ptn, "__OH__");
  endfor

endfunction
