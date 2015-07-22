libinterp_EXTRA_DIST =

libinterp_CLEANFILES =
libinterp_DISTCLEANFILES =
libinterp_MAINTAINERCLEANFILES =

## Search local directories before those specified by the user.
libinterp_liboctinterp_la_CPPFLAGS = \
  @OCTINTERP_DLL_DEFS@ \
  -I$(top_builddir)/liboctave -I$(top_srcdir)/liboctave \
  -I$(top_srcdir)/liboctave/cruft/misc \
  -I$(top_srcdir)/liboctave/array \
  -I$(top_builddir)/liboctave/numeric -I$(top_srcdir)/liboctave/numeric \
  -I$(top_builddir)/liboctave/operators -I$(top_srcdir)/liboctave/operators \
  -I$(top_srcdir)/liboctave/system \
  -I$(top_srcdir)/liboctave/util \
  -I$(srcdir)/libinterp/octave-value \
  -Ilibinterp -I$(srcdir)/libinterp \
  -I$(srcdir)/libinterp/operators \
  -Ilibinterp/parse-tree -I$(srcdir)/libinterp/parse-tree \
  -Ilibinterp/corefcn -I$(srcdir)/libinterp/corefcn \
  -I$(top_builddir)/libgnu -I$(top_srcdir)/libgnu \
  $(HDF5_CPPFLAGS)

libinterp_liboctinterp_la_CFLAGS = $(AM_CFLAGS) $(WARN_CFLAGS)

libinterp_liboctinterp_la_CXXFLAGS = $(AM_CFLAGS) $(WARN_CXXFLAGS)

octlib_LTLIBRARIES += libinterp/liboctinterp.la

## This is the subset of $(BUILT_SOURCES) that may be included by source
## files that are preprocessed to make $(DEF_FILES).  This ensures that
## files in $(BUILT_SOURCES) are built in the right dependency order.
GENERATED_MAKE_BUILTINS_INCS = \
  libinterp/corefcn/defaults.h \
  libinterp/corefcn/graphics-props.cc \
  libinterp/corefcn/graphics.h \
  libinterp/corefcn/mxarray.h \
  libinterp/oct-conf-features.h \
  libinterp/oct-conf.h \
  libinterp/parse-tree/oct-gperf.h \
  libinterp/parse-tree/oct-parse.h \
  libinterp/version.h

BUILT_SOURCES += \
  $(GENERATED_MAKE_BUILTINS_INCS) \
  libinterp/builtin-defun-decls.h \
  libinterp/builtins.cc \
  libinterp/corefcn/oct-errno.cc \
  libinterp/corefcn/oct-tex-lexer.cc \
  libinterp/corefcn/oct-tex-parser.cc \
  libinterp/corefcn/oct-tex-symbols.cc \
  libinterp/operators/ops.cc \
  libinterp/parse-tree/lex.cc \
  libinterp/parse-tree/oct-parse.cc

ULT_PARSER_SRC := \
  libinterp/corefcn/oct-tex-lexer.in.ll \
  libinterp/corefcn/oct-tex-parser.in.yy \
  libinterp/parse-tree/oct-parse.in.yy

GENERATED_PARSER_FILES := \
  libinterp/corefcn/oct-tex-lexer.ll \
  libinterp/corefcn/oct-tex-parser.yy \
  libinterp/parse-tree/oct-parse.yy

LIBINTERP_BUILT_DISTFILES = \
  $(GENERATED_PARSER_FILES) \
  libinterp/corefcn/oct-tex-parser.h \
  libinterp/corefcn/oct-tex-symbols.cc \
  libinterp/parse-tree/oct-gperf.h \
  libinterp/parse-tree/oct-parse.h

