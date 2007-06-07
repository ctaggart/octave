## Copyright (C) 2005 S�ren Hauberg
## 
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, write to the Free
## Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
## 02110-1301, USA.

## -*- texinfo -*-
## @deftypefn  {Command} pkg @var{command} @var{pkg_name}
## @deftypefnx {Command} pkg @var{command} @var{option} @var{pkg_name}
## This command interacts with the package manager. Different actions will
## be taken depending on the value of @var{command}.
##
## @table @samp
## @item install
## Install named packages.  For example,
## @example
## pkg install image-1.0.0.tar.gz
## @end example
## @noindent
## installs the package found in the file @code{image-1.0.0.tar.gz}.
##
## The @var{option} variable can contain options that affect the manner
## in which a package is installed. These options can be one or more of
##
## @table @code
## @item -nodeps
## The package manager will disable the dependency checking. That way it 
## is possible to install a package even if it depends on another package 
## that's not installed on the system. @strong{Use this option with care.}
##
## @item -noauto
## The package manager will not automatically load the installed package 
## when starting Octave, even if the package requests that it is.
##
## @item -auto
## The package manager will automatically load the installed package when 
## starting Octave, even if the package requests that it isn't.
##
## @item -local
## A local installation is forced, even if the user has system privileges.
##
## @item -global
## A global installation is forced, even if the user doesn't normally have
## system privileges
##
## @item -verbose
## The package manager will print the output of all of the commands that are 
## performed.
## @end table
##
## @item uninstall
## Uninstall named packages.  For example,
## @example
## pkg uninstall image
## @end example
## @noindent
## removes the @code{image} package from the system. If another installed
## package depends on the @code{image} package an error will be issued.
## The package can be uninstalled anyway by using the @code{-nodeps} option.
## @item load
## Add named packages to the path. After loading a package it is
## possible to use the functions provided by the package. For example,
## @example
## pkg load image
## @end example
## @noindent
## adds the @code{image} package to the path. It is possible to load all
## installed packages at once with the command
## @example
## pkg load all
## @end example
## @item unload
## Removes named packages from the path. After unloading a package it is
## no longer possible to use the functions provided by the package.
## This command behaves like the @code{load} command.
## @item list
## Show a list of the currently installed packages. By requesting one or two
## output argument it is possible to get a list of the currently installed
## packages. For example,
## @example
## installed_packages = pkg list;
## @end example
## @noindent
## returns a cell array containing a structure for each installed package.
## The command
## @example
## [@var{user_packages}, @var{system_packages}] = pkg list
## @end example
## @noindent
## splits the list of installed packages into those who are installed by
## the current user, and those installed by the system administrator.
## @item prefix
## Set the installation prefix directory. For example,
## @example
## pkg prefix ~/my_octave_packages
## @end example
## @noindent
## sets the installation prefix to @code{~/my_octave_packages}.
## Packages will be installed in this directory.
##
## It is possible to get the current installation prefix by requesting an
## output argument.  For example,
## @example
## p = pkg prefix
## @end example
## @item local_list
## Set the file in which to look for information on the locally
## installed packages. Locally installed packages are those that are
## typically available only to the current user. For example
## @example
## pkg local_list ~/.octave_packages
## @end example
## It is possible to get the current value of local_list with the following
## @example
## pkg local_list
## @end example
## @item global_list
## Set the file in which to look for, for information on the globally
## installed packages. Globally installed packages are those that are
## typically available to all users. For example
## @example
## pkg global_list /usr/share/octave/octave_packages
## @end example
## It is possible to get the current value of global_list with the following
## @example
## pkg global_list
## @end example
## @item rebuild
## Rebuilds the package database from the installed directories. This can 
## be used in cases where for some reason the package database is corrupted.
## It can also take the @code{-auto} and @code{-noauto} options to allow the
## autolaoding state of a package to be changed. For example
##
## @example
## pkg rebuild -noauto image
## @end example
##
## will remove the autoloading status of the image package.
## @item build
## Builds a binary form of a package or packages. The binary file produced
## will itself be an Octave package that can be installed normally with
## @code{pkg}. The form of the command to build a binary package is
##
## @example
## pkg build builddir image-1.0.0.tar.gz @dots{}
## @end example
##
## @noindent
## where @code{buiddir} is the name of a directory where the temporary
## installation will be produced and the binary packages will be found.
## The options @code{-verbose} and @code{-nodeps} are respected, while 
## the other options are ignored.
## @end table
## @end deftypefn

## PKG_ADD: mark_as_command pkg
## PKG_ADD: pkg ("load", "auto");

function [local_packages, global_packages] = pkg (varargin)
  ## Installation prefix (XXX: what should these be on windows?)
  persistent user_prefix = false;
  persistent prefix = -1;
  persistent local_list = tilde_expand (fullfile("~", ".octave_packages"));
  persistent global_list = fullfile (OCTAVE_HOME (), "share", "octave",
				     "octave_packages");
  mlock ();

  if (ispc () && ! isunix ())
    global_install = 1;
  else
    global_install = (geteuid() == 0);
  endif

  if (prefix == -1)
    if (global_install)
      prefix = fullfile (OCTAVE_HOME (), "share", "octave", "packages");
    else
      prefix = fullfile ("~", "octave");
    endif
    prefix = tilde_expand (prefix);
  endif

  ## Handle input
  if (length (varargin) == 0 || ! iscellstr (varargin))
    print_usage ();
  endif
  files = {};
  deps = true;
  auto = 0;
  action = "none";
  verbose = false;
  for i = 1:length (varargin)
    switch (varargin{i})
      case "-nodeps"
	deps = false;
      case "-noauto"
	auto = -1;
      case "-auto"
	auto = 1;
      case "-verbose"
	verbose = true;
      case "-local"
	global_install = false;
	if (! user_prefix)
	  prefix = fullfile ("~", "octave");
	endif
      case "-global"
	global_install = true;
	if (! user_prefix)
	  prefix = fullfile (OCTAVE_HOME (), "share", "octave", "packages");
	endif
      case {"list", "install", "uninstall", "load", "unload", ...
	    "prefix", "local_list", "global_list", "rebuild", "build"}
	if (strcmp (action,"none"))
	  action = varargin{i};
	else
	  error ("more than one action specified");
	endif
      otherwise
	files{end+1} = varargin{i};
    endswitch
  endfor

  ## Take action
  switch (action)
    case "list"
      if (nargout == 0)
	installed_packages (local_list, global_list);
      elseif (nargout == 1)
	local_packages = installed_packages (local_list, global_list);
      elseif (nargout == 2)
	[local_packages, global_packages] = installed_packages (local_list,
								global_list);
      else
	error ("too many output arguments requested");
      endif

    case "install"
      if (length (files) == 0)
	error ("you must specify at least one filename when calling 'pkg install'");
      endif
      install (files, deps, auto, prefix, verbose, local_list, 
	       global_list, global_install);

    case "uninstall"
      if (length (files) == 0)
	error ("you must specify at least one package when calling 'pkg uninstall'");
      endif
      uninstall (files, deps, verbose, local_list, 
		 global_list, global_install);

    case "load"
      if (length (files) == 0)
	error ("you must specify at least one package, 'all' or 'auto' when calling 'pkg load'");
      endif
      load_packages (files, deps, local_list, global_list);

    case "unload"
      if (length (files) == 0)
	error ("you must specify at least one package or 'all' when calling 'pkg unload'");
      endif
      unload_packages (files, deps, local_list, global_list);

    case "prefix"
      if (length (files) == 0 && nargout == 0)
	disp (prefix);
      elseif (length (files) == 0 && nargout == 1)
	local_packages = prefix;
      elseif (length (files) == 1 && nargout <= 1 && ischar (files{1}))
	prefix = files{1};
	## if (!strcmp(prefix(end), filesep))
	##   prefix(end+1) = filesep;
	## endif
	prefix = absolute_pathname (prefix);
	local_packages = prefix;
	user_prefix = true;
      else
	error ("you must specify a prefix directory, or request an output argument");
      endif

    case "local_list"
      if (length (files) == 0 && nargout == 0)
	disp (local_list);
      elseif (length (files) == 0 && nargout == 1)
	local_packages = local_list;
      elseif (length (files) == 1 && nargout == 0 && ischar (files{1}))
	local_list = absolute_pathname (files{1});
      else
	error ("you must specify a local_list file, or request an output argument");
      endif

    case "global_list"
      if (length (files) == 0 && nargout == 0)
	disp(global_list);
      elseif (length (files) == 0 && nargout == 1)
	local_packages = global_list;
      elseif (length (files) == 1 && nargout == 0 && ischar (files{1}))
	global_list = absolute_pathname (files{1});
      else
	error ("you must specify a global_list file, or request an output argument");
      endif

    case "rebuild"
      if (global_install)
	global_packages = rebuild (prefix, global_list, files, auto, verbose);
	global_packages = save_order (global_packages);
	save (global_list, "global_packages");
	if (nargout > 0)
	  local_packages = global_packages;
	endif
      else
	local_packages = rebuild (prefix, local_list, files, auto, verbose);
	local_packages = save_order (local_packages);
	save (local_list, "local_packages");
	if (nargout == 0)
	  clear ("local_packages");
	endif
      endif

    case "build"
      if (length (files) < 2)
	error ("you must specify at least the build directory and one filename\nwhen calling 'pkg build'");
      endif
      build (files, deps, auto, verbose);

    otherwise
      error ("you must specify a valid action for 'pkg'. See 'help pkg' for details");
  endswitch
