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

#if ! defined (octave_call_stack_h)
#define octave_call_stack_h 1

#include "octave-config.h"

#include <deque>
#include <string>

class octave_function;
class octave_map;
class octave_user_code;
class octave_user_script;
class octave_value;
class octave_value_list;

#include "quit.h"

#include "stack-frame.h"
#include "symscope.h"

namespace octave
{
  class tree_evaluator;
  class symbol_info_list;
  class unwind_protect;

  class
  OCTINTERP_API
  call_stack
  {
  public:

    typedef std::deque<stack_frame *> stack_frames;

    typedef stack_frames::iterator iterator;
    typedef stack_frames::const_iterator const_iterator;

    typedef stack_frames::reverse_iterator reverse_iterator;
    typedef stack_frames::const_reverse_iterator const_reverse_iterator;

    call_stack (tree_evaluator& evaluator);

    // Current function (top of stack).
    octave_function * current (void) const
    {
      octave_function *retval = nullptr;

      if (! m_cs.empty ())
        {
          const stack_frame *elt = m_cs[m_curr_frame];
          retval = elt->function ();
        }

      return retval;
    }

    // Current line in current function.
    int current_line (void) const;

    // Current column in current function.
    int current_column (void) const;

    // Caller function, may be built-in.

    octave_function * caller (void) const
    {
      return (m_curr_frame > 1
              ? m_cs[m_curr_frame-1]->function () : m_cs[0]->function ());
    }

    size_t current_frame (void) const { return m_curr_frame; }

    size_t size (void) const { return m_cs.size (); }

    const stack_frame& get_current_stack_frame (void) const
    {
      return *(m_cs[m_curr_frame]);
    }

    stack_frame& get_current_stack_frame (void)
    {
      return *(m_cs[m_curr_frame]);
    }

    symbol_scope top_scope (void) const
    {
      return m_cs[0]->get_scope ();
    }

    symbol_scope current_scope (void) const
    {
      // FIXME: Can m_curr_frame ever be invalid?
      return (m_curr_frame < m_cs.size ()
              ? m_cs[m_curr_frame]->get_scope () : symbol_scope ());
    }

    bool at_top_level (void) const
    {
      return current_scope () == top_scope ();
    }

    // Function at location N on the call stack (N == 0 is current), may
    // be built-in.
    octave_function * element (size_t n)
    {
      octave_function *retval = nullptr;

      if (m_cs.size () > n)
        {
          stack_frame *elt = m_cs[n];
          retval = elt->function ();
        }

      return retval;
    }

    // User code caller.
    octave_user_code * current_user_code (void) const;

    unwind_protect * curr_fcn_unwind_protect_frame (void) const;

    // Line in user code caller.
    int current_user_code_line (void) const;

    // Column in user code caller.
    int current_user_code_column (void) const;

    // Current function that we are debugging.
    octave_user_code * debug_user_code (void) const;

    // Line number in current function that we are debugging.
    int debug_user_code_line (void) const;

    // Column number in current function that we are debugging.
    int debug_user_code_column (void) const;

    std::string get_dispatch_class (void) const;

    void set_dispatch_class (const std::string& class_name);

    bool is_class_method_executing (std::string& dispatch_class) const;

    bool is_class_constructor_executing (std::string& dispatch_class) const;

    // Return TRUE if all elements on the call stack are scripts.
    bool all_scripts (void) const;

    stack_frame * get_static_link (size_t prev_frame) const;

    void push (const symbol_scope& scope);

    void push (octave_user_function *fcn, unwind_protect *up_frame,
               stack_frame *closure_frames = nullptr);

    void push (octave_user_script *script, unwind_protect *up_frame);

    void push (octave_function *fcn);

    void set_location (int l, int c)
    {
      if (! m_cs.empty ())
        {
          stack_frame *elt = m_cs.back ();

          elt->line (l);
          elt->column (c);
        }
    }

    void set_line (int l)
    {
      if (! m_cs.empty ())
        {
          stack_frame *elt = m_cs.back ();

          elt->line (l);
        }
    }

    void set_column (int c)
    {
      if (! m_cs.empty ())
        {
          stack_frame *elt = m_cs.back ();

          elt->column (c);
        }
    }

    bool goto_frame (size_t n = 0, bool verbose = false);

    void restore_frame (size_t n)
    {
      goto_frame (n);
    }

    size_t find_current_user_frame (void) const;
    stack_frame *current_user_frame (void) const;

    size_t dbupdown (size_t start, int n, bool verbose);
    size_t dbupdown (int n = -1, bool verbose = false);

    void goto_caller_frame (void);

    void goto_base_frame (void);

    std::list<stack_frame *>
    backtrace_frames (octave_idx_type& curr_user_frame) const;

    // List of raw stack frames.

    std::list<stack_frame *> backtrace_frames (void) const;

    // List of stack_info objects that can be used in liboctave and
    // stored in the execution_exception object.

    std::list<frame_info> backtrace_info (octave_idx_type& curr_user_frame,
                                          bool print_subfn = true) const;

    std::list<frame_info> backtrace_info (void) const;

    // The same as backtrace_info but in the form of a struct array
    // object that may be used in the interpreter.

    octave_map backtrace (octave_idx_type& curr_user_frame,
                          bool print_subfn = true) const;

    octave_map backtrace (void) const;

    octave_map empty_backtrace (void) const;

    void pop (void);

    void clear (void);

    symbol_info_list all_variables (void);

    std::list<symbol_record> glob (const std::string& pattern) const;

    std::list<symbol_record> regexp (const std::string& pattern) const;

    std::list<std::string> global_variable_names (void) const;

    std::list<std::string> top_level_variable_names (void) const;

    std::list<std::string> variable_names (void) const;

    void clear_global_variable (const std::string& name);

    void clear_global_variable_pattern (const std::string& pattern);

    void clear_global_variable_regexp(const std::string& pattern);

    void clear_global_variables (void);

    symbol_info_list glob_symbol_info (const std::string& pattern) const;

    symbol_info_list regexp_symbol_info (const std::string& pattern) const;

    symbol_info_list get_symbol_info (void);

    symbol_info_list top_scope_symbol_info (void) const;

    octave_value max_stack_depth (const octave_value_list& args, int nargout);

    void make_persistent (const symbol_record& sym);

    void make_global (const symbol_record& sym);

    octave_value global_varval (const std::string& name) const;

    octave_value& global_varref (const std::string& name);

    octave_value get_top_level_value (const std::string& name) const;

    void set_top_level_value (const std::string& name,
                              const octave_value& value);

    octave_value do_who (int argc, const string_vector& argv,
                         bool return_list, bool verbose = false);

    octave_value do_who_two (const string_vector& patterns, bool have_regexp,
                             bool return_list, bool verbose,
                             const std::string& msg = "");

    octave_value do_global_who_two (const string_vector& patterns,
                                    bool have_regexp, bool return_list,
                                    bool verbose, const std::string& msg = "");

    void clear_current_frame_values (void);

    void display (void) const;

    void set_auto_fcn_var (stack_frame::auto_var_type avt,
                           const octave_value& val);

    octave_value get_auto_fcn_var (stack_frame::auto_var_type avt) const;

  private:

    tree_evaluator& m_evaluator;

    // The current call stack.
    // FIXME: maybe we should be using a std::shared_ptr to manage the
    // individual stack frames?
    stack_frames m_cs;

    // FIXME: Could we eliminate this varaible and manage the current
    // frame in the evaluator class instead?  The current frame might
    // always be the top of the stack.  Restoring the previous/current
    // frame would be managed by other means, such as an
    // unwind_protect frame.
    size_t m_curr_frame;

    int m_max_stack_depth;

    std::map<std::string, octave_value> m_global_values;
  };
}

#endif
