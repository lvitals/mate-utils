# System Log Viewer Data

subdir('icons')

# Desktop file
desktop_conf = configuration_data()
desktop_conf.set('VERSION', meson.project_version())

logview_desktop_in = configure_file(
  input:         'mate-system-log.desktop.in.in',
  output:        'mate-system-log.desktop.in',
  configuration: desktop_conf,
)

i18n.merge_file(
  input:       logview_desktop_in,
  output:      'mate-system-log.desktop',
  type:        'desktop',
  po_dir:      po_dir,
  install:     true,
  install_dir: join_paths(datadir, 'applications'),
)

# GSettings schema
schema_conf = configuration_data()
schema_conf.set('GETTEXT_PACKAGE', 'mate-utils')

schema_file = configure_file(
  input:         'org.mate.system-log.gschema.xml.in',
  output:        'org.mate.system-log.gschema.xml',
  configuration: schema_conf,
  install:       true,
  install_dir:   join_paths(datadir, 'glib-2.0', 'schemas'),
)

custom_target('compile-schemas-logview',
  input: schema_file,
  output: 'gschemas.compiled',
  command: [
    find_program('glib-compile-schemas'),
    meson.current_build_dir()
  ],
  build_by_default: true,
)

# Man page
install_man('mate-system-log.1')