endfunction

function descriptions = rebuild (prefix, list, files, auto, verbose)
  if (isempty (files))
    [dirlist, err, msg] = readdir (prefix);
    if (err)
      error ("couldn't read directory %s: %s", prefix, msg);
    endif
    ## the two first entries of dirlist are "." and ".."
    dirlist([1,2]) = [];
  else
    old_descriptions = installed_packages (list, list);
    wd = pwd ();
    unwind_protect
      cd (prefix);
      dirlist = glob (cellfun(@(x) strcat(x, '-*'), files, 'UniformOutput', 0))
    unwind_protect_cleanup
      cd (wd);
    end_unwind_protect
  endif
  descriptions = {};
  for k = 1:length (dirlist)
    descfile = fullfile (prefix, dirlist{k}, "packinfo", "DESCRIPTION");
    if (verbose)
      printf ("recreating package description from %s\n", dirlist{k});
    endif
    if (exist (descfile, "file"))
      desc = get_description (descfile);
      desc.dir = fullfile (prefix, dirlist{k});
      if (auto != 0)
	if (exist (fullfile (desc.dir, "packinfo", ".autoload"), "file"))
	  unlink (fullfile (desc.dir, "packinfo", ".autoload"));
	endif
        if (auto < 0)
	  desc.autoload = 0;
	elseif (auto > 0)
	  desc.autoload = 1;
	  fclose (fopen (fullfile (desc.dir, "packinfo", ".autoload"), "wt"));
	endif
      else
	if (exist (fullfile (desc.dir, "packinfo", ".autoload"), "file"))
	  desc.autoload = 1;
	else
	  desc.autoload = 0;
	endif
      endif
      descriptions{end + 1} = desc;
    elseif (verbose)
      warning ("directory %s is not a valid package", dirlist{k});
    endif
  endfor

  if (! isempty (files))
    ## We are rebuilding for a particular package(s) so we should take
    ## care to keep the other untouched packages in the descriptions
    descriptions = {descriptions{:}, old_descriptions{:}};

    dup = [];
    for i = 1:length (descriptions)
      if (find (dup, i))
	continue;
      endif
      for j = (i+1):length (descriptions)
	if (find (dup, j))
	  continue;
	endif
	if (strcmp (descriptions{i}.name, descriptions{j}.name))
	  dup = [dup, j];
	endif
      endfor
    endfor
    if (! isempty (dup))
      descriptions (dup) = [];
    endif  
  endif
endfunction

function build (files, handle_deps, autoload, verbose)
  if (length (files) < 1)
    error ("insufficient number of files");
  endif
  builddir = files{1};
  if (! exist (builddir, "dir"))
    warning ("creating build directory %s", builddir);
    [status, msg] = mkdir (builddir);
    if (status != 1)
      error ("could not create installation directory: %s", msg);
    endif
  endif
  builddir = absolute_pathname (builddir);
  installdir = fullfile (builddir, "install");
  if (! exist (installdir, "dir"))
    [status, msg] = mkdir (installdir);
    if (status != 1)
      error ("could not create installation directory: %s", msg);
    endif
  endif
  files(1) = [];
  buildlist = fullfile (builddir, "octave_packages");
  install (files, handle_deps, autoload, installdir, verbose, 
	   buildlist, "", false);
  unwind_protect
    repackage (builddir, buildlist);
  unwind_protect_cleanup
    unload_packages ({"all"}, handle_deps, buildlist, "");
    if (exist (installdir, "dir"))
      rm_rf (installdir);
    endif
    if (exist (buildlist, "file"))
      unlink (buildlist);
    endif
  end_unwind_protect
endfunction

