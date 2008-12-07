/*

Copyright (C) 1993, 1994, 1995, 1996, 1997, 1999, 2000, 2001, 2002,
              2003, 2004, 2005, 2006, 2007 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#if !defined (octave_mx_inlines_h)
#define octave_mx_inlines_h 1

#include <cstddef>

#include "quit.h"

#include "oct-cmplx.h"

template <class R, class S>
inline void
mx_inline_fill_vs (R *r, size_t n, S s)
{
  for (size_t i = 0; i < n; i++)
    r[i] = s;
}

#define VS_OP_FCN(F, OP) \
  template <class R, class V, class S> \
  inline void \
  F ## _vs (R *r, const V *v, size_t n, S s) \
  { \
    for (size_t i = 0; i < n; i++) \
      r[i] = v[i] OP s; \
  }

VS_OP_FCN (mx_inline_add,      +)
VS_OP_FCN (mx_inline_subtract, -)
VS_OP_FCN (mx_inline_multiply, *)
VS_OP_FCN (mx_inline_divide,   /)

#define VS_OP(F, OP, R, V, S) \
  static inline R * \
  F (const V *v, size_t n, S s) \
  { \
    R *r = 0; \
    if (n > 0) \
      { \
	r = new R [n]; \
	F ## _vs (r, v, n, s); \
      } \
    return r; \
  }

#define VS_OPS(R, V, S) \
  VS_OP (mx_inline_add,      +, R, V, S) \
  VS_OP (mx_inline_subtract, -, R, V, S) \
  VS_OP (mx_inline_multiply, *, R, V, S) \
  VS_OP (mx_inline_divide,   /, R, V, S)

VS_OPS (double,  double,  double)
VS_OPS (Complex, double,  Complex)
VS_OPS (Complex, Complex, double)
VS_OPS (Complex, Complex, Complex)

VS_OPS (float,  float,  float)
VS_OPS (FloatComplex, float,  FloatComplex)
VS_OPS (FloatComplex, FloatComplex, float)
VS_OPS (FloatComplex, FloatComplex, FloatComplex)

#define SV_OP_FCN(F, OP) \
  template <class R, class S, class V> \
  inline void \
  F ## _sv (R *r, S s, const V *v, size_t n) \
  { \
    for (size_t i = 0; i < n; i++) \
      r[i] = s OP v[i]; \
  } \

SV_OP_FCN (mx_inline_add,      +)
SV_OP_FCN (mx_inline_subtract, -)
SV_OP_FCN (mx_inline_multiply, *)
SV_OP_FCN (mx_inline_divide,   /)

#define SV_OP(F, OP, R, S, V) \
  static inline R * \
  F (S s, const V *v, size_t n) \
  { \
    R *r = 0; \
    if (n > 0) \
      { \
	r = new R [n]; \
        F ## _sv (r, s, v, n); \
      } \
    return r; \
  }

#define SV_OPS(R, S, V) \
  SV_OP (mx_inline_add,      +, R, S, V) \
  SV_OP (mx_inline_subtract, -, R, S, V) \
  SV_OP (mx_inline_multiply, *, R, S, V) \
  SV_OP (mx_inline_divide,   /, R, S, V)

SV_OPS (double,  double,  double)
SV_OPS (Complex, double,  Complex)
SV_OPS (Complex, Complex, double)
SV_OPS (Complex, Complex, Complex)

SV_OPS (float,  float,  float)
SV_OPS (FloatComplex, float,  FloatComplex)
SV_OPS (FloatComplex, FloatComplex, float)
SV_OPS (FloatComplex, FloatComplex, FloatComplex)

#define VV_OP_FCN(F, OP) \
  template <class R, class T1, class T2> \
  inline void \
  F ## _vv (R *r, const T1 *v1, const T2 *v2, size_t n) \
  { \
    for (size_t i = 0; i < n; i++) \
      r[i] = v1[i] OP v2[i]; \
  } \

VV_OP_FCN (mx_inline_add,      +)
VV_OP_FCN (mx_inline_subtract, -)
VV_OP_FCN (mx_inline_multiply, *)
VV_OP_FCN (mx_inline_divide,   /)

#define VV_OP(F, OP, R, T1, T2) \
  static inline R * \
  F (const T1 *v1, const T2 *v2, size_t n) \
  { \
    R *r = 0; \
    if (n > 0) \
      { \
	r = new R [n]; \
	F ## _vv (r, v1, v2, n); \
      } \
    return r; \
  }

#define VV_OPS(R, T1, T2) \
  VV_OP (mx_inline_add,      +, R, T1, T2) \
  VV_OP (mx_inline_subtract, -, R, T1, T2) \
  VV_OP (mx_inline_multiply, *, R, T1, T2) \
  VV_OP (mx_inline_divide,   /, R, T1, T2)

VV_OPS (double,  double,  double)
VV_OPS (Complex, double,  Complex)
VV_OPS (Complex, Complex, double)
VV_OPS (Complex, Complex, Complex)

VV_OPS (float,  float,  float)
VV_OPS (FloatComplex, float,  FloatComplex)
VV_OPS (FloatComplex, FloatComplex, float)
VV_OPS (FloatComplex, FloatComplex, FloatComplex)

#define VS_OP2(F, OP, V, S) \
  static inline V * \
  F (V *v, size_t n, S s) \
  { \
    for (size_t i = 0; i < n; i++) \
      v[i] OP s; \
    return v; \
  }

#define VS_OP2S(V, S) \
  VS_OP2 (mx_inline_add2,      +=, V, S) \
  VS_OP2 (mx_inline_subtract2, -=, V, S) \
  VS_OP2 (mx_inline_multiply2, *=, V, S) \
  VS_OP2 (mx_inline_divide2,   /=, V, S) \
  VS_OP2 (mx_inline_copy,       =, V, S)

VS_OP2S (double,  double)
VS_OP2S (Complex, double)
VS_OP2S (Complex, Complex)

VS_OP2S (float,  float)
VS_OP2S (FloatComplex, float)
VS_OP2S (FloatComplex, FloatComplex)

#define VV_OP2(F, OP, T1, T2) \
  static inline T1 * \
  F (T1 *v1, const T2 *v2, size_t n) \
  { \
    for (size_t i = 0; i < n; i++) \
      v1[i] OP v2[i]; \
    return v1; \
  }

#define VV_OP2S(T1, T2) \
  VV_OP2 (mx_inline_add2,      +=, T1, T2) \
  VV_OP2 (mx_inline_subtract2, -=, T1, T2) \
  VV_OP2 (mx_inline_multiply2, *=, T1, T2) \
  VV_OP2 (mx_inline_divide2,   /=, T1, T2) \
  VV_OP2 (mx_inline_copy,       =, T1, T2)

VV_OP2S (double,  double)
VV_OP2S (Complex, double)
VV_OP2S (Complex, Complex)

VV_OP2S (float,  float)
VV_OP2S (FloatComplex, float)
VV_OP2S (FloatComplex, FloatComplex)

#define OP_EQ_FCN(T1, T2) \
  static inline bool \
  mx_inline_equal (const T1 *x, const T2 *y, size_t n) \
  { \
    for (size_t i = 0; i < n; i++) \
      if (x[i] != y[i]) \
	return false; \
    return true; \
  }

OP_EQ_FCN (bool,    bool)
OP_EQ_FCN (char,    char)
OP_EQ_FCN (double,  double)
OP_EQ_FCN (Complex, Complex)
OP_EQ_FCN (float,  float)
OP_EQ_FCN (FloatComplex, FloatComplex)

#define OP_DUP_FCN(OP, F, R, T) \
  static inline R * \
  F (const T *x, size_t n) \
  { \
    R *r = 0; \
    if (n > 0) \
      { \
	r = new R [n]; \
	for (size_t i = 0; i < n; i++) \
	  r[i] = OP (x[i]); \
      } \
    return r; \
  }

OP_DUP_FCN (, mx_inline_dup, double,  double)
OP_DUP_FCN (, mx_inline_dup, Complex, Complex)
OP_DUP_FCN (, mx_inline_dup, float, float)
OP_DUP_FCN (, mx_inline_dup, FloatComplex, FloatComplex)

// These should really return a bool *.  Also, they should probably be
// in with a collection of other element-by-element boolean ops.
OP_DUP_FCN (0.0 ==, mx_inline_not, double, double)
OP_DUP_FCN (0.0 ==, mx_inline_not, double, Complex)

OP_DUP_FCN (, mx_inline_make_complex, Complex, double)

OP_DUP_FCN (-, mx_inline_change_sign, double,  double)
OP_DUP_FCN (-, mx_inline_change_sign, Complex, Complex)

OP_DUP_FCN (real, mx_inline_real_dup, double,  Complex)
OP_DUP_FCN (imag, mx_inline_imag_dup, double,  Complex)
OP_DUP_FCN (conj, mx_inline_conj_dup, Complex, Complex)

OP_DUP_FCN (0.0 ==, mx_inline_not, float, float)
OP_DUP_FCN (static_cast<float>(0.0) ==, mx_inline_not, float, FloatComplex)

OP_DUP_FCN (, mx_inline_make_complex, FloatComplex, float)

OP_DUP_FCN (-, mx_inline_change_sign, float,  float)
OP_DUP_FCN (-, mx_inline_change_sign, FloatComplex, FloatComplex)

OP_DUP_FCN (real, mx_inline_real_dup, float,  FloatComplex)
OP_DUP_FCN (imag, mx_inline_imag_dup, float,  FloatComplex)
OP_DUP_FCN (conj, mx_inline_conj_dup, FloatComplex, FloatComplex)

// Avoid some code duplication.  Maybe we should use templates.

#define MX_CUMULATIVE_OP(RET_TYPE, ELT_TYPE, OP) \
 \
  octave_idx_type nr = rows (); \
  octave_idx_type nc = cols (); \
 \
  RET_TYPE retval (nr, nc); \
 \
  if (nr > 0 && nc > 0) \
    { \
      if ((nr == 1 && dim == -1) || dim == 1) \
	{ \
	  for (octave_idx_type i = 0; i < nr; i++) \
	    { \
	      ELT_TYPE t = elem (i, 0); \
	      for (octave_idx_type j = 0; j < nc; j++) \
		{ \
		  retval.elem (i, j) = t; \
		  if (j < nc - 1) \
		    t OP elem (i, j+1); \
		} \
	    } \
	} \
      else \
	{ \
	  for (octave_idx_type j = 0; j < nc; j++) \
	    { \
	      ELT_TYPE t = elem (0, j); \
	      for (octave_idx_type i = 0; i < nr; i++) \
		{ \
		  retval.elem (i, j) = t; \
		  if (i < nr - 1) \
		    t OP elem (i+1, j); \
		} \
	    } \
	} \
    } \
 \
  return retval

#define MX_BASE_REDUCTION_OP(RET_TYPE, ROW_EXPR, COL_EXPR, INIT_VAL, \
			     MT_RESULT) \
 \
  octave_idx_type nr = rows (); \
  octave_idx_type nc = cols (); \
 \
  RET_TYPE retval; \
 \
  if (nr > 0 && nc > 0) \
    { \
      if ((nr == 1 && dim == -1) || dim == 1) \
	{ \
	  retval.resize (nr, 1); \
	  for (octave_idx_type i = 0; i < nr; i++) \
	    { \
	      retval.elem (i, 0) = INIT_VAL; \
	      for (octave_idx_type j = 0; j < nc; j++) \
		{ \
		  ROW_EXPR; \
		} \
	    } \
	} \
      else \
	{ \
	  retval.resize (1, nc); \
	  for (octave_idx_type j = 0; j < nc; j++) \
	    { \
	      retval.elem (0, j) = INIT_VAL; \
	      for (octave_idx_type i = 0; i < nr; i++) \
		{ \
		  COL_EXPR; \
		} \
	    } \
	} \
    } \
  else if (nc == 0 && (nr == 0 || (nr == 1 && dim == -1))) \
    retval.resize (1, 1, MT_RESULT); \
  else if (nr == 0 && (dim == 0 || dim == -1)) \
    retval.resize (1, nc, MT_RESULT); \
  else if (nc == 0 && dim == 1) \
    retval.resize (nr, 1, MT_RESULT); \
  else \
    retval.resize (nr > 0, nc > 0); \
 \
  return retval

#define MX_REDUCTION_OP_ROW_EXPR(OP) \
  retval.elem (i, 0) OP elem (i, j)

#define MX_REDUCTION_OP_COL_EXPR(OP) \
  retval.elem (0, j) OP elem (i, j)

#define MX_REDUCTION_OP(RET_TYPE, OP, INIT_VAL, MT_RESULT) \
  MX_BASE_REDUCTION_OP (RET_TYPE, \
			MX_REDUCTION_OP_ROW_EXPR (OP), \
			MX_REDUCTION_OP_COL_EXPR (OP), \
			INIT_VAL, MT_RESULT)

#define MX_ANY_ALL_OP_ROW_CODE(TEST_OP, TEST_TRUE_VAL) \
  if (elem (i, j) TEST_OP 0.0) \
    { \
      retval.elem (i, 0) = TEST_TRUE_VAL; \
      break; \
    }

#define MX_ANY_ALL_OP_COL_CODE(TEST_OP, TEST_TRUE_VAL) \
  if (elem (i, j) TEST_OP 0.0) \
    { \
      retval.elem (0, j) = TEST_TRUE_VAL; \
      break; \
    }

#define MX_ANY_ALL_OP(DIM, INIT_VAL, TEST_OP, TEST_TRUE_VAL) \
  MX_BASE_REDUCTION_OP (boolMatrix, \
			MX_ANY_ALL_OP_ROW_CODE (TEST_OP, TEST_TRUE_VAL), \
			MX_ANY_ALL_OP_COL_CODE (TEST_OP, TEST_TRUE_VAL), \
			INIT_VAL, INIT_VAL)

#define MX_ALL_OP(DIM) MX_ANY_ALL_OP (DIM, true, ==, false)

#define MX_ANY_OP(DIM) MX_ANY_ALL_OP (DIM, false, !=, true)

#define MX_ND_ALL_EXPR elem (iter_idx) == 0 

#define MX_ND_ANY_EXPR elem (iter_idx) != 0

#define MX_ND_ALL_EVAL(TEST_EXPR) \
 if (retval(result_idx) && (TEST_EXPR)) \
   retval(result_idx) = 0;

#define MX_ND_ANY_EVAL(TEST_EXPR) \
 if (retval(result_idx) || (TEST_EXPR)) \
   retval(result_idx) = 1;
 
#define MX_ND_REDUCTION(EVAL_EXPR, INIT_VAL, RET_TYPE) \
 \
  RET_TYPE retval; \
 \
  dim_vector dv = this->dims (); \
  int nd = this->ndims (); \
 \
  int empty = false; \
 \
  for (int i = 0; i < nd; i++) \
    { \
      if (dv(i) == 0) \
        { \
          empty = true; \
          break; \
        } \
    } \
 \
  if (nd == 2 && dv(0) == 0 && dv(1) == 0) \
    { \
      retval.resize (dim_vector (1, 1), INIT_VAL); \
      return retval; \
    } \
 \
  /* We need to find first non-singleton dim.  */ \
 \
  if (dim == -1) \
    { \
      dim = 0; \
 \
      for (int i = 0; i < nd; i++) \
        { \
	  if (dv(i) != 1) \
	    { \
	      dim = i; \
	      break; \
	    } \
        } \
    } \
  else if (dim >= nd) \
    { \
      dim = nd++; \
      dv.resize (nd, 1); \
    } \
 \
  /* R = op (A, DIM) \
 \
     The strategy here is to access the elements of A along the \
     dimension  specified by DIM.  This means that we loop over each \
     element of R and adjust the index into A as needed.  Store the \
     cummulative product of all dimensions of A in CP_SZ.  The last \
     element of CP_SZ is the total number of elements of A.  */ \
 \
  Array<octave_idx_type> cp_sz (nd+1, 1); \
  for (int i = 1; i <= nd; i++) \
    cp_sz(i) = cp_sz(i-1)*dv(i-1); \
 \
  octave_idx_type reset_at = cp_sz(dim); \
  octave_idx_type base_incr = cp_sz(dim+1); \
  octave_idx_type incr = cp_sz(dim); \
  octave_idx_type base = 0; \
  octave_idx_type next_base = base + base_incr; \
  octave_idx_type iter_idx = base; \
  octave_idx_type n_elts = dv(dim); \
 \
  dv(dim) = 1; \
 \
  retval.resize (dv, INIT_VAL); \
 \
  if (! empty) \
    { \
      octave_idx_type nel = dv.numel (); \
 \
      octave_idx_type k = 1; \
 \
      for (octave_idx_type result_idx = 0; result_idx < nel; result_idx++) \
	{ \
	  OCTAVE_QUIT; \
 \
          for (octave_idx_type j = 0; j < n_elts; j++) \
	    { \
              OCTAVE_QUIT; \
 \
	      EVAL_EXPR; \
 \
	      iter_idx += incr; \
	    } \
 \
          if (k == reset_at) \
	    { \
	      base = next_base; \
	      next_base += base_incr; \
	      iter_idx = base; \
	      k = 1; \
	    } \
	  else \
	    { \
	      base++; \
	      iter_idx = base; \
	      k++; \
	     } \
	} \
    } \
 \
  retval.chop_trailing_singletons (); \
 \
  return retval

