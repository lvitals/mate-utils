# Disk Usage Analyzer Sources

baobab_resources = gnome.compile_resources('baobab-resources',
  '../data/org.mate.disk-usage-analyzer.gresource.xml',
  source_dir: '../data',
  c_name: 'baobab',
)

baobab_sources = files(
  'baobab.c',
  'baobab-cell-renderer-progress.c',
  'baobab-ringschart.c',
  'baobab-scan.c',
  'baobab-treeview.c',
  'baobab-utils.c',
  'callbacks.c',
  'baobab-prefs.c',
  'baobab-remote-connect-dialog.c',
  'baobab-chart.c',
  'baobab-treemap.c',
)

baobab_cflags = [
  '-DPREFIX="@0@"'.format(prefix),
  '-DSYSCONFDIR="@0@"'.format(sysconfdir),
  '-DLIBDIR="@0@"'.format(libdir),
  '-DDATADIR="@0@"'.format(datadir),
  '-DBAOBAB_PIX_DIR="@0@"'.format(join_paths(datadir, 'mate-disk-usage-analyzer', 'pixmaps')),
  '-DMATELOCALEDIR="@0@"'.format(localedir),
]

executable('mate-disk-usage-analyzer',
  sources: [baobab_sources, baobab_resources],
  c_args: baobab_cflags,
  dependencies: [glib_dep, gio_dep, gtk_dep, libgtop_dep, mate_desktop_dep, math_dep],
  export_dynamic: true,
  install: true,
)
