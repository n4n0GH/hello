<img src="hello_icon.png" align="right" width="128" /> hello.
======

hello. is a compilation of various color-schemes, themes, icons and more for the KDE Plasma desktop. It is unopinionated, putting _you_ in charge.

## Content

| Package | WIP Status |
| --- | --- |
| [**Color Scheme**](https://github.com/n4n0GH/hello/tree/master/color-scheme)<br/>A beautifully crafted set of color schemes, supporting light and dark modes alike. | 90% done |
| [**Application Style**](https://github.com/n4n0GH/hello/tree/master/kstyle)<br/>Taking the best parts of KDE's own Breeze set and improving upon it subtly. | 0% done |
| [**Window Decoration**](https://github.com/n4n0GH/hello/tree/master/window-decoration)<br/>Allowing for granular customization, yet looking absolutely astonishing in every setting. | 90% done |
| [**Plasma Theme**](https://github.com/n4n0GH/hello/tree/master/plasma-theme)<br/>Completing your desktop experience with a well balanced and elegant Plasma theme, complementing your light or dark color scheme. | 90% done |
| [**Effects**](https://github.com/n4n0GH/hello/tree/master/kwin-effects)<br/>Rounding off the overall beautiful experience with the hello shader set. | 30% done |

## Installation

### Build Dependencies

To build the packages you have to install some build-tools for your system first. If you already built something from source chances are you might have some of those installed.

#### Ubuntu
```
sudo apt install build-essential libkf5config-dev libkdecorations2-dev libqt5x11extras5-dev qtdeclarative5-dev extra-cmake-modules libkf5guiaddons-dev libkf5configwidgets-dev libkf5windowsystem-dev libkf5coreaddons-dev gettext
```

#### Arch Linux
```
sudo pacman -S cmake extra-cmake-modules kdecoration qt5-declarative qt5-x11extras
```

#### Fedora
```
sudo dnf install cmake extra-cmake-modules "cmake(Qt5Core)" "cmake(Qt5Gui)" "cmake(Qt5DBus)" "cmake(Qt5X11Extras)" "cmake(KF5GuiAddons)" "cmake(KF5WindowSystem)" "cmake(KF5I18n)" "cmake(KDecoration2)" "cmake(KF5CoreAddons)" "cmake(KF5ConfigWidgets)" cmake(Qt5UiTools)" "cmake(KF5GlobalAccel)" kwin-devel libepoxy-devel
```

If your system is not listed above or there are packages missing in this list, please open an issue or pull request so it can be fixed.

### Automated Installation

#### Shell Script
Give the [hello automagic installer](https://github.com/hello-kde/install-tool) a try. It will fetch the latest stable release from this repository and run all the commands listed down below so you can sit back and watch the terminal do all the work.

#### Gentoo ebuild
Thanks to [therealfarfetchd](https://github.com/therealfarfetchd) for providing https://github.com/therealfarfetchd/ebuilds/tree/master/kde-misc/hello which includes the entire project.

### Manual Installation
Clone the repository and create the build directory:
```
git clone https://github.com/n4n0GH/hello
cd hello && mkdir build && cd build
```

Start building with cmake:
```
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
make
sudo make install
```

Finally restart KWin and Plasma to clear their cache:
```
kwin_x11 --replace &
plasmashell --replace &
```

You should now be able to use KDE's system settings to enable and modify the theme.

## Donate

People have asked me to donate because they enjoyed these tools. Here's how you can donate:

1) Open your favorite search engine
2) Type in "donate homeless in my area" or "donate kids in my area" or similar, you get the idea
3) Donate whatever you would've given to me, to those organizations instead

### Why can't I donate to you?

It's not that I don't want or need the money, but I'm blessed enough to have a roof over my head, food to eat, am generally healthy and have access to machines that allow me to create. Your money can help people in need or projects that help people.

If you live in Germany, here are some projects I personally have donated to and am a believer of:

### Children's Hospice Sternenbrücke

Providing help to families whose children are terminally ill, giving them a nice place to be in for the last moments of their short lives, easing the burden of the parents and even helping families after their child's untimely demise.

https://sternenbruecke.de/jetzt-spenden/paypal

### Streetmagazine Hinz&Kunzt

Project to give homeless people a chance to reclaim their lives and get back on track. Similar to "Big Issue", but on a regional level.

https://www.hinzundkunzt.de/helfen/online-spenden/
