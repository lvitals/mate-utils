subdir('libmateui-deprecated')

libegg_deps = [
  glib_dep,
  gtk_dep,
  ice_dep,
  sm_dep,
]

subdir('mate-submodules/libegg')

libegg_dep = declare_dependency(
  link_with: egg,
  include_directories: include_directories('mate-submodules/libegg'),
)

subdir('data')
subdir('src')