function install (files, handle_deps, autoload, prefix, verbose, local_list, global_list, global_install)

  # Check that the directory in prefix exist. If it doesn't: create it!
  if (! exist (prefix, "dir"))
    warning ("creating installation directory %s", prefix);
    [status, msg] = mkdir (prefix);
    if (status != 1)
      error ("could not create installation directory: %s", msg);
    endif
  endif

  ## Get the list of installed packages
  [local_packages, global_packages] = installed_packages (local_list, 
							  global_list);

  installed_packages = {local_packages{:}, global_packages{:}};        

  if (global_install)
    packages = global_packages;
  else
    packages = local_packages;
  endif

  ## Uncompress the packages and read the DESCRIPTION files
  tmpdirs = packdirs = descriptions = {};
  try

    ## Warn about non existent files 
    for i = 1:length (files)
      if (isempty (glob(files{i}))) 
	warning ("file %s does not exist", files{i});
      endif
    endfor

    ## Unpack the package files and read the DESCRIPTION files
    files = glob (files);
    packages_to_uninstall = [];
    for i = 1:length (files)
      tgz = files{i};

      if (exist (tgz, "file"))
	## Create a temporary directory 
	tmpdir = tmpnam ();
	tmpdirs{end+1} = tmpdir;
        if (verbose)
	  printf ("mkdir (%s)\n", tmpdir);
	endif
	[status, msg] = mkdir (tmpdir);
	if (status != 1)
	  error ("couldn't create temporary directory: %s", msg);
	endif

	## Uncompress the package
	if (verbose)
	  printf ("untar (%s, %s)\n", tgz, tmpdir);
	endif
	untar (tgz, tmpdir);

	## Get the name of the directories produced by tar
	[dirlist, err, msg] = readdir (tmpdir);
	if (err)
	  error ("couldn't read directory produced by tar: %s", msg);
	endif

	if (length (dirlist) > 3)
	  error ("bundles of packages are not allowed")
	endif

	## the two first entries of dirlist are "." and ".."
	for k = 3:length (dirlist)
	  packdir = fullfile (tmpdir, dirlist{k});
	  packdirs{end+1} = packdir;

	  ## Make sure the package contains necessary files
	  verify_directory (packdir);

	  ## Read the DESCRIPTION file
	  filename = fullfile (packdir, "DESCRIPTION");
	  desc = get_description (filename);

	  ## Verify that package name corresponds with filename
	  [dummy, nm] = fileparts (tgz); 
	  if ((length (nm) >= length (desc.name))
	      && ! strcmp (desc.name, nm(1:length(desc.name))))
	    error ("package name '%s' doesn't correspond to its filename '%s'", desc.name, nm);
	  endif

	  ## Set default installation directory
	  desc.dir = fullfile (prefix, strcat (desc.name, "-", desc.version));

	  ## Save desc
	  descriptions{end+1} = desc;

	  ## Are any of the new packages already installed?
	  ## If so we'll remove the old version.
	  for j = 1:length (packages)
	    if (strcmp (packages{j}.name, desc.name))
	      packages_to_uninstall(end+1) = j;
	    endif
	  endfor
	endfor        
      endif
    endfor
  catch
    ## Something went wrong, delete tmpdirs
    for i = 1:length (tmpdirs)
      rm_rf (tmpdirs{i});
    endfor
    rethrow (lasterror ());
  end_try_catch

  ## Check dependencies
  if (handle_deps)
    ok = true;
    error_text = "";
    for i = 1:length (descriptions)
      desc = descriptions{i};
      idx2 = complement (i, 1:length(descriptions));
      if (global_install)
	## Global installation is not allowed to have dependencies on locally
	## installed packages
	idx1 = complement (packages_to_uninstall, 
			   1:length(global_packages));
	pseudo_installed_packages = {global_packages{idx1}, ...
				     descriptions{idx2}};
      else
	idx1 = complement (packages_to_uninstall, 
			   1:length(local_packages));
	pseudo_installed_packages = {local_packages{idx1}, ... 
				     global_packages{:}, ...
				     descriptions{idx2}};
      endif
      bad_deps = get_unsatisfied_deps (desc, pseudo_installed_packages);
      ## Are there any unsatisfied dependencies?
      if (! isempty (bad_deps))
	ok = false;
	for i = 1:length (bad_deps)
	  dep = bad_deps{i};
	  error_text = strcat (error_text, " ", desc.name, " needs ",
			       dep.package, " ", dep.operator, " ",
			       dep.version, "\n");
	endfor
      endif
    endfor

    ## Did we find any unsatisfied dependencies?
    if (! ok)
      error ("the following dependencies where unsatisfied:\n  %s", error_text);
    endif
  endif

  ## Prepare each package for installation
  try
    for i = 1:length (descriptions)
      desc = descriptions{i};
      pdir = packdirs{i};
      prepare_installation (desc, pdir);
      configure_make (desc, pdir, verbose);
    endfor
  catch
    ## Something went wrong, delete tmpdirs
    for i = 1:length (tmpdirs)
      rm_rf (tmpdirs{i});
    endfor
    rethrow (lasterror ());
  end_try_catch

  ## Uninstall the packages that will be replaced
  try
    for i = packages_to_uninstall
      if (global_install)
	uninstall ({global_packages{i}.name}, false, verbose, local_list, 
		   global_list, global_install);
      else
	uninstall ({local_packages{i}.name}, false, verbose, local_list, 
		   global_list, global_install);
      endif
    endfor
  catch
    ## Something went wrong, delete tmpdirs
    for i = 1:length (tmpdirs)
      rm_rf (tmpdirs{i});
    endfor
    rethrow (lasterror ());
  end_try_catch

  ## Install each package
  try
    for i = 1:length (descriptions)
      desc = descriptions{i};
      pdir = packdirs{i};
      copy_files (desc, pdir);
      create_pkgadddel (desc, pdir, "PKG_ADD");
      create_pkgadddel (desc, pdir, "PKG_DEL");
      finish_installation (desc, pdir)
    endfor
  catch
    ## Something went wrong, delete tmpdirs
    for i = 1:length (tmpdirs)
      rm_rf (tmpdirs{i});
    endfor
    for i = 1:length (descriptions)
      rm_rf (descriptions{i}.dir);
    endfor
    rethrow (lasterror ());
  end_try_catch

  ## Check if the installed directory is empty. If it is remove it
  ## from the list
  for i = length (descriptions):-1:1
    if (dirempty (descriptions{i}.dir, {"packinfo", "doc"}))
      warning ("package %s is empty\n", descriptions{i}.name);
      rm_rf (descriptions{i}.dir);
      descriptions(i) = [];
    endif
  endfor

  ## If the package requested that it is autoloaded, or the installer
  ## requested that it is, then mark the package as autoloaded.
  for i = length (descriptions):-1:1
    if (autoload > 0 || (autoload == 0 && isautoload (descriptions(i))))
      fclose (fopen (fullfile (descriptions{i}.dir, "packinfo", 
			       ".autoload"), "wt"));
      descriptions{i}.autoload = 1;
    endif
  endfor

  ## Add the packages to the package list
  try
    if (global_install)
      idx = complement (packages_to_uninstall, 1:length(global_packages));
      global_packages = save_order ({global_packages{idx}, descriptions{:}});
      save (global_list, "global_packages");
      installed_packages = {local_packages{:}, global_packages{:}};
    else
      idx = complement (packages_to_uninstall, 1:length(local_packages));
      local_packages = save_order ({local_packages{idx}, descriptions{:}});
      save (local_list, "local_packages");
      installed_packages = {local_packages{:}, global_packages{:}};
    endif
  catch
    ## Something went wrong, delete tmpdirs
    for i = 1:length (tmpdirs)
      rm_rf (tmpdirs{i});
    endfor
    for i = 1:length (descriptions)
      rm_rf (descriptions{i}.dir);
    endfor
    if (global_install)
      printf ("error: couldn't append to %s\n", global_list);
    else
      printf ("error: couldn't append to %s\n", local_list);
    endif
    rethrow (lasterror ());
  end_try_catch

  ## All is well, let's clean up
  for i = 1:length (tmpdirs)
    [status, msg] = rm_rf (tmpdirs{i});
    if (status != 1)
      warning ("couldn't clean up after my self: %s\n", msg);
    endif
  endfor

  ## Add the newly installed packages to the path, so the user
  ## can begin usings them. Only load them if they are marked autoload
  if (length (descriptions) > 0)
    idx = [];
    for i = 1:length (descriptions)
      if (isautoload (descriptions(i)))
	idx (end + 1) = i;
      endif
    endfor
    load_packages_and_dependencies (idx, handle_deps, installed_packages);
  endif
endfunction

