<p align="center"><img src="hello_icon.png" alt="hello" width="100" height="100"></p>

<h1 align="center">hello.</h1>

**hello** is a kwin window decoration, which aims to mimic macOS' appearance, but with **power ups**.

It also works great with the ![hello shaders](https://github.com/n4n0GH/hello/tree/master/kwin-effects) KWin effects.

## Preview

![](screenshot.png?raw=true)

Looks great  with light and dark color schemes. Shown here with ![Arc](https://github.com/PapirusDevelopmentTeam/arc-kde) and ![Arc-Dark](https://github.com/PapirusDevelopmentTeam/arc-kde) color schemes.

| Feature | Screenshot |
| --- | --- |
| Titlebars can use the color supplied by your system's color scheme... | ![](regularcolors.png?raw=true) |
| ...or match the window color of your color scheme... | ![](colormatching.png?raw=true) |
| ...or manually use a custom color picker to paint the titlebar and perfectly match even GTK applications! | ![](perfectcolor.png?raw=true) |
| Shadows and highlight lines make sure you don't lose track of your windows in low contrast environments. | ![](low_contrast.png?raw=true) |
| Embrace flat design, or use gradients and a titlebar separator; your choice! BreezewayMod will use your chosen color profile to bring you a smooth interface that fits _your_ desktop. |  ![](gradients.png?raw=true) |
| A nice, clean settings page gives you granular control over the theme's features. | ![](settings.gif?raw=true) |
| Take customization to the next level with window specific settings, enabling you to mix and match settings and colors just the way you like it. | ![](window_settings.png?raw=true) |
| Support for HiDPI screens comes out of the box as well. Just increase size of titlebar, buttons and border radius and you're all set. | ![](hidpi.png?raw=true)
| Hovering over window buttons reveals additional icons. You can choose to also always show the icons. | ![](buttonicons.png?raw=true) |

## Building and Installation

### Automated Installation

#### Ubuntu > 18.04
```
sudo add-apt-repository ppa:krisives/kde-hello
sudo apt install kde-hello
```
Made and maintained by [krisives](https://github.com/krisives)

#### Arch Linux
```
yay -S hello-kde-git
```
Made and maintained by ToxicSalt

### Manual Installation

Before proceeding with the actual installation process, make sure you have the following dependencies installed:

* cmake
* g++ / clang
* extra-cmake-modules
* libkdecorations2-dev
* libkf5guiaddons-dev
* libkf5configwidgets-dev
* libkf5windowsystem-dev
* libkf5package-dev
* libqt5x11extras5-dev
* gettext
* pkg-config *(optional)*

Download the [latest stable release](https://github.com/n4n0GH/hello/releases) or clone this repository for bleeding edge builds:

```
git clone https://github.com/n4n0GH/hello
```
It's recommended to clone the repository to where you can leave the built directory, just in case you want to uninstall **hello**. However, if you accidentally removed the directory, please check below for manually uninstallation instructions.

Change to `window-decoration` directory and create a new directory called `build`:

```
cd hello/window-decoration && mkdir build && cd build
```

To install, use `cmake`, `make` and `make install`:

```
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
make && sudo make install
```

It may be necessary to restart the KWin window manager after that, so either reboot your machine, or do it the cool way:

```
kwin_x11 --replace &
```

## Uninstalling

In the build directory, run:

```
sudo make uninstall
```

If you removed the installation directory, you'll have to delete the following files and folders:

```
/usr/lib64/cmake/hello/HelloConfig.cmake
/usr/lib64/cmake/hello/HelloConfigVersion.cmake
/usr/lib64/qt/plugins/org.kde.kdecoration2/hellodecoration.so
/usr/share/kservices5/hellodecorationconfig.desktop
/usr/lib64/libhellocommon5.so.5.16.5
/usr/lib64/libhellocommon5.so.5
```

## Contribution

If you have any questions regarding **hello**, you are welcome to [open an issue](https://github.com/n4n0GH/hello/issues).

## FAQ

**Q: I can see this theme in the KDE store, but I can't install it through KDE's settings menu, what's up with that?**

A: The KDE store is meant to provide easy access to Aurorae themes and KDE's settings menu is pretty good at downloading those themes and moving them around. BreezewayMod is written in C++ and not using the Aurorae engine. Sorry to say, but you're going to have to get your hands dirty in the terminal.

**Q: Why is blur not enabled by default on this theme?**

A: There's a massive ongoing issue with the KDecoration2 framework which will treat the titlebar - and windows in general - as if they had 90° corners, i.e. perfectly rectangular shapes. This leads to something I call "blur bleed", which enables you to see the blur effect being rendered _underneath_ the window when your theme uses rounded corners. This affects regular Aurorae themes and QML themes alike. Read more about it on the KDE bugtracker: https://bugs.kde.org/show_bug.cgi?id=395725

| Description | Showcase |
| --- | --- |
| This glitch in action. | ![](sierrabreeze.png?raw=true) |
| BreezewayMod doesn't have that problem because it doesn't rely on blur features until KDecoration2 gets fixed. | ![](hello.png?raw=true) |

**Q: But I really don't care about the glitches, I just want blur! How can I do this?**

A: Find the `hello.json` file inside the `kdecoration` directory. [On line 12](https://github.com/n4n0GH/hello/window-decoration/blob/master/kdecoration/breezeway.json#L12) you switch out `"blur": false,` with `"blur": true,` and then compile the theme according to the instructions. 

**Q: Does this work on Wayland?**

A: Nyes. The theme will render with glitches and is not ready for Wayland yet. Maybe when Wayland works stable on my system I'll get around to fixing that.
