dict_inc = include_directories(
  '.',
  '..',      # so <libgdict/gdict.h> resolves
  '../libgdict',
)

dict_cflags = [
  '-DPREFIX="@0@"'.format(prefix),
  '-DSYSCONFDIR="@0@"'.format(sysconfdir),
  '-DLIBDIR="@0@"'.format(libdir),
  '-DDATADIR="@0@"'.format(datadir),
  '-DPKGDATADIR="@0@"'.format(join_paths(datadir, 'mate-dictionary')),
  '-DMATELOCALEDIR="@0@"'.format(localedir),
  '-DGETTEXT_PACKAGE="mate-utils"',
]

# Build standalone mate-dictionary executable
mate_dictionary_sources = files(
  'gdict-about.c',
  'gdict-app.c',
  'gdict-common.c',
  'gdict-pref-dialog.c',
  'gdict-print.c',
  'gdict-sidebar.c',
  'gdict-source-dialog.c',
  'gdict-window.c',
  'main.c',
)

executable('mate-dictionary',
  sources: mate_dictionary_sources,
  c_args: dict_cflags,
  dependencies: [glib_dep, gio_dep, gtk_dep, mate_desktop_dep, libgdict_dep, math_dep],
  include_directories: dict_inc,
  install: true,
)

# Common applet sources (shared between in-process lib and out-of-process exe)
applet_sources = files(
  'gdict-about.c',
  'gdict-about.h',
  'gdict-aligned-window.c',
  'gdict-aligned-window.h',
  'gdict-applet.c',
  'gdict-applet.h',
  'gdict-common.c',
  'gdict-common.h',
  'gdict-pref-dialog.c',
  'gdict-pref-dialog.h',
  'gdict-print.c',
  'gdict-print.h',
  'gdict-sidebar.c',
  'gdict-sidebar.h',
  'gdict-source-dialog.c',
  'gdict-source-dialog.h',
)

if mate_panel_dep.found()
  applet_deps = [glib_dep, gobject_dep, gio_dep, gtk_dep, mate_panel_dep, mate_desktop_dep, libgdict_dep, math_dep]
  applet_cflags = dict_cflags + [
    '-DENABLE_IN_PROCESS=1',
    '-DPANEL_APPLET_FACTORY=MATE_PANEL_APPLET_IN_PROCESS_FACTORY',
  ]

  if have_wayland
    applet_deps += wayland_deps
    applet_cflags += ['-DENABLE_WAYLAND=1']
  endif

  if in_process
    # Build as shared module for in-process loading
    pkgmoduledir = join_paths(libdir, 'mate-utils')
    shared_module('mate-dictionary-applet',
      sources: applet_sources,
      c_args: applet_cflags,
      dependencies: applet_deps,
      include_directories: dict_inc,
      name_prefix: 'lib',
      install: true,
      install_dir: pkgmoduledir,
    )
  else
    applet_out_cflags = dict_cflags + [
      '-DPANEL_APPLET_FACTORY=MATE_PANEL_APPLET_OUT_PROCESS_FACTORY',
    ]
    executable('mate-dictionary-applet',
      sources: applet_sources,
      c_args: applet_out_cflags,
      dependencies: applet_deps,
      include_directories: dict_inc,
      install: true,
      install_dir: libexecdir,
    )
  endif
endif
