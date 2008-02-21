/*

Copyright (C) 2007 John W. Eaton

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cctype>
#include <cfloat>
#include <cstdlib>

#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <string>

#include "file-ops.h"
#include "file-stat.h"

#include "defun.h"
#include "error.h"
#include "graphics.h"
#include "input.h"
#include "ov.h"
#include "oct-obj.h"
#include "oct-map.h"
#include "ov-fcn-handle.h"
#include "parse.h"
#include "toplev.h"
#include "unwind-prot.h"

static void
gripe_set_invalid (const std::string& pname)
{
  error ("set: invalid value for %s property", pname.c_str ());
}

static Matrix
jet_colormap (void)
{
  Matrix cmap (64, 3, 0.0);

  for (octave_idx_type i = 0; i < 64; i++)
    {
      // This is the jet colormap.  It would be nice to be able
      // to feval the jet function but since there is a static
      // property object that includes a colormap_property
      // object, we need to initialize this before main is even
      // called, so calling an interpreted function is not
      // possible.

      double x = i / 63.0;

      if (x >= 3.0/8.0 && x < 5.0/8.0)
        cmap(i,0) = 4.0 * x - 3.0/2.0;
      else if (x >= 5.0/8.0 && x < 7.0/8.0)
        cmap(i,0) = 1.0;
      else if (x >= 7.0/8.0)
        cmap(i,0) = -4.0 * x + 9.0/2.0;

      if (x >= 1.0/8.0 && x < 3.0/8.0)
        cmap(i,1) = 4.0 * x - 1.0/2.0;
      else if (x >= 3.0/8.0 && x < 5.0/8.0)
        cmap(i,1) = 1.0;
      else if (x >= 5.0/8.0 && x < 7.0/8.0)
        cmap(i,1) = -4.0 * x + 7.0/2.0;

      if (x < 1.0/8.0)
        cmap(i,2) = 4.0 * x + 1.0/2.0;
      else if (x >= 1.0/8.0 && x < 3.0/8.0)
        cmap(i,2) = 1.0;
      else if (x >= 3.0/8.0 && x < 5.0/8.0)
        cmap(i,2) = -4.0 * x + 5.0/2.0;
    }

  return cmap;
}

static Matrix
default_colororder (void)
{
  Matrix retval (7, 3, 0.0);

  retval(0,2) = 1.0;

  retval(1,1) = 0.5;

  retval(2,0) = 1.0;

  retval(3,1) = 0.75;
  retval(3,2) = 0.75;

  retval(4,0) = 0.75;
  retval(4,2) = 0.75;

  retval(5,0) = 0.75;
  retval(5,1) = 0.75;

  retval(6,0) = 0.25;
  retval(6,1) = 0.25;
  retval(6,2) = 0.25;

  return retval;
}

static Matrix
default_lim (void)
{
  Matrix m (1, 2, 0);
  m(1) = 1;
  return m;
}

static Matrix
default_data (void)
{
  Matrix retval (1, 2);

  retval(0) = 0;
  retval(1) = 1;

  return retval;
}

static Matrix
default_axes_position (void)
{
  Matrix m (1, 4, 0.0);
  m(0) = 0.13;
  m(1) = 0.11;
  m(2) = 0.775;
  m(3) = 0.815;
  return m;
}

static Matrix
default_axes_outerposition (void)
{
  Matrix m (1, 4, 0.0);
  m(2) = m(3) = 1.0;
  return m;
}

static Matrix
default_figure_position (void)
{
  Matrix m (1, 4, 0.0);
  m(0) = 300;
  m(1) = 200;
  m(2) = 560;
  m(3) = 420;
  return m;
}

static void
xset_gcbo (const graphics_handle& h)
{
  graphics_object go = gh_manager::get_object (0);
  root_figure::properties& props =
      dynamic_cast<root_figure::properties&> (go.get_properties ());

  props.set_callbackobject (h.as_octave_value ());
}

static void
xreset_gcbo (void *)
{
  xset_gcbo (graphics_handle ());
}

// NOTE: "cb" is passed by value, because "function_value" method
//       is non-const; passing "cb" by const-reference is not
//       possible

static void
execute_callback (octave_value cb, const graphics_handle& h,
                  const octave_value& data)
{
  octave_value_list args;
  octave_function *fcn = 0;

  args(0) = h.as_octave_value ();
  if (data.is_defined ())
    args(1) = data;
  else
    args(1) = Matrix ();

  unwind_protect::begin_frame ("execute_callback");
  unwind_protect::add (xreset_gcbo);

  xset_gcbo (h);

  BEGIN_INTERRUPT_WITH_EXCEPTIONS;

  if (cb.is_function_handle ())
    fcn = cb.function_value ();
  else if (cb.is_string ())
    {
      int status;
      std::string s = cb.string_value ();

      eval_string (s, false, status);
    }
  else if (cb.is_cell () && cb.length () > 0
           && (cb.rows () == 1 || cb.columns () == 1)
           && cb.cell_value ()(0).is_function_handle ())
    {
      Cell c = cb.cell_value ();

      fcn = c(0).function_value ();
      if (! error_state)
        {
          for (int i = 0; i < c.length () ; i++)
            args(2+i) = c(i);
        }
    }
  else
    {
      std::string nm = cb.class_name ();
      error ("trying to execute non-executable object (class = %s)",
	     nm.c_str ());
    }

  if (fcn && ! error_state)
    feval (fcn, args);
  
  END_INTERRUPT_WITH_EXCEPTIONS;

  unwind_protect::run_frame ("execute_callback");
}

static Matrix
convert_position (const Matrix& pos, const caseless_str& from_units,
		  const caseless_str& to_units,
		  const Matrix& parent_dim = Matrix (1, 2, 0.0),
		  const graphics_backend& backend = graphics_backend ())
{
  Matrix retval (1, 4);
  double res = 0;

  if (from_units.compare ("pixels"))
    retval = pos;
  else if (from_units.compare ("normalized"))
    {
      retval(0) = pos(0) * parent_dim(0) + 1;
      retval(1) = pos(1) * parent_dim(1) + 1;
      retval(2) = pos(2) * parent_dim(0);
      retval(3) = pos(3) * parent_dim(1);
    }
  else if (from_units.compare ("characters"))
    {
      // FIXME: implement this
    }
  else
    {
      res = backend.get_screen_resolution ();

      double f = 0.0;

      if (from_units.compare ("points"))
	f = res / 72.0;
      else if (from_units.compare ("inches"))
	f = res;
      else if (from_units.compare ("centimeters"))
	f = res / 2.54;

      if (f > 0)
	{
	  retval(0) = pos(0) * f + 1;
	  retval(1) = pos(1) * f + 1;
	  retval(2) = pos(2) * f;
	  retval(3) = pos(3) * f;
	}
    }

  if (! to_units.compare ("pixels"))
    {
      if (to_units.compare ("normalized"))
	{
	  retval(0) = (retval(0) - 1) / parent_dim(0);
	  retval(1) = (retval(1) - 1) / parent_dim(1);
	  retval(2) /= parent_dim(0);
	  retval(3) /= parent_dim(1);
	}
      else if (to_units.compare ("characters"))
	{
	  // FIXME: implement this
	}
      else
	{
	  if (res <= 0)
	    res = backend.get_screen_resolution ();

	  double f = 0.0;

	  if (to_units.compare ("points"))
	    f = res / 72.0;
	  else if (to_units.compare ("inches"))
	    f = res;
	  else if (to_units.compare ("centimeters"))
	    f = res / 2.54;

	  if (f > 0)
	    {
	      retval(0) = (retval(0) - 1) / f;
	      retval(1) = (retval(1) - 1) / f;
	      retval(2) /= f;
	      retval(3) /= f;
	    }
	}
    }

  return retval;
}

static graphics_object
xget_ancestor (graphics_object go, const std::string& type)
{
  do
    {
      if (go.valid_object ())
	{
	  if (go.isa (type))
	    return go;
	  else
	    go = gh_manager::get_object (go.get_parent ());
	}
      else
	return graphics_object ();
    } while (true);
}

static octave_value
convert_cdata (const base_properties& props, const octave_value& cdata,
	       bool is_scaled, int cdim)
{
  dim_vector dv (cdata.dims ());

  if (dv.length () == cdim && dv(cdim-1) == 3)
    return cdata;

  Matrix cmap (1, 3, 0.0);
  Matrix clim (1, 2, 0.0);

  graphics_object go = gh_manager::get_object (props.get___myhandle__ ());
  graphics_object fig = xget_ancestor (go, "figure");

  if (fig.valid_object ())
    {
      Matrix _cmap = fig.get (caseless_str ("colormap")).matrix_value ();

      if (! error_state)
	cmap = _cmap;
    }

  if (is_scaled)
    {
      graphics_object ax = xget_ancestor (go, "axes");

      if (ax.valid_object ())
	{
	  Matrix _clim = ax.get (caseless_str ("clim")).matrix_value ();

	  if (! error_state)
	    clim = _clim;
	}
    }

  dv.resize (cdim);
  dv(cdim-1) = 3;

  NDArray a (dv);

  int lda = static_cast<int> (a.numel () / 3);
  int nc = cmap.rows ();

  double *av = a.fortran_vec ();
  const double *cmapv = cmap.data ();
  const double *cv = 0;
  const octave_uint8 *icv = 0;

  if (cdata.is_integer_type ())
    icv = cdata.uint8_array_value ().data ();
  else
    cv = cdata.array_value ().data ();

  for (int i = 0; i < lda; i++)
    {
      double x = (cv ? cv[i] : double (icv[i]));

      if (is_scaled)
	x = xround ((nc - 1) * (x - clim(0)) / (clim(1) - clim(0)));
      else
	x = xround (x - 1);

      if (x < 0)
	x = 0;
      else if (x >= nc)
	x = (nc - 1);

      int idx = static_cast<int> (x);

      av[i]       = cmapv[idx];
      av[i+lda]   = cmapv[idx+nc];
      av[i+2*lda] = cmapv[idx+2*nc];
    }

  return octave_value (a);
}

template<class T>
static void
get_array_limits (const Array<T>& m, double& emin, double& emax,
		  double& eminp)
{
  const T *data = m.data ();
  int n = m.numel ();

  for (int i = 0; i < n; i++)
    {
      double e = double (data[i]);

      if (! (xisinf (e) || xisnan (e)))
	{
	  if (e < emin)
	    emin = e;

	  if (e > emax)
	    emax = e;

	  if (e >= 0 && e < eminp)
	    eminp = e;
	}
    }
}

// ---------------------------------------------------------------------

radio_values::radio_values (const std::string& opt_string)
{
  size_t beg = 0;
  size_t len = opt_string.length ();
  bool done = len == 0;

  while (! done)
    {
      size_t end = opt_string.find ('|', beg);

      if (end == std::string::npos)
	{
	  end = len;
	  done = true;
	}

      std::string t = opt_string.substr (beg, end-beg);

      // Might want more error checking here...
      if (t[0] == '{')
	{
	  t = t.substr (1, t.length () - 2);
	  default_val = t;
	}
      else if (beg == 0) // ensure default value
	default_val = t;

      possible_vals.insert (t);

      beg = end + 1;
    }
}

bool
color_values::str2rgb (std::string str)
{
  double tmp_rgb[3] = {0, 0, 0};
  bool retval = true;
  unsigned int len = str.length();

  std::transform (str.begin (), str.end (), str.begin (), tolower);

  if (str.compare(0, len, "blue", 0, len) == 0)
    tmp_rgb[2] = 1;
  else if (str.compare(0, len, "black", 0, len) == 0 || 
	   str.compare(0, len, "k", 0, len) == 0)
    tmp_rgb[0] = tmp_rgb[1] = tmp_rgb[2] = 0;
  else if (str.compare(0, len, "red", 0, len) == 0)
    tmp_rgb[0] = 1;
  else if (str.compare(0, len, "green", 0, len) == 0)
    tmp_rgb[1] = 1;
  else if (str.compare(0, len, "yellow", 0, len) == 0)
    tmp_rgb[0] = tmp_rgb[1] = 1;
  else if (str.compare(0, len, "magenta", 0, len) == 0)
    tmp_rgb[0] = tmp_rgb[2] = 1;
  else if (str.compare(0, len, "cyan", 0, len) == 0)
    tmp_rgb[1] = tmp_rgb[2] = 1;
  else if (str.compare(0, len, "white", 0, len) == 0 ||
	   str.compare(0, len, "w", 0, len) == 0)
    tmp_rgb[0] = tmp_rgb[1] = tmp_rgb[2] = 1;
  else	
    retval = false;

  if (retval)
    {
      for (int i = 0; i < 3; i++)
	xrgb(i) = tmp_rgb[i];
    }

  return retval;
}

void
color_property::set (const octave_value& val)
{
  if (val.is_string ())
    {
      std::string s = val.string_value ();

      if (! s.empty ())
	{
	  if (radio_val.contains (s))
	    {
	      current_val = s;
	      current_type = radio_t;
	    }
          else
	    {
	      color_values col (s);
	      if (! error_state)
		{
		  color_val = col;
		  current_type = color_t;
		}
	      else
		error ("invalid value for color property \"%s\" (value = %s)",
               get_name ().c_str (), s.c_str ());
	    }	
	}
      else
	error ("invalid value for color property \"%s\"",
           get_name ().c_str ());
    }
  else if (val.is_real_matrix ())
    {
      Matrix m = val.matrix_value ();

      if (m.numel () == 3)
	{
	  color_values col (m (0), m (1), m(2));
	  if (! error_state)
	    {
	      color_val = col;
	      current_type = color_t;
	    }
	}
      else
	error ("invalid value for color property \"%s\"",
           get_name ().c_str ());
    }
  else 
    error ("invalid value for color property \"%s\"",
           get_name ().c_str ());
}

bool
array_property::validate (const octave_value& v)
{
  bool xok = false;

  // FIXME: should we always support []?
  if (v.is_empty () && v.is_double_type ())
    return true;

  // check value type
  if (type_constraints.size () > 0)
    {
      for (std::list<std::string>::const_iterator it = type_constraints.begin ();
           ! xok && it != type_constraints.end (); ++it)
        if ((*it) == v.class_name ())
          xok = true;
    }
  else
    xok = v.is_double_type ();

  if (xok)
    {
      dim_vector vdims = v.dims ();
      int vlen = vdims.length ();

      xok = false;

      // check value size
      if (size_constraints.size () > 0)
        for (std::list<dim_vector>::const_iterator it = size_constraints.begin ();
             ! xok && it != size_constraints.end (); ++it)
          {
            dim_vector itdims = (*it);

            if (itdims.length () == vlen)
              {
                xok = true;

                for (int i = 0; xok && i < vlen; i++)
                  if (itdims(i) >= 0 && itdims(i) != vdims(i))
                    xok = false;
              }
          }
      else
        return true;
    }

  return xok;
}

void
array_property::get_data_limits (void)
{
  xmin = xminp = octave_Inf;
  xmax = -octave_Inf;

  if (! data.is_empty ())
    {
      if (data.is_integer_type ())
	{
	  if (data.is_int8_type ())
	    get_array_limits (data.int8_array_value (), xmin, xmax, xminp);
	  else if (data.is_uint8_type ())
	    get_array_limits (data.uint8_array_value (), xmin, xmax, xminp);
	  else if (data.is_int16_type ())
	    get_array_limits (data.int16_array_value (), xmin, xmax, xminp);
	  else if (data.is_uint16_type ())
	    get_array_limits (data.uint16_array_value (), xmin, xmax, xminp);
	  else if (data.is_int32_type ())
	    get_array_limits (data.int32_array_value (), xmin, xmax, xminp);
	  else if (data.is_uint32_type ())
	    get_array_limits (data.uint32_array_value (), xmin, xmax, xminp);
	  else if (data.is_int64_type ())
	    get_array_limits (data.int64_array_value (), xmin, xmax, xminp);
	  else if (data.is_uint64_type ())
	    get_array_limits (data.uint64_array_value (), xmin, xmax, xminp);
	}
      else
	get_array_limits (data.array_value (), xmin, xmax, xminp);
    }
}

void
handle_property::set (const octave_value& v)
{
  double dv = v.double_value ();

  if (! error_state)
    {
      graphics_handle gh = gh_manager::lookup (dv);

      if (xisnan (gh.value ()) || gh.ok ())
        current_val = gh;
      else
        error ("set: invalid graphics handle (= %g) for property \"%s\"",
            dv, get_name ().c_str ());
    }
  else
    error ("set: invalid graphics handle for property \"%s\"",
        get_name ().c_str ());
}

bool
callback_property::validate (const octave_value& v) const
{
  // case 1: function handle
  // case 2: cell array with first element being a function handle
  // case 3: string corresponding to known function name
  // case 4: evaluatable string
  // case 5: empty matrix

  if (v.is_function_handle ())
    return true;
  else if (v.is_string ())
    // complete validation will be done at execution-time
    return true;
  else if (v.is_cell () && v.length () > 0
           && (v.rows() == 1 || v.columns () == 1)
           && v.cell_value ()(0).is_function_handle ())
    return true;
  else if (v.is_empty ())
    return true;

  return false;
}

void
callback_property::execute (const octave_value& data) const
{
  if (callback.is_defined () && ! callback.is_empty ())
    execute_callback (callback, get_parent (), data);
}

void
callback_property::execute (const octave_value& cb, const graphics_handle& h,
			    const octave_value& data)
{
  if (cb.is_defined () && ! cb.is_empty ())
    execute_callback (cb, h, data);
}

// ---------------------------------------------------------------------

void
property_list::set (const caseless_str& name, const octave_value& val)
{
  size_t offset = 0;

  size_t len = name.length ();

  if (len > 4)
    {
      caseless_str pfx = name.substr (0, 4);

      if (pfx.compare ("axes") || pfx.compare ("line")
	  || pfx.compare ("text"))
	offset = 4;
      else if (len > 5)
	{
	  pfx = name.substr (0, 5);

	  if (pfx.compare ("image") || pfx.compare ("patch"))
	    offset = 5;
	  else if (len > 6)
	    {
	      pfx = name.substr (0, 6);

	      if (pfx.compare ("figure"))
		offset = 6;
	      else if (len > 7)
		{
		  pfx = name.substr (0, 7);

		  if (pfx.compare ("surface"))
		    offset = 7;
		}
	    }
	}

      if (offset > 0)
	{
	  // FIXME -- should we validate property names and values here?

	  std::string pname = name.substr (offset);

	  std::transform (pfx.begin (), pfx.end (), pfx.begin (), tolower);
	  std::transform (pname.begin (), pname.end (), pname.begin (), tolower);

	  bool remove = false;
	  if (val.is_string ())
	    {
	      caseless_str tval = val.string_value ();

	      remove = tval.compare ("remove");
	    }

	  pval_map_type& pval_map = plist_map[pfx];

	  if (remove)
	    {
	      pval_map_iterator p = pval_map.find (pname);

	      if (p != pval_map.end ())
		pval_map.erase (p);
	    }
	  else
	    pval_map[pname] = val;
	}
    }

  if (offset == 0)
    error ("invalid default property specification");
}

octave_value
property_list::lookup (const caseless_str& name) const
{
  octave_value retval;

  size_t offset = 0;

  size_t len = name.length ();

  if (len > 4)
    {
      caseless_str pfx = name.substr (0, 4);

      if (pfx.compare ("axes") || pfx.compare ("line")
	  || pfx.compare ("text"))
	offset = 4;
      else if (len > 5)
	{
	  pfx = name.substr (0, 5);

	  if (pfx.compare ("image") || pfx.compare ("patch"))
	    offset = 5;
	  else if (len > 6)
	    {
	      pfx = name.substr (0, 6);

	      if (pfx.compare ("figure"))
		offset = 6;
	      else if (len > 7)
		{
		  pfx = name.substr (0, 7);

		  if (pfx.compare ("surface"))
		    offset = 7;
		}
	    }
	}

      if (offset > 0)
	{
	  std::string pname = name.substr (offset);

	  std::transform (pfx.begin (), pfx.end (), pfx.begin (), tolower);
	  std::transform (pname.begin (), pname.end (), pname.begin (), tolower);

	  plist_map_const_iterator p = find (pfx);

	  if (p != end ())
	    {
	      const pval_map_type& pval_map = p->second;

	      pval_map_const_iterator q = pval_map.find (pname);

	      if (q != pval_map.end ())
		retval = q->second;
	    }
	}
    }

  return retval;
}

Octave_map
property_list::as_struct (const std::string& prefix_arg) const
{
  Octave_map m;

  for (plist_map_const_iterator p = begin (); p != end (); p++)
    {
      std::string prefix = prefix_arg + p->first;

      const pval_map_type pval_map = p->second;

      for (pval_map_const_iterator q = pval_map.begin ();
	   q != pval_map.end ();
	   q++)
	m.assign (prefix + q->first, q->second);
    }

  return m;    
}

graphics_handle::graphics_handle (const octave_value& a)
  : val (octave_NaN)
{
  if (a.is_empty ())
    /* do nothing */;
  else
    {
      double tval = a.double_value ();

      if (! error_state)
	val = tval;
      else
	error ("invalid graphics handle");
    }
}

