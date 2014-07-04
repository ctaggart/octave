## Copyright (C) 2010 Søren Hauberg
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
## @deftypefn  {Function File} {} dump_demos ()
## @deftypefnx {Function File} {} dump_demos (@var{dirs})
## @deftypefnx {Function File} {} dump_demos (@var{dirs}, @var{mfile})
## @deftypefnx {Function File} {} dump_demos (@var{dirs}, @var{mfile}, @var{fmt})
## Produce a script, with the name specified by @var{mfile}, containing
## the demos in the directories, @var{dirs}.  The demos are assumed to produce
## graphical output, whose renderings are saved with the specified format,
## @var{fmt}.
##
## The defaults for each input are;
##
## @table @var
##   @item @var{dirs}
##   @code{@{"plot/appearance", "plot/draw", "plot/util"@}}
##   @item @var{mfile}
##   @code{"dump.m"}
##   @item @var{fmt}
##   @code{"png"}
## @end table
##
## For example, to produce PNG output for all demos of the functions
## in the plot directory;
##
## @example
## dump_demos plot dump.m png
## @end example
## @seealso{fntests, test, demo}
## @end deftypefn

## Author: Søren Hauberg  <soren@hauberg.org>

function dump_demos (dirs={"plot/appearance", "plot/draw", "plot/util"}, output="dump.m", fmt="png")

  if (nargin > 3)
    print_usage ();
  endif

  if (ischar (dirs))
    dirs = {dirs};
  elseif (! iscellstr (dirs))
    error ("dump_demos: DIRS must be a cell array of strings with directory names");
  endif

  [~, funcname, ext] = fileparts (output);
  if (isempty (ext))
    output = [output ".m"];
  endif

  ## Create script beginning (close figures, etc.)
  fid = fopen (output, "w");
  fprintf (fid, "%% DO NOT EDIT!  Generated automatically by dump_demos.m\n");
  fprintf (fid, "function %s ()\n", funcname);
  fprintf (fid, "close all\n");
  fprintf (fid, "more off\n");

  ## Run and print the demos in each directory
  for i = 1:numel (dirs)
    d = dirs{i};
    if (! is_absolute_filename (d))
      d = dir_in_loadpath (d);
    endif
    if (! exist (d, "dir"))
      error ("dump_demos: directory %s does not exist", d);
    endif
    dump_all_demos (d, fid, fmt);
  endfor

  ## Create script ending
  fprintf (fid, "end\n\n")

  ## Close script
  fclose (fid);
endfunction

function dump_all_demos (directory, fid, fmt)
  dirinfo = dir (fullfile (directory, "*.m"));
  flist = {dirinfo.name};
  ## Remove uigetdir, uigetfile, uiputfile, etc.
  flist = flist(! strncmp (flist, "ui", 2));
  ## Remove linkaxes, linkprops
  flist = flist(! strncmp (flist, "link", 4));
  ## Remove colormap
  flist = flist(! strncmp (flist, "colormap", 8));
  for i = 1:numel (flist)
    fcn = flist{i};
    fcn(end-1:end) = [];  # remove .m
    demos = get_demos (fcn);
    for d = 1:numel (demos)
      idx = sprintf ("%02d", d);
      fprintf (fid, "\ntry\n");
      fprintf (fid, "  %s\n\n", demos{d});
      fprintf (fid, "  drawnow;\n");
      fprintf (fid, "  fig = (get (0, 'currentfigure'));\n");
      fprintf (fid, "  if (~ isempty (fig))\n");
      fprintf (fid, "    figure (fig);\n");
      fprintf (fid, "    name = '%s_%s.%s';\n", fcn, idx, fmt);
      fprintf (fid, "    if (isempty (dir (name)))\n");
      fprintf (fid, "      fprintf ('Printing ""%%s"" ... ', name);\n")
      fprintf (fid, "      print ('-d%s', name);\n", fmt);
      fprintf (fid, "      fprintf ('done\\n');\n");
      fprintf (fid, "    else\n");
      fprintf (fid, "      fprintf ('File ""%%s"" exists.\\n', name);\n")
      fprintf (fid, "    end\n");
      fprintf (fid, "  end\n");
      # Temporary fix for cruft accumulating in figure window.
      fprintf (fid, "  close ('all');\n");
      fprintf (fid, "catch\n");
      fprintf (fid, "  fprintf ('ERROR in %s_%s: %%s\\n', lasterr ());\n", fcn, idx);
      fprintf (fid, "end\n\n");
    endfor
  endfor
  fprintf (fid, "close all\n");
endfunction

function retval = get_demos (fcn)
  [code, idx] = test (fcn, "grabdemo");
  num_demos = length (idx) - 1;
  retval = cell (1, num_demos);
  ## Now split the demos into a cell array
  for k = 1:num_demos
    retval{k} = oct2mat (code(idx(k):idx(k+1)-1));
  endfor
endfunction

function code = oct2mat (code)
  ## Simple hacks to make things Matlab compatible
  code = strrep (code, "%!", "%%");
  code = strrep (code, "!", "~");
  ## Simply replacing double quotes with single quotes
  ## causes problems with strings like 'hello "world"'
  ## More complicated regexprep targets only full double quoted strings
  code = regexprep (code, "^([^']*)\"(.*)\"", "$1'$2'",
                          "lineanchors", "dotexceptnewline");
  code = strrep (code, "#", "%");
  ## Fix the format specs for the errorbar demos changed by the line above
  code = strrep (code, "%r", "#r");
  code = strrep (code, "%~", "#~");
  endkeywords = {"endfor", "endfunction", "endif", "endwhile", "end_try_catch"};
  for k = 1:numel (endkeywords)
    code = strrep (code, endkeywords{k}, "end");
  endfor
  commentkeywords = {"unwind_protect", "end_unwind_protect"};
  for k = 1:numel (commentkeywords)
    code = strrep (code, commentkeywords{k}, ["%" commentkeywords{k}]);
  endfor

  ## Fix up sombrero which now has default argument in Octave
  code = strrep (code, "sombrero ()", "sombrero (41)");
endfunction

