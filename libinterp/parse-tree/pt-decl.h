/*

Copyright (C) 1996-2017 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#if ! defined (octave_pt_decl_h)
#define octave_pt_decl_h 1

#include "octave-config.h"

#include <string>

#include "base-list.h"
#include "oct-lvalue.h"
#include "pt-cmd.h"
#include "pt-id.h"
#include "pt-walk.h"
#include "symtab.h"

namespace octave
{
  class tree_evaluator;
  class tree_expression;
  class tree_identifier;

  // List of expressions that make up a declaration statement.

  class tree_decl_elt
  {
  public:

    tree_decl_elt (tree_identifier *i = 0, tree_expression *e = 0)
      : id (i), expr (e) { }

    // No copying!

    tree_decl_elt (const tree_decl_elt&) = delete;

    tree_decl_elt& operator = (const tree_decl_elt&) = delete;

    ~tree_decl_elt (void);

    bool is_defined (void) { return id ? id->is_defined () : false; }

    bool is_variable (void) { return id ? id->is_variable () : false; }

    void mark_as_formal_parameter (void)
    {
      if (id)
        id->mark_as_formal_parameter ();
    }

    bool lvalue_ok (void) { return id ? id->lvalue_ok () : false; }

    octave_lvalue lvalue (tree_evaluator *tw)
    {
      return id ? id->lvalue (tw) : octave_lvalue ();
    }

    tree_identifier * ident (void) { return id; }

    std::string name (void) { return id ? id->name () : ""; }

    tree_expression * expression (void) { return expr; }

    tree_decl_elt * dup (symbol_table::scope_id scope,
                         symbol_table::context_id context) const;

    void accept (tree_walker& tw)
    {
      tw.visit_decl_elt (*this);
    }

  private:

    // An identifier to tag with the declared property.
    tree_identifier *id;

    // An initializer expression (may be zero);
    tree_expression *expr;
  };

  class tree_decl_init_list : public octave::base_list<tree_decl_elt *>
  {
  public:

    tree_decl_init_list (void) { }

    tree_decl_init_list (tree_decl_elt *t) { append (t); }

    // No copying!

    tree_decl_init_list (const tree_decl_init_list&) = delete;

    tree_decl_init_list& operator = (const tree_decl_init_list&) = delete;

    ~tree_decl_init_list (void)
    {
      while (! empty ())
        {
          iterator p = begin ();
          delete *p;
          erase (p);
        }
    }

    tree_decl_init_list * dup (symbol_table::scope_id scope,
                               symbol_table::context_id context) const;

    void accept (tree_walker& tw)
    {
      tw.visit_decl_init_list (*this);
    }
  };

  // Base class for declaration commands -- global, static, etc.

  class tree_decl_command : public tree_command
  {
  public:

    tree_decl_command (const std::string& n, int l = -1, int c = -1)
      : tree_command (l, c), cmd_name (n), init_list (0) { }

    tree_decl_command (const std::string& n, tree_decl_init_list *t,
                       int l = -1, int c = -1)
      : tree_command (l, c), cmd_name (n), init_list (t) { }

    // No copying!

    tree_decl_command (const tree_decl_command&) = delete;

    tree_decl_command& operator = (const tree_decl_command&) = delete;

    ~tree_decl_command (void);

    tree_decl_init_list * initializer_list (void) { return init_list; }

    std::string name (void) { return cmd_name; }

  protected:

    // The name of this command -- global, static, etc.
    std::string cmd_name;

    // The list of variables or initializers in this declaration command.
    tree_decl_init_list *init_list;
  };

  // Global.

  class tree_global_command : public tree_decl_command
  {
  public:

    tree_global_command (int l = -1, int c = -1)
      : tree_decl_command ("global", l, c) { }

    tree_global_command (tree_decl_init_list *t, int l = -1, int c = -1)
      : tree_decl_command ("global", t, l, c) { }

    // No copying!

    tree_global_command (const tree_global_command&) = delete;

    tree_global_command& operator = (const tree_global_command&) = delete;

    ~tree_global_command (void) = default;

    tree_command * dup (symbol_table::scope_id scope,
                        symbol_table::context_id context) const;

    void accept (tree_walker& tw)
    {
      tw.visit_global_command (*this);
    }

  private:

    static void do_init (tree_decl_elt& elt);
  };

  // Persistent.

  class tree_persistent_command : public tree_decl_command
  {
  public:

    tree_persistent_command (int l = -1, int c = -1)
      : tree_decl_command ("persistent", l, c) { }

    tree_persistent_command (tree_decl_init_list *t, int l = -1, int c = -1)
      : tree_decl_command ("persistent", t, l, c) { }

    // No copying!

    tree_persistent_command (const tree_persistent_command&) = delete;

    tree_persistent_command& operator = (const tree_persistent_command&) = delete;

    ~tree_persistent_command (void) = default;

    tree_command * dup (symbol_table::scope_id scope,
                        symbol_table::context_id context) const;

    void accept (tree_walker& tw)
    {
      tw.visit_persistent_command (*this);
    }

  private:

    static void do_init (tree_decl_elt& elt);
  };
}

#if defined (OCTAVE_USE_DEPRECATED_FUNCTIONS)

OCTAVE_DEPRECATED ("use 'octave::tree_decl_elt' instead")
typedef octave::tree_decl_elt tree_decl_elt;

// tree_decl_init_list is derived from a template.

OCTAVE_DEPRECATED ("use 'octave::tree_decl_command' instead")
typedef octave::tree_decl_command tree_decl_command;

OCTAVE_DEPRECATED ("use 'octave::tree_global_command' instead")
typedef octave::tree_global_command tree_global_command;

OCTAVE_DEPRECATED ("use 'octave::tree_persistent_command' instead")
typedef octave::tree_persistent_command tree_persistent_command;

#endif

#endif
