## Copyright (C) 2010-2011 Ben Abbott <bpabbott@mac.com>
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
## @deftypefn  {Function File} {@var{C} =} textscan (@var{fid}, @var{format})
## @deftypefnx {Function File} {@var{C} =} textscan (@var{fid}, @var{format}, @var{n})
## @deftypefnx {Function File} {@var{C} =} textscan (@var{fid}, @var{format}, @var{param}, @var{value}, @dots{})
## @deftypefnx {Function File} {@var{C} =} textscan (@var{fid}, @var{format}, @var{n}, @var{param}, @var{value}, @dots{})
## @deftypefnx {Function File} {@var{C} =} textscan (@var{str}, @dots{})
## @deftypefnx {Function File} {[@var{C}, @var{position}] =} textscan (@dots{})
## Read data from a text file.
##
## The file associated with @var{fid} is read and parsed according to
## @var{format}.  The function behaves like @code{strread} except it works by
## parsing a file instead of a string.  See the documentation of
## @code{strread} for details.  
##
## In addition to the options supported by
## @code{strread}, this function supports a few more:
##
## @itemize
## @item "headerlines":
## The first @var{value} number of lines of @var{str} are skipped.
##
## @item "endofline":
## Specify a single character or "\r\n".  If no value is given, it will be
## inferred from the file.  If set to "" (empty string) EOLs are ignored as
## delimiters.
##
## @item "returnonerror":
## If set to numerical 1 or true (default), return normally when read errors
## have been encountered.  If set to 0 or false, return an error and no data.
## @end itemize
##
## The optional input, @var{n}, specifes the number of lines to be read from
## the file, associated with @var{fid}.
##
## The output, @var{C}, is a cell array whose length is given by the number
## of format specifiers.
##
## The second output, @var{position}, provides the position, in characters,
## from the beginning of the file.
##
## @seealso{dlmread, fscanf, load, strread, textread}
## @end deftypefn

function [C, position] = textscan (fid, format = "%f", varargin)

  ## Check input
  if (nargin < 1)
    print_usage ();
  endif

  if (isempty (format))
    format = "%f";
  endif

  if (! (isa (fid, "double") && fid > 0) && ! ischar (fid))
    error ("textscan: first argument must be a file id or character string");
  endif

  if (! ischar (format))
    error ("textscan: FORMAT must be a valid specification");
  endif

  if (nargin > 2 && isnumeric (varargin{1}))
    nlines = varargin{1};
    args = varargin(2:end);
  else
    nlines = Inf;
    args = varargin;
  endif

  if (! any (strcmpi (args, "emptyvalue")))
    ## Matlab returns NaNs for missing values
    args{end+1} = "emptyvalue";
    args{end+1} = NaN;
  endif

  ## Check default parameter values that differ for strread & textread

  ipos = find (strcmpi (args, "whitespace"));
  if (isempty (ipos))
    ## Matlab default whitespace = " \b\t"
    args{end+1} = "whitespace";
    args{end+1} = " \b\t";
    whitespace = " \b\t";
  else
    ## Check if there's at least one string format specifier
    fmt = strrep (format, "%", " %");
    [~, ~, ~, fmt] = regexp (fmt, '[^ ]+');
    fmt = strtrim (fmt(strmatch ("%", fmt)))
    has_str_fmt = all (cellfun ("isempty", strfind (strtrim (fmt(strmatch ("%", fmt))), 's')));
    ## If there is a format, AND whitespace value = empty, 
    ## don't add a space (char(32)) to whitespace
    if (! (isempty (args{ipos+1}) &&  has_str_fmt))
      args {ipos+1} = unique ([" " whitespace]);
    endif
  endif

  if (! any (strcmpi (args, "delimiter")))
    ## Matlab says default delimiter = whitespace.  
    ## strread() will pick this up further
    args{end+1} = "delimiter";
    args{end+1} = "";
  endif

  if (any (strcmpi (args, "returnonerror")))
    ## Because of the way strread() reads data (columnwise) this parameter
    ## can't be neatly implemented.  strread() will pick it up anyway
    warning ('ReturnOnError is not fully implemented');
  else
    ## Set default value (=true)
    args{end+1} = "returnonerror";
    args{end+1} = 1;
  endif

  if (ischar (fid))
    ## Read from a text string
    if (nargout == 2)
      error ("textscan: cannot provide position information for character input");
    endif
    str = fid;
  else
    ## Skip header lines if requested
    headerlines = find (strcmpi (args, "headerlines"), 1);
    ## Beware of zero valued headerline, fskipl would skip to EOF
    if (! isempty (headerlines) && (args{headerlines + 1} > 0))
      fskipl (fid, varargin{headerlines + 1});
      args(headerlines:headerlines+1) = []; 
    endif
    if (isfinite (nlines))
      str = "";
      ## FIXME: Can this be done without slow for loop?
      for n = 1:nlines
        str = strcat (str, fgets (fid));
      endfor
    else
      str = fread (fid, "char=>char").';
    endif
  endif

  ## Check for empty result
  if (isempty (str))
    warning ("textscan: no data read");
    C = [];
  else
    ## Check value of 'endofline'.  String or file doesn't seem to matter
    endofline = find (strcmpi (args, "endofline"), 1);
    if (! isempty (endofline))
      if (! ischar (args{endofline + 1})) 
        error ("textscan: character value required for EndOfLine"); 
      endif
    else
      ## Determine EOL from file.  Search for EOL candidates in first 3000 chars
      BUFLEN = 3000;
      ## First try DOS (CRLF)
      eol_srch_len = min (length (str), 3000);
      if (! isempty (findstr ("\r\n", str(1 : eol_srch_len))))
        eol_char = "\r\n";
      ## Perhaps old Macintosh? (CR)
      elseif (! isempty (findstr ("\r", str(1 : eol_srch_len))))
        eol_char = "\r";
      ## Otherwise, use plain UNIX (LF)
      else
        eol_char = "\n";
      endif
      ## Set up the default endofline param value
      args{end+1} = "endofline";
      args{end+1} = eol_char;
    endif

    ## Determine the number of data fields
    num_fields = numel (strfind (format, "%")) - ...
                 numel (idx_star = strfind (format, "%*"));

    ## Strip trailing EOL to avoid returning stray missing values (f. strread)
    if (strcmp (str(end-length (eol_char) + 1 : end), eol_char));
      str = str(1 : end-length (eol_char)); 
    endif

    ## Call strread to make it do the real work
    C = cell (1, num_fields);
    [C{:}] = strread (str, format, args{:});

    if (ischar (fid) && isfinite (nlines))
      C = cellfun (@(x) x(1:nlines), C, "uniformoutput", false);
    endif

    if (nargout == 2)
      position = ftell (fid);
    endif

  endif