#define MX_ND_REAL_OP_REDUCTION(ASN_EXPR, INIT_VAL) \
  MX_ND_REDUCTION (retval(result_idx) ASN_EXPR, INIT_VAL, NDArray)

#define MX_ND_COMPLEX_OP_REDUCTION(ASN_EXPR, INIT_VAL) \
  MX_ND_REDUCTION (retval(result_idx) ASN_EXPR, INIT_VAL, ComplexNDArray)

#define MX_ND_ANY_ALL_REDUCTION(EVAL_EXPR, VAL) \
  MX_ND_REDUCTION (EVAL_EXPR, VAL, boolNDArray)

#define MX_ND_CUMULATIVE_OP(RET_TYPE, ACC_TYPE, INIT_VAL, OP) \
 \
  RET_TYPE retval; \
 \
  dim_vector dv = this->dims (); \
  int nd = this->ndims (); \
 \
  bool empty = false; \
 \
  for (int i = 0; i < nd; i++) \
    { \
      if (dv(i) == 0) \
        { \
          empty = true; \
          break; \
        } \
    } \
 \
  /* We need to find first non-singleton dim.  */ \
 \
  if (dim == -1) \
    { \
      dim = 0; \
 \
      for (int i = 0; i < nd; i++) \
        { \
	  if (dv(i) != 1) \
	    { \
	      dim = i; \
	      break; \
	    } \
        } \
    } \
  else if (dim >= nd) \
    { \
      dim = nd++; \
      dv.resize (nd, 1); \
    } \
 \
  /* R = op (A, DIM) \
 \
     The strategy here is to access the elements of A along the \
     dimension  specified by DIM.  This means that we loop over each \
     element of R and adjust the index into A as needed.  Store the \
     cummulative product of all dimensions of A in CP_SZ.  The last \
     element of CP_SZ is the total number of elements of A.  */ \
 \
  Array<octave_idx_type> cp_sz (nd+1, 1); \
  for (int i = 1; i <= nd; i++) \
    cp_sz(i) = cp_sz(i-1)*dv(i-1); \
 \
  octave_idx_type reset_at = cp_sz(dim); \
  octave_idx_type base_incr = cp_sz(dim+1); \
  octave_idx_type incr = cp_sz(dim); \
  octave_idx_type base = 0; \
  octave_idx_type next_base = base + base_incr; \
  octave_idx_type iter_idx = base; \
  octave_idx_type n_elts = dv(dim); \
 \
  retval.resize (dv, INIT_VAL); \
 \
  if (! empty) \
    { \
      octave_idx_type nel = dv.numel () / n_elts; \
 \
      octave_idx_type k = 1; \
 \
      for (octave_idx_type i = 0; i < nel; i++) \
	{ \
	  OCTAVE_QUIT; \
 \
          ACC_TYPE prev_val = INIT_VAL; \
 \
	  for (octave_idx_type j = 0; j < n_elts; j++) \
	    { \
	      OCTAVE_QUIT; \
 \
	      if (j == 0) \
		{ \
		  retval(iter_idx) = elem (iter_idx); \
		  prev_val = retval(iter_idx); \
		} \
	      else \
		{ \
		  prev_val = prev_val OP elem (iter_idx); \
		  retval(iter_idx) = prev_val; \
		} \
 \
	      iter_idx += incr; \
	    } \
 \
	  if (k == reset_at) \
	    { \
	      base = next_base; \
	      next_base += base_incr; \
	      iter_idx = base; \
	      k = 1; \
	    } \
	  else \
	    { \
	      base++; \
	      iter_idx = base; \
	      k++; \
	     } \
	} \
    } \
 \
  retval.chop_trailing_singletons (); \
 \
  return retval

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
