# Search Tool Sources

search_tool_inc = include_directories(
  '.',
  '../mate-submodules/libegg',
  '../libmateui-deprecated',
)

search_tool_cflags = [
  '-DMATELOCALEDIR="@0@"'.format(localedir),
  '-DLOCALEDIR="@0@"'.format(localedir),
  '-DDATADIR="@0@"'.format(datadir),
  '-DGREP_COMMAND="grep"',
]

executable('mate-search-tool',
  sources: [
    'gsearchtool-support.c',
    'gsearchtool-callbacks.c',
    'gsearchtool.c',
  ],
  c_args: search_tool_cflags,
  dependencies: [glib_dep, gio_dep, gio_unix_dep, gtk_dep, libegg_dep, libmateui_deprecated_dep],
  include_directories: search_tool_inc,
  install: true,
)
