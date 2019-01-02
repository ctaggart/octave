/*

Copyright (C) 1993-2019 John W. Eaton
Copyright (C) 2009 VZLU Prague, a.s.

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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <sstream>

#include "fcn-info.h"
#include "interpreter-private.h"
#include "interpreter.h"
#include "ov-fcn.h"
#include "ov-usr-fcn.h"
#include "symrec.h"
#include "symscope.h"
#include "symtab.h"
#include "utils.h"

namespace octave
{
  void symbol_scope_rep::install_auto_fcn_vars (void)
  {
    install_auto_fcn_var (".argn.");
    install_auto_fcn_var (".ignored.");
    install_auto_fcn_var (".nargin.");
    install_auto_fcn_var (".nargout.");
    install_auto_fcn_var (".saved_warning_states.");
  }

  void symbol_scope_rep::install_auto_fcn_var (const std::string& name)
  {
    insert (name, true);
    mark_hidden (name);
    mark_automatic (name);
  }

  octave_value
  symbol_scope_rep::find (const std::string& name)
  {
    symbol_table& symtab
      = __get_symbol_table__ ("symbol_scope_rep::find");

    // Variable.

    table_iterator p = m_symbols.find (name);

    if (p != m_symbols.end ())
      {
        symbol_record sr = p->second;

        if (sr.is_global ())
          return symtab.global_varval (name);
        else
          {
            octave_value val = sr.varval (m_context);

            if (val.is_defined ())
              return val;
          }
      }

    // Subfunction.  I think it only makes sense to check for
    // subfunctions if we are currently executing a function defined
    // from a .m file.

    octave_value fcn = find_subfunction (name);

    if (fcn.is_defined ())
      return fcn;

    return symtab.fcn_table_find (name, ovl ());
  }

  symbol_record&
  symbol_scope_rep::insert (const std::string& name, bool force_add)
  {
    table_iterator p = m_symbols.find (name);

    if (p == m_symbols.end ())
      {
        symbol_record ret (name);

        auto t_parent = m_parent.lock ();

        if (is_nested () && t_parent && t_parent->look_nonlocal (name, ret))
          return m_symbols[name] = ret;
        else
          {
            if (m_is_static && ! force_add)
              ret.mark_added_static ();

            return m_symbols[name] = ret;
          }
      }
    else
      return p->second;
  }

  octave_value
  symbol_scope_rep::dump (void) const
  {
    std::map<std::string, octave_value> m
      = {{ "name", m_name },
         { "nesting_depth", m_nesting_depth },
         { "is_static", m_is_static },
         { "symbols", dump_symbols_map () },
         { "subfunction_names", string_vector (m_subfunction_names) },
         { "subfunctions", dump_function_map (m_subfunctions) }};

    return octave_value (m);
  }

  octave_value
  symbol_scope_rep::dump_symbols_map (void) const
  {
    std::map<std::string, octave_value> info_map;

    for (const auto& nm_sr : m_symbols)
      {
        std::string nm = nm_sr.first;
        const symbol_record& sr = nm_sr.second;
        info_map[nm] = sr.dump (m_context);
      }

    return octave_value (info_map);
  }

  octave_value
  symbol_scope_rep::find_subfunction (const std::string& name) const
  {
    subfunctions_const_iterator p = m_subfunctions.find (name);

    if (p != m_subfunctions.end ())
      return p->second;

    auto t_parent = m_parent.lock ();

    if (t_parent)
      return t_parent->find_subfunction (name);

    return octave_value ();
  }

  void
  symbol_scope_rep::mark_subfunctions_in_scope_as_private (const std::string& class_name)
  {
    for (auto& nm_sf : m_subfunctions)
      {
        octave_function *fcn = nm_sf.second.function_value ();

        if (fcn)
          fcn->mark_as_private_function (class_name);
      }
  }

  void
  symbol_scope_rep::set_parent (const std::shared_ptr<symbol_scope_rep>& parent)
  {
    m_parent = std::weak_ptr<symbol_scope_rep> (parent);
  }

  void
  symbol_scope_rep::set_primary_parent (const std::shared_ptr<symbol_scope_rep>& parent)
  {
    m_primary_parent = std::weak_ptr<symbol_scope_rep> (parent);
  }

  bool
  symbol_scope_rep::is_relative (const std::shared_ptr<symbol_scope_rep>& scope) const
  {
    if (is_nested ())
      {
        // Since is_nested is true, the following should always return a
        // valid scope.

        auto t_primary_parent = m_primary_parent.lock ();

        if (t_primary_parent)
          {
            // SCOPE is the primary parent of this scope: this scope is a
            // child of SCOPE.
            if (t_primary_parent == scope)
              return true;

            // SCOPE and this scope share the same primary parent: they are
            // siblings.
            if (t_primary_parent == scope->primary_parent_scope_rep ())
              return true;
          }
      }

    return false;
  }

  void
  symbol_scope_rep::update_nest (void)
  {
    auto t_parent = m_parent.lock ();

    if (t_parent)
      {
        // fix bad symbol_records
        for (auto& nm_sr : m_symbols)
          {
            symbol_record& ours = nm_sr.second;

            if (! ours.is_formal ()
                && is_nested () && t_parent->look_nonlocal (nm_sr.first, ours))
              {
                if (ours.is_global () || ours.is_persistent ())
                  error ("global and persistent may only be used in the topmost level in which a nested variable is used");
              }
          }

        // The scopes of nested functions are static.
        if (is_nested ())
          m_is_static = true;
      }
    else if (m_children.size ())
      {
        // Parents of nested functions have static scopes.
        m_is_static = true;
      }

    for (auto& scope_obj : m_children)
      scope_obj.update_nest ();
  }

  bool
  symbol_scope_rep::look_nonlocal (const std::string& name,
                                   symbol_record& result)
  {
    table_iterator p = m_symbols.find (name);
    if (p == m_symbols.end ())
      {
        auto t_parent = m_parent.lock ();

        if (is_nested () && t_parent)
          return t_parent->look_nonlocal (name, result);
      }
    else if (! p->second.is_automatic ())
      {
        result.bind_fwd_rep (shared_from_this (), p->second);
        return true;
      }

    return false;
  }

  void
  symbol_scope_rep::bind_script_symbols
    (const std::shared_ptr<symbol_scope_rep>& curr_scope)
  {
    for (auto& nm_sr : m_symbols)
      nm_sr.second.bind_fwd_rep (curr_scope,
                                 curr_scope->find_symbol (nm_sr.first));
  }

  void
  symbol_scope_rep::unbind_script_symbols (void)
  {
    for (auto& nm_sr : m_symbols)
      nm_sr.second.unbind_fwd_rep ();
  }
}
