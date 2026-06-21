[![Translation status](https://translate.fedoraproject.org/widget/mate-desktop/svg-badge.svg)](https://translate.fedoraproject.org/engage/mate-desktop/)

# mate-utils

Contains MATE Utility programs for the MATE Desktop, namely the following:

* mate-system-log          [logview]
* mate-search-tool         [gsearchtool]
* mate-dictionary          [mate-dictionary]
* mate-screenshot          [mate-screenshot]
* mate-disk-usage-analyzer [baobab]
* mate-disk-image-mounter  [mate-disk-image-mounter]

mate-utils is a fork of GNOME Utilities.

This software is licensed under the GNU GPL. For more on the license, see COPYING.

## Requirements

* intltool                 >= 0.50.1
* mate-common              >= 1.24.1
* GLib                     >= 2.50.0
* GIO                      >= 2.50.0
* GTK+                     >= 3.22.0
* libmate-panel-applet     >= 1.17.0
* libgtop                  >= 2.12.0
* libcanberra-gtk          >= 0.4
* udisks2                  >= 1.90.0 (required for Disk Image Mounter)

## Optional Dependencies

* systemd                  (systemd journal support in Logview)
* gdk-wayland-3.0          (Wayland support in mate-screenshot)

## Optional Runtime Dependencies

The following components are recommended when running mate-screenshot under Wayland:

* xdg-desktop-portal       (desktop portal integration)
* xdg-desktop-portal-wlr   (wlroots portal backend)
* grim                     (Wayland screenshot backend)
* slurp                    (interactive area selection)

Without these components, screenshot functionality may be limited when running under a Wayland session.

## Installation and Build (Meson)

To configure, compile, and install `mate-utils` using Meson, run:

```bash
meson setup build
meson compile -C build
meson install -C build
```

### Meson Build Options

You can customize the compilation by passing options to `meson setup` via `-D<option>=<value>`:

* `-Dmate-dictionary=true|false`  
  Build the Dictionary utility. Default: `true`

* `-Dbaobab=true|false`  
  Build the Disk Usage Analyzer (baobab) utility. Default: `true`

* `-Dmate-disk-image-mounter=true|false`  
  Build the Disk Image Mounter utility. Default: `true`

* `-Dgsearchtool=true|false`  
  Build the Search Tool (gsearchtool) utility. Default: `true`

* `-Dmate-screenshot=true|false`  
  Build the Screenshot utility. Default: `true`

* `-Dlogview=true|false`  
  Build the System Log Viewer (logview) utility. Default: `true`

* `-Din_process=true|false`  
  Build the panel applet in-process (required for Wayland). Default: `true`

* `-Dwayland=enabled|disabled|auto`  
  Enable Wayland support for dictionary applet and screenshot. Default: `auto`

* `-Dzlib=enabled|disabled|auto`  
  Enable ZLib support for Logview. Default: `enabled`

* `-Dsystemd=enabled|disabled|auto`  
  Enable systemd journal support in Logview. Default: `auto`

* `-Dgrep=auto|<path>`  
  Specify the path to the grep command. Default: `auto` (auto-detected)

* `-Denable-debug=true|false`  
  Enable debug messages. Default: `false`

---

## Legacy Configure Flags (Autotools)

* `--enable-disk-image-mounter`  
  Whether to build the Disk Image Mounter utility. Default: `yes`

* `--enable-gdict-applet`  
  Whether to build the Dictionary mate-panel applet. Default: `yes`

* `--enable-in-process`  
  Enable in-process build of dictionary applet (required for wayland). Default: `no`

* `--enable-wayland`  
  Enable Wayland support. Default: `auto`

* `--enable-zlib`  
  Enable ZLib support for Logview. Default: `yes`

* `--enable-systemd`  
  Enable systemd journal support in Logview. Default: `auto`

* `--with-grep`  
  Specify the path to the grep command. Default: auto-detected

* `--enable-debug`  
  Enable debug messages. Default: `no`