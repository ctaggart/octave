/*

Copyright (C) 2012-2019 Michael Goffioul

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

#if ! defined (octave_cdef_utils_h)
#define octave_cdef_utils_h 1

#include "octave-config.h"

#include <list>
#include <string>

class octave_value;
class cdef_class;
class cdef_package;
class Cell;

extern std::string
get_base_name (const std::string& nm);

extern void
make_function_of_class (const std::string& class_name,
                        const octave_value& fcn);

extern void
make_function_of_class (const cdef_class& cls, const octave_value& fcn);

extern cdef_class
lookup_class (const std::string& name, bool error_if_not_found = true,
              bool load_if_not_found = true);

extern cdef_class
lookup_class (const cdef_class& cls);

extern cdef_class
lookup_class (const octave_value& ov);

extern std::list<cdef_class>
lookup_classes (const Cell& cls_list);

extern octave_value
to_ov (const std::list<cdef_class>& class_list);

extern bool
is_dummy_method (const octave_value& fcn);

extern bool
is_superclass (const cdef_class& clsa, const cdef_class& clsb,
               bool allow_equal = true, int max_depth = -1);
extern bool
is_strict_superclass (const cdef_class& clsa, const cdef_class& clsb);

extern bool
is_direct_superclass (const cdef_class& clsa, const cdef_class& clsb);

extern cdef_package
lookup_package (const std::string& name, bool error_if_not_found = true,
                bool load_if_not_found = true);

extern cdef_class
get_class_context (std::string& name, bool& in_constructor);

extern cdef_class
get_class_context (void);

#endif