## Files that are created during build process and installed,
## BUT not distributed in tarball.
LIBINTERP_BUILT_NODISTFILES = \
  libinterp/corefcn/mxarray.h \
  libinterp/corefcn/oct-errno.cc \
  libinterp/corefcn/defaults.h \
  libinterp/corefcn/graphics.h \
  libinterp/builtin-defun-decls.h \
  libinterp/operators/ops.cc \
  libinterp/oct-conf.h \
  libinterp/oct-conf-features.h \
  libinterp/version.h \
  $(OPT_HANDLERS) \
  $(ALL_DEF_FILES) \
  libinterp/builtins.cc

libinterp_EXTRA_DIST += \
  libinterp/DOCSTRINGS \
  libinterp/config-features.sh \
  libinterp/find-defun-files.sh \
  libinterp/gendoc.pl \
  libinterp/genprops.awk \
  libinterp/mk-errno-list \
  libinterp/mk-pkg-add \
  libinterp/mkbuiltins \
  libinterp/mkdefs \
  libinterp/mkops \
  libinterp/oct-conf.in.h \
  libinterp/version.in.h \
  $(LIBINTERP_BUILT_DISTFILES)

octinclude_HEADERS += \
  libinterp/corefcn/graphics-props.cc \
  libinterp/parse-tree/oct-gperf.h \
  libinterp/builtins.h \
  libinterp/builtin-defun-decls.h \
  libinterp/octave.h \
  libinterp/options-usage.h \
  $(OCTAVE_VALUE_INC) \
  $(PARSE_TREE_INC) \
  $(PARSER_INC) \
  $(LIBINTERP_OPERATORS_INC) \
  $(COREFCN_INC)

nodist_octinclude_HEADERS += \
  libinterp/corefcn/mxarray.h \
  libinterp/corefcn/defaults.h \
  libinterp/corefcn/graphics.h \
  libinterp/oct-conf.h \
  libinterp/version.h

DIST_SRC += \
  libinterp/octave.cc \
  libinterp/version.cc \
  $(OCTAVE_VALUE_SRC) \
  $(PARSE_TREE_SRC) \
  $(PARSER_SRC) \
  $(COREFCN_SRC) \
  $(TEX_PARSER_SRC)

include libinterp/parse-tree/module.mk
include libinterp/octave-value/module.mk
include libinterp/operators/module.mk
include libinterp/template-inst/module.mk
include libinterp/corefcn/module.mk
include libinterp/dldfcn/module.mk

$(srcdir)/libinterp/dldfcn/module.mk: $(srcdir)/libinterp/dldfcn/config-module.sh $(srcdir)/libinterp/dldfcn/config-module.awk $(srcdir)/libinterp/dldfcn/module-files
	$(srcdir)/libinterp/dldfcn/config-module.sh $(top_srcdir)

if AMCOND_ENABLE_DYNAMIC_LINKING
  OCT_FILES = $(DLDFCN_LIBS:.la=.oct)
  DLD_LIBOCTINTERP_LIBADD = libinterp/liboctinterp.la
else
  OCT_FILES =
  DLD_LIBOCTINTERP_LIBADD =
endif

libinterp_liboctinterp_la_SOURCES = \
  libinterp/octave.cc \
  libinterp/version.cc \
  $(LIBINTERP_OPERATORS_SRC) \
  $(TEMPLATE_INST_SRC)

nodist_libinterp_liboctinterp_la_SOURCES = \
  libinterp/corefcn/mxarray.h \
  libinterp/corefcn/oct-errno.cc \
  libinterp/corefcn/defaults.h \
  libinterp/corefcn/graphics.h \
  libinterp/operators/ops.cc \
  libinterp/builtin-defun-decls.h \
  libinterp/builtins.cc \
  libinterp/oct-conf.h \
  libinterp/oct-conf-features.h \
  libinterp/version.h

libinterp_liboctinterp_la_LIBADD = \
  libinterp/octave-value/liboctave-value.la \
  libinterp/parse-tree/libparse-tree.la \
  libinterp/parse-tree/libparser.la \
  libinterp/corefcn/libcorefcn.la \
  libinterp/corefcn/libtex_parser.la \
  $(top_builddir)/liboctave/liboctave.la \
  $(LIBOCTINTERP_LINK_DEPS)

