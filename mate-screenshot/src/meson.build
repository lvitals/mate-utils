# Screenshot Tool Sources

screenshot_resources = gnome.compile_resources('screenshot-resources',
  '../data/org.mate.screenshot.gresource.xml',
  source_dir: '../data',
  c_name: 'screenshot',
)

screenshot_deps = [
  glib_dep,
  gio_dep,
  canberra_dep,
  gthread_dep,
  gtk_dep,
  x11_dep,
  xext_dep,
  math_dep,
]

screenshot_cflags = [
  '-DMATELOCALEDIR="@0@"'.format(localedir),
]

if have_gdk_wayland
  screenshot_deps += [gdk_wayland_dep, wayland_client_dep]
  screenshot_cflags += ['-DMATE_SCREENSHOT_ENABLE_WAYLAND=1']
endif

executable('mate-screenshot',
  sources: [
    'mate-screenshot.c',
    'screenshot-dialog.c',
    'screenshot-shadow.c',
    'screenshot-utils.c',
    'screenshot-save.c',
    'screenshot-xfer.c',
    screenshot_resources,
  ],
  c_args: screenshot_cflags,
  dependencies: screenshot_deps,
  export_dynamic: true,
  install: true,
)

# Install symlink mate-panel-screenshot -> mate-screenshot
install_symlink('mate-panel-screenshot',
  install_dir: get_option('bindir'),
  pointing_to: 'mate-screenshot',
)