function uninstall (pkgnames, handle_deps, verbose, local_list, 
		    global_list, global_install)
  ## Get the list of installed packages
  [local_packages, global_packages] = installed_packages(local_list, 
							 global_list);
  if (global_install)
    installed_packages = {local_packages{:}, global_packages{:}};
  else
    installed_packages = local_packages;
  endif

  num_packages = length (installed_packages);
  delete_idx = [];
  for i = 1:num_packages
    cur_name = installed_packages{i}.name;
    if (any (strcmp (cur_name, pkgnames)))
      delete_idx(end+1) = i;
    endif
  endfor

  ## Are all the packages that should be uninstalled already installed?
  if (length (delete_idx) != length (pkgnames))
    if (global_install)
      ## Try again for a locally installed package
      installed_packages = local_packages;

      num_packages = length (installed_packages);
      delete_idx = [];
      for i = 1:num_packages
	cur_name = installed_packages{i}.name;
	if (any (strcmp (cur_name, pkgnames)))
	  delete_idx(end+1) = i;
	endif
      endfor
      if (length (delete_idx) != length (pkgnames))
	## XXX: We should have a better error message
	warning ("some of the packages you want to uninstall are not installed");
      endif
    else
      ## XXX: We should have a better error message
      warning ("some of the packages you want to uninstall are not installed.");
    endif
  endif

  ## Compute the packages that will remain installed
  idx = complement (delete_idx, 1:num_packages);
  remaining_packages = {installed_packages{idx}};

  ## Check dependencies
  if (handle_deps)
    error_text = "";
    for i = 1:length (remaining_packages)
      desc = remaining_packages{i};
      bad_deps = get_unsatisfied_deps (desc, remaining_packages);

      ## Will the uninstallation break any dependencies?
      if (! isempty (bad_deps))
	for i = 1:length (bad_deps)
	  dep = bad_deps{i};
	  error_text = strcat (error_text, " ", desc.name, " needs ",
			       dep.package, " ", dep.operator, " ",
			       dep.version, "\n");
	endfor
      endif
    endfor

    if (! isempty (error_text))
      error ("the following dependencies where unsatisfied:\n  %s", error_text);
    endif
  endif

  ## Delete the directories containing the packages
  for i = delete_idx
    desc = installed_packages{i};
    ## If an 'on_uninstall.m' exist, call it!
    if (exist (fullfile (desc.dir, "packinfo", "on_uninstall.m"), "file"))
      wd = pwd ();
      cd (fullfile(desc.dir, "packinfo"));
      on_uninstall (desc);
      cd (wd);
    endif
    ## Do the actual deletion
    if (desc.loaded)
      rmpath (desc.dir);
      if (exist (fullfile (desc.dir, getarch()), "dir"))
	rmpath (fullfile (desc.dir, getarch ()));
      endif
    endif
    if (exist (desc.dir, "dir"))
      [status, msg] = rm_rf (desc.dir);
      if (status != 1)
	error ("couldn't delete directory %s: %s", desc.dir, msg);
      endif
    else
      warning ("directory %s previously lost", desc.dir);
    endif
  endfor

  ## Write a new ~/.octave_packages
  if (global_install)
    if (length (remaining_packages) == 0)
      unlink (global_list);
    else
      global_packages = save_order (remaining_packages);
      save (global_list, "global_packages");
    endif
  else
    if (length (remaining_packages) == 0)
      unlink (local_list);
    else
      local_packages = save_order (remaining_packages);
      save (local_list, "local_packages");
    endif
  endif

endfunction

##########################################################
##        A U X I L I A R Y    F U N C T I O N S        ##
##########################################################

function pth = absolute_pathname (pth)
  [status, msg, msgid] = fileattrib(pth);
  pth = msg.Name;
endfunction

function repackage (builddir, buildlist)
  packages = installed_packages (buildlist, buildlist);

  wd = pwd();
  for i = 1 : length(packages)
    pack = packages{i};
    unwind_protect
      cd (builddir);
      mkdir (pack.name);
      mkdir (fullfile (pack.name, "inst"));
      copyfile (fullfile (pack.dir, "*"), fullfile (pack.name, "inst"));
      movefile (fullfile (pack.name, "inst","packinfo", "*"), pack.name);
      if (exist (fullfile (pack.name, "inst","packinfo", ".autoload"), "file"))
	unlink (fullfile (pack.name, "inst","packinfo", ".autoload"));
      endif
      rmdir (fullfile (pack.name, "inst", "packinfo"));
      if (exist (fullfile (pack.name, "inst", "doc"), "dir"))
	movefile (fullfile (pack.name, "inst", "doc"), pack.name);
      endif
      if (exist (fullfile (pack.name, "inst", "bin"), "dir"))
	movefile (fullfile (pack.name, "inst", "bin"), pack.name);
      endif
      archdir = fullfile (pack.name, "inst", getarch ());
      if (exist (archdir, "dir"))
	if (exist (fullfile (pack.name, "inst", "PKG_ADD"), "file"))
	  unlink (fullfile (pack.name, "inst", "PKG_ADD"));
	endif
	if (exist (fullfile (pack.name, "inst", "PKG_DEL"), "file"))
	  unlink (fullfile (pack.name, "inst", "PKG_DEL"));
	endif
	if (exist (fullfile (archdir, "PKG_ADD"), "file"))
	  movefile (fullfile (archdir, "PKG_ADD"), 
		    fullfile (pack.name, "PKG_ADD"));
	endif
	if (exist (fullfile (archdir, "PKG_DEL"), "file"))
	  movefile (fullfile (archdir, "PKG_DEL"), 
		    fullfile (pack.name, "PKG_DEL")); 
	endif
      else
	if (exist (fullfile (pack.name, "inst", "PKG_ADD"), "file"))
	  movefile (fullfile (pack.name, "inst", "PKG_ADD"), 
		    fullfile (pack.name, "PKG_ADD"));
	endif 
	if (exist (fullfile (pack.name, "inst", "PKG_DEL"), "file"))
	  movefile (fullfile (pack.name, "inst", "PKG_DEL"), 
		    fullfile (pack.name, "PKG_DEL")); 
	endif	
      endif	
      tfile = strcat (pack.name, "-", pack.version, ".tar");
      tar (tfile, pack.name);

    unwind_protect_cleanup
      if (exist (pack.name, "dir"))
	rm_rf (pack.name);
      endif
      cd (wd);
    end_unwind_protect
  endfor
endfunction

function auto = isautoload (desc)
  auto = false;
  if (isfield (desc{1}, "autoload"))
    a = desc{1}.autoload;
    if ((isnumeric (a) && a > 0)
        || (ischar (a) && (strcmpi (a, "true")
			 || strcmpi (a, "on")
			 || strcmpi (a, "yes")
			 || strcmpi (a, "1"))))
      auto = true;
    endif
  endif
endfunction

function prepare_installation (desc, packdir)
  ## Is there a pre_install to call?
  if (exist (fullfile (packdir, "pre_install.m"), "file"))
    wd = pwd ();
    try
      cd (packdir);
      pre_install (desc); 
      cd (wd);
    catch
      cd (wd);
      rethrow (lasterror ());
    end_try_catch
  endif

  ## If the directory "inst" doesn't exist, we create it
  inst_dir = fullfile (packdir, "inst");
  if (! exist (inst_dir, "dir"))
    [status, msg] = mkdir (inst_dir);
    if (status != 1)
      rm_rf (desc.dir);
      error ("the 'inst' directory did not exist and could not be created: %s", 
	     msg);
    endif
  endif
endfunction