void
graphics_object::set (const octave_value_list& args)
{
  int nargin = args.length ();

  if (nargin == 0)
    rep->defaults ();
  else if (nargin % 2 == 0)
    {
      for (int i = 0; i < nargin; i += 2)
	{
	  caseless_str name = args(i).string_value ();

	  if (! error_state)
	    {
	      octave_value val = args(i+1);

	      if (val.is_string ())
		{
		  caseless_str tval = val.string_value ();

		  if (tval.compare ("default"))
		    val = get_default (name);
		  else if (tval.compare ("factory"))
		    val = get_factory_default (name);
		}

	      if (error_state)
		break;

	      rep->set (name, val);
	    }
	  else
	    error ("set: expecting argument %d to be a property name", i);
	}
    }
  else
    error ("set: invalid number of arguments");
}


graphics_handle
gh_manager::get_handle (const std::string& go_name)
{
  graphics_handle retval;

  if (go_name == "figure")
    {
      // We always want the lowest unused figure number.

      retval = 1;

      while (handle_map.find (retval) != handle_map.end ())
	retval++;
    }
  else
    {
      free_list_iterator p = handle_free_list.begin ();

      if (p != handle_free_list.end ())
	{
	  retval = *p;
	  handle_free_list.erase (p);
	}
      else
	{
	  static double maxrand = RAND_MAX + 2.0;

	  retval = graphics_handle (next_handle);

	  next_handle = ceil (next_handle) - 1.0 - (rand () + 1.0) / maxrand;
	}
    }

  return retval;
}

