/*

Copyright (C) 2004 David Bateman
Copyright (C) 1998-2004 Andy Adler

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/


#if !defined (octave_sparse_base_lu_h)
#define octave_sparse_base_lu_h 1

#include "MArray.h"

template <class lu_type, class lu_elt_type, class p_type, class p_elt_type>
class
sparse_base_lu
{
public:

  sparse_base_lu (void) { }

  sparse_base_lu (const sparse_base_lu& a) 
    : Lfact (a.Lfact), Ufact (a.Ufact), cond (a.cond), P (a.P), Q (a.Q) { }

  sparse_base_lu& operator = (const sparse_base_lu& a)
    {
      if (this != &a)
	{
	  Lfact = a.Lfact;
	  Ufact = a.Ufact;
	  cond = a.cond;
	  P = a.P;
	  Q = a.Q;
	}
      return *this;
    }

  ~sparse_base_lu (void) { }

  lu_type L (void) const { return Lfact; }

  lu_type U (void) const { return Ufact; }

  p_type Pc (void) const;

  p_type Pr (void) const;

  MArray<int> row_perm (void) const { return P; }

  MArray<int> col_perm (void) const { return Q; }

  double rcond (void) const { return cond; }

protected:

  lu_type Lfact;
  lu_type Ufact;

  double cond;

  MArray<int> P;
  MArray<int> Q;
};

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
