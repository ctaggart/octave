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

#include <cfloat>
#include <cmath>

#include "DASSL.h"
#include "f77-fcn.h"
#include "lo-error.h"

typedef int (*dassl_fcn_ptr) (const double&, const double*, const double*,
			      double*, int&, double*, int*);

typedef int (*dassl_jac_ptr) (const double&, const double*, const double*,
			      double*, const double&, double*, int*);

extern "C"
int F77_FUNC (ddassl, DDASSL) (dassl_fcn_ptr, const int&, double&,
			       double*, double*, double&, const int*,
			       const double*, const double*, int&,
			       double*, const int&, int*, const int&,
			       const double*, const int*,
			       dassl_jac_ptr);

static DAEFunc::DAERHSFunc user_fun;
static DAEFunc::DAEJacFunc user_jac;

static int nn;

DASSL::DASSL (void) : DAE ()
{
  liw = 0;
  lrw = 0;

  sanity_checked = false;

  info.resize (15);

  for (int i = 0; i < 15; i++)
    info.elem (i) = 0;
}

DASSL::DASSL (const ColumnVector& state, double time, DAEFunc& f)
  : DAE (state, time, f)
{
  n = size ();

  liw = 20 + n;
  lrw = 40 + 9*n + n*n;

  sanity_checked = false;

  info.resize (15, 0);
}

DASSL::DASSL (const ColumnVector& state, const ColumnVector& deriv,
	      double time, DAEFunc& f)
  : DAE (state, deriv, time, f)
{
  n = size ();

  DAEFunc::set_function (f.function ());
  DAEFunc::set_jacobian_function (f.jacobian_function ());

  liw = 20 + n;
  lrw = 40 + 9*n + n*n;

  sanity_checked = false;

  info.resize (15);

  for (int i = 0; i < 15; i++)
    info.elem (i) = 0;
}

int
ddassl_f (const double& time, const double *state, const double *deriv,
	  double *delta, int& ires, double *, int *)
{
  // XXX FIXME XXX -- would be nice to avoid copying the data.

  ColumnVector tmp_deriv (nn);
  ColumnVector tmp_state (nn);
  ColumnVector tmp_delta (nn);

  for (int i = 0; i < nn; i++)
    {
      tmp_deriv.elem (i) = deriv [i];
      tmp_state.elem (i) = state [i];
    }

  tmp_delta = user_fun (tmp_state, tmp_deriv, time, ires);

  if (ires >= 0)
    {
      if (tmp_delta.length () == 0)
	ires = -2;
      else
	{
	  for (int i = 0; i < nn; i++)
	    delta [i] = tmp_delta.elem (i);
	}
    }

  return 0;
}

int
ddassl_j (const double& time, const double *state, const double *deriv,
	  double *pd, const double& cj, double *, int *)
{
  // XXX FIXME XXX -- would be nice to avoid copying the data.

  ColumnVector tmp_state (nn);
  ColumnVector tmp_deriv (nn);

  for (int i = 0; i < nn; i++)
    {
      tmp_deriv.elem (i) = deriv [i];
      tmp_state.elem (i) = state [i];
    }

  Matrix tmp_pd = user_jac (tmp_state, tmp_deriv, time, cj);

  for (int j = 0; j < nn; j++)
    for (int i = 0; i < nn; i++)
      pd [nn * j + i] = tmp_pd.elem (i, j);

  return 0;
}

