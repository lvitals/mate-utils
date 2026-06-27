pkgmoduledir = join_paths(libdir, 'mate-utils')
appletdir    = join_paths(datadir, 'mate-panel', 'applets')
servicedir   = join_paths(datadir, 'dbus-1', 'services')
schemadir    = join_paths(datadir, 'glib-2.0', 'schemas')

# Standalone desktop file
desktop_conf = configuration_data()
desktop_conf.set('VERSION', meson.project_version())

gdict_desktop_in = configure_file(
  input:         'mate-dictionary.desktop.in.in',
  output:        'mate-dictionary.desktop.in',
  configuration: desktop_conf,
)

i18n.merge_file(
  input:       gdict_desktop_in,
  output:      'mate-dictionary.desktop',
  type:        'desktop',
  po_dir:      po_dir,
  install:     true,
  install_dir: join_paths(datadir, 'applications'),
)

# Dictionary sources
i18n.merge_file(
  input:       'default.desktop.in',
  output:      'default.desktop',
  type:        'desktop',
  po_dir:      po_dir,
  install:     true,
  install_dir: join_paths(datadir, 'mate-dict', 'sources'),
)

i18n.merge_file(
  input:       'thai.desktop.in',
  output:      'thai.desktop',
  type:        'desktop',
  po_dir:      po_dir,
  install:     true,
  install_dir: join_paths(datadir, 'mate-dict', 'sources'),
)

# .mate-panel-applet desktop file
if mate_panel_dep.found()
  if in_process
    applet_location   = join_paths(pkgmoduledir, 'libmate-dictionary-applet.so')
    applet_in_process = 'true'
  else
    applet_location   = join_paths(libexecdir, 'mate-dictionary-applet')
    applet_in_process = 'false'
  endif

  applet_conf = configuration_data()
  applet_conf.set('APPLET_LOCATION',   applet_location)
  applet_conf.set('APPLET_IN_PROCESS', applet_in_process)

  applet_desktop_in = configure_file(
    input:         'org.mate.DictionaryApplet.mate-panel-applet.desktop.in.in',
    output:        'org.mate.DictionaryApplet.mate-panel-applet.desktop.in',
    configuration: applet_conf,
  )

  i18n.merge_file(
    input:      applet_desktop_in,
    output:     'org.mate.DictionaryApplet.mate-panel-applet',
    type:       'desktop',
    po_dir:     po_dir,
    install:    true,
    install_dir: appletdir,
  )

  # DBus service file — only for out-of-process
  if not in_process
    service_location = join_paths(libexecdir, 'mate-dictionary-applet')
    if have_wayland
      service_location = 'env GDK_BACKEND=x11 ' + service_location
    endif
    service_conf = configuration_data()
    service_conf.set('SERVICE_LOCATION', service_location)

    configure_file(
      input:         'org.mate.panel.applet.DictionaryAppletFactory.service.in',
      output:        'org.mate.panel.applet.DictionaryAppletFactory.service',
      configuration: service_conf,
      install:       true,
      install_dir:   servicedir,
    )
  endif
endif

# GSettings schema
schema_conf = configuration_data()
schema_conf.set('datadir', datadir)
schema_conf.set('GETTEXT_PACKAGE', 'mate-utils')

schema_file = configure_file(
  input:         'org.mate.dictionary.gschema.xml.in',
  output:        'org.mate.dictionary.gschema.xml',
  configuration: schema_conf,
  install:       true,
  install_dir:   schemadir,
)

custom_target('compile-schemas-dictionary',
  input: schema_file,
  output: 'gschemas.compiled',
  command: [
    find_program('glib-compile-schemas'),
    meson.current_build_dir()
  ],
  build_by_default: true,
)

# Appdata
i18n.merge_file(
  input:       'mate-dictionary.appdata.xml.in',
  output:      'mate-dictionary.appdata.xml',
  type:        'xml',
  po_dir:      po_dir,
  install:     true,
  install_dir: join_paths(datadir, 'metainfo'),
)

# UI data files
install_data(
  'mate-dictionary-preferences.ui',
  'mate-dictionary-source.ui',
  'mate-dictionary-ui.xml',
  'dictionary-applet-menu.xml',
  install_dir: join_paths(datadir, 'mate-dictionary'),
)

# Man page
install_man('mate-dictionary.1')