function configure_make (desc, packdir, verbose)   
  ## Perform ./configure, make, make install in "src"
  if (exist (fullfile (packdir, "src"), "dir"))
    src = fullfile (packdir, "src");
    ## configure
    if (exist (fullfile (src, "configure"), "file"))
      [status, output] = shell (strcat ("cd ", src, "; ./configure --prefix=\"",
					 desc.dir, "\"",
					 " CC=", octave_config_info ("CC"),
					 " CXX=", octave_config_info ("CXX"),
					 " AR=", octave_config_info ("AR"),
					 " RANLIB=", octave_config_info ("RANLIB")));
      if (status != 0)
	rm_rf (desc.dir);
	error ("the configure script returned the following error: %s", output);
      elseif (verbose)
	printf("%s", output);
      endif

    endif

    ## make
    if (exist (fullfile (src, "Makefile"), "file"))
      [status, output] = shell (strcat ("export INSTALLDIR=\"", desc.dir,
					 "\"; make -C ", src));
      if (status != 0)
	rm_rf (desc.dir);
	error ("'make' returned the following error: %s", output);
      elseif (verbose)
	printf("%s", output);
      endif
    endif

    ## Copy files to "inst" and "inst/arch" (this is instead of 'make install')
    files = fullfile (src, "FILES");
    instdir = fullfile (packdir, "inst");
    archdir = fullfile (packdir, "inst", getarch ());
    ## Get file names
    if (exist (files, "file"))
      [fid, msg] = fopen (files, "r");
      if (fid < 0)
	error ("couldn't open %s: %s", files, msg);
      endif
      filenames = char (fread (fid))';
      fclose (fid);
      if (filenames(end) == "\n")
	filenames(end) = [];
      endif
      filenames = split_by (filenames, "\n");
      delete_idx =  [];
      for i = 1:length (filenames)
	if (! all (isspace (filenames{i})))
	  filenames{i} = fullfile (src, filenames{i});
	else
	  delete_idx(end+1) = i;
	endif
      endfor
      filenames(delete_idx) = [];
      idx1 = cellfun ("isempty", regexp (filenames, '^.*\.mex'));
      idx2 = cellfun ("isempty", regexp (filenames, '^.*\.oct'));
      mex = filenames;
      mex(idx1 != 0) = [];
      oct = filenames;
      oct(idx2 != 0) = [];
      archindependent = filenames;
      archindependent(idx1 == 0 | idx2 == 0) = [];
      archdependent = [oct, mex];
    else
      m = dir (fullfile (src, "*.m"));
      oct = dir (fullfile (src, "*.oct"));
      mex = dir (fullfile (src, "*.mex"));
      archdependent = "";
      archindependent = "";
      filenames = "";
      if (length (m) > 0)
	filenames = sprintf (fullfile (src, "%s "), m.name);
	archindependent = sprintf (fullfile (src, "%s "), m.name);
      endif
      if (length (oct) > 0)
	filenames = strcat (filenames, " ", sprintf(fullfile(src, "%s "), ...
						    oct.name));
	archdependent = strcat (archdependent, " ", ...
			 sprintf(fullfile(src, "%s "), oct.name));
      endif
      if (length (mex) > 0)
	filenames = strcat (filenames, " ", sprintf(fullfile(src, "%s "), ...
						    mex.name));
	archdependent = strcat (archdependent, " ", ...
			 sprintf(fullfile(src, "%s "), mex.name));
      endif
      filenames = split_by (filenames, " ");
      archdependent = split_by (archdependent, " ");
      archindependent = split_by (archindependent, " ");
    endif

    ## Copy the files
    if (! all (isspace (filenames)))
	if (! exist (instdir, "dir")) 
	  mkdir (instdir);
	endif
	if (! all (isspace (archindependent)))
	  if (verbose)
	    printf ("copyfile");
	    printf (" %s", archindependent{:});
	    printf ("%s\n", instdir);
	  endif
	  [status, output] = copyfile (archindependent, instdir);
	  if (status != 1)
	    rm_rf (desc.dir);
	    error ("Couldn't copy files from 'src' to 'inst': %s", output);
	  endif
        endif
	if (! all (isspace (archdependent)))
	  if (verbose)
	    printf ("copyfile");
	    printf (" %s", archdependent{:});
	    printf (" %s\n", archdir);
	  endif
	  if (! exist (archdir, "dir")) 
	    mkdir (archdir);
	  endif
	  [status, output] = copyfile (archdependent, archdir);
	  if (status != 1)
	    rm_rf (desc.dir);
	    error ("Couldn't copy files from 'src' to 'inst': %s", output);
	  endif
        endif
    endif
  endif
endfunction

function pkg = extract_pkg (nm, pat)
  fid = fopen (nm, "rt");
  pkg = "";
  if (fid >= 0)
    while (! feof (fid))
      ln = fgetl (fid);
      if (ln > 0)
	t = regexp (ln, pat, "tokens");
	if (! isempty (t))
          pkg = strcat (pkg, "\n", t{1}{1});
	endif
      endif
    endwhile
    if (! isempty (pkg))
      pkg = strcat (pkg, "\n");
    endif
    fclose (fid);
  endif
endfunction

function create_pkgadddel (desc, packdir, nm)
  instpkg = fullfile (desc.dir, nm);
  instfid = fopen (instpkg, "wt");
  ## If it is exists, most of the  PKG_* file should go into the 
  ## architecture dependent directory so that the autoload/mfilename 
  ## commands work as expected. The only part that doesn't is the
  ## part in the main directory.
  if (exist (fullfile (desc.dir, getarch ()), "dir"))
    archpkg = fullfile (desc.dir, getarch (), nm);
    archfid = fopen (archpkg, "wt");
  else
    archpkg = instpkg;
    archfid = instfid;
  endif

  if (archfid >= 0 && instfid >= 0)
    ## Search all dot-m files for PKG commands
    lst = dir (fullfile(packdir, "inst", "*.m"));
    for i = 1:length (lst)
      nam = fullfile(packdir, "inst", lst(i).name);
      fwrite (instfid, extract_pkg (nam, ['^[#%][#%]* *' nm ': *(.*)$']));
    endfor

    ## Search all C++ source files for PKG commands
    lst = dir (fullfile(packdir, "src", "*.cc"));
    for i = 1:length (lst)
      nam = fullfile(packdir, "src", lst(i).name);
      fwrite (archfid, extract_pkg (nam, ['^//* *' nm ': *(.*)$']));
      fwrite (archfid, extract_pkg (nam, ['^/\** *' nm ': *(.*) *\*/$']));
    endfor

    ## Add developer included PKG commands
    packdirnm = fullfile (packdir, nm);
    if (exist (packdirnm, "file"))
      fid = fopen (packdirnm, "rt");
      if (fid >= 0)
        while (! feof (fid))
          ln = fgets (fid);
          if (ln > 0)
            fwrite (archfid, ln);
          endif
        endwhile
        fclose (fid);
      endif
    endif

    ## If the files is empty remove it
    fclose (instfid);
    t = dir (instpkg);
    if (t.bytes <= 0)
      unlink (instpkg);
    endif

    if (instfid != archfid)
      fclose (archfid);
      t = dir (archpkg);
      if (t.bytes <= 0)
        unlink (archpkg);
      endif
    endif
  endif
endfunction