# Increment these as needed and according to the rules in the libtool manual:
libinterp_liboctinterp_current = 3
libinterp_liboctinterp_revision = 0
libinterp_liboctinterp_age = 0

libinterp_liboctinterp_version_info = $(libinterp_liboctinterp_current):$(libinterp_liboctinterp_revision):$(libinterp_liboctinterp_age)

libinterp_liboctinterp_la_LDFLAGS = \
  -version-info $(libinterp_liboctinterp_version_info) \
  $(NO_UNDEFINED_LDFLAG) \
  -bindir $(bindir) \
  $(LIBOCTINTERP_LINK_OPTS)

## Section for defining and creating DEF_FILES

ULT_DIST_SRC := \
  $(filter-out $(GENERATED_PARSER_FILES), $(DIST_SRC)) $(ULT_PARSER_SRC)

SRC_DEF_FILES := $(shell $(srcdir)/libinterp/find-defun-files.sh "$(srcdir)" $(ULT_DIST_SRC))

DLDFCN_DEF_FILES = $(DLDFCN_SRC:.cc=.df)

## builtins.cc depends on $(DEF_FILES), so DEF_FILES should only include
## .df files that correspond to sources included in liboctave.
if AMCOND_ENABLE_DYNAMIC_LINKING
  DEF_FILES = $(SRC_DEF_FILES)
else
  DEF_FILES = $(SRC_DEF_FILES) $(DLDFCN_DEF_FILES)
endif

ALL_DEF_FILES = $(SRC_DEF_FILES) $(DLDFCN_DEF_FILES)

$(SRC_DEF_FILES): libinterp/mkdefs Makefile

$(DEF_FILES): $(OPT_HANDLERS) $(LIBOCTAVE_OPT_INC)

## FIXME: The following two variables are deprecated and should be removed
##        in Octave version 3.12.
DLL_CDEFS = @OCTINTERP_DLL_DEFS@
DLL_CXXDEFS = @OCTINTERP_DLL_DEFS@

## Rule to build a DEF file from a .cc file
## See also module.mk files for overrides when speciall CPPFLAGS are needed.
## FIXME: Shouldn't the build stop if CPP fails here?  Yes (10/31/2013)
%.df: %.cc $(GENERATED_MAKE_BUILTINS_INCS)
	$(AM_V_GEN)rm -f $@-t $@-t1 $@ && \
	$(CXXCPP) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	  $(libinterp_liboctinterp_la_CPPFLAGS) $(LLVM_CPPFLAGS) $(CPPFLAGS) \
	  $(libinterp_liboctinterp_la_CXXFLAGS) \
	  -DMAKE_BUILTINS $< > $@-t1 && \
	$(srcdir)/libinterp/mkdefs $(srcdir)/libinterp $< < $@-t1 > $@-t && \
	rm -f $@-t1 && \
	mv $@-t $@

## Rules to build test files

LIBINTERP_TST_FILES_SRC := $(shell $(top_srcdir)/build-aux/find-files-with-tests.sh "$(srcdir)" $(ULT_DIST_SRC) $(DLDFCN_SRC))

LIBINTERP_TST_FILES := $(addsuffix -tst,$(LIBINTERP_TST_FILES_SRC))

libinterptestsdir := $(octtestsdir)

nobase_libinterptests_DATA = $(LIBINTERP_TST_FILES)

## Override Automake's rule that forces a .hh extension on us even
## though we don't want it.  It would be super awesome if automake
## would allow users to choose the header file extension.
.yy.cc:
	$(AM_V_YACC)$(am__skipyacc) $(SHELL) $(YLWRAP) $< y.tab.c $@ y.tab.h $*.h y.output $*.output -- $(YACCCOMPILE)

## Special rules:
## Mostly for sources which must be built before rest of compilation.

