// pr-output.h                                               -*- C++ -*-
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

#if !defined (octave_pr_output_h)
#define octave_pr_output_h 1

class ostream;

class Matrix;
class Complex;
class ComplexMatrix;
class charMatrix;
class Range;

extern void octave_print_internal (ostream& os, double d,
				   int pr_as_read_syntax = 0);

extern void octave_print_internal (ostream& os, const Matrix& m,
				   int pr_as_read_syntax = 0);

extern void octave_print_internal (ostream& os, const Complex& c,
				   int pr_as_read_syntax = 0);

extern void octave_print_internal (ostream& os, const ComplexMatrix& cm,
				   int pr_as_read_syntax = 0);

extern void octave_print_internal (ostream& os, const Range& r,
				   int pr_as_read_syntax = 0);

extern void octave_print_internal (ostream& os, const charMatrix& chm,
				   int pr_as_read_syntax = 0,
				   int pr_as_string = 0);

extern void set_format_style (int argc, char **argv);

// XXX FIXME XXX -- these should probably be somewhere else.

extern int any_element_is_inf_or_nan (const Matrix& a);

extern int any_element_is_inf_or_nan (const ComplexMatrix& a);

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