function copy_files (desc, packdir, bindir)
  ## Create the installation directory
  if (! exist (desc.dir, "dir"))
    [status, output] = mkdir (desc.dir);
    if (status != 1)
      error ("couldn't create installation directory %s : %s", 
      desc.dir, output);
    endif
  endif

  ## Copy the files from "inst" to installdir
  instdir = fullfile (packdir, "inst");
  if (! dirempty (instdir))
    [status, output] = copyfile (fullfile (instdir, "*"), desc.dir);
    if (status != 1)
      rm_rf (desc.dir);
      error ("couldn't copy files to the installation directory");
    endif
  endif

  ## Create the "packinfo" directory
  packinfo = fullfile (desc.dir, "packinfo");
  [status, msg] = mkdir (packinfo);
  if (status != 1)
    rm_rf (desc.dir);
    error ("couldn't create packinfo directory: %s", msg);
  endif

  ## Copy DESCRIPTION
  [status, output] = copyfile (fullfile (packdir, "DESCRIPTION"), packinfo);
  if (status != 1)
   rm_rf (desc.dir);
   error ("couldn't copy DESCRIPTION: %s", output);
  endif

  ## Copy COPYING
  [status, output] = copyfile (fullfile (packdir, "COPYING"), packinfo);
  if (status != 1)
   rm_rf (desc.dir);
   error ("couldn't copy COPYING: %s", output);
  endif

  ## If the file ChangeLog exists, copy it
  fChangeLog = fullfile(packdir, "ChangeLog");
  if (exist (fChangeLog, "file"))
    [status, output] = copyfile (fChangeLog, packinfo);
    if (status != 1)
      rm_rf (desc.dir);
      error ("couldn't copy ChangeLog file: %s", output);
    endif
  endif

  ## Is there an INDEX file to copy or should we generate one?
  fINDEX = fullfile (packdir, "INDEX");
  if (exist(fINDEX, "file"))
    [status, output] = copyfile (fINDEX, packinfo);
    if (status != 1)
      rm_rf (desc.dir);
      error ("couldn't copy INDEX file: %s", output);
    endif
  else
    try
      write_INDEX (desc, fullfile (packdir, "inst"),
		   fullfile (packinfo, "INDEX"));
    catch
      rm_rf (desc.dir);
      rethrow (lasterror ());
    end_try_catch
  endif

  ## Is there an 'on_uninstall.m' to install?
  fon_uninstall = fullfile(packdir, "on_uninstall.m");
  if (exist (fon_uninstall, "file"))
    [status, output] = copyfile (fon_uninstall, packinfo);
    if (status != 1)
      rm_rf (desc.dir);
      error ("couldn't copy on_uninstall.m: %s", output);
    endif
  endif

  ## Is there a doc/ directory that needs to be installed
  docdir = fullfile (packdir, "doc");
  if (exist (docdir, "dir") && ! dirempty (docdir))
   [status, output] = copyfile (docdir, desc.dir);
  endif

  ## Is there a bin/ directory that needs to be installed
  bindir = fullfile (packdir, "bin");
  if (exist (bindir, "dir") && ! dirempty (bindir))
   [status, output] = copyfile (bindir, desc.dir);
  endif
endfunction

function finish_installation (desc, packdir)
  ## Is there a post-install to call?
  if (exist (fullfile (packdir, "post_install.m"), "file"))
    wd = pwd ();
    try
      cd (packdir);
      post_install (desc);
      cd (wd);
    catch
      cd (wd);
      rm_rf (desc.dir);
      rethrow (lasterror ());
    end_try_catch
  endif
endfunction

## This function makes sure the package contains the
## essential files.
function verify_directory (dir)
  needed_files = {"COPYING", "DESCRIPTION"};
  for f = needed_files
    if (! exist (fullfile (dir, f{1}), "file"))
      error ("package is missing file: %s", f{1});
    endif
  endfor
endfunction

## This function parses the DESCRIPTION file
function desc = get_description (filename)
  [fid, msg] = fopen (filename, "r");
  if (fid == -1)
    error ("the DESCRIPTION file %s could not be read: %s", filename, msg);
  endif

  desc = struct ();

  line = fgetl (fid);
  while (line != -1)
    if (line(1) == "#")
      ## Comments, do nothing
    elseif (isspace(line(1)))
      ## Continuation lines
      if (exist ("keyword", "var") && isfield (desc, keyword))
	desc.(keyword) = strcat (desc.(keyword), " ", rstrip(line));
      endif
    else
      ## Keyword/value pair
      colon = find (line == ":");
      if (length (colon) == 0)
	disp ("skipping line");
      else
	colon = colon(1);
	keyword = tolower (strip (line(1:colon-1)));
	value   = strip (line (colon+1:end));
	if (length (value) == 0)
	    fclose (fid);
	    error ("the keyword %s has an empty value", desc.keywords{end});
	endif
	desc.(keyword) = value;
      endif
    endif
    line = fgetl (fid);
  endwhile
  fclose (fid);

  ## Make sure all is okay
  needed_fields = {"name", "version", "date", "title", ...
		   "author", "maintainer", "description"};
  for f = needed_fields
    if (! isfield (desc, f{1}))
      error ("description is missing needed field %s", f{1});
    endif
  endfor
  desc.version = fix_version (desc.version);
  if (isfield (desc, "depends"))
    desc.depends = fix_depends (desc.depends);
  else
    desc.depends = "";
  endif
  desc.name = tolower (desc.name);
endfunction

## Makes sure the version string v is a valid x.y.z version string
## Examples: "0.1" => "0.1.0", "monkey" => error(...)
function out = fix_version (v)
  dots = find (v == ".");
  if (length (dots) == 1)
    major = str2num (v(1:dots-1));
    minor = str2num (v(dots+1:end));
    if (length (major) != 0 && length (minor) != 0)
      out = sprintf ("%d.%d.0", major, minor);
      return;
    endif
  elseif (length (dots) == 2)
    major = str2num (v(1:dots(1)-1));
    minor = str2num (v(dots(1)+1:dots(2)-1));
    rev   = str2num (v(dots(2)+1:end));
    if (length (major) != 0 && length (minor) != 0 && length (rev) != 0)
      out = sprintf ("%d.%d.%d", major, minor, rev);
      return;
    endif
  endif
  error ("bad version string: %s", v);
endfunction

## Makes sure the depends field is of the right format.
## This function returns a cell of structures with the following fields:
##   package, version, operator
function deps_cell = fix_depends (depends)
  deps = split_by (tolower (depends), ",");
  deps_cell = cell (1, length (deps));

  ## For each dependency
  for i = 1:length (deps)
    dep = deps{i};
    lpar = find (dep == "(");
    rpar = find (dep == ")");
    ## Does the dependency specify a version
    ## Example: package(>= version)
    if (length (lpar) == 1 && length (rpar) == 1)
      package = tolower (strip (dep(1:lpar-1)));
      sub = dep(lpar(1)+1:rpar(1)-1);
      parts = split_by (sub, " ");
      idx = [];
      for r = 1:size (parts, 1)
	if (length (parts{r}) > 0)
	  idx(end+1) = r;
	endif
      endfor

      if (length (idx) != 2)
	error ("incorrect syntax for dependency `%s' in the DESCRIPTION file\n",
	       dep);
      endif
      operator = parts{idx(1)};
      if (! any (strcmp (operator, {">", ">=", "<=", "<", "=="})))
	error ("unsupported operator: %s", operator);
      endif
      version  = fix_version (parts{idx(2)});

  ## If no version is specified for the dependency
  ## we say that the version should be greater than 
  ## or equal to 0.0.0
  else
    package = tolower (strip (dep));
    operator = ">=";
    version  = "0.0.0";
  endif
  deps_cell{i} = struct ("package", package, "operator", operator,
			 "version", version);
  endfor
