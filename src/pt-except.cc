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
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#if defined (__GNUG__)
#pragma implementation
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Nonzero means we're breaking out of a loop or function body.
extern int breaking;

// Nonzero means we're jumping to the end of a loop.
extern int continuing;

// Nonzero means we're returning from a function.  Global because it
// is also needed in tree-expr.cc.
extern int returning;

#include "error.h"
#include "oct-lvalue.h"
#include "ov.h"
#include "pt-cmd.h"
#include "pt-except.h"
#include "pt-exp.h"
#include "pt-stmt.h"
#include "pt-walk.h"
#include "unwind-prot.h"
#include "variables.h"

// Simple exception handling.

tree_try_catch_command::~tree_try_catch_command (void)
{
  delete try_code;
  delete catch_code;
}

static void
do_catch_code (void *ptr)
{
  tree_statement_list *list = static_cast<tree_statement_list *> (ptr);

  // Set up for letting the user print any messages from errors that
  // occurred in the body of the try_catch statement.

  buffer_error_messages = 0;
  bind_global_error_variable ();
  add_unwind_protect (clear_global_error_variable, 0);

  // Similarly, if we have seen a return or break statement, allow all
  // the catch code to run before returning or handling the break.
  // We don't have to worry about continue statements because they can
  // only occur in loops.

  unwind_protect_int (returning);
  returning = 0;

  unwind_protect_int (breaking);
  breaking = 0;

  if (list)
    list->eval ();

  // This is the one for breaking.  (The unwind_protects are popped
  // off the stack in the reverse of the order they are pushed on).

  // XXX FIXME XXX -- inside a try-catch, should break work like
  // a return, or just jump to the end of the try_catch block?
  // The following code makes it just jump to the end of the block.

  run_unwind_protect ();
  if (breaking)
    breaking--;

  // This is the one for returning.

  if (returning)
    discard_unwind_protect ();
  else
    run_unwind_protect ();

  run_unwind_protect ();
}

void
tree_try_catch_command::eval (void)
{
  begin_unwind_frame ("tree_try_catch::eval");

  add_unwind_protect (do_catch_code, catch_code);

  if (catch_code)
    {
      unwind_protect_int (buffer_error_messages);
      buffer_error_messages = 1;
    }

  if (try_code)
    try_code->eval ();

  if (catch_code && error_state)
    {
      error_state = 0;
      run_unwind_frame ("tree_try_catch::eval");
    }
  else
    {
      error_state = 0;
      discard_unwind_frame ("tree_try_catch::eval");
    }
}

void
tree_try_catch_command::accept (tree_walker& tw)
{
  tw.visit_try_catch_command (*this);
}

// Simple exception handling.

tree_unwind_protect_command::~tree_unwind_protect_command (void)
{
  delete unwind_protect_code;
  delete cleanup_code;
}

static void
do_unwind_protect_cleanup_code (void *ptr)
{
  tree_statement_list *list = static_cast<tree_statement_list *> (ptr);

  // We want to run the cleanup code without error_state being set,
  // but we need to restore its value, so that any errors encountered
  // in the first part of the unwind_protect are not completely
  // ignored.

  unwind_protect_int (error_state);
  error_state = 0;

  // Similarly, if we have seen a return or break statement, allow all
  // the cleanup code to run before returning or handling the break.
  // We don't have to worry about continue statements because they can
  // only occur in loops.

  unwind_protect_int (returning);
  returning = 0;

  unwind_protect_int (breaking);
  breaking = 0;

  if (list)
    list->eval ();

  // This is the one for breaking.  (The unwind_protects are popped
  // off the stack in the reverse of the order they are pushed on).

  // XXX FIXME XXX -- inside an unwind_protect, should break work like
  // a return, or just jump to the end of the unwind_protect block?
  // The following code makes it just jump to the end of the block.

  run_unwind_protect ();
  if (breaking)
    breaking--;

  // This is the one for returning.

  if (returning)
    discard_unwind_protect ();
  else
    run_unwind_protect ();

  // We don't want to ignore errors that occur in the cleanup code, so
  // if an error is encountered there, leave error_state alone.
  // Otherwise, set it back to what it was before.

  if (error_state)
    discard_unwind_protect ();
  else
    run_unwind_protect ();
}

void
tree_unwind_protect_command::eval (void)
{
  add_unwind_protect (do_unwind_protect_cleanup_code, cleanup_code);

  if (unwind_protect_code)
    unwind_protect_code->eval ();

  run_unwind_protect ();
}

void
tree_unwind_protect_command::accept (tree_walker& tw)
{
  tw.visit_unwind_protect_command (*this);
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
