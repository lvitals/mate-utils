# System Log Viewer Sources

logview_marshal = gnome.genmarshal('logview-marshal',
  sources: 'logview-marshal.list',
  prefix: 'logview_marshal',
  install_header: false,
)

logview_resources = gnome.compile_resources('logview-resources',
  '../data/org.mate.system-log.gresource.xml',
  source_dir: '../data',
  c_name: 'logview',
)

logview_sources = files(
  'logview-app.c',
  'logview-main.c',
  'logview-manager.c',
  'logview-utils.c',
  'logview-loglist.c',
  'logview-window.c',
  'logview-log.c',
  'logview-findbar.c',
  'logview-prefs.c',
  'logview-filter.c',
  'logview-filter-manager.c',
)

logview_cflags = [
  '-DG_LOG_DOMAIN="mate-system-log"',
  '-DMATELOCALEDIR="@0@"'.format(localedir),
  '-DDATADIR="@0@"'.format(datadir),
  '-DLOGVIEWINSTALLPREFIX="@0@/"'.format(prefix),
]

logview_deps = [glib_dep, gio_dep, gthread_dep, gtk_dep, math_dep]

if have_systemd
  logview_deps += systemd_dep
endif

if have_zlib
  logview_deps += zlib_dep
endif

executable('mate-system-log',
  sources: [logview_sources, logview_marshal, logview_resources],
  c_args: logview_cflags,
  dependencies: logview_deps,
  install: true,
)