## oct-conf.h must depend on Makefile.
## Calling configure may change default/config values.
## However, calling configure will also regenerate the Makefiles from
## Makefile.am and trigger the rules below.
libinterp/oct-conf.h: libinterp/oct-conf.in.h Makefile
	$(AM_V_GEN)$(do_subst_config_vals)

libinterp/oct-conf-features.h: $(top_builddir)/config.h libinterp/config-features.sh
	$(AM_V_GEN)rm -f $@-t && \
	$(srcdir)/libinterp/config-features.sh $< > $@-t && \
	$(simple_move_if_change_rule)

libinterp/version.h: libinterp/version.in.h Makefile
	$(AM_V_GEN)rm -f $@-t && \
	$(SED) < $< \
	  -e "s|%NO_EDIT_WARNING%|DO NOT EDIT!  Generated automatically from $(<F) by Make.|" \
	  -e "s|%OCTAVE_API_VERSION%|\"${OCTAVE_API_VERSION}\"|" \
	  -e "s|%OCTAVE_COPYRIGHT%|\"${OCTAVE_COPYRIGHT}\"|" \
          -e "s|%OCTAVE_MAJOR_VERSION%|${OCTAVE_MAJOR_VERSION}|" \
          -e "s|%OCTAVE_MINOR_VERSION%|${OCTAVE_MINOR_VERSION}|" \
          -e "s|%OCTAVE_PATCH_VERSION%|${OCTAVE_PATCH_VERSION}|" \
	  -e "s|%OCTAVE_RELEASE_DATE%|\"${OCTAVE_RELEASE_DATE}\"|" \
	  -e "s|%OCTAVE_VERSION%|\"${OCTAVE_VERSION}\"|" > $@-t && \
	$(simple_move_if_change_rule)

libinterp/builtins.cc: $(DEF_FILES) libinterp/mkbuiltins
	$(AM_V_GEN)rm -f $@-t && \
	$(srcdir)/libinterp/mkbuiltins --source $(DEF_FILES) > $@-t && \
	$(simple_move_if_change_rule)

libinterp/builtin-defun-decls.h: $(SRC_DEF_FILES) libinterp/mkbuiltins
	$(AM_V_GEN)rm -f $@-t && \
	$(srcdir)/libinterp/mkbuiltins --header $(SRC_DEF_FILES) > $@-t && \
	$(simple_move_if_change_rule)

if AMCOND_ENABLE_DYNAMIC_LINKING
DLDFCN_PKG_ADD_FILE = libinterp/dldfcn/PKG_ADD

libinterp/dldfcn/PKG_ADD: $(DLDFCN_DEF_FILES) libinterp/mk-pkg-add
	$(AM_V_GEN)rm -f $@-t && \
	$(srcdir)/libinterp/mk-pkg-add $(DLDFCN_DEF_FILES) > $@-t && \
	$(simple_move_if_change_rule)
endif

if AMCOND_BUILD_DOCS

libinterp/.DOCSTRINGS: $(ALL_DEF_FILES) libinterp/gendoc.pl
	$(AM_V_GEN)rm -f $@-t $@ && \
	if [ "x$(srcdir)" != "x." ] && [ -f $(srcdir)/libinterp/DOCSTRINGS ] && [ ! -f DOCSTRINGS ]; then \
		cp $(srcdir)/libinterp/DOCSTRINGS libinterp/DOCSTRINGS; \
		touch -r $(srcdir)/libinterp/DOCSTRINGS libinterp/DOCSTRINGS; \
	fi && \
	$(PERL) $(srcdir)/libinterp/gendoc.pl $(ALL_DEF_FILES) > $@-t && \
	mv $@-t $@ && \
	$(top_srcdir)/build-aux/move-if-change $@ libinterp/DOCSTRINGS && \
	touch $@

OCTAVE_INTERPRETER_TARGETS += \
  libinterp/.DOCSTRINGS

endif