ColumnVector
DASSL::do_integrate (double tout)
{
  ColumnVector retval;

  if (restart)
    {
      restart = false;
      info.elem (0) = 0;
    }

  if (iwork.length () != liw)
    iwork.resize (liw);

  if (rwork.length () != lrw)
    rwork.resize (lrw);

  integration_error = false;

  user_fun = DAEFunc::fun;
  user_jac = DAEFunc::jac;

  if (user_jac)
    info.elem (4) = 1;
  else
    info.elem (4) = 0;

  double *px = x.fortran_vec ();
  double *pxdot = xdot.fortran_vec ();

  nn = n;

  if (! sanity_checked)
    {
      int ires = 0;

      ColumnVector res = (*user_fun) (x, xdot, t, ires);

      if (res.length () != x.length ())
	{
	  (*current_liboctave_error_handler)
	    ("dassl: inconsistent sizes for state and residual vectors");

	  integration_error = true;
	  return retval;
	}

      sanity_checked = true;
    }
  
  if (stop_time_set)
    {
      rwork.elem (0) = stop_time;
      info.elem (3) = 1;
    }
  else
    info.elem (3) = 0;

  Array<double> abs_tol = absolute_tolerance ();
  Array<double> rel_tol = relative_tolerance ();

  int abs_tol_len = abs_tol.length ();
  int rel_tol_len = rel_tol.length ();

  if (abs_tol_len == 1 && rel_tol_len == 1)
    {
      info.elem (1) = 0;
    }
  else if (abs_tol_len == n && rel_tol_len == n)
    {
      info.elem (1) = 1;
    }
  else
    {
      (*current_liboctave_error_handler)
	("dassl: inconsistent sizes for tolerance arrays");

      integration_error = true;
      return retval;
    }

  if (initial_step_size () >= 0.0)
    {
      rwork.elem (2) = initial_step_size ();
      info.elem (7) = 1;
    }
  else
    info.elem (7) = 0;

  if (maximum_step_size () >= 0.0)
    {
      rwork.elem (1) = maximum_step_size ();
      info.elem (6) = 1;
    }
  else
    info.elem (6) = 0;

  double *dummy = 0;
  int *idummy = 0;

  int *pinfo = info.fortran_vec ();
  int *piwork = iwork.fortran_vec ();
  double *prwork = rwork.fortran_vec ();
  double *pabs_tol = abs_tol.fortran_vec ();
  double *prel_tol = rel_tol.fortran_vec ();

// again:

  F77_XFCN (ddassl, DDASSL, (ddassl_f, n, t, px, pxdot, tout, pinfo,
			     prel_tol, pabs_tol, istate, prwork, lrw,
			     piwork, liw, dummy, idummy, ddassl_j));

  if (f77_exception_encountered)
    {
      integration_error = true;
      (*current_liboctave_error_handler) ("unrecoverable error in dassl");
    }
  else
    {
      switch (istate)
	{
	case 1: // A step was successfully taken in intermediate-output
	        // mode. The code has not yet reached TOUT.
	case 2: // The integration to TSTOP was successfully completed
	        // (T=TSTOP) by stepping exactly to TSTOP.
	case 3: // The integration to TOUT was successfully completed
	        // (T=TOUT) by stepping past TOUT.  Y(*) is obtained by
	        // interpolation.  YPRIME(*) is obtained by interpolation.
	  retval = x;
	  t = tout;
	  break;

	case -1: // A large amount of work has been expended.  (~500 steps).
	case -2: // The error tolerances are too stringent.
	case -3: // The local error test cannot be satisfied because you
	         // specified a zero component in ATOL and the
		 // corresponding computed solution component is zero.
		 // Thus, a pure relative error test is impossible for
		 // this component.
	case -6: // DDASSL had repeated error test failures on the last
		 // attempted step.
	case -7: // The corrector could not converge.
	case -8: // The matrix of partial derivatives is singular.
	case -9: // The corrector could not converge.  There were repeated
		 // error test failures in this step.
	case -10: // The corrector could not converge because IRES was
		  // equal to minus one.
	case -11: // IRES equal to -2 was encountered and control is being
		  // returned to the calling program.
	case -12: // DDASSL failed to compute the initial YPRIME.
	case -33: // The code has encountered trouble from which it cannot
		  // recover. A message is printed explaining the trouble
		  // and control is returned to the calling program. For
		  // example, this occurs when invalid input is detected.
	  integration_error = true;
	  break;

	default:
	  integration_error = true;
	  (*current_liboctave_error_handler)
	    ("unrecognized value of istate (= %d) returned from ddassl",
	     istate);
	  break;
	}
    }

  return retval;
}

Matrix
DASSL::do_integrate (const ColumnVector& tout)
{
  Matrix dummy;
  return integrate (tout, dummy);
}