void
gh_manager::do_free (const graphics_handle& h)
{
  if (h.ok ())
    {
      if (h.value () != 0)
	{
	  iterator p = handle_map.find (h);

	  if (p != handle_map.end ())
	    {
	      p->second.get_properties ().set_beingdeleted (true);
	      p->second.get_properties ().execute_deletefcn ();

	      handle_map.erase (p);

	      if (h.value () < 0)
		handle_free_list.insert (h);
	    }
	  else
	    error ("graphics_handle::free: invalid object %g", h.value ());
	}
      else
	error ("graphics_handle::free: can't delete root figure");
    }
}

gh_manager *gh_manager::instance = 0;

static void
xset (const graphics_handle& h, const caseless_str& name,
      const octave_value& val)
{
  graphics_object obj = gh_manager::get_object (h);
  obj.set (name, val);
}

static void
xset (const graphics_handle& h, const octave_value_list& args)
{
  if (args.length () > 0)
    {
      graphics_object obj = gh_manager::get_object (h);
      obj.set (args);
    }
}


static octave_value
xget (const graphics_handle& h, const caseless_str& name)
{
  graphics_object obj = gh_manager::get_object (h);
  return obj.get (name);
}

static graphics_handle
reparent (const octave_value& ov, const std::string& who,
	  const std::string& property, const graphics_handle& new_parent,
	  bool adopt = true)
{
  graphics_handle h = octave_NaN;

  double val = ov.double_value ();

  if (! error_state)
    {
      h = gh_manager::lookup (val);

      if (h.ok ())
	{
	  graphics_object obj = gh_manager::get_object (h);
	  
	  graphics_handle parent_h = obj.get_parent ();

	  graphics_object parent_obj = gh_manager::get_object (parent_h);

	  parent_obj.remove_child (h);

	  if (adopt)
	    obj.set ("parent", new_parent.value ());
	  else
	    obj.reparent (new_parent);
	}
      else
	error ("%s: invalid graphics handle (= %g) for %s",
	       who.c_str (), val, property.c_str ());
    }
  else
    error ("%s: expecting %s to be a graphics handle",
	   who.c_str (), property.c_str ());

  return h;
}

// This function is NOT equivalent to the scripting language function gcf.
graphics_handle
gcf (void)
{
  octave_value val = xget (0, "currentfigure");

  return val.is_empty () ? octave_NaN : val.double_value ();
}

// This function is NOT equivalent to the scripting language function gca.
graphics_handle
gca (void)
{
  octave_value val = xget (gcf (), "currentaxes");

  return val.is_empty () ? octave_NaN : val.double_value ();
}

static void
adopt (const graphics_handle& p, const graphics_handle& h)
{
  graphics_object parent_obj = gh_manager::get_object (p);

  parent_obj.adopt (h);
}

static bool
is_handle (const graphics_handle& h)
{
  return h.ok ();
}

static bool
is_handle (double val)
{
  graphics_handle h = gh_manager::lookup (val);

  return h.ok ();
}

static bool
is_handle (const octave_value& val)
{
  return val.is_real_scalar () && is_handle (val.double_value ());
}

static bool
is_figure (double val)
{
  graphics_object obj = gh_manager::get_object (val);

  return obj && obj.isa ("figure");
}

static void
xcreatefcn (const graphics_handle& h)
{
  graphics_object obj = gh_manager::get_object (h);
  obj.get_properties ().execute_createfcn  ();
}

// ---------------------------------------------------------------------

static int
compare (const void *a_arg, const void *b_arg)
{
  double a = *(static_cast<const double *> (a_arg));
  double b = *(static_cast<const double *> (b_arg));

  return a > b ? 1 : (a < b) ? -1 : 0;
}

static Matrix
maybe_set_children (const Matrix& kids, const octave_value& val)
{
  const Matrix new_kids = val.matrix_value ();

  bool ok = true;

  if (! error_state)
    {
      if (kids.numel () == new_kids.numel ())
	{
	  Matrix t1 = kids;
	  Matrix t2 = new_kids;

	  t1.qsort (compare);
	  t2.qsort (compare);

	  if (t1 != t2)
	    ok = false;
	}      else
	ok = false;

      if (! ok)
	error ("set: new children must be a permutation of existing children");
    }
  else
    {
      ok = false;
      error ("set: expecting children to be array of graphics handles");
    }

  return ok ? new_kids : kids;
}

void
base_properties::set_from_list (base_graphics_object& obj,
				property_list& defaults)
{
  std::string go_name = graphics_object_name ();

  property_list::plist_map_const_iterator p = defaults.find (go_name);

  if (p != defaults.end ())
    {
      const property_list::pval_map_type pval_map = p->second;

      for (property_list::pval_map_const_iterator q = pval_map.begin ();
	   q != pval_map.end ();
	   q++)
	{
	  std::string pname = q->first;

	  obj.set (pname, q->second);

	  if (error_state)
	    {
	      error ("error setting default property %s", pname.c_str ());
	      break;
	    }
	}
    }
}

octave_value
base_properties::get (const caseless_str& name) const
{
  octave_value retval;

  if (name.compare ("tag"))
    retval = get_tag ();
  else if (name.compare ("type"))
    retval = get_type ();
  else if (name.compare ("__modified__"))
    retval = is_modified ();
  else if (name.compare ("parent"))
    retval = get_parent ().as_octave_value ();
  else if (name.compare ("children"))
    retval = children;
  else if (name.compare ("busyaction"))
    retval = get_busyaction ();
  else if (name.compare ("buttondownfcn"))
    retval = get_buttondownfcn ();
  else if (name.compare ("clipping"))
    retval = get_clipping ();
  else if (name.compare ("createfcn"))
    retval = get_createfcn ();
  else if (name.compare ("deletefcn"))
    retval = get_deletefcn ();
  else if (name.compare ("handlevisibility"))
    retval = get_handlevisibility ();
  else if (name.compare ("hittest"))
    retval = get_hittest ();
  else if (name.compare ("interruptible"))
    retval = get_interruptible ();
  else if (name.compare ("selected"))
    retval = get_selected ();
  else if (name.compare ("selectionhighlight"))
    retval = get_selectionhighlight ();
  else if (name.compare ("uicontextmenu"))
    retval = get_uicontextmenu ();
  else if (name.compare ("userdata"))
    retval = get_userdata ();
  else if (name.compare ("visible"))
    retval = get_visible ();
  else if (name.compare ("beingdeleted"))
    retval = get_beingdeleted ();
  else
  {
    std::map<caseless_str, property>::const_iterator it = all_props.find (name);

    if (it != all_props.end ())
      retval = it->second.get ();
    else
      error ("get: unknown property \"%s\"", name.c_str ());
  }

  return retval;
}

octave_value
base_properties::get (bool all) const
{
  Octave_map m;

  for (std::map<caseless_str, property>::const_iterator it = all_props.begin ();
       it != all_props.end (); ++it)
    if (all || ! it->second.is_hidden ())
      m.assign (it->second.get_name (), it->second.get ());

  m.assign ("tag", get_tag ());
  m.assign ("type", get_type ());
  if (all)
    m.assign ("__modified__", is_modified ());
  m.assign ("parent", get_parent ().as_octave_value ());
  m.assign ("children", children);
  m.assign ("busyaction", get_busyaction ());
  m.assign ("buttondownfcn", get_buttondownfcn ());
  m.assign ("clipping", get_clipping ());
  m.assign ("createfcn", get_createfcn ());
  m.assign ("deletefcn", get_deletefcn ());
  m.assign ("handlevisibility", get_handlevisibility ());
  m.assign ("hittest", get_hittest ());
  m.assign ("interruptible", get_interruptible ());
  m.assign ("selected", get_selected ());
  m.assign ("selectionhighlight", get_selectionhighlight ());
  m.assign ("uicontextmenu", get_uicontextmenu ());
  m.assign ("userdata", get_userdata ());
  m.assign ("visible", get_visible ());
  m.assign ("beingdeleted", get_beingdeleted ());

  return m;
}

void
base_properties::set (const caseless_str& name, const octave_value& val)
{
  if (name.compare ("tag"))
    set_tag (val);
  else if (name.compare ("__modified__"))
    __modified__ = val;
  else if (name.compare ("parent"))
    set_parent (val);
  else if (name.compare ("children"))
    maybe_set_children (children, val);
  else if (name.compare ("busyaction"))
    set_busyaction (val);
  else if (name.compare ("buttondownfcn"))
    set_buttondownfcn (val);
  else if (name.compare ("clipping"))
    set_clipping (val);
  else if (name.compare ("createfcn"))
    set_createfcn (val);
  else if (name.compare ("deletefcn"))
    set_deletefcn (val);
  else if (name.compare ("handlevisibility"))
    set_handlevisibility (val);
  else if (name.compare ("hittest"))
    set_hittest (val);
  else if (name.compare ("interruptible"))
    set_interruptible (val);
  else if (name.compare ("selected"))
    set_selected (val);
  else if (name.compare ("selectionhighlight"))
    set_selectionhighlight (val);
  else if (name.compare ("uicontextmenu"))
    set_uicontextmenu (val);
  else if (name.compare ("userdata"))
    set_userdata (val);
  else if (name.compare ("visible"))
    set_visible (val);
  else
  {
    std::map<caseless_str, property>::iterator it = all_props.find (name);

    if (it != all_props.end ())
      it->second.set (val);
    else
      error ("set: unknown property \"%s\"", name.c_str ());
  }

  if (! error_state && ! name.compare ("__modified__"))
    mark_modified ();
}

property
base_properties::get_property (const caseless_str& name) const
{
  std::map<caseless_str, property>::const_iterator it = all_props.find (name);

  if (it == all_props.end ())
    return property ();
  else
    return it->second;
}

void
base_properties::remove_child (const graphics_handle& h)
{
  octave_idx_type k = -1;
  octave_idx_type n = children.numel ();
  for (octave_idx_type i = 0; i < n; i++)
    {
      if (h.value () == children(i))
	{
	  k = i;
	  break;
	}
    }

  if (k >= 0)
    {
      Matrix new_kids (1, n-1);
      octave_idx_type j = 0;
      for (octave_idx_type i = 0; i < n; i++)
	{
	  if (i != k)
	    new_kids(j++) = children(i);
	}
      children = new_kids;
      mark_modified ();
    }
}

void
base_properties::set_parent (const octave_value& val)
{
  double tmp = val.double_value ();

  graphics_handle new_parent = octave_NaN;

  if (! error_state)
    {
      new_parent = gh_manager::lookup (tmp);

      if (new_parent.ok ())
	{
	  graphics_object parent_obj = gh_manager::get_object (get_parent ());

	  parent_obj.remove_child (__myhandle__);

	  parent = new_parent.as_octave_value ();

	  ::adopt (parent.handle_value (), __myhandle__);
	}
      else
	error ("set: invalid graphics handle (= %g) for parent", tmp);
    }
  else
    error ("set: expecting parent to be a graphics handle");
}

void
base_properties::mark_modified (void)
{
  __modified__ = "on";
  graphics_object parent_obj = gh_manager::get_object (get_parent ());
  if (parent_obj)
    parent_obj.mark_modified ();
}

void
base_properties::override_defaults (base_graphics_object& obj)
{
  graphics_object parent_obj = gh_manager::get_object (get_parent ());
  parent_obj.override_defaults (obj);
}

void
base_properties::update_axis_limits (const std::string& axis_type) const
{
  graphics_handle h = (get_type () == "axes") ? __myhandle__ : get_parent ();

  graphics_object obj = gh_manager::get_object (h);

  if (obj.isa ("axes"))
    obj.update_axis_limits (axis_type);
}

void
base_properties::delete_children (void)
{
  octave_idx_type n = children.numel ();

  for (octave_idx_type i = 0; i < n; i++)
    gh_manager::free (children(i));
}

