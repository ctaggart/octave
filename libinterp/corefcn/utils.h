/*

Copyright (C) 1993-2019 John W. Eaton

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#if ! defined (octave_utils_h)
#define octave_utils_h 1

#include "octave-config.h"

#include <cstdarg>

#include <iosfwd>
#include <string>
#include <list>

#include "dMatrix.h"
#include "lo-utils.h"

class octave_value;
class octave_value_list;
class string_vector;

namespace octave
{
  extern OCTINTERP_API bool valid_identifier (const char *s);
  extern OCTINTERP_API bool valid_identifier (const std::string& s);

  extern OCTINTERP_API bool
  same_file (const std::string& f, const std::string& g);

  extern OCTINTERP_API int almost_match (const std::string& std,
                                         const std::string& s,
                                         int min_match_len = 1,
                                         int case_sens = 1);

  extern OCTINTERP_API int
  keyword_almost_match (const char * const *std, int *min_len,
                        const std::string& s, int min_toks_to_match,
                        int max_toks);

  extern OCTINTERP_API std::string
  search_path_for_file (const std::string&, const string_vector&);

  extern OCTINTERP_API string_vector
  search_path_for_all_files (const std::string&, const string_vector&);

  extern OCTINTERP_API std::string
  file_in_path (const std::string&, const std::string&);

  extern OCTINTERP_API std::string
  find_data_file_in_load_path  (const std::string& fcn,
                                const std::string& file,
                                bool require_regular_file = false);

  extern OCTINTERP_API std::string contents_file_in_path (const std::string&);

  extern OCTINTERP_API std::string fcn_file_in_path (const std::string&);

  extern OCTINTERP_API std::string do_string_escapes (const std::string& s);

  extern OCTINTERP_API const char * undo_string_escape (char c);

  extern OCTINTERP_API std::string undo_string_escapes (const std::string& s);

  extern OCTINTERP_API void
  check_dimensions (dim_vector& dim, const char *warnfor);

  extern OCTINTERP_API void
  get_dimensions (const octave_value& a, const char *warn_for,
                  dim_vector& dim);

  extern OCTINTERP_API void
  get_dimensions (const octave_value& a, const octave_value& b,
                  const char *warn_for, octave_idx_type& nr,
                  octave_idx_type& nc);

  extern OCTINTERP_API void
  get_dimensions (const octave_value& a, const char *warn_for,
                  octave_idx_type& nr, octave_idx_type& nc);

  extern OCTINTERP_API octave_idx_type
  dims_to_numel (const dim_vector& dims, const octave_value_list& idx);

  extern OCTINTERP_API Matrix
  identity_matrix (octave_idx_type nr, octave_idx_type nc);

  extern OCTINTERP_API FloatMatrix
  float_identity_matrix (octave_idx_type nr, octave_idx_type nc);

  extern OCTINTERP_API size_t
  format (std::ostream& os, const char *fmt, ...);

  extern OCTINTERP_API size_t
  format (std::ostream& os, const std::string& enc, const char *fmt, ...);

  extern OCTINTERP_API size_t
  vformat (std::ostream& os, const char *fmt, va_list args);

  extern OCTINTERP_API size_t
  vformat (std::ostream& os, const std::string& enc,
           const char *fmt, va_list args);

  extern OCTINTERP_API std::string
  vasprintf (const char *fmt, va_list args);

  extern OCTINTERP_API std::string asprintf (const char *fmt, ...);

  extern OCTINTERP_API void sleep (double seconds,
                                   bool do_graphics_events = false);

  extern OCTINTERP_API
  octave_value_list
  do_simple_cellfun (octave_value_list (*fun) (const octave_value_list&, int),
                     const char *fun_name, const octave_value_list& args,
                     int nargout);

  extern OCTINTERP_API
  octave_value
  do_simple_cellfun (octave_value_list (*fun) (const octave_value_list&, int),
                     const char *fun_name, const octave_value_list& args);
}

#if defined (OCTAVE_USE_DEPRECATED_FUNCTIONS)

OCTAVE_DEPRECATED (5, "use 'octave::valid_identifier' instead")
extern OCTINTERP_API bool
valid_identifier (const char *s);

OCTAVE_DEPRECATED (5, "use 'octave::valid_identifier' instead")
extern OCTINTERP_API bool
valid_identifier (const std::string& s);

OCTAVE_DEPRECATED (5, "use 'octave::same_file' instead")
extern OCTINTERP_API bool
same_file (const std::string& f, const std::string& g);

OCTAVE_DEPRECATED (5, "use 'octave::almost_match' instead")
extern OCTINTERP_API int
almost_match (const std::string& std, const std::string& s,
              int min_match_len = 1, int case_sens = 1);

OCTAVE_DEPRECATED (5, "use 'octave::keyword_almost_match' instead")
extern OCTINTERP_API int
keyword_almost_match (const char * const *std, int *min_len,
                      const std::string& s, int min_toks_to_match,
                      int max_toks);

OCTAVE_DEPRECATED (5, "use 'octave::search_path_for_file' instead")
extern OCTINTERP_API std::string
search_path_for_file (const std::string& path, const string_vector& names);

OCTAVE_DEPRECATED (5, "use 'octave::search_path_for_all_files' instead")
extern OCTINTERP_API string_vector
search_path_for_all_files (const std::string& path, const string_vector& names);

OCTAVE_DEPRECATED (5, "use 'octave::file_in_path' instead")
extern OCTINTERP_API std::string
file_in_path (const std::string& name, const std::string& suffix);

OCTAVE_DEPRECATED (5, "use 'octave::find_data_file_in_load_path ' instead")
extern OCTINTERP_API std::string
find_data_file_in_load_path  (const std::string& fcn, const std::string& file,
                              bool require_regular_file = false);

OCTAVE_DEPRECATED (5, "use 'octave::contents_file_in_path' instead")
extern OCTINTERP_API std::string
contents_file_in_path (const std::string& s);

OCTAVE_DEPRECATED (5, "use 'octave::fcn_file_in_path' instead")
extern OCTINTERP_API std::string
fcn_file_in_path (const std::string& s);

OCTAVE_DEPRECATED (5, "use 'octave::do_string_escapes' instead")
extern OCTINTERP_API std::string
do_string_escapes (const std::string& s);

OCTAVE_DEPRECATED (5, "use 'octave::undo_string_escape' instead")
extern OCTINTERP_API const char *
undo_string_escape (char c);

OCTAVE_DEPRECATED (5, "use 'octave::undo_string_escapes' instead")
extern OCTINTERP_API std::string
undo_string_escapes (const std::string& s);

OCTAVE_DEPRECATED (5, "use 'octave::check_dimensions' instead")
extern OCTINTERP_API void
check_dimensions (dim_vector& dim, const char *warnfor);

OCTAVE_DEPRECATED (5, "use 'octave::get_dimensions' instead")
extern OCTINTERP_API void
get_dimensions (const octave_value& a, const char *warn_for,
                dim_vector& dim);

OCTAVE_DEPRECATED (5, "use 'octave::get_dimensions' instead")
extern OCTINTERP_API void
get_dimensions (const octave_value& a, const octave_value& b,
                const char *warn_for, octave_idx_type& nr,
                octave_idx_type& nc);

OCTAVE_DEPRECATED (5, "use 'octave::get_dimensions' instead")
extern OCTINTERP_API void
get_dimensions (const octave_value& a, const char *warn_for,
                octave_idx_type& nr, octave_idx_type& nc);

OCTAVE_DEPRECATED (5, "use 'octave::dims_to_numel' instead")
extern OCTINTERP_API octave_idx_type
dims_to_numel (const dim_vector& dims, const octave_value_list& idx);

OCTAVE_DEPRECATED (5, "use 'octave::identity_matrix' instead")
extern OCTINTERP_API Matrix
identity_matrix (octave_idx_type nr, octave_idx_type nc);

OCTAVE_DEPRECATED (5, "use 'octave::float_identity_matrix' instead")
extern OCTINTERP_API FloatMatrix
float_identity_matrix (octave_idx_type nr, octave_idx_type nc);

template <typename ... Args>
OCTAVE_DEPRECATED (5, "use 'octave::format' instead")
size_t
octave_format (std::ostream& os, const char *fmt, Args&& ... args)
{
  return octave::format (os, fmt, std::forward<Args> (args) ...);
}

OCTAVE_DEPRECATED (5, "use 'octave::vformat' instead")
extern OCTINTERP_API size_t
octave_vformat (std::ostream& os, const char *fmt, va_list args);

OCTAVE_DEPRECATED (5, "use 'octave::vasprintf' instead")
extern OCTINTERP_API std::string
octave_vasprintf (const char *fmt, va_list args);

template <typename ... Args>
OCTAVE_DEPRECATED (5, "use 'octave::asprintf' instead")
std::string
octave_asprintf (const char *fmt, Args&& ... args)
{
  return octave::asprintf (fmt, std::forward<Args> (args) ...);
}

OCTAVE_DEPRECATED (5, "use 'octave::sleep' instead")
extern OCTINTERP_API void
octave_sleep (double seconds);

OCTAVE_DEPRECATED (5, "use 'octave::do_simple_cellfun' instead")
extern OCTINTERP_API octave_value_list
do_simple_cellfun (octave_value_list (*fun) (const octave_value_list&, int),
                   const char *fun_name, const octave_value_list& args,
                   int nargout);

OCTAVE_DEPRECATED (5, "use 'octave::do_simple_cellfun' instead")
extern OCTINTERP_API octave_value
do_simple_cellfun (octave_value_list (*fun) (const octave_value_list&, int),
                   const char *fun_name, const octave_value_list& args);

#endif

#endif
