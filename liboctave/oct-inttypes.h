/*

Copyright (C) 2004 John W. Eaton

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

#if !defined (octave_inttypes_h)
#define octave_inttypes_h 1

#include <limits>

#include <iostream>

#include "data-conv.h"

typedef signed char octave_int8_t;
typedef TWO_BYTE_INT octave_int16_t;
typedef FOUR_BYTE_INT octave_int32_t;
typedef EIGHT_BYTE_INT octave_int64_t;

typedef unsigned char octave_uint8_t;
typedef unsigned TWO_BYTE_INT octave_uint16_t;
typedef unsigned FOUR_BYTE_INT octave_uint32_t;
typedef unsigned EIGHT_BYTE_INT octave_uint64_t;

template <class T1, class T2>
class
octave_int_binop_traits
{
public:
  // The return type for a T1 by T2 binary operation.
  typedef T1 TR;
};

#define OCTAVE_INT_BINOP_TRAIT(T1, T2, T3) \
  template<> \
  class octave_int_binop_traits <T1, T2> \
  { \
  public: \
    typedef T3 TR; \
  }

OCTAVE_INT_BINOP_TRAIT (octave_int8_t, octave_int8_t, octave_int8_t);
OCTAVE_INT_BINOP_TRAIT (octave_int8_t, octave_int16_t, octave_int8_t);
OCTAVE_INT_BINOP_TRAIT (octave_int8_t, octave_int32_t, octave_int8_t);
OCTAVE_INT_BINOP_TRAIT (octave_int8_t, octave_int64_t, octave_int8_t);
OCTAVE_INT_BINOP_TRAIT (octave_int8_t, octave_uint8_t, octave_int8_t);
OCTAVE_INT_BINOP_TRAIT (octave_int8_t, octave_uint16_t, octave_int8_t);
OCTAVE_INT_BINOP_TRAIT (octave_int8_t, octave_uint32_t, octave_int8_t);
OCTAVE_INT_BINOP_TRAIT (octave_int8_t, octave_uint64_t, octave_int8_t);

OCTAVE_INT_BINOP_TRAIT (octave_int16_t, octave_int8_t, octave_int16_t);
OCTAVE_INT_BINOP_TRAIT (octave_int16_t, octave_int16_t, octave_int16_t);
OCTAVE_INT_BINOP_TRAIT (octave_int16_t, octave_int32_t, octave_int16_t);
OCTAVE_INT_BINOP_TRAIT (octave_int16_t, octave_int64_t, octave_int16_t);
OCTAVE_INT_BINOP_TRAIT (octave_int16_t, octave_uint8_t, octave_int16_t);
OCTAVE_INT_BINOP_TRAIT (octave_int16_t, octave_uint16_t, octave_int16_t);
OCTAVE_INT_BINOP_TRAIT (octave_int16_t, octave_uint32_t, octave_int16_t);
OCTAVE_INT_BINOP_TRAIT (octave_int16_t, octave_uint64_t, octave_int16_t);

OCTAVE_INT_BINOP_TRAIT (octave_int32_t, octave_int8_t, octave_int32_t);
OCTAVE_INT_BINOP_TRAIT (octave_int32_t, octave_int16_t, octave_int32_t);
OCTAVE_INT_BINOP_TRAIT (octave_int32_t, octave_int32_t, octave_int32_t);
OCTAVE_INT_BINOP_TRAIT (octave_int32_t, octave_int64_t, octave_int32_t);
OCTAVE_INT_BINOP_TRAIT (octave_int32_t, octave_uint8_t, octave_int32_t);
OCTAVE_INT_BINOP_TRAIT (octave_int32_t, octave_uint16_t, octave_int32_t);
OCTAVE_INT_BINOP_TRAIT (octave_int32_t, octave_uint32_t, octave_int32_t);
OCTAVE_INT_BINOP_TRAIT (octave_int32_t, octave_uint64_t, octave_int32_t);

OCTAVE_INT_BINOP_TRAIT (octave_int64_t, octave_int8_t, octave_int64_t);
OCTAVE_INT_BINOP_TRAIT (octave_int64_t, octave_int16_t, octave_int64_t);
OCTAVE_INT_BINOP_TRAIT (octave_int64_t, octave_int32_t, octave_int64_t);
OCTAVE_INT_BINOP_TRAIT (octave_int64_t, octave_int64_t, octave_int64_t);
OCTAVE_INT_BINOP_TRAIT (octave_int64_t, octave_uint8_t, octave_int64_t);
OCTAVE_INT_BINOP_TRAIT (octave_int64_t, octave_uint16_t, octave_int64_t);
OCTAVE_INT_BINOP_TRAIT (octave_int64_t, octave_uint32_t, octave_int64_t);
OCTAVE_INT_BINOP_TRAIT (octave_int64_t, octave_uint64_t, octave_int64_t);

OCTAVE_INT_BINOP_TRAIT (octave_uint8_t, octave_int8_t, octave_int8_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint8_t, octave_int16_t, octave_int8_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint8_t, octave_int32_t, octave_int8_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint8_t, octave_int64_t, octave_int8_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint8_t, octave_uint8_t, octave_uint8_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint8_t, octave_uint16_t, octave_uint8_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint8_t, octave_uint32_t, octave_uint8_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint8_t, octave_uint64_t, octave_uint8_t);

OCTAVE_INT_BINOP_TRAIT (octave_uint16_t, octave_int8_t, octave_int16_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint16_t, octave_int16_t, octave_int16_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint16_t, octave_int32_t, octave_int16_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint16_t, octave_int64_t, octave_int16_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint16_t, octave_uint8_t, octave_uint16_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint16_t, octave_uint16_t, octave_uint16_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint16_t, octave_uint32_t, octave_uint16_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint16_t, octave_uint64_t, octave_uint16_t);

OCTAVE_INT_BINOP_TRAIT (octave_uint32_t, octave_int8_t, octave_int32_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint32_t, octave_int16_t, octave_int32_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint32_t, octave_int32_t, octave_int32_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint32_t, octave_int64_t, octave_int32_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint32_t, octave_uint8_t, octave_uint32_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint32_t, octave_uint16_t, octave_uint32_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint32_t, octave_uint32_t, octave_uint32_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint32_t, octave_uint64_t, octave_uint32_t);

OCTAVE_INT_BINOP_TRAIT (octave_uint64_t, octave_int8_t, octave_int64_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint64_t, octave_int16_t, octave_int64_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint64_t, octave_int32_t, octave_int64_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint64_t, octave_int64_t, octave_int64_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint64_t, octave_uint8_t, octave_uint64_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint64_t, octave_uint16_t, octave_uint64_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint64_t, octave_uint32_t, octave_uint64_t);
OCTAVE_INT_BINOP_TRAIT (octave_uint64_t, octave_uint64_t, octave_uint64_t);

template <class T1, class T2>
inline T2
octave_int_fit_to_range (const T1& x, const T2& mn, const T2& mx)
{
  return (x > mx ? mx : (x < mn ? mn : static_cast<T2> (x)));
}

#define OCTAVE_INT_FIT_TO_RANGE(r, T) \
  octave_int_fit_to_range (r, std::numeric_limits<T>::min (), std::numeric_limits<T>::max ())

#define OCTAVE_INT_MIN_VAL2(T1, T2) \
  std::numeric_limits<typename octave_int_binop_traits<T1, T2>::TR>::min ()

#define OCTAVE_INT_MAX_VAL2(T1, T2) \
  std::numeric_limits<typename octave_int_binop_traits<T1, T2>::TR>::max ()

#define OCTAVE_INT_FIT_TO_RANGE2(r, T1, T2) \
  octave_int_fit_to_range (r, \
                           OCTAVE_INT_MIN_VAL2 (T1, T2), \
                           OCTAVE_INT_MAX_VAL2 (T1, T2))

template <class T>
class
octave_int
{
public:

  octave_int (void) : ival () { }

  template <class U>
  octave_int (U i) : ival (OCTAVE_INT_FIT_TO_RANGE (i, T)) { }

  octave_int (bool b) : ival (b) { }

  template <class U>
  octave_int (const octave_int<U>& i)
    : ival (OCTAVE_INT_FIT_TO_RANGE (i.value (), T)) { }

  octave_int (const octave_int<T>& i) : ival (i.ival) { }

  octave_int& operator = (const octave_int<T>& i)
  {
    ival = i.ival;
    return *this;
  }

  ~octave_int (void) { }
  
  T value (void) const { return ival; }

  bool operator ! (void) const { return ! ival; }

  octave_int<T> operator + (void) const { return *this; }

  octave_int<T> operator - (void) const
  {
    return std::numeric_limits<T>::is_signed ? -ival : 0;
  }

  operator double (void) const { return static_cast<double> (value ()); }

  octave_int<T>& operator += (const octave_int<T>& x)
  {
    double t = static_cast<double> (value ());
    double tx = static_cast<double> (x.value ());
    ival = OCTAVE_INT_FIT_TO_RANGE (t + tx, T);
    return *this;
  }

  octave_int<T>& operator -= (const octave_int<T>& x)
  {
    double t = static_cast<double> (value ());
    double tx = static_cast<double> (x.value ());
    ival = OCTAVE_INT_FIT_TO_RANGE (t - tx, T);
    return *this;
  }

  octave_int<T> min (void) const { return std::numeric_limits<T>::min (); }
  octave_int<T> max (void) const { return std::numeric_limits<T>::max (); }

  static int nbits (void) { return sizeof (T) * CHAR_BIT; }

private:

  T ival;
};

template <class T>
std::ostream&
operator << (std::ostream& os, const octave_int<T>& ival)
{
  os << ival.value ();
  return os;
}

template <class T>
std::istream&
operator >> (std::istream& is, octave_int<T>& ival)
{
  T tmp = 0;
  is >> tmp;
  ival = tmp;
  return is;
}

typedef octave_int<octave_int8_t> octave_int8;
typedef octave_int<octave_int16_t> octave_int16;
typedef octave_int<octave_int32_t> octave_int32;
typedef octave_int<octave_int64_t> octave_int64;

typedef octave_int<octave_uint8_t> octave_uint8;
typedef octave_int<octave_uint16_t> octave_uint16;
typedef octave_int<octave_uint32_t> octave_uint32;
typedef octave_int<octave_uint64_t> octave_uint64;

#define OCTAVE_INT_BIN_OP(OP) \
 \
  template <class T1, class T2> \
  octave_int<typename octave_int_binop_traits<T1, T2>::TR> \
  operator OP (const octave_int<T1>& x, const octave_int<T2>& y) \
  { \
    double tx = static_cast<double> (x.value ()); \
    double ty = static_cast<double> (y.value ()); \
    double r = tx OP ty; \
    return OCTAVE_INT_FIT_TO_RANGE2 (r, T1, T2); \
  }

OCTAVE_INT_BIN_OP(+)
OCTAVE_INT_BIN_OP(-)
OCTAVE_INT_BIN_OP(*)
OCTAVE_INT_BIN_OP(/)

#define OCTAVE_INT_BITCMP_OP(OP) \
 \
  template <class T> \
  octave_int<T> \
  operator OP (const octave_int<T>& x, const octave_int<T>& y) \
  { \
    return x.value () OP y.value (); \
  }

OCTAVE_INT_BITCMP_OP (&)
OCTAVE_INT_BITCMP_OP (|)
OCTAVE_INT_BITCMP_OP (^)

#define OCTAVE_INT_BITSHIFT_OP(OP) \
 \
  template <class T1, class T2> \
  octave_int<T1> \
  operator OP (const octave_int<T1>& x, const T2& y) \
  { \
    return x.value () OP y; \
  }

OCTAVE_INT_BITSHIFT_OP (<<)
OCTAVE_INT_BITSHIFT_OP (>>)

template <class T>
octave_int<T>
bitshift (const octave_int<T>& a, int n)
{
  if (n > 0)
    return a << n;
  else if (n < 0)
    return a >> -n;
  else
    return a;
}

#define OCTAVE_INT_CMP_OP(OP) \
 \
  template <class T1, class T2> \
  bool \
  operator OP (const octave_int<T1>& x, const octave_int<T2>& y) \
  { \
    return x.value () OP y.value (); \
  }

OCTAVE_INT_CMP_OP (<)
OCTAVE_INT_CMP_OP (<=)
OCTAVE_INT_CMP_OP (>=)
OCTAVE_INT_CMP_OP (>)
OCTAVE_INT_CMP_OP (==)
OCTAVE_INT_CMP_OP (!=)

#define OCTAVE_INT_CONCAT_FN(TYPE) \
intNDArray< TYPE > \
concat (const intNDArray< TYPE >& ra, const intNDArray< TYPE >& rb, \
	const Array<int>& ra_idx) \
{ \
  intNDArray< TYPE > retval (ra); \
  retval.insert (rb, ra_idx); \
  return retval; \
}

#define OCTAVE_INT_CONCAT_DECL(TYPE) \
intNDArray< TYPE > \
concat (const intNDArray< TYPE >& ra, const intNDArray< TYPE >& rb, \
	const Array<int>& ra_idx);

#undef OCTAVE_INT_TRAIT
#undef OCTAVE_INT_BINOP_TRAIT
#undef OCTAVE_INT_MIN_VAL
#undef OCTAVE_INT_MAX_VAL
#undef OCTAVE_INT_FIT_TO_RANGE
#undef OCTAVE_INT_MIN_VAL2
#undef OCTAVE_INT_MAX_VAL2
#undef OCTAVE_INT_FIT_TO_RANGE2
#undef OCTAVE_INT_BIN_OP
#undef OCTAVE_INT_CMP_OP

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