graphics_backend
base_properties::get_backend (void) const
{
  graphics_object go = gh_manager::get_object (get_parent ());

  if (go)
    return go.get_backend ();
  else
    return graphics_backend ();
}

void
base_properties::update_boundingbox (void)
{
  Matrix kids = get_children ();

  for (int i = 0; i < kids.numel (); i++)
    {
      graphics_object go = gh_manager::get_object (kids(i));

      if (go.valid_object ())
	go.get_properties ().update_boundingbox ();
    }
}

// ---------------------------------------------------------------------

class gnuplot_backend : public base_graphics_backend
{
public:
  gnuplot_backend (void)
      : base_graphics_backend ("gnuplot") { }

  ~gnuplot_backend (void) { }

  bool is_valid (void) const { return true; }
 
  void close_figure (const octave_value& pstream) const
    {
      if (! pstream.is_empty())
	{
	  octave_value_list args;
	  Matrix fids = pstream.matrix_value ();

	  if (! error_state)
	    {
	      args(1) = "\nquit;\n";
	      args(0) = octave_value (fids (0));
	      feval ("fputs", args);
	      args.resize (1);
	      feval ("fflush", args);
	      feval ("pclose", args);
	      if (fids.numel () > 1)
		{
		  args(0) = octave_value (fids (1));
		  feval ("pclose", args);
		}
	    }
	}
    }

  void redraw_figure (const graphics_handle& fh) const
    {
      octave_value_list args;
      args(0) = fh.as_octave_value ();
      feval ("gnuplot_drawnow", args);
    }

  void print_figure (const graphics_handle& fh, const std::string& term,
		     const std::string& file, bool mono,
		     const std::string& debug_file) const
    {
      octave_value_list args;
      if (! debug_file.empty ())
	args(4) = debug_file;
      args(3) = mono;
      args(2) = file;
      args(1) = term;
      args(0) = fh.as_octave_value ();
      feval ("gnuplot_drawnow", args);
    }

  Matrix get_canvas_size (const graphics_handle&) const
    {
      Matrix sz (1, 2, 0.0);
      return sz;
    }

  double get_screen_resolution (void) const
    { return 72.0; }

  Matrix get_screen_size (void) const
    { return Matrix (1, 2, 0.0); }
};

graphics_backend
graphics_backend::default_backend (void)
{
  if (available_backends.size () == 0)
    register_backend (new gnuplot_backend ());

  return available_backends["gnuplot"];
}

std::map<std::string, graphics_backend> graphics_backend::available_backends;

// ---------------------------------------------------------------------

#include "graphics-props.cc"

// ---------------------------------------------------------------------

void
root_figure::properties::set_currentfigure (const octave_value& v)
{
  graphics_handle val (v);

  if (error_state)
    return;

  if (xisnan (val.value ()) || is_handle (val))
    {
      currentfigure = val;

      gh_manager::push_figure (val);
    }
  else
    gripe_set_invalid ("currentfigure");
}

void
root_figure::properties::set_callbackobject (const octave_value& v)
{
  graphics_handle val (v);

  if (error_state)
    return;

  if (xisnan (val.value ()))
    {
      if (! cbo_stack.empty ())
	{
	  val = cbo_stack.front ();

	  cbo_stack.pop_front ();
	}

      callbackobject = val;
    }
  else if (is_handle (val))
    {
      if (get_callbackobject ().ok ())
	cbo_stack.push_front (get_callbackobject ());

      callbackobject = val;
    }
  else
    gripe_set_invalid ("callbackobject");
}

property_list
root_figure::factory_properties = root_figure::init_factory_properties ();

// ---------------------------------------------------------------------

void
figure::properties::set_currentaxes (const octave_value& v)
{
  graphics_handle val (v);

  if (error_state)
    return;

  if (xisnan (val.value ()) || is_handle (val))
    currentaxes = val;
  else
    gripe_set_invalid ("currentaxes");
}

void
figure::properties::set_visible (const octave_value& val)
{
  std::string s = val.string_value ();

  if (! error_state)
    {
      if (s == "on")
	xset (0, "currentfigure", __myhandle__.value ());

      visible = val;
    }
}

void
figure::properties::close (bool pop)
{
  if (backend)
    backend.close_figure (get___plot_stream__ ());

  if (pop)
    {
      gh_manager::pop_figure (__myhandle__);

      graphics_handle cf = gh_manager::current_figure ();

      xset (0, "currentfigure", cf.value ());
    }
}

Matrix
figure::properties::get_boundingbox (bool) const
{
  graphics_backend b = get_backend ();
  // FIXME: screen size should be obtained from root object
  Matrix screen_size = b.get_screen_size ();
  Matrix pos;

  pos = convert_position (get_position ().matrix_value (), get_units (),
			  "pixels", screen_size, b);

  pos(0)--;
  pos(1)--;
  pos(1) = screen_size(1) - pos(1) - pos(3);

  return pos;
}

void
figure::properties::set_boundingbox (const Matrix& bb)
{
  graphics_backend b = get_backend ();
  // FIXME: screen size should be obtained from root object
  Matrix screen_size = b.get_screen_size ();
  Matrix pos = bb;

  pos(1) = screen_size(1) - pos(1) - pos(3);
  pos(1)++;
  pos(0)++;
  pos = convert_position (pos, "pixels", get_units (), screen_size, b);

  set_position (pos);
}

void
figure::properties::set_position (const octave_value& v)
{
  if (! error_state)
    {
      Matrix old_bb, new_bb;

      old_bb = get_boundingbox ();
      position = v;
      new_bb = get_boundingbox ();

      if (old_bb != new_bb)
	{
	  // FIXME: maybe this should be converted into a more generic
	  //        call like "update_gui (this)"
	  get_backend ().set_figure_position (__myhandle__, new_bb);
	  if (old_bb(2) != new_bb(2) || old_bb(3) != new_bb(3))
	    {
	      execute_resizefcn ();
	      update_boundingbox ();
	    }
	}

      mark_modified ();
    }
}

octave_value
figure::get_default (const caseless_str& name) const
{
  octave_value retval = default_properties.lookup (name);

  if (retval.is_undefined ())
    {
      graphics_handle parent = get_parent ();
      graphics_object parent_obj = gh_manager::get_object (parent);

      retval = parent_obj.get_default (name);
    }

  return retval;
}

// ---------------------------------------------------------------------

void
axes::properties::set_title (const octave_value& v)
{
  graphics_handle val = ::reparent (v, "set", "title", __myhandle__, false);

  if (! error_state)
    {
      gh_manager::free (title.handle_value ());
      title = val;
    }
}

void
axes::properties::set_xlabel (const octave_value& v)
{
  graphics_handle val = ::reparent (v, "set", "xlabel", __myhandle__, false);

  if (! error_state)
    {
      gh_manager::free (xlabel.handle_value ());
      xlabel = val;
    }
}

void
axes::properties::set_ylabel (const octave_value& v)
{
  graphics_handle val = ::reparent (v, "set", "ylabel", __myhandle__, false);

  if (! error_state)
    {
      gh_manager::free (ylabel.handle_value ());
      ylabel = val;
    }
}

void
axes::properties::set_zlabel (const octave_value& v)
{
  graphics_handle val = ::reparent (v, "set", "zlabel", __myhandle__, false);

  if (! error_state)
    {
      gh_manager::free (zlabel.handle_value ());
      zlabel = val;
    }
}

void
axes::properties::set_defaults (base_graphics_object& obj,
				const std::string& mode)
{
  position = default_axes_position ();
  title = graphics_handle ();
  box = "on";
  key = "off";
  keybox = "off";
  keypos = 1.0;
  colororder = default_colororder ();
  dataaspectratio = Matrix (1, 3, 1.0);
  dataaspectratiomode = "auto";
  layer = "bottom";

  Matrix tlim (1, 2, 0.0);
  tlim(1) = 1;
  xlim = tlim;
  ylim = tlim;
  zlim = tlim;
  
  Matrix cl (1, 2, 0);
  cl(1) = 1;
  clim = cl;
  
  xlimmode = "auto";
  ylimmode = "auto";
  zlimmode = "auto";
  climmode = "auto";
  xlabel = graphics_handle ();
  ylabel = graphics_handle ();
  zlabel = graphics_handle ();
  xgrid = "off";
  ygrid = "off";
  zgrid = "off";
  xminorgrid = "off";
  yminorgrid = "off";
  zminorgrid = "off";
  xtick = Matrix ();
  ytick = Matrix ();
  ztick = Matrix ();
  xtickmode = "auto";
  ytickmode = "auto";
  ztickmode = "auto";
  xticklabel = "";
  yticklabel = "";
  zticklabel = "";
  xticklabelmode = "auto";
  yticklabelmode = "auto";
  zticklabelmode = "auto";
  color = color_values (1, 1, 1);
  xcolor = color_values ("black");
  ycolor = color_values ("black");
  zcolor = color_values ("black");
  xscale = "linear";
  yscale = "linear";
  zscale = "linear";
  xdir = "normal";
  ydir = "normal";
  zdir = "normal";
  yaxislocation = "left";
  xaxislocation = "bottom";

  // Note: camera properties will be set through update_transform
  camerapositionmode = "auto";
  cameratargetmode = "auto";
  cameraupvectormode = "auto";
  cameraviewanglemode = "auto";
  plotboxaspectratio = Matrix (1, 3, 1.0);
  drawmode = "normal";
  fontangle = "normal";
  fontname = "Helvetica";
  fontsize = 12;
  fontunits = "points";
  fontweight = "normal";
  gridlinestyle = ":";
  linestyleorder = "-";
  linewidth = 0.5;
  minorgridlinestyle = ":";
  // Note: plotboxaspectratio will be set through update_aspectratiors
  plotboxaspectratiomode = "auto";
  projection = "orthographic";
  tickdir = "in";
  tickdirmode = "auto";
  ticklength = Matrix (1, 2, 0.1);
  tightinset = Matrix (1, 4, 0.0);

  sx = "linear";
  sy = "linear";
  sz = "linear";

  Matrix tview (1, 2, 0.0);
  tview(1) = 90;
  view = tview;

  visible = "on";
  nextplot = "replace";

  // FIXME -- this is not quite right; we should preserve
  // "position" and "units".

  if (mode != "replace")
    {
      Matrix touterposition (1, 4, 0.0);
      touterposition(2) = 1;
      touterposition(3) = 1;
      outerposition = touterposition;
    }

  activepositionproperty = "outerposition";
  __colorbar__  = "none";

  delete_children ();

  children = Matrix ();

  update_transform ();

  override_defaults (obj);
}

graphics_handle
axes::properties::get_title (void) const
{
  if (! title.handle_value ().ok ())
    title = gh_manager::make_graphics_handle ("text", __myhandle__);

  return title.handle_value ();
}

graphics_handle
axes::properties::get_xlabel (void) const
{
  if (! xlabel.handle_value ().ok ())
    xlabel = gh_manager::make_graphics_handle ("text", __myhandle__);

  return xlabel.handle_value ();
}

graphics_handle
axes::properties::get_ylabel (void) const
{
  if (! ylabel.handle_value ().ok ())
    ylabel = gh_manager::make_graphics_handle ("text", __myhandle__);

  return ylabel.handle_value ();
}

graphics_handle
axes::properties::get_zlabel (void) const
{
  if (! zlabel.handle_value ().ok ())
    zlabel = gh_manager::make_graphics_handle ("text", __myhandle__);

  return zlabel.handle_value ();
}

