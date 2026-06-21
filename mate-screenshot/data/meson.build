# Screenshot Tool Data

# Desktop file
i18n.merge_file(
  input:       'mate-screenshot.desktop.in',
  output:      'mate-screenshot.desktop',
  type:        'desktop',
  po_dir:      po_dir,
  install:     true,
  install_dir: join_paths(datadir, 'applications'),
)

# AppData xml
i18n.merge_file(
  input:       'mate-screenshot.appdata.xml.in',
  output:      'mate-screenshot.appdata.xml',
  type:        'xml',
  po_dir:      po_dir,
  install:     true,
  install_dir: join_paths(datadir, 'metainfo'),
)

# GSettings schema
schema_conf = configuration_data()
schema_conf.set('GETTEXT_PACKAGE', 'mate-utils')

schema_file = configure_file(
  input:         'org.mate.screenshot.gschema.xml.in',
  output:        'org.mate.screenshot.gschema.xml',
  configuration: schema_conf,
  install:       true,
  install_dir:   join_paths(datadir, 'glib-2.0', 'schemas'),
)

custom_target('compile-schemas-screenshot',
  input: schema_file,
  output: 'gschemas.compiled',
  command: [
    find_program('glib-compile-schemas'),
    meson.current_build_dir()
  ],
  build_by_default: true,
)

# Man page
install_man('mate-screenshot.1')
