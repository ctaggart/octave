/*

Copyright (C) 2007-2019 John W. Eaton

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

#include "oct-time.h"

#include "errwarn.h"
#include "interpreter-private.h"
#include "load-path.h"
#include "ovl.h"
#include "ov.h"
#include "ov-class.h"
#include "ov-typeinfo.h"
#include "ops.h"
#include "symtab.h"
#include "parse.h"

// class ops.

#define DEF_CLASS_UNOP(name)                                            \
  static octave_value                                                   \
  oct_unop_ ## name (const octave_value& a)                             \
  {                                                                     \
    octave_value retval;                                                \
                                                                        \
    std::string class_name = a.class_name ();                           \
                                                                        \
    octave::symbol_table& symtab                                        \
      = octave::__get_symbol_table__ ("oct_unop_" #name);               \
                                                                        \
    octave_value meth = symtab.find_method (#name, class_name);         \
                                                                        \
    if (meth.is_undefined ())                                           \
      error ("%s method not defined for %s class", #name,               \
             class_name.c_str ());                                      \
                                                                        \
    octave_value_list args;                                             \
                                                                        \
    args(0) = a;                                                        \
                                                                        \
    octave_value_list tmp = octave::feval (meth.function_value (), args, 1); \
                                                                        \
    if (tmp.length () > 0)                                              \
      retval = tmp(0);                                                  \
                                                                        \
    return retval;                                                      \
  }

DEF_CLASS_UNOP (not)
DEF_CLASS_UNOP (uplus)
DEF_CLASS_UNOP (uminus)

// In case of (conjugate) transpose first check for overloaded class method.
// If not overloaded, just transpose the underlying map_value, if the number
// of dimensions is maximal two.  Matlab compatibility.

// FIXME: Default transposition for classdef arrays.

#define DEF_CLASS_UNOP_TRANS(name)                                       \
  static octave_value                                                    \
  oct_unop_ ## name (const octave_value& a)                              \
  {                                                                      \
    octave_value retval;                                                 \
                                                                         \
    std::string class_name = a.class_name ();                            \
                                                                         \
    octave::symbol_table& symtab                                         \
      = octave::__get_symbol_table__ ("oct_unop_" #name);                \
                                                                         \
    octave_value meth = symtab.find_method (#name, class_name);          \
                                                                         \
    if (meth.is_undefined ())                                            \
      {                                                                  \
        if (a.ndims () > 2)                                              \
          error ("#name not defined for N-D objects");                   \
                                                                         \
        if (! a.is_classdef_object ())                                   \
          {                                                              \
            const octave_class& v                                        \
              = dynamic_cast<const octave_class&> (a.get_rep ());        \
                                                                         \
            return octave_value (v.map_value ().transpose (),            \
                                 v.class_name (),                        \
                                 v.parent_class_name_list ());           \
          }                                                              \
        else                                                             \
          error ("%s method not defined for %s class", #name,            \
             class_name.c_str ());                                       \
      }                                                                  \
                                                                         \
    octave_value_list args;                                              \
                                                                         \
    args(0) = a;                                                         \
                                                                         \
    octave_value_list tmp = octave::feval (meth.function_value (), args, 1); \
                                                                         \
    if (tmp.length () > 0)                                               \
      retval = tmp(0);                                                   \
                                                                         \
    return retval;                                                       \
  }

DEF_CLASS_UNOP_TRANS (transpose)
DEF_CLASS_UNOP_TRANS (ctranspose)

// The precedence of the oct_binop_*-functions is as follows:
//
// 1.   If exactly one of the arguments is a user defined class object, then
//      the function of that operand's class is invoked.
//
// 2.   If both arguments are user defined class objects, then
// 2.1  The superior class function is invoked.
// 2.2  The leftmost class function is invoked if both classes are the same
//      or their precedence is not defined by superiorto/inferiorto.

#define DEF_CLASS_BINOP(name)                                           \
  static octave_value                                                   \
  oct_binop_ ## name (const octave_value& a1, const octave_value& a2)   \
  {                                                                     \
    octave_value retval;                                                \
                                                                        \
    octave::symbol_table& symtab                                        \
      = octave::__get_symbol_table__ ("oct_binop_" #name);              \
                                                                        \
    std::string dispatch_type = a1.class_name ();                       \
                                                                        \
    if (! a1.isobject ()                                                \
        || (a1.isobject () && a2.isobject ()                            \
            && symtab.is_superiorto (a2.class_name (), dispatch_type))) \
      dispatch_type = a2.class_name ();                                 \
                                                                        \
    octave_value meth = symtab.find_method (#name, dispatch_type);      \
                                                                        \
    if (meth.is_undefined ())                                           \
      error ("%s method not defined for %s class", #name,               \
             dispatch_type.c_str ());                                   \
                                                                        \
    octave_value_list args;                                             \
                                                                        \
    args(1) = a2;                                                       \
    args(0) = a1;                                                       \
                                                                        \
    octave_value_list tmp = octave::feval (meth.function_value (), args, 1); \
                                                                        \
    if (tmp.length () > 0)                                              \
      retval = tmp(0);                                                  \
                                                                        \
    return retval;                                                      \
  }

DEF_CLASS_BINOP (plus)
DEF_CLASS_BINOP (minus)
DEF_CLASS_BINOP (mtimes)
DEF_CLASS_BINOP (mrdivide)
DEF_CLASS_BINOP (mpower)
DEF_CLASS_BINOP (mldivide)
DEF_CLASS_BINOP (lt)
DEF_CLASS_BINOP (le)
DEF_CLASS_BINOP (eq)
DEF_CLASS_BINOP (ge)
DEF_CLASS_BINOP (gt)
DEF_CLASS_BINOP (ne)
DEF_CLASS_BINOP (times)
DEF_CLASS_BINOP (rdivide)
DEF_CLASS_BINOP (power)
DEF_CLASS_BINOP (ldivide)
DEF_CLASS_BINOP (and)
DEF_CLASS_BINOP (or)

#define INSTALL_CLASS_UNOP_TI(ti, op, f)                        \
  ti.install_unary_class_op (octave_value::op, oct_unop_ ## f)

#define INSTALL_CLASS_BINOP_TI(ti, op, f)                       \
  ti.install_binary_class_op (octave_value::op, oct_binop_ ## f)

void
install_class_ops (octave::type_info& ti)
{
  INSTALL_CLASS_UNOP_TI (ti, op_not, not);
  INSTALL_CLASS_UNOP_TI (ti, op_uplus, uplus);
  INSTALL_CLASS_UNOP_TI (ti, op_uminus, uminus);
  INSTALL_CLASS_UNOP_TI (ti, op_transpose, transpose);
  INSTALL_CLASS_UNOP_TI (ti, op_hermitian, ctranspose);

  INSTALL_CLASS_BINOP_TI (ti, op_add, plus);
  INSTALL_CLASS_BINOP_TI (ti, op_sub, minus);
  INSTALL_CLASS_BINOP_TI (ti, op_mul, mtimes);
  INSTALL_CLASS_BINOP_TI (ti, op_div, mrdivide);
  INSTALL_CLASS_BINOP_TI (ti, op_pow, mpower);
  INSTALL_CLASS_BINOP_TI (ti, op_ldiv, mldivide);
  INSTALL_CLASS_BINOP_TI (ti, op_lt, lt);
  INSTALL_CLASS_BINOP_TI (ti, op_le, le);
  INSTALL_CLASS_BINOP_TI (ti, op_eq, eq);
  INSTALL_CLASS_BINOP_TI (ti, op_ge, ge);
  INSTALL_CLASS_BINOP_TI (ti, op_gt, gt);
  INSTALL_CLASS_BINOP_TI (ti, op_ne, ne);
  INSTALL_CLASS_BINOP_TI (ti, op_el_mul, times);
  INSTALL_CLASS_BINOP_TI (ti, op_el_div, rdivide);
  INSTALL_CLASS_BINOP_TI (ti, op_el_pow, power);
  INSTALL_CLASS_BINOP_TI (ti, op_el_ldiv, ldivide);
  INSTALL_CLASS_BINOP_TI (ti, op_el_and, and);
  INSTALL_CLASS_BINOP_TI (ti, op_el_or, or);
}
