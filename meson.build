project('mate-utils', 'c',
  version: '1.28.1',
  meson_version: '>= 0.61.0',
  default_options: [
    'warning_level=1',
    'c_std=gnu99',
  ]
)

gnome = import('gnome')
i18n  = import('i18n')
cc    = meson.get_compiler('c')

prefix     = get_option('prefix')
datadir    = join_paths(prefix, get_option('datadir'))
libdir     = join_paths(prefix, get_option('libdir'))
libexecdir = join_paths(prefix, get_option('libexecdir'))
sysconfdir = join_paths(prefix, get_option('sysconfdir'))
localedir  = join_paths(datadir, 'locale')

add_project_arguments('-DHAVE_CONFIG_H', language: 'c')

# Debug options
enable_debug = get_option('enable-debug')
if enable_debug
  add_project_arguments('-DGDICT_ENABLE_DEBUG', language: 'c')
else
  add_project_arguments('-DG_DISABLE_CAST_CHECKS', language: 'c')
endif

conf = configuration_data()
conf.set_quoted('PACKAGE',          'mate-utils')
conf.set_quoted('VERSION',          meson.project_version())
conf.set_quoted('GETTEXT_PACKAGE',  'mate-utils')
conf.set_quoted('DATADIR',          datadir)
conf.set_quoted('PKGDATADIR',       join_paths(datadir, 'mate-dictionary'))
conf.set_quoted('LIBDIR',           libdir)
conf.set_quoted('SYSCONFDIR',       sysconfdir)
conf.set_quoted('PREFIX',           prefix)
conf.set_quoted('MATELOCALEDIR',    localedir)
conf.set_quoted('GDICTSOURCESDIR',  join_paths(datadir, 'mate-dict', 'sources'))
conf.set('ENABLE_NLS',              1)

# Grep path
grep_opt = get_option('grep')
if grep_opt == 'auto'
  grep_cmd = find_program('grep', required: true).full_path()
else
  grep_cmd = grep_opt
endif
conf.set_quoted('GREP_COMMAND', grep_cmd)

if cc.has_header('unistd.h')
  conf.set('HAVE_UNISTD_H', 1)
endif

if cc.has_header_symbol('sys/socket.h', 'AF_INET6')
  conf.set('ENABLE_IPV6', 1)
endif

# Common dependencies
glib_dep        = dependency('glib-2.0',    version: '>= 2.50.0')
gobject_dep     = dependency('gobject-2.0', version: '>= 2.50.0')
gio_dep         = dependency('gio-2.0',     version: '>= 2.50.0')
gio_unix_dep    = dependency('gio-unix-2.0', version: '>= 2.18.0')
gthread_dep     = dependency('gthread-2.0', version: '>= 2.50.0')
gtk_dep         = dependency('gtk+-3.0',    version: '>= 3.22.0')
mate_desktop_dep = dependency('mate-desktop-2.0', version: '>= 1.27.1')
math_dep        = cc.find_library('m', required: false)

# Optional panel applet dependency
mate_panel_dep  = dependency('libmatepanelapplet-4.0', version: '>= 1.25.2', required: false)

# Optional feature dependencies
libgtop_dep     = dependency('libgtop-2.0', version: '>= 2.12.0', required: get_option('baobab'))
canberra_dep    = dependency('libcanberra-gtk3', version: '>= 0.4', required: get_option('baobab') or get_option('mate-screenshot'))
udisks2_dep     = dependency('udisks2', version: '>= 1.90.0', required: get_option('mate-disk-image-mounter'))

# libegg (X11 / Session Manager) dependencies
sm_dep          = dependency('sm', version: '>= 1.0.0', required: get_option('gsearchtool'))
ice_dep         = dependency('ice', version: '>= 1.0.0', required: get_option('gsearchtool'))

# X11 dependencies for mate-screenshot
x11_dep         = dependency('x11', required: get_option('mate-screenshot'))
xext_dep        = dependency('xext', required: get_option('mate-screenshot'))

# Systemd & Zlib for logview
systemd_dep     = dependency('libsystemd', version: '>= 234', required: get_option('systemd'))
have_systemd    = systemd_dep.found()
if have_systemd
  conf.set('HAVE_SYSTEMD', 1)
endif

zlib_dep        = dependency('zlib', required: get_option('zlib'))
have_zlib       = zlib_dep.found()
if have_zlib
  conf.set('HAVE_ZLIB', 1)
endif

in_process = get_option('in_process')

wayland_opt = get_option('wayland')
wayland_deps = []
have_wayland = false
if wayland_opt.allowed()
  gtk_layer_shell_dep = dependency('gtk-layer-shell-0', version: '>= 0.7.0',
    required: wayland_opt)
  wayland_client_dep = dependency('wayland-client', required: wayland_opt)
  gdk_wayland_dep = dependency('gdk-wayland-3.0', version: '>= 3.22.0',
    required: wayland_opt)
  if gtk_layer_shell_dep.found() and wayland_client_dep.found() and gdk_wayland_dep.found()
    have_wayland = true
    wayland_deps = [gtk_layer_shell_dep, wayland_client_dep, gdk_wayland_dep]
  endif
endif

if have_wayland and not in_process
  error('Wayland support requires in_process=true')
endif

# Wayland support for mate-screenshot
have_gdk_wayland = false
if not wayland_opt.disabled()
  gdk_wayland_dep = dependency('gdk-wayland-3.0', version: '>= 3.22.0', required: wayland_opt)
  wayland_client_dep = dependency('wayland-client', required: wayland_opt)
  if gdk_wayland_dep.found() and wayland_client_dep.found()
    have_gdk_wayland = true
  endif
endif

if have_gdk_wayland
  conf.set('MATE_SCREENSHOT_ENABLE_WAYLAND', 1)

  # Check optional runtime dependencies for mate-screenshot under Wayland
  grim_bin = find_program('grim', required: false)
  slurp_bin = find_program('slurp', required: false)
  portal_bin = find_program('xdg-desktop-portal', required: false)

  if not grim_bin.found()
    warning('grim not found. It is recommended when running mate-screenshot under Wayland.')
  endif
  if not slurp_bin.found()
    warning('slurp not found. It is recommended when running mate-screenshot under Wayland.')
  endif
  if not portal_bin.found()
    warning('xdg-desktop-portal not found. It is recommended when running mate-screenshot under Wayland.')
  endif
endif

config_h = configure_file(output: 'config.h', configuration: conf)
config_inc = include_directories('.')
add_project_arguments('-I' + meson.current_build_dir(), language: 'c')

po_dir = join_paths(meson.project_source_root(), 'po')

# Subdirectories
if get_option('mate-dictionary')
  subdir('mate-dictionary')
endif

if get_option('baobab')
  subdir('baobab')
endif

if get_option('mate-disk-image-mounter')
  subdir('mate-disk-image-mounter')
endif

if get_option('gsearchtool')
  subdir('gsearchtool')
endif

if get_option('mate-screenshot')
  subdir('mate-screenshot')
endif

if get_option('logview')
  subdir('logview')
endif

subdir('po')

gnome.post_install(glib_compile_schemas: true)
