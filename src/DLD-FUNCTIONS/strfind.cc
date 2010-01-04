/*

Copyright (C) 2009 Jaroslav Hajek

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <climits>
#include <algorithm>
#include <deque>

#include "Cell.h"
#include "ov.h"
#include "defun-dld.h"
#include "unwind-prot.h"
#include "gripes.h"
#include "utils.h"

// This allows safe indexing with char. In C++, char may be (and often is) signed!
#define ORD(ch) static_cast<unsigned char>(ch)

// This is the quick search algorithm, as described at
// http://www-igm.univ-mlv.fr/~lecroq/string/node19.html
static void 
qs_preprocess (const Array<char>& needle,
               octave_idx_type table[UCHAR_MAX])
{
  const char *x = needle.data ();
  octave_idx_type m = needle.numel ();

   for (octave_idx_type i = 0; i < UCHAR_MAX; i++)
      table[i] = m + 1;
   for (octave_idx_type i = 0; i < m; i++)
      table[ORD(x[i])] = m - i;
}


static Array<octave_idx_type> 
qs_search (const Array<char>& needle,
           const Array<char>& haystack,
           const octave_idx_type table[UCHAR_MAX],
           bool overlaps = true)
{
  const char *x = needle.data ();
  octave_idx_type m = needle.numel ();
  const char *y = haystack.data ();
  octave_idx_type n = haystack.numel ();

  // We'll use deque because it typically has the most favorable properties for
  // the operation we need.
  std::deque<octave_idx_type> accum;
  if (n >= m)
    {
      octave_idx_type j = 0;

      if (overlaps)
        {
          while (j < n - m) 
            {
              if (std::equal (x, x + m, y + j))
                accum.push_back (j);
              j += table[ORD(y[j + m])];
            }
        }
      else
        {
          while (j < n - m) 
            {
              if (std::equal (x, x + m, y + j))
                {
                  accum.push_back (j);
                  j += m;
                }
              else
                j += table[ORD(y[j + m])];
            }
        }

      if (j == n - m && std::equal (x, x + m, y + j))
        accum.push_back (j);
    }

  octave_idx_type nmatch = accum.size ();
  Array<octave_idx_type> result (dim_vector (std::min (1, nmatch), nmatch));
  octave_idx_type k = 0;
  for (std::deque<octave_idx_type>::const_iterator iter = accum.begin (); 
       iter != accum.end (); iter++)
    {
      result.xelem (k++) = *iter;
    }

  return result;
}

DEFUN_DLD (strfind, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{idx} =} strfind (@var{str}, @var{pattern})\n\
@deftypefnx {Loadable Function} {@var{idx} =} strfind (@var{cellstr}, @var{pattern})\n\
Search for @var{pattern} in the string @var{str} and return the\n\
starting index of every such occurrence in the vector @var{idx}.\n\
If there is no such occurrence, or if @var{pattern} is longer\n\
than @var{str}, then @var{idx} is the empty array @code{[]}.\n\
\n\
If the cell array of strings @var{cellstr} is specified instead of the\n\
string @var{str}, then @var{idx} is a cell array of vectors, as specified\n\
above.  Examples:\n\
\n\
@example\n\
@group\n\
strfind (\"abababa\", \"aba\")\n\
     @result{} [1, 3, 5]\n\
\n\
strfind (@{\"abababa\", \"bebebe\", \"ab\"@}, \"aba\")\n\
     @result{} ans =\n\
        @{\n\
          [1,1] =\n\
\n\
             1   3   5\n\
\n\
          [1,2] = [](1x0)\n\
          [1,3] = [](1x0)\n\
        @}\n\
@end group\n\
@end example\n\
@seealso{findstr, strmatch, strcmp, strncmp, strcmpi, strncmpi, find}\n\
@end deftypefn")
{
  octave_value retval;
  int nargin = args.length ();

  if (nargin == 2)
    {
      octave_value argstr = args(0), argp = args(1);
      if (argp.is_string ())
        {
          Array<char> needle = argp.char_array_value ();
          OCTAVE_LOCAL_BUFFER (octave_idx_type, table, UCHAR_MAX);
          qs_preprocess (needle, table);

          if (argstr.is_string ())
            retval = octave_value (qs_search (needle, argstr.char_array_value (), table), 
                                   true, true);
          else if (argstr.is_cell ())
            {
              const Cell argsc = argstr.cell_value ();
              Cell retc (argsc.dims ());
              octave_idx_type ns = argsc.numel ();

              for (octave_idx_type i = 0; i < ns; i++)
                {
                  octave_value argse = argsc(i);
                  if (argse.is_string ())
                    retc(i) = octave_value (qs_search (needle, argse.char_array_value (), table), 
                                            true, true);
                  else
                    {
                      error ("strfind: each cell element must be a string");
                      break;
                    }
                }

              retval = retc;
            }
          else
            error ("strfind: first argument must be a string or cell array of strings");
        }
      else
        error ("strfind: pattern must be a string");
    }
  else
    print_usage ();

  return retval;
}

/*

%!error strfind ();
%!error strfind ("foo", "bar", 1);
%!error strfind ("foo", 100);
%!error strfind (100, "foo");

%!assert (strfind ("abababa", "aba"), [1, 3, 5]);
%!assert (strfind ({"abababa", "bla", "bla"}, "a"), {[1, 3, 5, 7], 3, 3});
%!assert (strfind ("Linux _is_ user-friendly. It just isn't ignorant-friendly or idiot-friendly.", "friendly"), [17, 50, 68]);

*/