void
axes::properties::remove_child (const graphics_handle& h)
{
  if (title.handle_value ().ok () && h == title.handle_value ())
    title = gh_manager::make_graphics_handle ("text", __myhandle__);
  else if (xlabel.handle_value ().ok () && h == xlabel.handle_value ())
    xlabel = gh_manager::make_graphics_handle ("text", __myhandle__);
  else if (ylabel.handle_value ().ok () && h == ylabel.handle_value ())
    ylabel = gh_manager::make_graphics_handle ("text", __myhandle__);
  else if (zlabel.handle_value ().ok () && h == zlabel.handle_value ())
    zlabel = gh_manager::make_graphics_handle ("text", __myhandle__);
  else
    base_properties::remove_child (h);
}

void
axes::properties::delete_children (void)
{
  base_properties::delete_children ();

  gh_manager::free (title.handle_value ());
  gh_manager::free (xlabel.handle_value ());
  gh_manager::free (ylabel.handle_value ());
  gh_manager::free (zlabel.handle_value ());
}

inline Matrix
xform_matrix (void)
{
  Matrix m (4, 4, 0.0);
  for (int i = 0; i < 4; i++)
    m(i,i) = 1;
  return m;
}

inline ColumnVector
xform_vector (void)
{
  ColumnVector v (4, 0.0);
  v(3) = 1;
  return v;
}

inline ColumnVector
xform_vector (double x, double y, double z)
{
  ColumnVector v (4, 1.0);
  v(0) = x; v(1) = y; v(2) = z;
  return v;
}

inline ColumnVector
transform (const Matrix& m, double x, double y, double z)
{
  return (m * xform_vector (x, y, z));
}

inline Matrix
xform_scale (double x, double y, double z)
{
  Matrix m (4, 4, 0.0);
  m(0,0) = x; m(1,1) = y; m(2,2) = z; m(3,3) = 1;
  return m;
}

inline Matrix
xform_translate (double x, double y, double z)
{
  Matrix m = xform_matrix ();
  m(0,3) = x; m(1,3) = y; m(2,3) = z; m(3,3) = 1;
  return m;
}

inline void
scale (Matrix& m, double x, double y, double z)
{
  m = m * xform_scale (x, y, z);
}

inline void
translate (Matrix& m, double x, double y, double z)
{
  m = m * xform_translate (x, y, z);
}

inline void
xform (ColumnVector& v, const Matrix& m)
{
  v = m*v;
}

inline void
scale (ColumnVector& v, double x, double y, double z)
{
  v(0) *= x;
  v(1) *= y;
  v(2) *= z;
}

inline void
translate (ColumnVector& v, double x, double y, double z)
{
  v(0) += x;
  v(1) += y;
  v(2) += z;
}

inline void
normalize (ColumnVector& v)
{
  double fact = 1.0/sqrt(v(0)*v(0)+v(1)*v(1)+v(2)*v(2));
  scale (v, fact, fact, fact);
}

inline double
dot (const ColumnVector& v1, const ColumnVector& v2)
{
  return (v1(0)*v2(0)+v1(1)*v2(1)+v1(2)*v2(2));
}

inline double
norm (const ColumnVector& v)
{
  return sqrt (dot (v, v));
}

inline ColumnVector
cross (const ColumnVector& v1, const ColumnVector& v2)
{
  ColumnVector r = xform_vector ();
  r(0) = v1(1)*v2(2)-v1(2)*v2(1);
  r(1) = v1(2)*v2(0)-v1(0)*v2(2);
  r(2) = v1(0)*v2(1)-v1(1)*v2(0);
  return r;
}

inline Matrix
unit_cube (void)
{
  static double data[32] = {
      0,0,0,1,
      1,0,0,1,
      0,1,0,1,
      0,0,1,1,
      1,1,0,1,
      1,0,1,1,
      0,1,1,1,
      1,1,1,1};
  Matrix m (4, 8);
  memcpy (m.fortran_vec (), data, sizeof(double)*32);
  return m;
}

inline ColumnVector
cam2xform (const Array<double>& m)
{
  ColumnVector retval (4, 1.0);
  memcpy (retval.fortran_vec (), m.fortran_vec (), sizeof(double)*3);
  return retval;
}

inline RowVector
xform2cam (const ColumnVector& v)
{
  return v.extract_n (0, 3).transpose ();
}

void
axes::properties::update_camera (void)
{
  double xd = (xdir_is ("normal") ? 1 : -1);
  double yd = (ydir_is ("normal") ? 1 : -1);
  double zd = (zdir_is ("normal") ? 1 : -1);

  Matrix xlimits = sx.scale (get_xlim ().matrix_value ());
  Matrix ylimits = sy.scale (get_ylim ().matrix_value ());
  Matrix zlimits = sz.scale (get_zlim ().matrix_value ());

  double xo = xlimits(xd > 0 ? 0 : 1);
  double yo = ylimits(yd > 0 ? 0 : 1);
  double zo = zlimits(zd > 0 ? 0 : 1);
  
  Matrix pb  = get_plotboxaspectratio ().matrix_value ();
  
  bool autocam = (camerapositionmode_is ("auto")
		  && cameratargetmode_is ("auto")
	    	  && cameraupvectormode_is ("auto")
		  && cameraviewanglemode_is ("auto"));
  bool dowarp = (autocam && dataaspectratiomode_is("auto")
		 && plotboxaspectratiomode_is ("auto"));

  ColumnVector c_eye (xform_vector ());
  ColumnVector c_center (xform_vector ());
  ColumnVector c_upv (xform_vector ());
  
  if (cameratargetmode_is ("auto"))
    {
      c_center(0) = (xlimits(0)+xlimits(1))/2;
      c_center(1) = (ylimits(0)+ylimits(1))/2;
      c_center(2) = (zlimits(0)+zlimits(1))/2;

      cameratarget = xform2cam (c_center);
    }
  else
    c_center = cam2xform (get_cameratarget ().matrix_value ());
  
  if (camerapositionmode_is ("auto"))
    {
      Matrix tview = get_view ().matrix_value ();
      double az = tview(0), el = tview(1);
      double d = 5*sqrt(pb(0)*pb(0)+pb(1)*pb(1)+pb(2)*pb(2));

      if (el == 90 || el == -90)
	c_eye(2) = d*signum(el);
      else
	{
	  az *= M_PI/180.0;
	  el *= M_PI/180.0;
	  c_eye(0) = d*cos(el)*sin(az);
	  c_eye(1) = -d*cos(el)*cos(az);
	  c_eye(2) = d*sin(el);
	}
      c_eye(0) = c_eye(0)*(xlimits(1)-xlimits(0))/(xd*pb(0))+c_center(0);
      c_eye(1) = c_eye(1)*(ylimits(1)-ylimits(0))/(yd*pb(1))+c_center(1);
      c_eye(2) = c_eye(2)*(zlimits(1)-zlimits(0))/(zd*pb(2))+c_center(2);

      cameraposition = xform2cam (c_eye);
    }
  else
    c_eye = cam2xform (get_cameraposition ().matrix_value ());

  if (cameraupvectormode_is ("auto"))
    {
      Matrix tview = get_view ().matrix_value ();
      double az = tview(0), el = tview(1);

      if (el == 90 || el == -90)
	{
	  c_upv(0) = -sin(az*M_PI/180.0)*(xlimits(1)-xlimits(0))/pb(0);
	  c_upv(1) = cos(az*M_PI/180.0)*(ylimits(1)-ylimits(0))/pb(1);
	}
      else
	c_upv(2) = 1;

      cameraupvector = xform2cam (c_upv);
    }
  else
    c_upv = cam2xform (get_cameraupvector ().matrix_value ());

  Matrix x_view = xform_matrix ();
  Matrix x_projection = xform_matrix ();
  Matrix x_viewport = xform_matrix ();
  Matrix x_normrender = xform_matrix ();
  Matrix x_pre = xform_matrix ();
  
  x_render = xform_matrix ();
  x_render_inv = xform_matrix ();

  scale (x_pre, pb(0), pb(1), pb(2));
  translate (x_pre, -0.5, -0.5, -0.5);
  scale (x_pre, xd/(xlimits(1)-xlimits(0)), yd/(ylimits(1)-ylimits(0)),
	 zd/(zlimits(1)-zlimits(0)));
  translate (x_pre, -xo, -yo, -zo);

  xform (c_eye, x_pre);
  xform (c_center, x_pre);
  scale (c_upv, pb(0)/(xlimits(1)-xlimits(0)), pb(1)/(ylimits(1)-ylimits(0)), 
	 pb(2)/(zlimits(1)-zlimits(0)));
  translate (c_center, -c_eye(0), -c_eye(1), -c_eye(2));

  ColumnVector F (c_center), f (F), UP (c_upv);
  normalize (f);
  normalize (UP);

  if (std::abs (dot (f, UP)) > 1e-15)
    {
      double fa = 1/sqrt(1-f(2)*f(2));
      scale (UP, fa, fa, fa);
    }

  ColumnVector s = cross (f, UP);
  ColumnVector u = cross (s, f);

  scale (x_view, 1, 1, -1);
  Matrix l = xform_matrix ();
  l(0,0) = s(0); l(0,1) = s(1); l(0,2) = s(2);
  l(1,0) = u(0); l(1,1) = u(1); l(1,2) = u(2);
  l(2,0) = -f(0); l(2,1) = -f(1); l(2,2) = -f(2);
  x_view = x_view * l;
  translate (x_view, -c_eye(0), -c_eye(1), -c_eye(2));
  scale (x_view, pb(0), pb(1), pb(2));
  translate (x_view, -0.5, -0.5, -0.5);

  Matrix x_cube = x_view * unit_cube ();
  ColumnVector cmin = x_cube.row_min (), cmax = x_cube.row_max ();
  double xM = cmax(0)-cmin(0);
  double yM = cmax(1)-cmin(1);

  Matrix bb = get_boundingbox (true);

  double v_angle;

  if (cameraviewanglemode_is ("auto"))
    {
      double af;

      // FIXME: Was this really needed? When compared to Matlab, it
      // does not seem to be required. Need investigation with concrete
      // backend to see results visually.
      if (false && dowarp)
        af = 1.0 / (xM > yM ? xM : yM);
      else
        {
          if ((bb(2)/bb(3)) > (xM/yM))
            af = 1.0 / yM;
          else
            af = 1.0 / xM;
        }
      v_angle = 2 * (180.0 / M_PI) * atan (1 / (2 * af * norm (F)));

      cameraviewangle = v_angle;
    }
  else
    v_angle = get_cameraviewangle ();

  double pf = 1 / (2 * tan ((v_angle / 2) * M_PI / 180.0) * norm (F));
  scale (x_projection, pf, pf, 1);

  if (dowarp)
    {
      xM *= pf;
      yM *= pf;
      translate (x_viewport, bb(0)+bb(2)/2, bb(1)+bb(3)/2, 0);
      scale (x_viewport, bb(2)/xM, -bb(3)/yM, 1);
    }
  else
    {
      double pix = 1;
      if (autocam)
	{
	  if ((bb(2)/bb(3)) > (xM/yM))
	    pix = bb(3);
	  else
	    pix = bb(2);
	}
      else
	pix = (bb(2) < bb(3) ? bb(2) : bb(3));
      translate (x_viewport, bb(0)+bb(2)/2, bb(1)+bb(3)/2, 0);
      scale (x_viewport, pix, -pix, 1);
    }

  x_normrender = x_viewport * x_projection * x_view;

  x_cube = x_normrender * unit_cube ();
  cmin = x_cube.row_min ();
  cmax = x_cube.row_max ();
  x_zlim.resize (1, 2);
  x_zlim(0) = cmin(2);
  x_zlim(1) = cmax(2);

  x_render = x_normrender;
  scale (x_render, xd/(xlimits(1)-xlimits(0)), yd/(ylimits(1)-ylimits(0)),
	 zd/(zlimits(1)-zlimits(0)));
  translate (x_render, -xo, -yo, -zo);

  x_viewtransform = x_view;
  x_projectiontransform = x_projection;
  x_viewporttransform = x_viewport;
  x_normrendertransform = x_normrender;
  x_rendertransform = x_render;

  x_render_inv = x_render.inverse ();

  // Note: these matrices are a slight modified version of the regular
  // matrices, more suited for OpenGL rendering (x_gl_mat1 => light
  // => x_gl_mat2)
  x_gl_mat1 = x_view;
  scale (x_gl_mat1, xd/(xlimits(1)-xlimits(0)), yd/(ylimits(1)-ylimits(0)),
	 zd/(zlimits(1)-zlimits(0)));
  translate (x_gl_mat1, -xo, -yo, -zo);
  x_gl_mat2 = x_viewport * x_projection;
}

