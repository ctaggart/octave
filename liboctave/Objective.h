// Objective.h                                          -*- C++ -*-
/*

Copyright (C) 1992, 1993, 1994, 1995 John W. Eaton

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

#if !defined (octave_Objective_h)
#define octave_Objective_h 1

#if defined (__GNUG__)
#pragma interface
#endif

#include "dColVector.h"

#ifndef Vector
#define Vector ColumnVector
#endif

typedef double (*objective_fcn) (const Vector&);
typedef Vector (*gradient_fcn) (const Vector&);

class Objective
{
 public:

  Objective (void);
  Objective (const objective_fcn);
  Objective (const objective_fcn, const gradient_fcn);

  Objective (const Objective& a);

  Objective& operator = (const Objective& a);

  objective_fcn objective_function (void) const;

  Objective& set_objective_function (const objective_fcn);

  gradient_fcn gradient_function (void) const;

  Objective& set_gradient_function (const gradient_fcn);

 private:

  objective_fcn phi;
  gradient_fcn grad;

};

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