OCTAVE_INTERPRETER_TARGETS += \
  $(OCT_FILES) \
  $(DLDFCN_PKG_ADD_FILE)

if AMCOND_BUILD_DOCS
install-data-hook: install-oct install-built-in-docstrings
else
install-data-hook: install-oct uninstall-built-in-docstrings
endif

uninstall-local: uninstall-oct uninstall-built-in-docstrings

if AMCOND_ENABLE_DYNAMIC_LINKING
install-oct:
	$(MKDIR_P) $(DESTDIR)$(octfiledir)
	if [ -n "`cat $(DLDFCN_PKG_ADD_FILE)`" ]; then \
	  $(INSTALL_DATA) $(DLDFCN_PKG_ADD_FILE) $(DESTDIR)$(octfiledir)/PKG_ADD; \
	fi
	cd $(DESTDIR)$(octlibdir) && \
	for ltlib in $(DLDFCN_LIBS); do \
	  f=`echo $$ltlib | $(SED) 's,.*/,,'`; \
	  dl=`$(SED) -n -e "s/dlname='\([^']*\)'/\1/p" < $$f`; \
	  if [ -n "$$dl" ]; then \
	    $(INSTALL_PROGRAM) $$dl $(DESTDIR)$(octfiledir)/`echo $$f | $(SED) 's,^lib,,; s,\.la$$,.oct,'`; \
	  else \
	    echo "error: dlname is empty in $$ltlib!"; \
	    exit 1; \
	  fi; \
	  lnames=`$(SED) -n -e "s/library_names='\([^']*\)'/\1/p" < $$f`; \
	  if [ -n "$$lnames" ]; then \
	    rm -f $$f $$lnames $$dl; \
	  fi \
	done

uninstall-oct:
	for f in $(notdir $(OCT_FILES)); do \
	  rm -f $(DESTDIR)$(octfiledir)/$$f; \
	done
	rm -f $(DESTDIR)$(octfiledir)/PKG_ADD
endif
.PHONY: install-oct uninstall-oct

if AMCOND_BUILD_DOCS
install-built-in-docstrings:
	$(MKDIR_P) $(DESTDIR)$(octetcdir)
	$(INSTALL_DATA) libinterp/DOCSTRINGS $(DESTDIR)$(octetcdir)/built-in-docstrings

uninstall-built-in-docstrings:
	rm -f $(DESTDIR)$(octetcdir)/built-in-docstrings
endif
.PHONY: install-built-in-docstrings uninstall-built-in-docstrings

EXTRA_DIST += $(libinterp_EXTRA_DIST)

libinterp_CLEANFILES += \
  $(DLDFCN_PKG_ADD_FILE) \
  libinterp/corefcn/graphics-props.cc \
  libinterp/corefcn/oct-tex-parser.output \
  libinterp/parse-tree/oct-parse.output

libinterp_DISTCLEANFILES += \
  libinterp/.DOCSTRINGS \
  libinterp/DOCSTRINGS \
  $(LIBINTERP_BUILT_NODISTFILES) \
  $(OCT_FILES) \
  $(LIBINTERP_TST_FILES)

libinterp_MAINTAINERCLEANFILES += \
  $(LIBINTERP_BUILT_DISTFILES)

BUILT_DISTFILES += $(LIBINTERP_BUILT_DISTFILES)
BUILT_NODISTFILES += $(LIBINTERP_BUILT_NODISTFILES)

CLEANFILES += $(libinterp_CLEANFILES)
DISTCLEANFILES += $(libinterp_DISTCLEANFILES)
MAINTAINERCLEANFILES += $(libinterp_MAINTAINERCLEANFILES)

libinterp-clean:
	rm -f $(libinterp_CLEANFILES)

libinterp-distclean: libinterp-clean
	rm -f $(libinterp_DISTCLEANFILES)

libinterp-maintainer-clean: libinterp-distclean
	rm -f $(libinterp_MAINTAINERCLEANFILES)