void
axes::properties::update_aspectratios (void)
{
  Matrix xlimits = get_xlim ().matrix_value ();
  Matrix ylimits = get_ylim ().matrix_value ();
  Matrix zlimits = get_zlim ().matrix_value ();

  double dx = (xlimits(1)-xlimits(0));
  double dy = (ylimits(1)-ylimits(0));
  double dz = (zlimits(1)-zlimits(0));

  if (dataaspectratiomode_is ("auto"))
    {
      double dmin = xmin (xmin (dx, dy), dz);
      Matrix da (1, 3, 0.0);

      da(0) = dx/dmin;
      da(1) = dy/dmin;
      da(2) = dz/dmin;

      dataaspectratio = da;
    }

  if (plotboxaspectratiomode_is ("auto"))
    {
      if (dataaspectratiomode_is ("auto"))
	plotboxaspectratio = Matrix (1, 3, 1.0);
      else
	{
	  Matrix da = get_dataaspectratio ().matrix_value ();
	  Matrix pba (1, 3, 0.0);

	  pba(0) = dx/da(0);
	  pba(1) = dy/da(1);
	  pba(2) = dz/da(2);
	}
    }
  
  // FIXME: if plotboxaspectratiomode is "manual", limits
  // and/or dataaspectratio might be adapted
}

// The INTERNAL flag defines whether position or outerposition is used.

Matrix
axes::properties::get_boundingbox (bool internal) const
{
  graphics_object obj = gh_manager::get_object (get_parent ());
  Matrix parent_bb = obj.get_properties ().get_boundingbox (true);
  Matrix pos = (internal ?
		  get_position ().matrix_value ()
		  : get_outerposition ().matrix_value ());


  pos = convert_position (pos, get_units (), "pixels",
			  parent_bb.extract_n (0, 2, 1, 2), get_backend ());
  pos(0)--;
  pos(1)--;
  pos(1) = parent_bb(3) - pos(1) - pos(3);

  return pos;
}

ColumnVector
graphics_xform::xform_vector (double x, double y, double z)
{ return ::xform_vector (x, y, z); }

Matrix
graphics_xform::xform_eye (void)
{ return ::xform_matrix (); }

ColumnVector
graphics_xform::transform (double x, double y, double z,
			   bool use_scale) const
{
  if (use_scale)
    {
      x = sx.scale (x);
      y = sy.scale (y);
      z = sz.scale (z);
    }

  return ::transform (xform, x, y, z);
}

ColumnVector
graphics_xform::untransform (double x, double y, double z,
			     bool use_scale) const
{
  ColumnVector v = ::transform (xform_inv, x, y, z);

  if (use_scale)
    {
      v(0) = sx.unscale (v(0));
      v(1) = sy.unscale (v(1));
      v(2) = sz.unscale (v(2));
    }

  return v;
}

octave_value
axes::get_default (const caseless_str& name) const
{
  octave_value retval = default_properties.lookup (name);

  if (retval.is_undefined ())
    {
      graphics_handle parent = get_parent ();
      graphics_object parent_obj = gh_manager::get_object (parent);

      retval = parent_obj.get_default (name);
    }

  return retval;
}

// FIXME: Remove in case all data_property are converted into
// array_property
static void
check_limit_vals (double& min_val, double& max_val, double& min_pos,
		  const data_property& data)
{
  double val = data.min_val ();
  if (! (xisinf (val) || xisnan (val)) && val < min_val)
    min_val = val;
  val = data.max_val ();
  if (! (xisinf (val) || xisnan (val)) && val > max_val)
    max_val = val;
  val = data.min_pos ();
  if (! (xisinf (val) || xisnan (val)) && val > 0 && val < min_pos)
    min_pos = val;
}

// FIXME: Maybe this should go into array_property class?
static void
check_limit_vals (double& min_val, double& max_val, double& min_pos,
		  const array_property& data)
{
  double val = data.min_val ();
  if (! (xisinf (val) || xisnan (val)) && val < min_val)
    min_val = val;
  val = data.max_val ();
  if (! (xisinf (val) || xisnan (val)) && val > max_val)
    max_val = val;
  val = data.min_pos ();
  if (! (xisinf (val) || xisnan (val)) && val > 0 && val < min_pos)
    min_pos = val;
}

// magform(x) Returns (a, b), where x = a * 10^b, a >= 1., and b is
// integral.

static void magform (double x, double& a, int& b)
{
  if (x == 0)
    {
      a = 0;
      b = 0;
    }
  else
    {
      double l = std::log10 (std::abs (x));
      double r = std::fmod (l, 1.);
      a = std::pow (10.0, r);
      b = static_cast<int> (l-r);
      if (a < 1)
	{
	  a *= 10;
	  b -= 1;
	}

      if (x < 0)
	a = -a;
    }
}

// A translation from Tom Holoryd's python code at
// http://kurage.nimh.nih.gov/tomh/tics.py
// FIXME -- add log ticks

double
axes::properties::calc_tick_sep (double lo, double hi)
{
  int ticint = 5;

  // Reference: Lewart, C. R., "Algorithms SCALE1, SCALE2, and
  // SCALE3 for Determination of Scales on Computer Generated
  // Plots", Communications of the ACM, 10 (1973), 639-640.
  // Also cited as ACM Algorithm 463.

  double a;
  int b, x;

  magform ((hi-lo)/ticint, a, b);

  static const double sqrt_2 = sqrt (2.0);
  static const double sqrt_10 = sqrt (10.0);
  static const double sqrt_50 = sqrt (50.0);

  if (a < sqrt_2)
    x = 1;
  else if (a < sqrt_10)
    x = 2;
  else if (a < sqrt_50)
    x = 5;
  else
    x = 10;

  return x * std::pow (10., b);

}

// Attempt to make "nice" limits from the actual max and min of the
// data.  For log plots, we will also use the smallest strictly positive
// value.

Matrix
axes::properties::get_axis_limits (double xmin, double xmax, double min_pos, bool logscale)
{
  Matrix retval;

  double min_val = xmin;
  double max_val = xmax;

  if (! (xisinf (min_val) || xisinf (max_val)))
    {
      if (logscale)
	{
	  if (xisinf (min_pos))
	    {
	      // warning ("axis: logscale with no positive values to plot");
	      return retval;
	    }

	  if (min_val <= 0)
	    {
	      warning ("axis: omitting nonpositive data in log plot");
	      min_val = min_pos;
	    }
	  // FIXME -- maybe this test should also be relative?
	  if (std::abs (min_val - max_val) < sqrt (DBL_EPSILON))
	    {
	      min_val *= 0.9;
	      max_val *= 1.1;
	    }
	  min_val = pow (10, floor (log10 (min_val)));
	  max_val = pow (10, ceil (log10 (max_val)));
	}
      else
	{
	  if (min_val == 0 && max_val == 0)
	    {
	      min_val = -1;
	      max_val = 1;
	    }
	  // FIXME -- maybe this test should also be relative?
	  else if (std::abs (min_val - max_val) < sqrt (DBL_EPSILON))
	    {
	      min_val -= 0.1 * std::abs (min_val);
	      max_val += 0.1 * std::abs (max_val);
	    }

	  double tick_sep = calc_tick_sep (min_val , max_val);
	  min_val = tick_sep * std::floor (min_val / tick_sep);
	  max_val = tick_sep * ceil (max_val / tick_sep);
	}
    }

  retval.resize (1, 2);

  retval(0) = min_val;
  retval(1) = max_val;

  return retval;
}

void 
axes::properties::calc_ticks_and_lims (array_property& lims, array_property& ticks, bool limmode_is_auto)
{

  // FIXME -- add log ticks and lims

  if (lims.get ().is_empty ())
    return;

  double lo = (lims.get ().matrix_value ()) (0);
  double hi = (lims.get ().matrix_value ()) (1);
  
  double tick_sep = calc_tick_sep (lo , hi);

  int i1 = static_cast<int> (std::floor (lo / tick_sep));
  int i2 = static_cast<int> (std::ceil (hi / tick_sep));

  if (limmode_is_auto)
    {
      // adjust limits to include min and max tics
      Matrix tmp_lims (1,2);
      tmp_lims(0) = tick_sep * i1;
      tmp_lims(1) = tick_sep * i2;

      lims = tmp_lims;
    }
  else
    {
      // adjust min and max tics if they are out of limits
      i1 = static_cast<int> (std::ceil (lo / tick_sep));
      i2 = static_cast<int> (std::floor (hi / tick_sep));
    }
      
  Matrix tmp_ticks (1, i2-i1+1);
  for (int i = 0; i <= i2-i1; i++)
    tmp_ticks (i) = tick_sep * (i+i1);

  ticks = tmp_ticks;
}

static bool updating_axis_limits = false;

