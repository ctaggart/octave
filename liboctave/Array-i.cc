// Array-i.cc                                            -*- C++ -*-
/*

Copyright (C) 1996 John W. Eaton

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

// Instantiate Arrays of integer values.

#include "Array.h"
#include "Array.cc"

template class Array<int>;

template void assign (Array<int>&, const Array<int>&);
template void assign (Array<int>&, const Array<short>&);
template void assign (Array<int>&, const Array<char>&);

#include "Array2.h"
#include "Array2.cc"

template class Array2<int>;

template void assign (Array2<int>&, const Array2<int>&);
template void assign (Array2<int>&, const Array2<short>&);
template void assign (Array2<int>&, const Array2<char>&);

#include "DiagArray2.h"
#include "DiagArray2.cc"

template class DiagArray2<int>;

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
