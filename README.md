[![Translation status](https://translate.fedoraproject.org/widget/mate-desktop/svg-badge.svg)](https://translate.fedoraproject.org/engage/mate-desktop/)

# mate-utils

Contains MATE Utility programs for the MATE Desktop, namely the following:

* mate-system-log          [logview]
* mate-search-tool         [gsearchtool]
* mate-dictionary          [mate-dictionary]
* mate-screenshot          [mate-screenshot]
* mate-disk-usage-analyzer [baobab]

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

## Configure Flags

* `--enable-zlib`  
  Enable ZLib support for Logview.  
  Default: `yes`

* `--enable-systemd`  
  Enable systemd journal support in Logview.  
  Default: `auto`

* `--enable-wayland`  
  Enable Wayland support in mate-screenshot.  
  Default: `auto`

* `--with-grep`  
  Specify the path to the grep command.  
  Default: auto-detected

* `--enable-debug`  
  Enable debug messages.  
  Default: `no`