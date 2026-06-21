gdict_version_h = configure_file(
  input:  'gdict-version.h.in',
  output: 'gdict-version.h',
  configuration: configuration_data({
    'GDICT_MAJOR_VERSION': 1,
    'GDICT_MINOR_VERSION': 1,
    'GDICT_MICRO_VERSION': 0,
    'GDICT_VERSION':       '1.1.0',
  })
)

gdict_sources_h = files(
  'gdict-context.h',
  'gdict-client-context.h',
  'gdict-database-chooser.h',
  'gdict-defbox.h',
  'gdict-source-chooser.h',
  'gdict-source-loader.h',
  'gdict-source.h',
  'gdict-speller.h',
  'gdict-strategy-chooser.h',
  'gdict-utils.h',
)

gdict_enum_sources = gnome.mkenums('gdict-enum-types',
  sources: gdict_sources_h,
  c_template: 'gdict-enum-types.c.in',
  h_template: 'gdict-enum-types.h.in',
  install_header: false,
)

gdict_marshal_sources = gnome.genmarshal('gdict-marshal',
  sources: 'gdict-marshal.list',
  prefix: 'gdict_marshal',
  install_header: false,
)

libgdict_cflags = [
  '-DG_LOG_DOMAIN="Gdict"',
  '-DDATADIR="@0@"'.format(datadir),
  '-DLIBDIR="@0@"'.format(libdir),
  '-DSYSCONFDIR="@0@"'.format(sysconfdir),
  '-DPREFIX="@0@"'.format(prefix),
  '-DMATELOCALEDIR="@0@"'.format(localedir),
  '-DGDICTSOURCESDIR="@0@"'.format(join_paths(datadir, 'mate-dict', 'sources')),
  '-DGDICT_ENABLE_INTERNALS',
]

libgdict = static_library('matedict',
  sources: [
    gdict_enum_sources,
    gdict_marshal_sources,
    'gdict-context.c',
    'gdict-client-context.c',
    'gdict-database-chooser.c',
    'gdict-defbox.c',
    'gdict-source-chooser.c',
    'gdict-source-loader.c',
    'gdict-source.c',
    'gdict-speller.c',
    'gdict-strategy-chooser.c',
    'gdict-utils.c',
  ],
  c_args: libgdict_cflags,
  dependencies: [glib_dep, gobject_dep, gio_dep, gtk_dep],
  include_directories: include_directories('.'),
)

libgdict_dep = declare_dependency(
  link_with: libgdict,
  include_directories: include_directories('.'),
  sources: [gdict_enum_sources[1], gdict_marshal_sources[1]],
)
