# Disk Usage Analyzer Data

# Desktop file
desktop_conf = configuration_data()
desktop_conf.set('VERSION', meson.project_version())

baobab_desktop_in = configure_file(
  input:         'mate-disk-usage-analyzer.desktop.in.in',
  output:        'mate-disk-usage-analyzer.desktop.in',
  configuration: desktop_conf,
)

i18n.merge_file(
  input:       baobab_desktop_in,
  output:      'mate-disk-usage-analyzer.desktop',
  type:        'desktop',
  po_dir:      po_dir,
  install:     true,
  install_dir: join_paths(datadir, 'applications'),
)

# AppData file
i18n.merge_file(
  input:       'mate-disk-usage-analyzer.appdata.xml.in',
  output:      'mate-disk-usage-analyzer.appdata.xml',
  type:        'xml',
  po_dir:      po_dir,
  install:     true,
  install_dir: join_paths(datadir, 'metainfo'),
)

# GSettings schema
schema_conf = configuration_data()
schema_conf.set('GETTEXT_PACKAGE', 'mate-utils')

schema_file = configure_file(
  input:         'org.mate.disk-usage-analyzer.gschema.xml.in',
  output:        'org.mate.disk-usage-analyzer.gschema.xml',
  configuration: schema_conf,
  install:       true,
  install_dir:   join_paths(datadir, 'glib-2.0', 'schemas'),
)

custom_target('compile-schemas-baobab',
  input: schema_file,
  output: 'gschemas.compiled',
  command: [
    find_program('glib-compile-schemas'),
    meson.current_build_dir()
  ],
  build_by_default: true,
)

# Man page
install_man('mate-disk-usage-analyzer.1')