endfunction


%!test
%! str = "1,  2,  3,  4\n 5,  ,  ,  8\n 9, 10, 11, 12";
%! fmtstr = "%f %d %f %s";
%! c = textscan (str, fmtstr, 2, "delimiter", ",", "emptyvalue", -Inf);
%! assert (isequal (c{1}, [1;5]));
%! assert (length (c{1}), 2);
%! assert (iscellstr (c{4}));
%! assert (isequal (c{3}, [3; -Inf]));

%!test
%! b = [10:10:100];
%! b = [b; 8*b/5];
%! str = sprintf ("%g miles/hr = %g kilometers/hr\n", b);
%! fmt = "%f miles/hr = %f kilometers/hr";
%! c = textscan (str, fmt);
%! assert (b(1,:)', c{1});
%! assert (b(2,:)', c{2});

#%!test
#%! str = "13, 72, NA, str1, 25\r\n// Middle line\r\n36, na, 05, str3, 6";
#%! a = textscan(str, '%d %n %f %s %n', 'delimiter', ',','treatAsEmpty', {'NA', 'na'},'commentStyle', '//');
#%! assert (a{1}, [13; 36]);
#%! assert (a{2}, [72; NaN]);
#%! assert (a{3}, [NaN; 5]);
#%! assert (a{4}, {"str1"; "str3"});
#%! assert (a{5}, [25; 6]);

%!test
%! str = "Km:10 = hhhBjjj miles16hour\r\n";
%! str = [str "Km:15 = hhhJjjj miles241hour\r\n"];
%! str = [str "Km:2 = hhhRjjj miles3hour\r\n"];
%! str = [str "Km:25 = hhhZ\r\n"];
%! fmt = "Km:%d = hhh%1sjjj miles%dhour";
%! a = textscan (str, fmt, 'delimiter', ' ');
%! assert (a{1}', [10 15 2 25], 1e-5);
%! assert (a{2}', {'B' 'J' 'R' 'Z'});
%! assert (a{3}', [16 241 3 NaN], 1e-5);