void
axes::update_axis_limits (const std::string& axis_type)
{
  if (updating_axis_limits)
    return;

  Matrix kids = xproperties.get_children ();

  octave_idx_type n = kids.numel ();

  double min_val = octave_Inf;
  double max_val = -octave_Inf;
  double min_pos = octave_Inf;

  char update_type = 0;

  Matrix limits;

  if (axis_type == "xdata" || axis_type == "xscale"
      || axis_type == "xldata" || axis_type == "xudata"
      || axis_type == "xlimmode")
    {
      if (xproperties.xlimmode_is ("auto"))
	{
	  for (octave_idx_type i = 0; i < n; i++)
	    {
	      graphics_object obj = gh_manager::get_object (kids(i));

	      if (obj.isa ("line") || obj.isa ("image")
		  || obj.isa ("patch") || obj.isa ("surface"))
		{
		  data_property xdata = obj.get_xdata_property ();

		  check_limit_vals (min_val, max_val, min_pos, xdata);

		  if (obj.isa ("line"))
		    {
		      data_property xldata = obj.get_xldata_property ();
		      data_property xudata = obj.get_xudata_property ();

		      check_limit_vals (min_val, max_val, min_pos, xldata);
		      check_limit_vals (min_val, max_val, min_pos, xudata);
		    }
		}
	    }

	  limits = xproperties.get_axis_limits (min_val, max_val, min_pos,
						xproperties.xscale_is ("log"));

	  update_type = 'x';
	}
    }
  else if (axis_type == "ydata" || axis_type == "yscale"
	   || axis_type == "ldata" || axis_type == "udata"
	   || axis_type == "ylimmode")
    {
      if (xproperties.ylimmode_is ("auto"))
	{
	    for (octave_idx_type i = 0; i < n; i++)
	    {
	      graphics_object obj = gh_manager::get_object (kids(i));

	      if (obj.isa ("line") || obj.isa ("image")
		|| obj.isa ("patch") || obj.isa ("surface"))
		{
		  data_property ydata = obj.get_ydata_property ();

		  check_limit_vals (min_val, max_val, min_pos, ydata);

		  if (obj.isa ("line"))
		    {
		      data_property ldata = obj.get_ldata_property ();
		      data_property udata = obj.get_udata_property ();

		      check_limit_vals (min_val, max_val, min_pos, ldata);
		      check_limit_vals (min_val, max_val, min_pos, udata);
		    }
		}
	    }

	  limits = xproperties.get_axis_limits (min_val, max_val, min_pos,
						xproperties.yscale_is ("log"));

	  update_type = 'y';
	}
    }
  else if (axis_type == "zdata" || axis_type == "zscale"
	   || axis_type == "zlimmode")
    {
      if (xproperties.zlimmode_is ("auto"))
	{
	  for (octave_idx_type i = 0; i < n; i++)
	    {
	      graphics_object obj = gh_manager::get_object (kids(i));

	      if (obj.isa ("line") || obj.isa ("patch") || obj.isa ("surface"))
		{
		  data_property zdata = obj.get_zdata_property ();

		  check_limit_vals (min_val, max_val, min_pos, zdata);
		}
	    }

	  limits = xproperties.get_axis_limits (min_val, max_val, min_pos,
						xproperties.zscale_is ("log"));

	  update_type = 'z';
	}
    }
  else if (axis_type == "cdata" || axis_type == "climmode")
    {
      if (xproperties.climmode_is ("auto"))
	{
	  for (octave_idx_type i = 0; i < n; i++)
	    {
	      graphics_object obj = gh_manager::get_object (kids(i));

	      if (obj.isa ("image") || obj.isa ("patch") || obj.isa ("surface"))
		{
		  array_property cdata = obj.get_cdata_property ();

		  check_limit_vals (min_val, max_val, min_pos, cdata);
		}
	    }

	  if (min_val == max_val)
	    max_val = min_val + 1;

	  limits.resize (1, 2);

	  limits(0) = min_val;
	  limits(1) = max_val;

	  update_type = 'c';
	}

    }

  unwind_protect_bool (updating_axis_limits);
  updating_axis_limits = true;

  switch (update_type)
    {
    case 'x':
      xproperties.set_xlim (limits);
      xproperties.set_xlimmode ("auto");
      xproperties.update_xlim ();
      break;

    case 'y':
      xproperties.set_ylim (limits);
      xproperties.set_ylimmode ("auto");
      xproperties.update_ylim ();
      break;

    case 'z':
      xproperties.set_zlim (limits);
      xproperties.set_zlimmode ("auto");
      xproperties.update_zlim ();
      break;

    case 'c':
      xproperties.set_clim (limits);
      xproperties.set_climmode ("auto");
      break;

    default:
      break;
    }

  xproperties.update_transform ();

  unwind_protect::run ();
}

// ---------------------------------------------------------------------

// Note: "line" code is entirely auto-generated

// ---------------------------------------------------------------------

// Note: "text" code is entirely auto-generated

// ---------------------------------------------------------------------

// Note: "image" code is entirely auto-generated

// ---------------------------------------------------------------------

octave_value
patch::properties::get_color_data (void) const
{
  return convert_cdata (*this, get_facevertexcdata (),
			cdatamapping_is ("scaled"), 2);
}

// ---------------------------------------------------------------------

octave_value
surface::properties::get_color_data (void) const
{
  return convert_cdata (*this, get_cdata (), cdatamapping_is ("scaled"), 3);
}

inline void
cross_product (double x1, double y1, double z1,
	       double x2, double y2, double z2,
	       double& x, double& y, double& z)
{
  x += (y1 * z2 - z1 * y2);
  y += (z1 * x2 - x1 * z2);
  z += (x1 * y2 - y1 * x2);
}

void
surface::properties::update_normals (void)
{
  if (normalmode_is ("auto"))
    {
      Matrix x = get_xdata ().matrix_value ();
      Matrix y = get_ydata ().matrix_value ();
      Matrix z = get_zdata ().matrix_value ();

      int p = z.columns (), q = z.rows ();
      int i1, i2, i3;
      int j1, j2, j3;

      bool x_mat = (x.rows () == q);
      bool y_mat = (y.columns () == p);

      NDArray n (dim_vector (q, p, 3), 0.0);

      i1 = i2 = i3 = 0;
      j1 = j2 = j3 = 0;

      // FIXME: normal computation at boundaries
      for (int i = 1; i < (p-1); i++)
	{
	  if (y_mat)
	    {
	      i1 = i-1;
	      i2 = i;
	      i3 = i+1;
	    }

	  for (int j = 1; j < (q-1); j++)
	    {
	      if (x_mat)
		{
		  j1 = j-1;
		  j2 = j;
		  j3 = j+1;
		}

	      double& nx = n(j, i, 0);
	      double& ny = n(j, i, 1);
	      double& nz = n(j, i, 2);

	      cross_product (x(j3,i)-x(j2,i), y(j+1,i2)-y(j,i2), z(j+1,i)-z(j,i),
			     x(j2,i+1)-x(j2,i), y(j,i3)-y(j,i2), z(j,i+1)-z(i,j),
			     nx, ny, nz);
	      cross_product (x(j2,i-1)-x(j2,i), y(j,i1)-y(j,i2), z(j,i-1)-z(j,i),
			     x(j3,i)-x(j2,i), y(j+1,i2)-y(j,i2), z(j+1,i)-z(i,j),
			     nx, ny, nz);
	      cross_product (x(j1,i)-x(j2,i), y(j-1,i2)-y(j,i2), z(j-1,i)-z(j,i),
			     x(j2,i-1)-x(j2,i), y(j,i1)-y(j,i2), z(j,i-1)-z(i,j),
			     nx, ny, nz);
	      cross_product (x(j2,i+1)-x(j2,i), y(j,i3)-y(j,i2), z(j,i+1)-z(j,i),
			     x(j1,i)-x(j2,i), y(j-1,i2)-y(j,i2), z(j-1,i)-z(i,j),
			     nx, ny, nz);

	      double d = - sqrt (nx*nx + ny*ny + nz*nz);

	      nx /= d;
	      ny /= d;
	      nz /= d;
	    }
	}

      vertexnormals = n;
    }
}

// ---------------------------------------------------------------------

octave_value
base_graphics_object::get_default (const caseless_str& name) const
{
  graphics_handle parent = get_parent ();
  graphics_object parent_obj = gh_manager::get_object (parent);

  return parent_obj.get_default (type () + name);
}

octave_value
base_graphics_object::get_factory_default (const caseless_str& name) const
{
  graphics_object parent_obj = gh_manager::get_object (0);

  return parent_obj.get_factory_default (type () + name);
}

// We use a random value for the handle to avoid issues with plots and
// scalar values for the first argument.
gh_manager::gh_manager (void)
  : handle_map (), handle_free_list (),
    next_handle (-1.0 - (rand () + 1.0) / (RAND_MAX + 2.0))
{
  handle_map[0] = graphics_object (new root_figure ());
}

graphics_handle
gh_manager::do_make_graphics_handle (const std::string& go_name,
				     const graphics_handle& p, bool do_createfcn)
{
  graphics_handle h = get_handle (go_name);

  base_graphics_object *go = 0;

  if (go_name == "figure")
    go = new figure (h, p);
  else if (go_name == "axes")
    go = new axes (h, p);
  else if (go_name == "line")
    go = new line (h, p);
  else if (go_name == "text")
    go = new text (h, p);
  else if (go_name == "image")
    go = new image (h, p);
  else if (go_name == "patch")
    go = new patch (h, p);
  else if (go_name == "surface")
    go = new surface (h, p);
  if (go)
    {
      handle_map[h] = graphics_object (go);
      if (do_createfcn)
        go->get_properties ().execute_createfcn ();
    }
  else
    error ("gh_manager::do_make_graphics_handle: invalid object type `%s'",
	   go_name.c_str ());

  return h;
}

graphics_handle
gh_manager::do_make_figure_handle (double val)
{
  graphics_handle h = val;

  handle_map[h] = graphics_object (new figure (h, 0));

  return h;
}

void
gh_manager::do_push_figure (const graphics_handle& h)
{
  do_pop_figure (h);

  figure_list.push_front (h);
}

void
gh_manager::do_pop_figure (const graphics_handle& h)
{
  for (figure_list_iterator p = figure_list.begin ();
       p != figure_list.end ();
       p++)
    {
      if (*p == h)
	{
	  figure_list.erase (p);
	  break;
	}
    }
}

property_list::plist_map_type
root_figure::init_factory_properties (void)
{
  property_list::plist_map_type plist_map;

  plist_map["figure"] = figure::properties::factory_defaults ();
  plist_map["axes"] = axes::properties::factory_defaults ();
  plist_map["line"] = line::properties::factory_defaults ();
  plist_map["text"] = text::properties::factory_defaults ();
  plist_map["image"] = image::properties::factory_defaults ();
  plist_map["patch"] = patch::properties::factory_defaults ();
  plist_map["surface"] = surface::properties::factory_defaults ();

  return plist_map;
}

// ---------------------------------------------------------------------

DEFUN (ishandle, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} ishandle (@var{h})\n\
Return true if @var{h} is a graphics handle and false otherwise.\n\
@end deftypefn")
{
  octave_value retval;

  if (args.length () == 1)
    retval = is_handle (args(0));
  else
    print_usage ();

  return retval;
}

DEFUN (set, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} set (@var{h}, @var{p}, @var{v}, @dots{})\n\
Set the named property value or vector @var{p} to the value @var{v}\n\
for the graphics handle @var{h}.\n\
@end deftypefn")
{
  octave_value retval;

  int nargin = args.length ();

  if (nargin > 0)
    {
      ColumnVector hcv (args(0).vector_value ());

      if (! error_state)
        {
	  bool request_drawnow = false;

          for (octave_idx_type n = 0; n < hcv.length (); n++) 
            {
              graphics_object obj = gh_manager::get_object (hcv(n));

              if (obj)
                {
                  obj.set (args.splice (0, 1));

                  request_drawnow = true;
                }
              else
		{
		  error ("set: invalid handle (= %g)", hcv(n));
		  break;
		}
            }

	  if (! error_state && request_drawnow)
	    Vdrawnow_requested = true;
        }
      else
        error ("set: expecting graphics handle as first argument");
    }
  else
    print_usage ();

  return retval;
}

DEFUN (get, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} get (@var{h}, @var{p})\n\
Return the named property @var{p} from the graphics handle @var{h}.\n\
If @var{p} is omitted, return the complete property list for @var{h}.\n\
If @var{h} is a vector, return a cell array including the property\n\
values or lists respectively.\n\
@end deftypefn")
{
  octave_value retval;
  octave_value_list vlist;

  int nargin = args.length ();

  if (nargin == 1 || nargin == 2)
    {
      ColumnVector hcv (args(0).vector_value ());

      if (! error_state)
        {
	  octave_idx_type len = hcv.length ();

	  vlist.resize (len);

          for (octave_idx_type n = 0; n < len; n++)
            {
              graphics_object obj = gh_manager::get_object (hcv(n));

              if (obj)
                {
                  if (nargin == 1)
                    vlist(n) = obj.get ();
                  else
                    {
                      caseless_str property = args(1).string_value ();

                      if (! error_state)
                        vlist(n) = obj.get (property);
                      else
			{
			  error ("get: expecting property name as second argument");
			  break;
			}
                    }
                }
              else
		{
		  error ("get: invalid handle (= %g)", hcv(n));
		  break;
		}
            }
        }
      else
        error ("get: expecting graphics handle as first argument");
    }
  else
    print_usage ();

  if (! error_state)
    {
      octave_idx_type len = vlist.length ();

      if (len > 1)
	retval = Cell (vlist);
      else if (len == 1)
	retval = vlist(0);
    }

  return retval;
}

