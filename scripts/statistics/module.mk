FCN_FILE_DIRS += scripts/statistics

%canon_reldir%_FCN_FILES = \
  %reldir%/center.m \
  %reldir%/corrcoef.m \
  %reldir%/corr.m \
  %reldir%/cov.m \
  %reldir%/discrete_cdf.m \
  %reldir%/discrete_inv.m \
  %reldir%/discrete_pdf.m \
  %reldir%/discrete_rnd.m \
  %reldir%/empirical_cdf.m \
  %reldir%/empirical_inv.m \
  %reldir%/empirical_pdf.m \
  %reldir%/empirical_rnd.m \
  %reldir%/histc.m \
  %reldir%/iqr.m \
  %reldir%/kendall.m \
  %reldir%/kurtosis.m \
  %reldir%/mean.m \
  %reldir%/meansq.m \
  %reldir%/median.m \
  %reldir%/mode.m \
  %reldir%/moment.m \
  %reldir%/prctile.m \
  %reldir%/quantile.m \
  %reldir%/range.m \
  %reldir%/ranks.m \
  %reldir%/run_count.m \
  %reldir%/runlength.m \
  %reldir%/skewness.m \
  %reldir%/spearman.m \
  %reldir%/statistics.m \
  %reldir%/std.m \
  %reldir%/var.m \
  %reldir%/zscore.m

%canon_reldir%dir = $(fcnfiledir)/statistics

%canon_reldir%_DATA = $(%canon_reldir%_FCN_FILES)

FCN_FILES += \
  $(%canon_reldir%_FCN_FILES) \
  $(%canon_reldir%_PRIVATE_FCN_FILES)

PKG_ADD_FILES += %reldir%/PKG_ADD

DIRSTAMP_FILES += %reldir%/$(octave_dirstamp)