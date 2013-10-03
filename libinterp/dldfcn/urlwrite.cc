// urlwrite and urlread, a curl front-end for octave
/*

Copyright (C) 2006-2012 Alexander Barth
Copyright (C) 2009 David Bateman

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

// Author: Alexander Barth <abarth@marine.usf.edu>
// Adapted-By: jwe

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "dir-ops.h"
#include "file-ops.h"
#include "file-stat.h"
#include "oct-env.h"
#include "oct-handle.h"
#include "glob-match.h"
#include "singleton-cleanup.h"

#include "defun-dld.h"
#include "error.h"
#include "oct-obj.h"
#include "ov-cell.h"
#include "pager.h"
#include "oct-map.h"
#include "oct-refcount.h"
#include "unwind-prot.h"

static void
delete_file (const std::string& file)
{
  octave_unlink (file);
}

#ifdef HAVE_CURL

#include <curl/curl.h>
#include <curl/curlver.h>
#include <curl/easy.h>

static int
write_data (void *buffer, size_t size, size_t nmemb, void *streamp)
{
  std::ostream& stream = *(static_cast<std::ostream*> (streamp));
  stream.write (static_cast<const char*> (buffer), size*nmemb);
  return (stream.fail () ? 0 : size * nmemb);
}

static int
read_data (void *buffer, size_t size, size_t nmemb, void *streamp)
{
  std::istream& stream = *(static_cast<std::istream*> (streamp));
  stream.read (static_cast<char*> (buffer), size*nmemb);
  if (stream.eof ())
    return stream.gcount ();
  else
    return (stream.fail () ? 0 : size * nmemb);
}

static size_t
throw_away (void *, size_t size, size_t nmemb, void *)
{
  return static_cast<size_t>(size * nmemb);
}

class curl_object
{
private:

// I'd love to rewrite this as a private method of the curl_object
// class, but you can't pass the va_list from the wrapper SETOPT to
// the curl_easy_setopt function.
#define SETOPT(option, parameter) \
  do \
    { \
      CURLcode res = curl_easy_setopt (curl, option, parameter); \
      if (res != CURLE_OK) \
        { \
          error ("%s", curl_easy_strerror (res)); \
          return; \
        } \
    } \
  while (0)

// Sames as above, but return retval.
#define SETOPTR(option, parameter) \
  do \
    { \
      CURLcode res = curl_easy_setopt (curl, option, parameter); \
      if (res != CURLE_OK) \
        { \
          error ("%s", curl_easy_strerror (res)); \
          return retval; \
        } \
    } \
  while (0)

  class curl_object_rep
  {
  private:

    static void reset_path (const curl_object_rep *curl_rep)
    {
      curl_rep->cwd ("..");
    }

  public:

    curl_object_rep (void)
      : count (1), curl (curl_easy_init ()), host (), userpwd (),
        valid (false), ascii_mode (false), errnum ()
    {
      if (! curl)
        error ("can not create curl object");
    }

    curl_object_rep (const std::string& host_arg, const std::string& user_arg,
               const std::string& passwd)
      : count (1), curl (curl_easy_init ()), host (host_arg),
        userpwd (), valid (true), ascii_mode (false), errnum ()
    {
      if (!curl)
        {
          valid = false;
          error ("can not create curl object");
          return;
        }

      init (user_arg, passwd, std::cin, octave_stdout);

      std::string url ("ftp://" + host_arg);
      SETOPT (CURLOPT_URL, url.c_str ());

      // Setup the link, with no transfer.
      perform ();
    }

    curl_object_rep (const std::string& url, const std::string& method,
                     const Cell& param, std::ostream& os, bool& retval)
      : count (1), curl (curl_easy_init ()), host (), userpwd (),
        valid (true), ascii_mode (false), errnum ()
    {
      retval = false;

      if (!curl)
        {
          valid = false;
          error ("can not create curl object");
          return;
        }

      init ("", "", std::cin, os);

      SETOPT (CURLOPT_NOBODY, 0);

      // Restore the default HTTP request method to GET after setting
      // NOBODY to true and back to false.  This is needed for backward
      // compatibility with versions of libcurl < 7.18.2.
      SETOPT (CURLOPT_HTTPGET, 1);

      // Don't need to store the parameters here as we can't change
      // the URL after the object is created
      std::string query_string = form_query_string (param);

      if (method == "get")
        {
          query_string = url + "?" + query_string;
          SETOPT (CURLOPT_URL, query_string.c_str ());
        }
      else if (method == "post")
        {
          SETOPT (CURLOPT_URL, url.c_str ());
          SETOPT (CURLOPT_POSTFIELDS, query_string.c_str ());
        }
      else
        SETOPT (CURLOPT_URL, url.c_str ());

      retval = perform (false);
    }

    ~curl_object_rep (void)
    {
      if (curl)
        curl_easy_cleanup (curl);
    }

    bool is_valid (void) const { return valid; }

    bool perform (bool curlerror = true) const
    {
      bool retval = false;

      BEGIN_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;

      errnum = curl_easy_perform (curl);
      if (errnum != CURLE_OK)
        {
          if (curlerror)
            error ("%s", curl_easy_strerror (errnum));
        }
      else
        retval = true;

      END_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;

      return retval;
    }

    std::string lasterror (void) const
    {
      return std::string (curl_easy_strerror (errnum));
    }

    void set_ostream (std::ostream& os) const
    {
      SETOPT (CURLOPT_WRITEDATA, static_cast<void*> (&os));
    }

    void set_istream (std::istream& is) const
    {
      SETOPT (CURLOPT_READDATA, static_cast<void*> (&is));
    }

    void ascii (void)
    {
      SETOPT (CURLOPT_TRANSFERTEXT, 1);
      ascii_mode = true;
    }

    void binary (void)
    {
      SETOPT (CURLOPT_TRANSFERTEXT, 0);
      ascii_mode = false;
    }

    bool is_ascii (void) const { return ascii_mode; }

    bool is_binary (void) const { return !ascii_mode; }

    void cwd (const std::string& path) const
    {
      struct curl_slist *slist = 0;
      std::string cmd = "cwd " + path;
      slist = curl_slist_append (slist, cmd.c_str ());
      SETOPT (CURLOPT_POSTQUOTE, slist);
      perform ();
      SETOPT (CURLOPT_POSTQUOTE, 0);
      curl_slist_free_all (slist);
    }

    void del (const std::string& file) const
    {
      struct curl_slist *slist = 0;
      std::string cmd = "dele " + file;
      slist = curl_slist_append (slist, cmd.c_str ());
      SETOPT (CURLOPT_POSTQUOTE, slist);
      perform ();
      SETOPT (CURLOPT_POSTQUOTE, 0);
      curl_slist_free_all (slist);
    }

    void rmdir (const std::string& path) const
    {
      struct curl_slist *slist = 0;
      std::string cmd = "rmd " + path;
      slist = curl_slist_append (slist, cmd.c_str ());
      SETOPT (CURLOPT_POSTQUOTE, slist);
      perform ();
      SETOPT (CURLOPT_POSTQUOTE, 0);
      curl_slist_free_all (slist);
    }

    bool mkdir (const std::string& path, bool curlerror = true) const
    {
      bool retval = false;
      struct curl_slist *slist = 0;
      std::string cmd = "mkd " + path;
      slist = curl_slist_append (slist, cmd.c_str ());
      SETOPTR (CURLOPT_POSTQUOTE, slist);
      retval = perform (curlerror);
      SETOPTR (CURLOPT_POSTQUOTE, 0);
      curl_slist_free_all (slist);
      return retval;
    }

    void rename (const std::string& oldname, const std::string& newname) const
    {
      struct curl_slist *slist = 0;
      std::string cmd = "rnfr " + oldname;
      slist = curl_slist_append (slist, cmd.c_str ());
      cmd = "rnto " + newname;
      slist = curl_slist_append (slist, cmd.c_str ());
      SETOPT (CURLOPT_POSTQUOTE, slist);
      perform ();
      SETOPT (CURLOPT_POSTQUOTE, 0);
      curl_slist_free_all (slist);
    }

    void put (const std::string& file, std::istream& is) const
    {
      std::string url = "ftp://" + host + "/" + file;
      SETOPT (CURLOPT_URL, url.c_str ());
      SETOPT (CURLOPT_UPLOAD, 1);
      SETOPT (CURLOPT_NOBODY, 0);
      set_istream (is);
      perform ();
      set_istream (std::cin);
      SETOPT (CURLOPT_NOBODY, 1);
      SETOPT (CURLOPT_UPLOAD, 0);
      url = "ftp://" + host;
      SETOPT (CURLOPT_URL, url.c_str ());
    }

    void get (const std::string& file, std::ostream& os) const
    {
      std::string url = "ftp://" + host + "/" + file;
      SETOPT (CURLOPT_URL, url.c_str ());
      SETOPT (CURLOPT_NOBODY, 0);
      set_ostream (os);
      perform ();
      set_ostream (octave_stdout);
      SETOPT (CURLOPT_NOBODY, 1);
      url = "ftp://" + host;
      SETOPT (CURLOPT_URL, url.c_str ());
    }

    void mget_directory (const std::string& directory,
                         const std::string& target) const
    {
      std::string sep = file_ops::dir_sep_str ();
      file_stat fs (directory);

      if (!fs || !fs.is_dir ())
        {
          std::string msg;
          int status = octave_mkdir (directory, 0777, msg);

          if (status < 0)
            {
              error ("__ftp_mget__: can't create directory %s%s%s. %s",
                     target.c_str (), sep.c_str (), directory.c_str (),
                     msg.c_str ());

              return;
            }
        }

      cwd (directory);

      if (! error_state)
        {
          unwind_protect_safe frame;

          frame.add_fcn (reset_path, this);

          string_vector sv = list ();

          for (octave_idx_type i = 0; i < sv.length (); i++)
            {
              time_t ftime;
              bool fisdir;
              double fsize;

              get_fileinfo (sv(i), fsize, ftime, fisdir);

              if (fisdir)
                mget_directory (sv(i), target + directory + sep);
              else
                {
                  std::string realfile = target + directory + sep + sv(i);

                  std::ofstream ofile (realfile.c_str (),
                                       std::ios::out | std::ios::binary);

                  if (! ofile.is_open ())
                    {
                      error ("__ftp_mget__: unable to open file");
                      break;
                    }

                  unwind_protect_safe frame2;

                  frame2.add_fcn (delete_file, realfile);

                  get (sv(i), ofile);

                  ofile.close ();

                  if (!error_state)
                    frame2.discard ();
                  else
                    frame2.run ();
                }

              if (error_state)
                break;
            }
        }
    }

    string_vector mput_directory (const std::string& base,
                                  const std::string& directory) const
    {
      string_vector retval;

      std::string realdir
        = (base.length () == 0
           ? directory : base + file_ops::dir_sep_str () + directory);

      if (! mkdir (directory, false))
        warning ("__ftp_mput__: can not create the remote directory ""%s""",
                 realdir.c_str ());

      cwd (directory);

      if (! error_state)
        {
          unwind_protect_safe frame;

          frame.add_fcn (reset_path, this);

          dir_entry dirlist (realdir);

          if (dirlist)
            {
              string_vector files = dirlist.read ();

              for (octave_idx_type i = 0; i < files.length (); i++)
                {
                  std::string file = files (i);

                  if (file == "." || file == "..")
                    continue;

                  std::string realfile = realdir + file_ops::dir_sep_str () + file;
                  file_stat fs (realfile);

                  if (! fs.exists ())
                    {
                      error ("__ftp__mput: file ""%s"" does not exist",
                             realfile.c_str ());
                      break;
                    }

                  if (fs.is_dir ())
                    {
                      retval.append (mput_directory (realdir, file));

                      if (error_state)
                        break;
                    }
                  else
                    {
                      // FIXME Does ascii mode need to be flagged here?
                      std::ifstream ifile (realfile.c_str (), std::ios::in |
                                           std::ios::binary);

                      if (! ifile.is_open ())
                        {
                          error ("__ftp_mput__: unable to open file ""%s""",
                                 realfile.c_str ());
                          break;
                        }

                      put (file, ifile);

                      ifile.close ();

                      if (error_state)
                        break;

                      retval.append (realfile);
                    }
                }
            }
          else
            error ("__ftp_mput__: can not read the directory ""%s""",
                   realdir.c_str ());
        }

      return retval;
    }

    void dir (void) const
    {
      std::string url = "ftp://" + host + "/";
      SETOPT (CURLOPT_URL, url.c_str ());
      SETOPT (CURLOPT_NOBODY, 0);
      perform ();
      SETOPT (CURLOPT_NOBODY, 1);
      url = "ftp://" + host;
      SETOPT (CURLOPT_URL, url.c_str ());
    }

    string_vector list (void) const
    {
      string_vector retval;
      std::ostringstream buf;
      std::string url = "ftp://" + host + "/";
      SETOPTR (CURLOPT_WRITEDATA, static_cast<void*> (&buf));
      SETOPTR (CURLOPT_URL, url.c_str ());
      SETOPTR (CURLOPT_DIRLISTONLY, 1);
      SETOPTR (CURLOPT_NOBODY, 0);
      perform ();
      SETOPTR (CURLOPT_NOBODY, 1);
      url = "ftp://" + host;
      SETOPTR (CURLOPT_WRITEDATA, static_cast<void*> (&octave_stdout));
      SETOPTR (CURLOPT_DIRLISTONLY, 0);
      SETOPTR (CURLOPT_URL, url.c_str ());

      // Count number of directory entries
      std::string str = buf.str ();
      octave_idx_type n = 0;
      size_t pos = 0;
      while (true)
        {
          pos = str.find_first_of ('\n', pos);
          if (pos == std::string::npos)
            break;
          pos++;
          n++;
        }
      retval.resize (n);
      pos = 0;
      for (octave_idx_type i = 0; i < n; i++)
        {
          size_t newpos = str.find_first_of ('\n', pos);
          if (newpos == std::string::npos)
            break;

          retval(i) = str.substr(pos, newpos - pos);
          pos = newpos + 1;
        }
      return retval;
    }

    void get_fileinfo (const std::string& filename, double& filesize,
                       time_t& filetime, bool& fileisdir) const
    {
      std::string path = pwd ();

      std::string url = "ftp://" + host + "/" + path + "/" + filename;
      SETOPT (CURLOPT_URL, url.c_str ());
      SETOPT (CURLOPT_FILETIME, 1);
      SETOPT (CURLOPT_HEADERFUNCTION, throw_away);
      SETOPT (CURLOPT_WRITEFUNCTION, throw_away);

      // FIXME
      // The MDTM command fails for a directory on the servers I tested
      // so this is a means of testing for directories. It also means
      // I can't get the date of directories!

      if (! perform (false))
        {
          fileisdir = true;
          filetime = -1;
          filesize = 0;
        }
      else
        {
          fileisdir = false;
          time_t ft;
          curl_easy_getinfo (curl, CURLINFO_FILETIME, &ft);
          filetime = ft;
          double fs;
          curl_easy_getinfo (curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &fs);
          filesize = fs;
        }

      SETOPT (CURLOPT_WRITEFUNCTION, write_data);
      SETOPT (CURLOPT_HEADERFUNCTION, 0);
      SETOPT (CURLOPT_FILETIME, 0);
      url = "ftp://" + host;
      SETOPT (CURLOPT_URL, url.c_str ());

      // The MDTM command seems to reset the path to the root with the
      // servers I tested with, so cd again into the correct path. Make
      // the path absolute so that this will work even with servers that
      // don't end up in the root after an MDTM command.
      cwd ("/" + path);
    }

    std::string pwd (void) const
    {
      struct curl_slist *slist = 0;
      std::string retval;
      std::ostringstream buf;

      slist = curl_slist_append (slist, "pwd");
      SETOPTR (CURLOPT_POSTQUOTE, slist);
      SETOPTR (CURLOPT_HEADERFUNCTION, write_data);
      SETOPTR (CURLOPT_WRITEHEADER, static_cast<void *>(&buf));

      perform ();
      retval = buf.str ();

      // Can I assume that the path is alway in "" on the last line
      size_t pos2 = retval.rfind ('"');
      size_t pos1 = retval.rfind ('"', pos2 - 1);
      retval = retval.substr (pos1 + 1, pos2 - pos1 - 1);

      SETOPTR (CURLOPT_HEADERFUNCTION, 0);
      SETOPTR (CURLOPT_WRITEHEADER, 0);
      SETOPTR (CURLOPT_POSTQUOTE, 0);
      curl_slist_free_all (slist);

      return retval;
    }

    octave_refcount<size_t> count;

  private:

    CURL *curl;
    std::string host;
    std::string userpwd;
    bool valid;
    bool ascii_mode;
    mutable CURLcode errnum;

    // No copying!

    curl_object_rep (const curl_object_rep& ov);

    curl_object_rep& operator = (const curl_object_rep&);

    void init (const std::string& user, const std::string& passwd,
               std::istream& is, std::ostream& os)
    {
      // No data transfer by default
      SETOPT (CURLOPT_NOBODY, 1);

      // Set the username and password
      userpwd = user;
      if (! passwd.empty ())
        userpwd += ":" + passwd;
      if (! userpwd.empty ())
        SETOPT (CURLOPT_USERPWD, userpwd.c_str ());

      // Define our callback to get called when there's data to be written.
      SETOPT (CURLOPT_WRITEFUNCTION, write_data);

      // Set a pointer to our struct to pass to the callback.
      SETOPT (CURLOPT_WRITEDATA, static_cast<void*> (&os));

      // Define our callback to get called when there's data to be read
      SETOPT (CURLOPT_READFUNCTION, read_data);

      // Set a pointer to our struct to pass to the callback.
      SETOPT (CURLOPT_READDATA, static_cast<void*> (&is));

      // Follow redirects.
      SETOPT (CURLOPT_FOLLOWLOCATION, true);

      // Don't use EPSV since connecting to sites that don't support it
      // will hang for some time (3 minutes?) before moving on to try PASV
      // instead.
      SETOPT (CURLOPT_FTP_USE_EPSV, false);

      SETOPT (CURLOPT_NOPROGRESS, true);
      SETOPT (CURLOPT_FAILONERROR, true);

      SETOPT (CURLOPT_POSTQUOTE, 0);
      SETOPT (CURLOPT_QUOTE, 0);
    }

    std::string form_query_string (const Cell& param)
    {
      std::ostringstream query;

      for (int i = 0; i < param.numel (); i += 2)
        {
          std::string name = param(i).string_value ();
          std::string text = param(i+1).string_value ();

          // Encode strings.
          char *enc_name = curl_easy_escape (curl, name.c_str (),
                                             name.length ());
          char *enc_text = curl_easy_escape (curl, text.c_str (),
                                             text.length ());

          query << enc_name << "=" << enc_text;

          curl_free (enc_name);
          curl_free (enc_text);

          if (i < param.numel ()-1)
            query << "&";
        }

      query.flush ();

      return query.str ();
    }
#undef SETOPT
#undef SETOPTR
  };

public:

  curl_object (void) : rep (new curl_object_rep ()) { }

  curl_object (const std::string& host, const std::string& user,
               const std::string& passwd)
    : rep (new curl_object_rep (host, user, passwd)) { }

  curl_object (const std::string& url, const std::string& method,
               const Cell& param, std::ostream& os, bool& retval)
    : rep (new curl_object_rep (url, method, param, os, retval)) { }

  curl_object (const curl_object& h) : rep (h.rep)
  {
    rep->count++;
  }

  ~curl_object (void)
  {
    if (--rep->count == 0)
      delete rep;
  }

  curl_object& operator = (const curl_object& h)
  {
    if (this != &h)
      {
        if (--rep->count == 0)
          delete rep;

        rep = h.rep;
        rep->count++;
      }

    return *this;
  }

  bool is_valid (void) const { return rep->is_valid (); }

  std::string lasterror (void) const { return rep->lasterror (); }

  void set_ostream (std::ostream& os) const { rep->set_ostream (os); }

  void set_istream (std::istream& is) const { rep->set_istream (is); }

  void ascii (void) const { rep->ascii (); }

  void binary (void) const { rep->binary (); }

  bool is_ascii (void) const { return rep->is_ascii (); }

  bool is_binary (void) const { return rep->is_binary (); }

  void cwd (const std::string& path) const { rep->cwd (path); }

  void del (const std::string& file) const { rep->del (file); }

  void rmdir (const std::string& path) const { rep->rmdir (path); }

  bool mkdir (const std::string& path, bool curlerror = true) const
  {
    return rep->mkdir (path, curlerror);
  }

  void rename (const std::string& oldname, const std::string& newname) const
  {
    rep->rename (oldname, newname);
  }

  void put (const std::string& file, std::istream& is) const
  {
    rep->put (file, is);
  }

  void get (const std::string& file, std::ostream& os) const
  {
    rep->get (file, os);
  }

  void mget_directory (const std::string& directory,
                       const std::string& target) const
  {
    return rep->mget_directory (directory, target);
  }

  string_vector mput_directory (const std::string& base,
                                const std::string& directory) const
  {
    return mput_directory (base, directory);
  }

  void dir (void) const { rep->dir (); }

  string_vector list (void) const { return rep->list (); }

  void get_fileinfo (const std::string& filename, double& filesize,
                     time_t& filetime, bool& fileisdir) const
  {
    rep->get_fileinfo (filename, filesize, filetime, fileisdir);
  }

  std::string pwd (void) const { return rep->pwd (); }

private:

  curl_object_rep *rep;
};

typedef octave_handle curl_handle;

class OCTINTERP_API ch_manager
{
protected:

  ch_manager (void)
    : handle_map (), handle_free_list (),
      next_handle (-1.0 - (rand () + 1.0) / (RAND_MAX + 2.0)) { }

public:

  static void create_instance (void);

  static bool instance_ok (void)
  {
    bool retval = true;

    if (! instance)
      create_instance ();

    if (! instance)
      {
        ::error ("unable to create ch_manager!");

        retval = false;
      }

    return retval;
  }

  static void cleanup_instance (void) { delete instance; instance = 0; }

  static curl_handle get_handle (void)
  {
    return instance_ok ()
      ? instance->do_get_handle () : curl_handle ();
  }

  static void free (const curl_handle& h)
  {
    if (instance_ok ())
      instance->do_free (h);
  }

  static curl_handle lookup (double val)
  {
    return instance_ok () ? instance->do_lookup (val) : curl_handle ();
  }

  static curl_handle lookup (const octave_value& val)
  {
    return val.is_real_scalar ()
      ? lookup (val.double_value ()) : curl_handle ();
  }

  static curl_object get_object (double val)
  {
    return get_object (lookup (val));
  }

  static curl_object get_object (const octave_value& val)
  {
    return get_object (lookup (val));
  }

  static curl_object get_object (const curl_handle& h)
  {
    return instance_ok () ? instance->do_get_object (h) : curl_object ();
  }

  static curl_handle make_curl_handle (const std::string& host,
                                       const std::string& user,
                                       const std::string& passwd)
  {
    return instance_ok ()
      ? instance->do_make_curl_handle (host, user, passwd) : curl_handle ();
  }

  static Matrix handle_list (void)
  {
    return instance_ok () ? instance->do_handle_list () : Matrix ();
  }

private:

  static ch_manager *instance;

  typedef std::map<curl_handle, curl_object>::iterator iterator;
  typedef std::map<curl_handle, curl_object>::const_iterator const_iterator;

  typedef std::set<curl_handle>::iterator free_list_iterator;
  typedef std::set<curl_handle>::const_iterator const_free_list_iterator;

  // A map of handles to curl objects.
  std::map<curl_handle, curl_object> handle_map;

  // The available curl handles.
  std::set<curl_handle> handle_free_list;

  // The next handle available if handle_free_list is empty.
  double next_handle;

  curl_handle do_get_handle (void);

  void do_free (const curl_handle& h);

  curl_handle do_lookup (double val)
  {
    iterator p = (xisnan (val) ? handle_map.end () : handle_map.find (val));

    return (p != handle_map.end ()) ? p->first : curl_handle ();
  }

  curl_object do_get_object (const curl_handle& h)
  {
    iterator p = (h.ok () ? handle_map.find (h) : handle_map.end ());

    return (p != handle_map.end ()) ? p->second : curl_object ();
  }

  curl_handle do_make_curl_handle (const std::string& host,
                                   const std::string& user,
                                   const std::string& passwd)
  {
    curl_handle h = get_handle ();

    curl_object obj (host, user, passwd);

    if (! error_state)
      handle_map[h] = obj;
    else
      {
        do_free (h);

        h = curl_handle ();
      }

    return h;
  }

  Matrix do_handle_list (void)
  {
    Matrix retval (1, handle_map.size ());

    octave_idx_type i = 0;
    for (const_iterator p = handle_map.begin (); p != handle_map.end (); p++)
      {
        curl_handle h = p->first;

        retval(i++) = h.value ();
      }

    return retval;
  }
};

void
ch_manager::create_instance (void)
{
  instance = new ch_manager ();

  if (instance)
    singleton_cleanup_list::add (cleanup_instance);
}

static double
make_handle_fraction (void)
{
  static double maxrand = RAND_MAX + 2.0;

  return (rand () + 1.0) / maxrand;
}

curl_handle
ch_manager::do_get_handle (void)
{
  curl_handle retval;

  // Curl handles are negative integers plus some random fractional
  // part.  To avoid running out of integers, we recycle the integer
  // part but tack on a new random part each time.

  free_list_iterator p = handle_free_list.begin ();

  if (p != handle_free_list.end ())
    {
      retval = *p;
      handle_free_list.erase (p);
    }
  else
    {
      retval = curl_handle (next_handle);

      next_handle = std::ceil (next_handle) - 1.0 - make_handle_fraction ();
    }

  return retval;
}

void
ch_manager::do_free (const curl_handle& h)
{
  if (h.ok ())
    {
      iterator p = handle_map.find (h);

      if (p != handle_map.end ())
        {
          // Curl handles are negative integers plus some random
          // fractional part.  To avoid running out of integers, we
          // recycle the integer part but tack on a new random part
          // each time.

          handle_map.erase (p);

          if (h.value () < 0)
            handle_free_list.insert (std::ceil (h.value ()) - make_handle_fraction ());
        }
      else
        error ("ch_manager::free: invalid object %g", h.value ());
    }
}

ch_manager *ch_manager::instance = 0;

#endif

DEFUN_DLD (urlwrite, args, nargout,
  "-*- texinfo -*-\n\
@deftypefn  {Loadable Function} {} urlwrite (@var{url}, @var{localfile})\n\
@deftypefnx {Loadable Function} {@var{f} =} urlwrite (@var{url}, @var{localfile})\n\
@deftypefnx {Loadable Function} {[@var{f}, @var{success}] =} urlwrite (@var{url}, @var{localfile})\n\
@deftypefnx {Loadable Function} {[@var{f}, @var{success}, @var{message}] =} urlwrite (@var{url}, @var{localfile})\n\
Download a remote file specified by its @var{url} and save it as\n\
@var{localfile}.  For example:\n\
\n\
@example\n\
@group\n\
urlwrite (\"ftp://ftp.octave.org/pub/octave/README\",\n\
          \"README.txt\");\n\
@end group\n\
@end example\n\
\n\
The full path of the downloaded file is returned in @var{f}.  The\n\
variable @var{success} is 1 if the download was successful,\n\
otherwise it is 0 in which case @var{message} contains an error\n\
message.  If no output argument is specified and an error occurs,\n\
then the error is signaled through Octave's error handling mechanism.\n\
\n\
This function uses libcurl.  Curl supports, among others, the HTTP,\n\
FTP and FILE protocols.  Username and password may be specified in\n\
the URL, for example:\n\
\n\
@example\n\
@group\n\
urlwrite (\"http://username:password@@example.com/file.txt\",\n\
          \"file.txt\");\n\
@end group\n\
@end example\n\
\n\
GET and POST requests can be specified by @var{method} and @var{param}.\n\
The parameter @var{method} is either @samp{get} or @samp{post}\n\
and @var{param} is a cell array of parameter and value pairs.\n\
For example:\n\
\n\
@example\n\
@group\n\
urlwrite (\"http://www.google.com/search\", \"search.html\",\n\
          \"get\", @{\"query\", \"octave\"@});\n\
@end group\n\
@end example\n\
@seealso{urlread}\n\
@end deftypefn")
{
  octave_value_list retval;

#ifdef HAVE_CURL

  int nargin = args.length ();

  // verify arguments
  if (nargin != 2 && nargin != 4)
    {
      print_usage ();
      return retval;
    }

  std::string url = args(0).string_value ();

  if (error_state)
    {
      error ("urlwrite: URL must be a character string");
      return retval;
    }

  // name to store the file if download is succesful
  std::string filename = args(1).string_value ();

  if (error_state)
    {
      error ("urlwrite: LOCALFILE must be a character string");
      return retval;
    }

  std::string method;
  Cell param; // empty cell array

  if (nargin == 4)
    {
      method = args(2).string_value ();

      if (error_state)
        {
          error ("urlwrite: METHOD must be \"get\" or \"post\"");
          return retval;
        }

      if (method != "get" && method != "post")
        {
          error ("urlwrite: METHOD must be \"get\" or \"post\"");
          return retval;
        }

      param = args(3).cell_value ();

      if (error_state)
        {
          error ("urlwrite: parameters (PARAM) for get and post requests must be given as a cell");
          return retval;
        }


      if (param.numel () % 2 == 1 )
        {
          error ("urlwrite: number of elements in PARAM must be even");
          return retval;
        }
    }

  // The file should only be deleted if it doesn't initially exist, we
  // create it, and the download fails.  We use unwind_protect to do
  // it so that the deletion happens no matter how we exit the function.

  file_stat fs (filename);

  std::ofstream ofile (filename.c_str (), std::ios::out | std::ios::binary);

  if (! ofile.is_open ())
    {
      error ("urlwrite: unable to open file");
      return retval;
    }

  unwind_protect_safe frame;

  frame.add_fcn (delete_file, filename);

  bool ok;
  curl_object curl = curl_object (url, method, param, ofile, ok);

  ofile.close ();

  if (!error_state)
    frame.discard ();
  else
    frame.run ();

  if (nargout > 0)
    {
      if (ok)
        {
          retval(2) = std::string ();
          retval(1) = true;
          retval(0) = octave_env::make_absolute (filename);
        }
      else
        {
          retval(2) = curl.lasterror ();
          retval(1) = false;
          retval(0) = std::string ();
        }
    }

  if (nargout < 2 && ! ok)
    error ("urlwrite: curl: %s", curl.lasterror ().c_str ());

#else
  gripe_disabled_feature ("urlwrite", "urlwrite");
#endif

  return retval;
}

DEFUN_DLD (urlread, args, nargout,
  "-*- texinfo -*-\n\
@deftypefn  {Loadable Function} {@var{s} =} urlread (@var{url})\n\
@deftypefnx {Loadable Function} {[@var{s}, @var{success}] =} urlread (@var{url})\n\
@deftypefnx {Loadable Function} {[@var{s}, @var{success}, @var{message}] =} urlread (@var{url})\n\
@deftypefnx {Loadable Function} {[@dots{}] =} urlread (@var{url}, @var{method}, @var{param})\n\
Download a remote file specified by its @var{url} and return its content\n\
in string @var{s}.  For example:\n\
\n\
@example\n\
s = urlread (\"ftp://ftp.octave.org/pub/octave/README\");\n\
@end example\n\
\n\
The variable @var{success} is 1 if the download was successful,\n\
otherwise it is 0 in which case @var{message} contains an error\n\
message.  If no output argument is specified and an error occurs,\n\
then the error is signaled through Octave's error handling mechanism.\n\
\n\
This function uses libcurl.  Curl supports, among others, the HTTP,\n\
FTP and FILE protocols.  Username and password may be specified in the\n\
URL@.  For example:\n\
\n\
@example\n\
s = urlread (\"http://user:password@@example.com/file.txt\");\n\
@end example\n\
\n\
GET and POST requests can be specified by @var{method} and @var{param}.\n\
The parameter @var{method} is either @samp{get} or @samp{post}\n\
and @var{param} is a cell array of parameter and value pairs.\n\
For example:\n\
\n\
@example\n\
@group\n\
s = urlread (\"http://www.google.com/search\", \"get\",\n\
            @{\"query\", \"octave\"@});\n\
@end group\n\
@end example\n\
@seealso{urlwrite}\n\
@end deftypefn")
{
  // Octave's return value
  octave_value_list retval;

#ifdef HAVE_CURL

  int nargin = args.length ();

  // verify arguments
  if (nargin != 1  && nargin != 3)
    {
      print_usage ();
      return retval;
    }

  std::string url = args(0).string_value ();

  if (error_state)
    {
      error ("urlread: URL must be a character string");
      return retval;
    }

  std::string method;
  Cell param; // empty cell array

  if (nargin == 3)
    {
      method = args(1).string_value ();

      if (error_state)
        {
          error ("urlread: METHOD must be \"get\" or \"post\"");
          return retval;
        }

      if (method != "get" && method != "post")
        {
          error ("urlread: METHOD must be \"get\" or \"post\"");
          return retval;
        }

      param = args(2).cell_value ();

      if (error_state)
        {
          error ("urlread: parameters (PARAM) for get and post requests must be given as a cell");
          return retval;
        }

      if (param.numel () % 2 == 1 )
        {
          error ("urlread: number of elements in PARAM must be even");
          return retval;
        }
    }

  std::ostringstream buf;

  bool ok;
  curl_object curl = curl_object (url, method, param, buf, ok);

  if (nargout > 0)
    {
      // Return empty string if no error occured.
      retval(2) = ok ? "" : curl.lasterror ();
      retval(1) = ok;
      retval(0) = buf.str ();
    }

  if (nargout < 2 && ! ok)
    error ("urlread: curl: %s", curl.lasterror().c_str());

#else
  gripe_disabled_feature ("urlread", "urlread");
#endif

  return retval;
}

DEFUN_DLD (__ftp__, args, ,
  "-*- texinfo -*-\n\
@deftypefn  {Loadable Function} {@var{handle} =} __ftp__ (@var{host})\n\
@deftypefnx {Loadable Function} {@var{handle} =} __ftp__ (@var{host}, @var{username}, @var{password})\n\
Undocumented internal function\n\
@end deftypefn")
{
  octave_value retval;

#ifdef HAVE_CURL
  int nargin = args.length ();
  std::string host;
  std::string user = "anonymous";
  std::string passwd = "";

  if (nargin < 1 || nargin > 3)
    {
      print_usage ();
      return retval;
    }
  else
    {
      host = args(0).string_value ();

      if (nargin > 1)
        user = args(1).string_value ();

      if (nargin > 2)
        passwd = args(2).string_value ();

      if (! error_state)
        {
          curl_handle ch = ch_manager::make_curl_handle (host, user, passwd);

          if (! error_state)
            retval = ch.value ();
        }
    }
#else
  gripe_disabled_feature ("__ftp__", "FTP");
#endif

  return retval;
}

DEFUN_DLD (__ftp_pwd__, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} __ftp_pwd__ (@var{handle})\n\
Undocumented internal function\n\
@end deftypefn")
{
  octave_value retval;
#ifdef HAVE_CURL
  int nargin = args.length ();

  if (nargin != 1)
    error ("__ftp_pwd__: incorrect number of arguments");
  else
    {
      const curl_object curl = ch_manager::get_object (args(0));

      if (curl.is_valid ())
        retval = curl.pwd ();
      else
        error ("__ftp_pwd__: invalid ftp handle");
    }
#else
  gripe_disabled_feature ("__ftp_pwd__", "FTP");
#endif

  return retval;
}

DEFUN_DLD (__ftp_cwd__, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} __ftp_cwd__ (@var{handle}, @var{path})\n\
Undocumented internal function\n\
@end deftypefn")
{
#ifdef HAVE_CURL
  int nargin = args.length ();

  if (nargin != 1 && nargin != 2)
    error ("__ftp_cwd__: incorrect number of arguments");
  else
    {
      const curl_object curl = ch_manager::get_object (args(0));

      if (curl.is_valid ())
        {
          std::string path = "";

          if (nargin > 1)
            path = args(1).string_value ();

          if (! error_state)
            curl.cwd (path);
          else
            error ("__ftp_cwd__: expecting path as second argument");
        }
      else
        error ("__ftp_cwd__: invalid ftp handle");
    }
#else
  gripe_disabled_feature ("__ftp_cwd__", "FTP");
#endif

  return octave_value ();
}

DEFUN_DLD (__ftp_dir__, args, nargout,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} __ftp_dir__ (@var{handle})\n\
Undocumented internal function\n\
@end deftypefn")
{
  octave_value retval;
#ifdef HAVE_CURL
  int nargin = args.length ();

  if (nargin != 1)
    error ("__ftp_dir__: incorrect number of arguments");
  else
    {
      const curl_object curl = ch_manager::get_object (args(0));

      if (curl.is_valid ())
        {
          if (nargout == 0)
            curl.dir ();
          else
            {
              string_vector sv = curl.list ();
              octave_idx_type n = sv.length ();

              if (n == 0)
                {
                  string_vector flds (5);

                  flds(0) = "name";
                  flds(1) = "date";
                  flds(2) = "bytes";
                  flds(3) = "isdir";
                  flds(4) = "datenum";

                  retval = octave_map (flds);
                }
              else
                {
                  octave_map st;

                  Cell filectime (dim_vector (n, 1));
                  Cell filesize (dim_vector (n, 1));
                  Cell fileisdir (dim_vector (n, 1));
                  Cell filedatenum (dim_vector (n, 1));

                  st.assign ("name", Cell (sv));

                  for (octave_idx_type i = 0; i < n; i++)
                    {
                      time_t ftime;
                      bool fisdir;
                      double fsize;

                      curl.get_fileinfo (sv(i), fsize, ftime, fisdir);

                      fileisdir (i) = fisdir;
                      filectime (i) = ctime (&ftime);
                      filesize (i) = fsize;
                      filedatenum (i) = double (ftime);
                    }

                  st.assign ("date", filectime);
                  st.assign ("bytes", filesize);
                  st.assign ("isdir", fileisdir);
                  st.assign ("datenum", filedatenum);

                  retval = st;
                }
            }
        }
      else
        error ("__ftp_dir__: invalid ftp handle");
    }
#else
  gripe_disabled_feature ("__ftp_dir__", "FTP");
#endif

  return retval;
}

DEFUN_DLD (__ftp_ascii__, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} __ftp_ascii__ (@var{handle})\n\
Undocumented internal function\n\
@end deftypefn")
{
#ifdef HAVE_CURL
  int nargin = args.length ();

  if (nargin != 1)
    error ("__ftp_ascii__: incorrect number of arguments");
  else
    {
      const curl_object curl = ch_manager::get_object (args(0));

      if (curl.is_valid ())
        curl.ascii ();
      else
        error ("__ftp_ascii__: invalid ftp handle");
    }
#else
  gripe_disabled_feature ("__ftp_ascii__", "FTP");
#endif

  return octave_value ();
}

DEFUN_DLD (__ftp_binary__, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} __ftp_binary__ (@var{handle})\n\
Undocumented internal function\n\
@end deftypefn")
{
#ifdef HAVE_CURL
  int nargin = args.length ();

  if (nargin != 1)
    error ("__ftp_binary__: incorrect number of arguments");
  else
    {
      const curl_object curl = ch_manager::get_object (args(0));

      if (curl.is_valid ())
        curl.binary ();
      else
        error ("__ftp_binary__: invalid ftp handle");
    }
#else
  gripe_disabled_feature ("__ftp_binary__", "FTP");
#endif

  return octave_value ();
}

DEFUN_DLD (__ftp_close__, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} __ftp_close__ (@var{handle})\n\
Undocumented internal function\n\
@end deftypefn")
{
#ifdef HAVE_CURL
  int nargin = args.length ();

  if (nargin != 1)
    error ("__ftp_close__: incorrect number of arguments");
  else
    {
      curl_handle h = ch_manager::lookup (args(0));

      if (h.ok ())
        ch_manager::free (h);
      else
        error ("__ftp_close__: invalid ftp handle");
    }
#else
  gripe_disabled_feature ("__ftp_close__", "FTP");
#endif

  return octave_value ();
}

DEFUN_DLD (__ftp_mode__, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} __ftp_mode__ (@var{handle})\n\
Undocumented internal function\n\
@end deftypefn")
{
  octave_value retval;
#ifdef HAVE_CURL
  int nargin = args.length ();

  if (nargin != 1)
    error ("__ftp_mode__: incorrect number of arguments");
  else
    {
      const curl_object curl = ch_manager::get_object (args(0));

      if (curl.is_valid ())
        retval = (curl.is_ascii () ? "ascii" : "binary");
      else
        error ("__ftp_binary__: invalid ftp handle");
    }
#else
  gripe_disabled_feature ("__ftp_mode__", "FTP");
#endif

  return retval;
}

DEFUN_DLD (__ftp_delete__, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} __ftp_delete__ (@var{handle}, @var{path})\n\
Undocumented internal function\n\
@end deftypefn")
{
#ifdef HAVE_CURL
  int nargin = args.length ();

  if (nargin != 2)
    error ("__ftp_delete__: incorrect number of arguments");
  else
    {
      const curl_object curl = ch_manager::get_object (args(0));

      if (curl.is_valid ())
        {
          std::string file = args(1).string_value ();

          if (! error_state)
            curl.del (file);
          else
            error ("__ftp_delete__: expecting file name as second argument");
        }
      else
        error ("__ftp_delete__: invalid ftp handle");
    }
#else
  gripe_disabled_feature ("__ftp_delete__", "FTP");
#endif

  return octave_value ();
}

DEFUN_DLD (__ftp_rmdir__, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} __ftp_rmdir__ (@var{handle}, @var{path})\n\
Undocumented internal function\n\
@end deftypefn")
{
#ifdef HAVE_CURL
  int nargin = args.length ();

  if (nargin != 2)
    error ("__ftp_rmdir__: incorrect number of arguments");
  else
    {
      const curl_object curl = ch_manager::get_object (args(0));

      if (curl.is_valid ())
        {
          std::string dir = args(1).string_value ();

          if (! error_state)
            curl.rmdir (dir);
          else
            error ("__ftp_rmdir__: expecting directory name as second argument");
        }
      else
        error ("__ftp_rmdir__: invalid ftp handle");
    }
#else
  gripe_disabled_feature ("__ftp_rmdir__", "FTP");
#endif

  return octave_value ();
}

DEFUN_DLD (__ftp_mkdir__, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} __ftp_mkdir__ (@var{handle}, @var{path})\n\
Undocumented internal function\n\
@end deftypefn")
{
#ifdef HAVE_CURL
  int nargin = args.length ();

  if (nargin != 2)
    error ("__ftp_mkdir__: incorrect number of arguments");
  else
    {
      const curl_object curl = ch_manager::get_object (args(0));

      if (curl.is_valid ())
        {
          std::string dir = args(1).string_value ();

          if (! error_state)
            curl.mkdir (dir);
          else
            error ("__ftp_mkdir__: expecting directory name as second argument");
        }
      else
        error ("__ftp_mkdir__: invalid ftp handle");
    }
#else
  gripe_disabled_feature ("__ftp_mkdir__", "FTP");
#endif

  return octave_value ();
}

DEFUN_DLD (__ftp_rename__, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} __ftp_rename__ (@var{handle}, @var{path})\n\
Undocumented internal function\n\
@end deftypefn")
{
#ifdef HAVE_CURL
  int nargin = args.length ();

  if (nargin != 3)
    error ("__ftp_rename__: incorrect number of arguments");
  else
    {
      const curl_object curl = ch_manager::get_object (args(0));

      if (curl.is_valid ())
        {
          std::string oldname = args(1).string_value ();
          std::string newname = args(2).string_value ();

          if (! error_state)
            curl.rename (oldname, newname);
          else
            error ("__ftp_rename__: expecting file names for second and third arguments");
        }
      else
        error ("__ftp_rename__: invalid ftp handle");
    }
#else
  gripe_disabled_feature ("__ftp_rename__", "FTP");
#endif

  return octave_value ();
}

DEFUN_DLD (__ftp_mput__, args, nargout,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} __ftp_mput__ (@var{handle}, @var{files})\n\
Undocumented internal function\n\
@end deftypefn")
{
  string_vector retval;

#ifdef HAVE_CURL
  int nargin = args.length ();

  if (nargin != 2)
    error ("__ftp_mput__: incorrect number of arguments");
  else
    {
      const curl_object curl = ch_manager::get_object (args(0));

      if (curl.is_valid ())
        {
          std::string pat = args(1).string_value ();

          if (! error_state)
            {
              glob_match pattern (file_ops::tilde_expand (pat));
              string_vector files = pattern.glob ();

              for (octave_idx_type i = 0; i < files.length (); i++)
                {
                  std::string file = files (i);

                  file_stat fs (file);

                  if (! fs.exists ())
                    {
                      error ("__ftp__mput: file does not exist");
                      break;
                    }

                  if (fs.is_dir ())
                    {
                      retval.append (curl.mput_directory ("", file));
                      if (error_state)
                        break;
                    }
                  else
                    {
                      // FIXME Does ascii mode need to be flagged here?
                      std::ifstream ifile (file.c_str (), std::ios::in |
                                           std::ios::binary);

                      if (! ifile.is_open ())
                        {
                          error ("__ftp_mput__: unable to open file");
                          break;
                        }

                      curl.put (file, ifile);

                      ifile.close ();

                      if (error_state)
                        break;

                      retval.append (file);
                    }
                }
            }
          else
            error ("__ftp_mput__: expecting file name patter as second argument");
        }
      else
        error ("__ftp_mput__: invalid ftp handle");
    }
#else
  gripe_disabled_feature ("__ftp_mput__", "FTP");
#endif

  return (nargout > 0 ? octave_value (retval) : octave_value ());
}

DEFUN_DLD (__ftp_mget__, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} __ftp_mget__ (@var{handle}, @var{files})\n\
Undocumented internal function\n\
@end deftypefn")
{
#ifdef HAVE_CURL
  int nargin = args.length ();

  if (nargin != 2 && nargin != 3)
    error ("__ftp_mget__: incorrect number of arguments");
  else
    {
      const curl_object curl = ch_manager::get_object (args(0));

      if (curl.is_valid ())
        {
          std::string file = args(1).string_value ();
          std::string target;

          if (nargin == 3)
            target = args(2).string_value () + file_ops::dir_sep_str ();

          if (! error_state)
            {
              string_vector sv = curl.list ();
              octave_idx_type n = 0;
              glob_match pattern (file);

              for (octave_idx_type i = 0; i < sv.length (); i++)
                {
                  if (pattern.match (sv(i)))
                    {
                      n++;

                      time_t ftime;
                      bool fisdir;
                      double fsize;

                      curl.get_fileinfo (sv(i), fsize, ftime, fisdir);

                      if (fisdir)
                        curl.mget_directory (sv(i), target);
                      else
                        {
                          std::ofstream ofile ((target + sv(i)).c_str (),
                                               std::ios::out |
                                               std::ios::binary);

                          if (! ofile.is_open ())
                            {
                              error ("__ftp_mget__: unable to open file");
                              break;
                            }

                          unwind_protect_safe frame;

                          frame.add_fcn (delete_file, target + sv(i));

                          curl.get (sv(i), ofile);

                          ofile.close ();

                          if (!error_state)
                            frame.discard ();
                          else
                            frame.run ();
                        }

                      if (error_state)
                        break;
                    }
                }
              if (n == 0)
                error ("__ftp_mget__: file not found");
            }
          else
            error ("__ftp_mget__: expecting file name and target as second and third arguments");
        }
      else
        error ("__ftp_mget__: invalid ftp handle");
    }
#else
  gripe_disabled_feature ("__ftp_mget__", "FTP");
#endif

  return octave_value ();
}
