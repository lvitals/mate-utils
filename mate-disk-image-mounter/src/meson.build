# Disk Image Mounter Sources

mounter_cflags = [
  '-DG_LOG_DOMAIN="mate-disk-image-mounter"',
  '-DMATELOCALEDIR="@0@"'.format(localedir),
]

executable('mate-disk-image-mounter',
  sources: 'main.c',
  c_args: mounter_cflags,
  dependencies: [glib_dep, udisks2_dep, gtk_dep],
  install: true,
)