endfunction

## Strips the text of spaces from the right
## Example: "  hello world  " => "  hello world" (XXX: is this the same as deblank?)
function text = rstrip (text)
  chars = find (! isspace (text));
  if (length (chars) > 0)
    ## XXX: shouldn't it be text = text(1:chars(end));
    text = text (chars(1):end);
  else
    text = "";
  endif
endfunction

## Strips the text of spaces from the left and the right
## Example: "  hello world  " => "hello world"
function text = strip (text)
  chars = find (! isspace (text));
  if (length (chars) > 0)
    text = text(chars(1):chars(end));
  else
    text = "";
  endif
endfunction

## Splits the text into a cell array of strings by sep
## Example: "A, B" => {"A", "B"} (with sep = ",")
function out = split_by (text, sep)
  text_matrix = split (text, sep);
  num_words = size (text_matrix, 1);
  out = cell (num_words, 1);
  for i = 1:num_words
    out{i} = strip (text_matrix(i, :));
  endfor
endfunction

## Creates an INDEX file for a package that doesn't provide one.
##   'desc'  describes the package.
##   'dir'   is the 'inst' direcotyr in temporary directory.
##   'INDEX' is the name (including path) of resulting INDEX file.
function write_INDEX (desc, dir, INDEX)
  ## Get names of functions in dir
  [files, err, msg] = readdir (dir);
  if (err)
    error ("couldn't read directory %s: %s", dir, msg);
  endif

  ## Check for architecture dependent files
  arch = getarch();
  tmpdir = fullfile (dir, arch);
  if (exist (tmpdir, "dir"))
    [files2, err, msg] = readdir (tmpdir);
    if (err)
      error ("couldn't read directory %s: %s", tmpdir, msg);
    endif
    files = [files; files2];    
  endif

  functions = {};
  for i = 1:length (files)
    file = files{i};
    lf = length (file);
    if (lf > 2 && strcmp (file(end-1:end), ".m"))
      functions{end+1} = file(1:end-2);
    elseif (lf > 4 && strcmp (file(end-3:end), ".oct"))
      functions{end+1} = file(1:end-4);
    endif
  endfor

  ## Does desc have a categories field?
  if (! isfield (desc, "categories"))
    error ("the DESCRIPTION file must have a Categories field, when no INDEX file is given");
  endif
  categories = split_by (desc.categories, ",");
  if (length (categories) < 1)
      error ("the Category field is empty");
  endif

  ## Write INDEX
  fid = fopen (INDEX, "w");
  if (fid == -1)
    error ("couldn't open %s for writing.", INDEX);
  endif
  fprintf (fid, "%s >> %s\n", desc.name, desc.title);
  fprintf (fid, "%s\n", categories{1});
  fprintf (fid, "  %s\n", functions{:});
  fclose (fid);
endfunction

function bad_deps = get_unsatisfied_deps (desc, installed_packages)
  bad_deps = {};

  ## For each dependency
  for i = 1:length (desc.depends)
    dep = desc.depends{i};

    ## Is the current dependency Octave?
    if (strcmp (dep.package, "octave"))
      if (! compare_versions (OCTAVE_VERSION, dep.version, dep.operator))
        bad_deps{end+1} = dep;
      endif
    ## Is the current dependency not Octave?
    else
      ok = false;
      for i = 1:length (installed_packages)
	cur_name = installed_packages{i}.name;
	cur_version = installed_packages{i}.version;
	if (strcmp (dep.package, cur_name)
	    && compare_versions (cur_version, dep.version, dep.operator))
	  ok = true;
	  break;
	endif
      endfor
      if (! ok)
        bad_deps{end+1} = dep;
      endif
    endif
  endfor
endfunction

function [out1, out2] = installed_packages (local_list, global_list)
  ## Get the list of installed packages
  try
    local_packages = load (local_list).local_packages;
  catch
    local_packages = {};
  end_try_catch
  try
    global_packages = load (global_list).global_packages;
  catch
    global_packages = {};
  end_try_catch
  installed_packages = {local_packages{:}, global_packages{:}};

  ## Eliminate duplicates in the installed package list.
  ## Locally installed packages take precedence
  dup = [];
  for i = 1:length (installed_packages)
    if (find (dup, i))
      continue;
    endif
    for j = (i+1):length (installed_packages)
      if (find (dup, j))
	continue;
      endif
      if (strcmp (installed_packages{i}.name, installed_packages{j}.name))
	dup = [dup, j];
      endif
    endfor
  endfor
  if (! isempty(dup))
    installed_packages(dup) = [];
  endif  

  ## Now check if the package is loaded
  tmppath = strrep (path(), "\\", "/");
  for i = 1:length (installed_packages)
    if (regexp (tmppath, strrep (installed_packages{i}.dir, "\\", "/")))
      installed_packages{i}.loaded = true;
    else
      installed_packages{i}.loaded = false;
    endif
  endfor
  for i = 1:length (local_packages)
    if (regexp (tmppath, strrep (local_packages{i}.dir, "\\", "/")))
      local_packages{i}.loaded = true;
    else
      local_packages{i}.loaded = false;
    endif
  endfor
  for i = 1:length (global_packages)
    if (regexp (tmppath, strrep (global_packages{i}.dir, "\\", "/")))
      global_packages{i}.loaded = true;
    else
      global_packages{i}.loaded = false;
    endif
  endfor

  ## Should we return something?
  if (nargout == 2)
    out1 = local_packages;
    out2 = global_packages;
    return;
  elseif (nargout == 1)
    out1 = installed_packages;
    return;
  endif

  ## We shouldn't return something, so we'll print something
  num_packages = length (installed_packages);
  if (num_packages == 0)
    printf ("no packages installed.\n");
    return;
  endif

  ## Compute the maximal lengths of name, version, and dir
  h1 = "Package Name";
  h2 = "Version";
  h3 = "Installation directory";
  max_name_length = length (h1); 
  max_version_length = length (h2);
  names = cell (num_packages, 1); 
  for i = 1:num_packages
    max_name_length = max (max_name_length,
			   length (installed_packages{i}.name));
    max_version_length = max (max_version_length,
			      length (installed_packages{i}.version));
    names{i} = installed_packages{i}.name;
  endfor
  h1 = postpad (h1, max_name_length + 1, " ");
  h2 = postpad (h2, max_version_length, " ");;

  ## Print a header
  header = sprintf("%s | %s | %s\n", h1, h2, h3);
  printf (header);
  tmp = sprintf (repmat ("-", 1, length(header)-1));
  tmp(length(h1)+2) = "+";
  tmp(length(h1)+length(h2)+5) = "+";
  printf ("%s\n", tmp);

  ## Print the packages
  format = sprintf ("%%%ds %%1s| %%%ds | %%s\n", max_name_length,
		    max_version_length);
  [dummy, idx] = sort (names);
  for i = 1:num_packages
    cur_name = installed_packages{idx(i)}.name;
    cur_version = installed_packages{idx(i)}.version;
    cur_dir = installed_packages{idx(i)}.dir;
    if (installed_packages{idx(i)}.loaded)
      cur_loaded = "*";
    else
      cur_loaded = " ";
    endif
    printf (format, cur_name, cur_loaded, cur_version, cur_dir);
  endfor
