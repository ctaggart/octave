// xdiv.h                                               -*- C++ -*-
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

#if !defined (octave_xdiv_h)
#define octave_xdiv_h 1

class Complex;
class Matrix;
class ComplexMatrix;
class tree_constant;

extern tree_constant xdiv (const Matrix& a, const Matrix& b);
extern tree_constant xdiv (const Matrix& a, const ComplexMatrix& b);
extern tree_constant xdiv (const ComplexMatrix& a, const Matrix& b);
extern tree_constant xdiv (const ComplexMatrix& a, const ComplexMatrix& b);

extern tree_constant x_el_div (double a, const Matrix& b);
extern tree_constant x_el_div (double a, const ComplexMatrix& b);
extern tree_constant x_el_div (const Complex a, const Matrix& b);
extern tree_constant x_el_div (const Complex a, const ComplexMatrix& b);

extern tree_constant xleftdiv (const Matrix& a, const Matrix& b);
extern tree_constant xleftdiv (const Matrix& a, const ComplexMatrix& b);
extern tree_constant xleftdiv (const ComplexMatrix& a, const Matrix& b);
extern tree_constant xleftdiv (const ComplexMatrix& a, const ComplexMatrix& b);

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