Matrix
DASSL::integrate (const ColumnVector& tout, Matrix& xdot_out)
{
  Matrix retval;
  int n_out = tout.capacity ();

  if (n_out > 0 && n > 0)
    {
      retval.resize (n_out, n);
      xdot_out.resize (n_out, n);

      for (int i = 0; i < n; i++)
	{
	  retval.elem (0, i) = x.elem (i);
	  xdot_out.elem (0, i) = xdot.elem (i);
	}

      for (int j = 1; j < n_out; j++)
	{
	  ColumnVector x_next = do_integrate (tout.elem (j));

	  if (integration_error)
	    return retval;

	  for (int i = 0; i < n; i++)
	    {
	      retval.elem (j, i) = x_next.elem (i);
	      xdot_out.elem (j, i) = xdot.elem (i);
	    }
	}
    }

  return retval;
}

Matrix
DASSL::do_integrate (const ColumnVector& tout, const ColumnVector& tcrit)
{
  Matrix dummy;
  return integrate (tout, dummy, tcrit);
}

Matrix
DASSL::integrate (const ColumnVector& tout, Matrix& xdot_out,
		  const ColumnVector& tcrit) 
{
  Matrix retval;
  int n_out = tout.capacity ();

  if (n_out > 0 && n > 0)
    {
      retval.resize (n_out, n);
      xdot_out.resize (n_out, n);

      for (int i = 0; i < n; i++)
	{
	  retval.elem (0, i) = x.elem (i);
	  xdot_out.elem (0, i) = xdot.elem (i);
	}

      int n_crit = tcrit.capacity ();

      if (n_crit > 0)
	{
	  int i_crit = 0;
	  int i_out = 1;
	  double next_crit = tcrit.elem (0);
	  double next_out;
	  while (i_out < n_out)
	    {
	      bool do_restart = false;

	      next_out = tout.elem (i_out);
	      if (i_crit < n_crit)
		next_crit = tcrit.elem (i_crit);

	      bool save_output;
	      double t_out;

	      if (next_crit == next_out)
		{
		  set_stop_time (next_crit);
		  t_out = next_out;
		  save_output = true;
		  i_out++;
		  i_crit++;
		  do_restart = true;
		}
	      else if (next_crit < next_out)
		{
		  if (i_crit < n_crit)
		    {
		      set_stop_time (next_crit);
		      t_out = next_crit;
		      save_output = false;
		      i_crit++;
		      do_restart = true;
		    }
		  else
		    {
		      clear_stop_time ();
		      t_out = next_out;
		      save_output = true;
		      i_out++;
		    }
		}
	      else
		{
		  set_stop_time (next_crit);
		  t_out = next_out;
		  save_output = true;
		  i_out++;
		}

	      ColumnVector x_next = do_integrate (t_out);

	      if (integration_error)
		return retval;

	      if (save_output)
		{
		  for (int i = 0; i < n; i++)
		    {
		      retval.elem (i_out-1, i) = x_next.elem (i);
		      xdot_out.elem (i_out-1, i) = xdot.elem (i);
		    }
		}

	      if (do_restart)
		force_restart ();
	    }
	}
      else
	{
	  retval = integrate (tout, xdot_out);

	  if (integration_error)
	    return retval;
	}
    }

  return retval;
}

std::string
DASSL::error_message (void) const
{
  std::string retval;

  switch (istate)
    {
    case 1:
      retval = "a step was successfully taken in intermediate-output mode.";
      break;

    case 2:
      retval = "integration completed by stepping exactly to TOUT";
      break;

    case 3:
      retval = "integration to tout completed by stepping past TOUT";
      break;

    case -1:
      retval = "a large amount of work has been expended";
      break;

    case -2:
      retval = "the error tolerances are too stringent";
      break;

    case -3:
      retval = "error weight became zero during problem.\
  (solution component i vanished, and atol or atol(i) == 0)";
      break;

    case -6:
      retval = "repeated error test failures on the last attempted step";
      break;

    case -7:
      retval = "the corrector could not converge";
      break;

    case -8:
      retval = "the matrix of partial derivatives is singular";
      break;

    case -9:
      retval = "the corrector could not converge (repeated test failures)";
      break;

    case -10:
      retval = "corrector could not converge because IRES was -1";
      break;

    case -11:
      retval = "return requested in user-supplied function";
      break;

    case -12:
      retval = "failed to compute consistent initial conditions";
      break;

    case -33:
      retval = "unrecoverable error (see printed message)";
      break;

    default:
      retval = "unknown error state";
      break;
    }

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
