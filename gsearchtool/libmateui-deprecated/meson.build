# Deprecated MATE UI convenience library

libmateui_deprecated = static_library('mateui-deprecated',
  sources: 'gsearchtool-entry.c',
  dependencies: [glib_dep, gtk_dep],
)

libmateui_deprecated_dep = declare_dependency(
  link_with: libmateui_deprecated,
  include_directories: include_directories('.'),
)