endfunction

function load_packages (files, handle_deps, local_list, global_list)
  installed_packages = installed_packages (local_list, global_list);
  num_packages = length (installed_packages);

  ## Read package names and installdirs into a more convenient format
  pnames = pdirs = cell (1, num_packages);
  for i = 1:num_packages
    pnames{i} = installed_packages{i}.name;
    pdirs{i} = installed_packages{i}.dir;
  endfor

  ## load all
  if (length (files) == 1 && strcmp (files{1}, "all"))
    idx = [1:length(installed_packages)];
  ## load auto
  elseif (length (files) == 1 && strcmp (files{1}, "auto")) 
    idx = [];
    for i = 1:length (installed_packages)
      if (exist (fullfile (pdirs{i}, "packinfo", ".autoload"), "file"))
	idx (end + 1) = i;
      endif
    endfor
  ## load package_name1 ...
  else
    idx = [];
    for i = 1:length (files)
      idx2 = find (strcmp (pnames, files{i}));
      if (! any (idx2))
	  error ("package %s is not installed", files{i});
      endif
      idx (end + 1) = idx2;
    endfor
  endif

  ## Load the packages, but take care of the ordering of dependencies
  load_packages_and_dependencies (idx, handle_deps, installed_packages);
endfunction

function unload_packages (files, handle_deps, local_list, global_list)
  installed_packages = installed_packages (local_list, global_list);
  num_packages = length (installed_packages);

  ## Read package names and installdirs into a more convenient format
  pnames = pdirs = cell (1, num_packages);
  for i = 1:num_packages
    pnames{i} = installed_packages{i}.name;
    pdirs{i} = installed_packages{i}.dir;
    pdeps{i} = installed_packages{i}.depends;
  endfor

  ## Get the current octave path
  p = split_by (path(), pathsep ());

  ## unload all
  if (length (files) == 1 && strcmp (files{1}, "all"))
      dirs = pdirs;
  ## unload package_name1 ...
  else
    dirs = {};
    for i = 1:length (files)
      idx = strcmp (pnames, files{i});
      if (! any (idx))
	error ("package %s is not installed", files{i});
      endif
	dirs{end+1} = pdirs{idx};
      endfor
  endif

  ## Check for architecture dependent directories
  arch = getarch();
  archdirs = {};
  for i = 1:length (dirs)
    tmpdir = fullfile (dirs{i}, arch);
    if (exist (tmpdir, "dir"))
      archdirs{end + 1} = dirs{i};
      archdirs{end + 1} = tmpdir;
    endif
  endfor
  if (length (archdirs) > 0)
    dirs = archdirs;
  endif

  ## Unload the packages
  for i = 1:length (dirs)
    d = dirs{i};
    idx = strcmp (p, d);
    if (any (idx))
      rmpath (d);
      ## XXX: We should also check if we need to remove items from EXEC_PATH
    endif
  endfor
endfunction

function [status_out, msg_out] = rm_rf (dir)
  crr = confirm_recursive_rmdir ();
  unwind_protect
    confirm_recursive_rmdir (false);
    [status, msg] = rmdir (dir, "s");
  unwind_protect_cleanup
    confirm_recursive_rmdir (crr);
  end_unwind_protect
  if (nargout > 0)
    status_out = status;
  endif
  if (nargout > 1)
    msg_out = msg;
  endif
endfunction

function emp = dirempty (nm, ign)
  if (nargin < 2)
    ign = {".", ".."};
  else
    ign = [{".", ".."}, ign];
  endif
  l = dir (nm);
  for i = 1:length (l)
    found = false;
    for j = 1:length (ign)
      if (strcmp (l(i).name, ign{j}))
        found = true;
        break;
      endif
    endfor
    if (! found)
      emp = false;
      return
    endif
  endfor
  emp = true;
endfunction

function arch = getarch ()
  persistent _arch = strcat (octave_config_info("canonical_host_type"), ...
			     "-", octave_config_info("api_version"));
  arch = _arch;
endfunction

function [status, output] = shell (cmd)
  persistent have_sh;

  cmd = strrep (cmd, "\\", "/");
  if (ispc () && ! isunix ())
    if (isempty(have_sh))
      if (system("sh.exe -c \"exit\""))
        have_sh = false;
      else
        have_sh = true;
      endif
    endif
    if (have_sh)
      [status, output] = system (strcat ("sh.exe -c \"", cmd, "\""));
    else
      error ("Can not find the command shell")
    endif
  else
    [status, output] = system (cmd);
  endif
endfunction

function newdesc = save_order (desc)
  newdesc = {};
  for i = 1 : length(desc)
    deps = desc{i}.depends;
    if (isempty (deps) || (length (deps) == 1 && 
			  strcmp(deps{1}.package, "octave")))
      newdesc {end + 1} = desc{i};
    else
      tmpdesc = {};
      for k = 1 : length (deps)
        for j = 1 : length (desc)
          if (strcmp (desc{j}.name, deps{k}.package))
            tmpdesc {end + 1} = desc{j};
	    break;
          endif
        endfor
      endfor
      if (! isempty (tmpdesc))					     
        newdesc = {newdesc{:}, save_order(tmpdesc){:}, desc{i}};
      else
        newdesc {end + 1} = desc{i};
      endif
    endif
  endfor
  ## Eliminate the duplicates
  idx = [];
  for i = 1 : length (newdesc)
    for j = (i + 1) : length (newdesc)
      if (strcmp (newdesc{i}.name, newdesc{j}.name))
        idx (end + 1) = j;
      endif
    endfor
  endfor
  newdesc(idx) = [];
endfunction

function load_packages_and_dependencies (idx, handle_deps, installed_packages)
  idx = load_package_dirs (idx, [], handle_deps, installed_packages);
  arch = getarch ();
  dirs = {};
  execpath = EXEC_PATH ();
  for i = idx;
    ndir = installed_packages{i}.dir;
    dirs {end + 1} = ndir;
    if (exist (fullfile (dirs{end}, "bin"), "dir"))
      execpath = strcat (fullfile(dirs{end}, "bin"), ":", execpath);
    endif
    tmpdir = fullfile (ndir, arch);
    if (exist (tmpdir, "dir"))
      dirs{end + 1} = tmpdir;
    endif
  endfor

  ## Load the packages
  if (length (dirs) > 0)
    addpath (dirs{:});
  endif

  ## Add the binaries to exec_path
  if (! strcmp (EXEC_PATH, execpath))
    EXEC_PATH (execpath);
  endif
endfunction

function idx = load_package_dirs (lidx, idx, handle_deps, installed_packages)
  for i = lidx
    if (installed_packages{i}.loaded)
      continue;
    else
      if (handle_deps)
        deps = installed_packages{i}.depends;
        if ((length (deps) > 1) || (length (deps) == 1 && 
	  		  ! strcmp(deps{1}.package, "octave")))
          tmplidx = [];
          for k = 1 : length (deps)
            for j = 1 : length (installed_packages)
              if (strcmp (installed_packages{j}.name, deps{k}.package))
                tmplidx (end + 1) = j;
	        break;
              endif
            endfor
          endfor
          idx = load_package_dirs (tmplidx, idx, handle_deps, 
				 installed_packages);
        endif
      endif
      if (isempty (find(idx == i)))
        idx (end + 1) = i;
      endif
    endif
  endfor
endfunction
