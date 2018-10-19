/*

Copyright (C) 2002-2018 John W. Eaton

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#if ! defined (octave_octave_h)
#define octave_octave_h 1

#include "octave-config.h"

#if defined  (__cplusplus)

#include <list>
#include <string>

#include "str-vec.h"

namespace octave
{
  // Command line arguments.  See also options-usage.h.

  class OCTINTERP_API cmdline_options
  {
  public:

    cmdline_options (void);

    cmdline_options (int argc, char **argv);

    cmdline_options (const cmdline_options&) = default;

    cmdline_options& operator = (const cmdline_options&) = default;

    int sys_argc (void) const { return m_all_args.numel (); }
    char **sys_argv (void) const { return m_all_args.c_str_vec (); }

    bool debug_jit (void) const { return m_debug_jit; }
    bool echo_commands (void) const { return m_echo_commands; }

    OCTAVE_DEPRECATED (4.4, "use 'gui' instead")
    bool force_gui (void) const { return m_gui; }

    bool forced_interactive (void) const { return m_forced_interactive; }
    bool forced_line_editing (void) const { return m_forced_line_editing; }
    bool gui (void) const { return m_gui; }
    bool inhibit_startup_message (void) const { return m_inhibit_startup_message; }
    bool jit_compiler (void) const { return m_jit_compiler; }
    bool line_editing (void) const { return m_line_editing; }

    OCTAVE_DEPRECATED (4.4, "use '! gui' instead")
    bool no_gui (void) const { return ! gui (); }

    bool no_window_system (void) const { return m_no_window_system; }
    bool persist (void) const { return m_persist; }
    bool read_history_file (void) const { return m_read_history_file; }
    bool read_init_files (void) const { return m_read_init_files; }
    bool read_site_files (void) const { return m_read_site_files; }
    bool set_initial_path (void) const { return m_set_initial_path; }
    bool traditional (void) const { return m_traditional; }
    bool verbose_flag (void) const { return m_verbose_flag; }
    std::string code_to_eval (void) const { return m_code_to_eval; }
    std::list<std::string> command_line_path (void) const { return m_command_line_path; }
    std::string docstrings_file (void) const { return m_docstrings_file; }
    std::string doc_cache_file (void) const { return m_doc_cache_file; }
    std::string exec_path (void) const { return m_exec_path; }
    std::string image_path (void) const { return m_image_path; }
    std::string info_file (void) const { return m_info_file; }
    std::string info_program (void) const { return m_info_program; }
    std::string texi_macros_file (void) const {return m_texi_macros_file; }
    string_vector all_args (void) const { return m_all_args; }
    string_vector remaining_args (void) const { return m_remaining_args; }

    void debug_jit (bool arg) { m_debug_jit = arg; }
    void echo_commands (bool arg) { m_echo_commands = arg; }

    OCTAVE_DEPRECATED (4.4, "use 'gui' instead")
    void force_gui (bool arg) { m_gui = arg; }

    void forced_line_editing (bool arg) { m_forced_line_editing = arg; }
    void forced_interactive (bool arg) { m_forced_interactive = arg; }
    void gui (bool arg) { m_gui = arg; }
    void inhibit_startup_message (bool arg) { m_inhibit_startup_message = arg; }
    void jit_compiler (bool arg) { m_jit_compiler = arg; }
    void line_editing (bool arg) { m_line_editing = arg; }

    OCTAVE_DEPRECATED (4.4, "this has been removed and is the default now")
    void no_gui (bool) { return; }

    void no_window_system (bool arg) { m_no_window_system = arg; }
    void persist (bool arg) { m_persist = arg; }
    void read_history_file (bool arg) { m_read_history_file = arg; }
    void read_init_files (bool arg) { m_read_init_files = arg; }
    void read_site_files (bool arg) { m_read_site_files = arg; }
    void set_initial_path (bool arg) { m_set_initial_path = arg; }
    void traditional (bool arg) { m_traditional = arg; }
    void verbose_flag (bool arg) { m_verbose_flag = arg; }
    void code_to_eval (const std::string& arg) { m_code_to_eval = arg; }
    void command_line_path (const std::list<std::string>& arg) { m_command_line_path = arg; }
    void docstrings_file (const std::string& arg) { m_docstrings_file = arg; }
    void doc_cache_file (const std::string& arg) { m_doc_cache_file = arg; }
    void exec_path (const std::string& arg) { m_exec_path = arg; }
    void image_path (const std::string& arg) { m_image_path = arg; }
    void info_file (const std::string& arg) { m_info_file = arg; }
    void info_program (const std::string& arg) { m_info_program = arg; }
    void texi_macros_file (const std::string& arg) { m_texi_macros_file = arg; }
    void all_args (const string_vector& arg) { m_all_args = arg; }
    void remaining_args (const string_vector& arg) { m_remaining_args = arg; }

  private:

    // TRUE means enable debug tracing for the JIT compiler.
    // (--debug-jit)
    bool m_debug_jit = false;

    // If TRUE, echo commands as they are read and executed.
    // (--echo-commands, -x)
    bool m_echo_commands = false;

    // If TRUE, start the GUI.
    // (--gui) and (--force-gui) for backwards compatibility
    bool m_gui = false;

    // TRUE means the user forced this shell to be interactive.
    // (--interactive, -i)
    bool m_forced_interactive = false;

    // If TRUE, force readline command line editing.
    // (--line-editing)
    bool m_forced_line_editing = false;

    // TRUE means we don't print the usual startup message.
    // (--quiet; --silent; -q)
    bool m_inhibit_startup_message = false;

    // TRUE means enable the JIT compiler.
    // (--jit-compiler)
    bool m_jit_compiler = false;

    // TRUE means we are using readline.
    // (--no-line-editing)
    bool m_line_editing = true;

    // If TRUE, ignore the window system even if it is available.
    // (--no-window-system, -W)
    bool m_no_window_system = false;

    // If TRUE, don't exit after evaluating code given by --eval option.
    // (--persist)
    bool m_persist = false;

    // If TRUE, initialize history list from saved history file.
    // (--no-history; -H)
    bool m_read_history_file = true;

    // TRUE means we read ~/.octaverc and ./.octaverc.
    // (--norc; --no-init-file; -f)
    bool m_read_init_files = true;

    // TRUE means we read the site-wide octaverc files.
    // (--norc; --no-site-file; -f)
    bool m_read_site_files = true;

    // TRUE means we set the initial path to configured defaults.
    // (--no-init-path)
    bool m_set_initial_path = true;

    // If TRUE use traditional (maximally MATLAB compatible) settings
    // (--traditional)
    bool m_traditional = false;

    // If TRUE, print verbose info in some cases.
    // (--verbose; -V)
    bool m_verbose_flag = false;

    // The code to evaluate at startup
    // (--eval CODE)
    std::string m_code_to_eval;

    // The value of "path" specified on the command line.
    // (--path; -p)
    std::list<std::string> m_command_line_path;

    // The value for "built_in_docstrings_file" specified on the
    // command line.
    // (--built-in-docstrings-file)
    std::string m_docstrings_file;

    // The value for "doc_cache_file" specified on the command line.
    // (--doc-cache-file)
    std::string m_doc_cache_file;

    // The value for "EXEC_PATH" specified on the command line.
    // (--exec-path)
    std::string m_exec_path;

    // The value for "IMAGE_PATH" specified on the command line.
    // (--image-path)
    std::string m_image_path;

    // The value for "info_file" specified on the command line.
    // (--info-file)
    std::string m_info_file;

    // The value for "info_program" specified on the command line.
    // (--info-program)
    std::string m_info_program;

    // The value for "texi_macos_file" specified on the command line.
    // (--texi-macros-file)
    std::string m_texi_macros_file;

    // All arguments passed to the argc, argv constructor.
    string_vector m_all_args;

    // Arguments remaining after parsing.
    string_vector m_remaining_args;
  };

  // The application object contains a pointer to the current
  // interpreter and the interpreter contains a pointer back to the
  // application context so we need a forward declaration for one (or
  // both) of them...

  class interpreter;

  // Base class for an Octave application.

  class OCTINTERP_API application
  {
  public:

    application (const cmdline_options& opts = cmdline_options ());

    application (int argc, char **argv);

    // No copying, at least not yet...

    application (const application&) = delete;

    application& operator = (const application&) = delete;

    virtual ~application (void);

    int sys_argc (void) const { return m_options.sys_argc (); }
    char **sys_argv (void) const { return m_options.sys_argv (); }

    void set_program_names (const std::string& pname);

    void intern_argv (const string_vector& args);

    cmdline_options options (void) const { return m_options; }

    bool have_eval_option_code (void) const { return m_have_eval_option_code; }

    bool have_script_file (void) const { return m_have_script_file; }

    bool is_octave_program (void) const { return m_is_octave_program; }

    bool interpreter_initialized (void);

    virtual interpreter& create_interpreter (void);

    virtual void initialize_interpreter (void);

    virtual int execute_interpreter (void);

    virtual void delete_interpreter (void);

    virtual int execute (void) = 0;

    virtual bool gui_running (void) const { return false; }
    virtual void gui_running (bool) { }

    void program_invocation_name (const std::string& nm) { m_program_invocation_name = nm; }

    void program_name (const std::string& nm) { m_program_name = nm; }

    void forced_interactive (bool arg) { m_options.forced_interactive (arg); }

    void interactive (bool arg);

    static application * app (void) { return instance; }

    static std::string program_invocation_name (void)
    {
      return instance ? instance->m_program_invocation_name : "";
    }

    static std::string program_name (void)
    {
      return instance ? instance->m_program_name : "";
    }

    static string_vector argv (void)
    {
      return instance ? instance->m_argv : string_vector ();
    }

    static bool is_gui_running (void)
    {
      return instance ? instance->gui_running () : false;
    }

    // Convenience functions.

    static bool forced_interactive (void);
    static bool interactive (void);

  private:

    // The application instance;  There should be only one.
    static application *instance;

    void init (void);

  protected:

    // The name used to invoke Octave.
    std::string m_program_invocation_name;

    // The last component of octave_program_invocation_name.
    std::string m_program_name;

    // The current argument vector (may change if we are running a
    // script with --persist because after the script is done, the
    // arguments revert to the full list given to the octave
    // interpreter at startup.
    string_vector m_argv;

    cmdline_options m_options;

    // TRUE means we have --eval CODE
    bool m_have_eval_option_code = false;

    // TRUE if there is a command line argument that looks like the
    // name of a file to execute.
    bool m_have_script_file = false;

    // TRUE if this is a program and no interpreter and interaction is
    // needed.  For example, an octave program with shebang line, or code
    // from eval without persist.
    bool m_is_octave_program = false;

    // If TRUE, the GUI should be started.
    bool m_gui_running = false;

    interpreter *m_interpreter = nullptr;
  };

  class OCTINTERP_API cli_application : public application
  {
  public:

    cli_application (const cmdline_options& opts = cmdline_options ())
      : application (opts)
    { }

    cli_application (int argc, char **argv)
      : application (argc, argv)
    { }

    // No copying, at least not yet...

    cli_application (const cli_application&) = delete;

    cli_application& operator = (const cli_application&) = delete;

    ~cli_application (void) = default;

    int execute (void);
  };
}

#endif

#if defined  (__cplusplus)
extern "C" {
#endif

OCTAVE_DEPRECATED (4.4, "see the Octave documentation for other options")
extern OCTINTERP_API int octave_main (int argc, char **argv, int embedded);

#if defined  (__cplusplus)
}
#endif

#endif