DEFUN (__get__, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} __get__ (@var{h})\n\
Return all properties from the graphics handle @var{h}.\n\
If @var{h} is a vector, return a cell array including the property\n\
values or lists respectively.\n\
@end deftypefn")
{
  octave_value retval;
  octave_value_list vlist;

  int nargin = args.length ();

  if (nargin == 1)
    {
      ColumnVector hcv (args(0).vector_value ());

      if (! error_state)
        {
          octave_idx_type len = hcv.length ();

          vlist.resize (len);

          for (octave_idx_type n = 0; n < len; n++)
            {
              graphics_object obj = gh_manager::get_object (hcv(n));

              if (obj)
                vlist(n) = obj.get (true);
              else
                {
                  error ("get: invalid handle (= %g)", hcv(n));
                  break;
                }
            }
        }
      else
        error ("get: expecting graphics handle as first argument");
    }
  else
    print_usage ();

  if (! error_state)
    {
      octave_idx_type len = vlist.length ();

      if (len > 1)
        retval = Cell (vlist);
      else if (len == 1)
        retval = vlist(0);
    }

  return retval;
}

static octave_value
make_graphics_object (const std::string& go_name,
		      const octave_value_list& args)
{
  octave_value retval;

  double val = args(0).double_value ();

  if (! error_state)
    {
      graphics_handle parent = gh_manager::lookup (val);

      if (parent.ok ())
	{
	  graphics_handle h
	    = gh_manager::make_graphics_handle (go_name, parent, false);

	  if (! error_state)
	    {
	      adopt (parent, h);

	      xset (h, args.splice (0, 1));
	      xcreatefcn (h);

	      retval = h.value ();

	      if (! error_state)
		Vdrawnow_requested = true;
	    }
	  else
	    error ("__go%s__: unable to create graphics handle",
		   go_name.c_str ());
	}
      else
	error ("__go_%s__: invalid parent", go_name.c_str ());
    }
  else
    error ("__go_%s__: invalid parent", go_name.c_str ());

  return retval;
}

DEFUN (__go_figure__, args, ,
   "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} __go_figure__ (@var{fignum})\n\
Undocumented internal function.\n\
@end deftypefn")
{
  octave_value retval;

  if (args.length () > 0)
    {
      double val = args(0).double_value ();

      if (! error_state)
	{
	  if (is_figure (val))
	    {
	      graphics_handle h = gh_manager::lookup (val);

	      xset (h, args.splice (0, 1));

	      retval = h.value ();
	    }
	  else
	    {
	      graphics_handle h = octave_NaN;

	      if (xisnan (val))
		h = gh_manager::make_graphics_handle ("figure", 0, false);
	      else if (val > 0 && D_NINT (val) == val)
		h = gh_manager::make_figure_handle (val);
	      else
		error ("__go_figure__: invalid figure number");

	      if (! error_state && h.ok ())
		{
		  adopt (0, h);

		  xset (h, args.splice (0, 1));
		  xcreatefcn (h);

		  retval = h.value ();
		}
	      else
		error ("__go_figure__: failed to create figure handle");
	    }
	}
      else
	error ("__go_figure__: expecting figure number to be double value");
    }
  else
    print_usage ();

  return retval;
}

#define GO_BODY(TYPE) \
  octave_value retval; \
 \
  if (args.length () > 0) \
    retval = make_graphics_object (#TYPE, args); \
  else \
    print_usage (); \
 \
  return retval

DEFUN (__go_axes__, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} __go_axes__ (@var{parent})\n\
Undocumented internal function.\n\
@end deftypefn")
{
  GO_BODY (axes);
}

DEFUN (__go_line__, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} __go_line__ (@var{parent})\n\
Undocumented internal function.\n\
@end deftypefn")
{
  GO_BODY (line);
}

DEFUN (__go_text__, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} __go_text__ (@var{parent})\n\
Undocumented internal function.\n\
@end deftypefn")
{
  GO_BODY (text);
}

DEFUN (__go_image__, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} __go_image__ (@var{parent})\n\
Undocumented internal function.\n\
@end deftypefn")
{
  GO_BODY (image);
}

DEFUN (__go_surface__, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} __go_surface__ (@var{parent})\n\
Undocumented internal function.\n\
@end deftypefn")
{
  GO_BODY (surface);
}

DEFUN (__go_patch__, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} __go_patch__ (@var{parent})\n\
Undocumented internal function.\n\
@end deftypefn")
{
  GO_BODY (patch);
}

DEFUN (__go_delete__, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} __go_delete__ (@var{h})\n\
Undocumented internal function.\n\
@end deftypefn")
{
  octave_value_list retval;

  if (args.length () == 1)
    {
      graphics_handle h = octave_NaN;

      double val = args(0).double_value ();

      if (! error_state)
	{
	  h = gh_manager::lookup (val);

	  if (h.ok ())
	    {
	      graphics_object obj = gh_manager::get_object (h);

	      graphics_handle parent_h = obj.get_parent ();

	      graphics_object parent_obj = gh_manager::get_object (parent_h);

              // NOTE: free the handle before removing it from its parent's
              //       children, such that the object's state is correct when
              //       the deletefcn callback is executed

	      gh_manager::free (h);

	      parent_obj.remove_child (h);
	    }
	  else
	    error ("delete: invalid graphics object (= %g)", val);
	}
      else
	error ("delete: invalid graphics object");
    }
  else
    print_usage ();

  return retval;
}

DEFUN (__go_axes_init__, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} __go_axes_init__ (@var{h}, @var{mode})\n\
Undocumented internal function.\n\
@end deftypefn")
{
  octave_value retval;

  int nargin = args.length ();

  std::string mode = "";

  if (nargin == 2)
    {
      mode = args(1).string_value ();

      if (error_state)
	return retval;
    }

  if (nargin == 1 || nargin == 2)
    {
      graphics_handle h = octave_NaN;

      double val = args(0).double_value ();

      if (! error_state)
	{
	  h = gh_manager::lookup (val);

	  if (h.ok ())
	    {
	      graphics_object obj = gh_manager::get_object (h);

	      obj.set_defaults (mode);
	    }
	  else
	    error ("__go_axes_init__: invalid graphics object (= %g)", val);
	}
      else
	error ("__go_axes_init__: invalid graphics object");
    }
  else
    print_usage ();

  return retval;
}

DEFUN (__go_handles__, , ,
   "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} __go_handles__ ()\n\
Undocumented internal function.\n\
@end deftypefn")
{
  return octave_value (gh_manager::handle_list ());
}

DEFUN (__go_figure_handles__, , ,
   "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} __go_figure_handles__ ()\n\
Undocumented internal function.\n\
@end deftypefn")
{
  return octave_value (gh_manager::figure_handle_list ());
}

DEFUN (available_backends, args, ,
   "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} available_backends ()\n\
Returns resgistered graphics backends.\n\
@end deftypefn")
{
  return octave_value (graphics_backend::available_backends_list ());
}

static void
clear_drawnow_request (void *)
{
  Vdrawnow_requested = false;
}

DEFUN (drawnow, args, ,
   "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} __go_drawnow__ ()\n\
@deftypefnx {Built-in Function} {} __go_drawnow__ (@var{term}, @var{file}, @var{mono}, @var{debug_file})\n\
Undocumented internal function.\n\
@end deftypefn")
{
  static int drawnow_executing = 0;
  static bool __go_close_all_registered__ = false;

  octave_value retval;

  unwind_protect::begin_frame ("Fdrawnow");
  unwind_protect::add (clear_drawnow_request);

  unwind_protect_int (drawnow_executing);

  if (++drawnow_executing <= 1)
    {
      if (! __go_close_all_registered__)
	{
	  octave_add_atexit_function ("__go_close_all__");

	  __go_close_all_registered__ = true;
	}

      if (args.length () == 0)
	{
	  Matrix hlist = gh_manager::figure_handle_list ();

	  for (int i = 0; ! error_state && i < hlist.length (); i++)
	    {
	      graphics_handle h = gh_manager::lookup (hlist(i));

	      if (h.ok () && h != 0)
		{
		  graphics_object go = gh_manager::get_object (h);
		  figure::properties& fprops = dynamic_cast <figure::properties&> (go.get_properties ());

		  if (fprops.is_modified ())
		    {
		      if (fprops.is_visible ())
			fprops.get_backend ().redraw_figure (h);
		      else if (! fprops.get___plot_stream__ ().is_empty ())
			{
			  fprops.close (false);
			  fprops.set___plot_stream__ (Matrix ());
			  fprops.set___enhanced__ (false);
			}
		      fprops.set_modified (false);
		    }
		}
	    }
	}
      else if (args.length () >= 2 && args.length () <= 4)
	{
	  std::string term, file, debug_file;
	  bool mono;

	  term = args(0).string_value ();

	  if (! error_state)
	    {
	      file = args(1).string_value ();

	      if (! error_state)
		{
		  size_t pos = file.find_last_of (file_ops::dir_sep_chars);

		  if (pos != NPOS)
		    {
		      file_stat fs (file.substr (0, pos));

		      if (! (fs && fs.is_dir ()))
			error ("drawnow: nonexistent directory `%s'",
			       file.substr (0, pos).c_str ());
		    }

		  mono = (args.length () >= 3 ? args(2).bool_value () : false);

		  if (! error_state)
		    {
		      debug_file = (args.length () > 3 ? args(3).string_value ()
				    : "");

		      if (! error_state)
			{
			  graphics_handle h = gcf ();

			  if (h.ok ())
			    {
			      graphics_object go = gh_manager::get_object (h);

			      go.get_backend ()
				.print_figure (h, term, file, mono, debug_file);
			    }
			  else
			    error ("drawnow: nothing to draw");
			}
		      else
			error ("drawnow: invalid debug_file, expected a string value");
		    }
		  else
		    error ("drawnow: invalid colormode, expected a boolean value");
		}
	      else
		error ("drawnow: invalid file, expected a string value");
	    }
	  else
	    error ("drawnow: invalid terminal, expected a string value");
	}
      else
	print_usage ();
    }

  unwind_protect::run_frame ("Fdrawnow");

  return retval;
}

octave_value
get_property_from_handle (double handle, const std::string& property,
			  const std::string& func)
{
  graphics_object obj = gh_manager::get_object (handle);
  octave_value retval;

  if (obj)
    {
      caseless_str p = std::string (property);
      retval = obj.get (p);
    }
  else
    error ("%s: invalid handle (= %g)", func.c_str(), handle);

  return retval;
}

bool
set_property_in_handle (double handle, const std::string& property,
			const octave_value& arg, const std::string& func)
{
  graphics_object obj = gh_manager::get_object (handle);
  int ret = false;

  if (obj)
    {
      caseless_str p = std::string (property);
      obj.set (p, arg);
      if (!error_state)
	ret = true;
    }
  else
    error ("%s: invalid handle (= %g)", func.c_str(), handle);

  return ret;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
