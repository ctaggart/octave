//                                  -*- C++ -*-
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

#if !defined (octave_ComplexCHOL_h)
#define octave_ComplexCHOL_h 1

#if defined (__GNUG__)
#pragma interface
#endif

class ostream;

#include "CMatrix.h"

class ComplexCHOL
{
friend class ComplexMatrix;

public:

  ComplexCHOL (void) { }

  ComplexCHOL::ComplexCHOL (const ComplexMatrix& a) { init (a); }

  ComplexCHOL::ComplexCHOL (const ComplexMatrix& a, int& info)
    {
      info = init (a);
    }

  ComplexCHOL::ComplexCHOL (const ComplexCHOL& a)
    {
      chol_mat = a.chol_mat;
    }

  ComplexCHOL& ComplexCHOL::operator = (const ComplexCHOL& a)
    {
      chol_mat = a.chol_mat;

      return *this;
    }

  ComplexMatrix ComplexCHOL::chol_matrix (void) const
    {
      return chol_mat;
    }

  friend ostream& operator << (ostream& os, const ComplexCHOL& a);

private:

  int init (const ComplexMatrix& a);

  ComplexMatrix chol_mat;
};

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