DEFUN_DLD (strrep, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} strrep (@var{s}, @var{x}, @var{y})\n\
Replace all occurrences of the substring @var{x} of the string @var{s}\n\
with the string @var{y} and return the result.  For example,\n\
\n\
@example\n\
@group\n\
strrep (\"This is a test string\", \"is\", \"&%$\")\n\
     @result{} \"Th&%$ &%$ a test string\"\n\
@end group\n\
@end example\n\
@seealso{regexprep, strfind, findstr}\n\
@end deftypefn")
{
  octave_value retval;
  int nargin = args.length ();

  if (nargin == 3)
    {
      octave_value argstr = args(0), argp = args(1), argr = args(2);
      if (argp.is_string () && argp.is_string () && argr.is_string ())
        {
          const Array<char> str = argstr.char_array_value ();
          const Array<char> pat = argp.char_array_value ();
          const Array<char> rep = argr.char_array_value ();
          Array<char> ret = str;

          octave_idx_type siz = str.numel (), psiz = pat.numel (), rsiz = rep.numel ();

          if (psiz != 0)
            {
              OCTAVE_LOCAL_BUFFER (octave_idx_type, table, UCHAR_MAX);
              qs_preprocess (pat, table);

              // Look up matches, without overlaps.
              const Array<octave_idx_type> idx = qs_search (pat, str, table, false);
              octave_idx_type nidx = idx.numel ();

              if (nidx)
                {
                  // Compute result size.
                  octave_idx_type retsiz = siz + nidx * (rsiz - psiz);
                  ret.clear (dim_vector (1, retsiz));
                  const char *src = str.data (), *reps = rep.data ();
                  char *dest = ret.fortran_vec ();

                  octave_idx_type k = 0;
                  for (octave_idx_type i = 0; i < nidx; i++)
                    {
                      octave_idx_type j = idx(i);
                      dest = std::copy (src + k, src + j, dest);
                      dest = std::copy (reps, reps + rsiz, dest);
                      k = j + psiz;
                    }

                  std::copy (src + k, src + siz, dest);
                }
            }

          retval = ret;
        }
      else
        error ("strrep: all arguments must be strings");
    }
  else
    print_usage ();

  return retval;
}

/*

%!assert(strcmp (strrep ("This is a test string", "is", "&%$"),
%! "Th&%$ &%$ a test string"));

%!error strrep ();

%!error strrep ("foo", "bar", 3, 4);

*/
