/*

Copyright (C) 1996, 1997 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, write to the Free
Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.

*/

#if !defined (octave_mex_function_h)
#define octave_mex_function_h 1

#include <string>

#include "oct-shlib.h"

#include "ov-fcn.h"
#include "ov-builtin.h"
#include "ov-typeinfo.h"

class octave_shlib;

class octave_value;
class octave_value_list;

// Dynamically-linked functions.

class
octave_mex_function : public octave_function
{
public:

  octave_mex_function (void) { }

  octave_mex_function (void *fptr, bool fmex, const octave_shlib& shl,
		       const std::string& nm = std::string ());

  ~octave_mex_function (void);

  octave_value subsref (const std::string&,
			const std::list<octave_value_list>&)
    {
      panic_impossible ();
      return octave_value ();
    }

  octave_value_list subsref (const std::string& type,
			     const std::list<octave_value_list>& idx,
			     int nargout);

  octave_function *function_value (bool = false) { return this; }

  void mark_fcn_file_up_to_date (const octave_time& t) { t_checked = t; }

  std::string fcn_file_name (void) const;

  octave_time time_parsed (void) const;

  octave_time time_checked (void) const { return t_checked; }

  bool is_system_fcn_file (void) const { return system_fcn_file; }

  bool is_builtin_function (void) const { return false; }

  bool is_mex_function (void) const { return true; }

  octave_value_list
  do_multi_index_op (int nargout, const octave_value_list& args);

  void atexit (void (*fcn) (void)) { exit_fcn_ptr = fcn; }

private:

  void *mex_fcn_ptr;

  void (*exit_fcn_ptr) (void);

  bool have_fmex;

  octave_shlib sh_lib;

  // The time the file was last checked to see if it needs to be
  // parsed again.
  mutable octave_time t_checked;

  // True if this function came from a file that is considered to be a
  // system function.  This affects whether we check the time stamp
  // on the file to see if it has changed.
  bool system_fcn_file;

  // No copying!

  octave_mex_function (const octave_mex_function& fn);

  octave_mex_function& operator = (const octave_mex_function& fn);

  DECLARE_OCTAVE_ALLOCATOR

  DECLARE_OV_TYPEID_FUNCTIONS_AND_DATA
};

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
