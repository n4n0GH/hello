# hello shaders

This KWin effect is the perfect companion for the "hello." KWin window decoration; it rounds the corners of all applications smoothly and draws a nice, thin accent line on the edge of each window to give it that little special something.

# Dependencies
Debian based (Ubuntu, Kubuntu, KDE Neon):
```
sudo apt install git pkg-config cmake g++ gettext extra-cmake-modules qttools5-dev libqt5x11extras5-dev libkf5configwidgets-dev libkf5crash-dev libkf5globalaccel-dev libkf5kio-dev libkf5notifications-dev kinit-dev kwin-dev 
```
Fedora based:
```
sudo dnf install git cmake gcc-c++ extra-cmake-modules qt5-qttools-devel qt5-qttools-static qt5-qtx11extras-devel kf5-kconfigwidgets-devel kf5-kcrash-devel kf5-kguiaddons-devel kf5-kglobalaccel-devel kf5-kio-devel kf5-ki18n-devel kf5-knotifications-devel kf5-kinit-devel kwin-devel qt5-qtbase-devel libepoxy-devel
```

# Installation
Clone the repository and change into the new directory:
```
git clone https://github.com/n4n0GH/hello/ && cd hello/kwin-effects
```
Run all necessary commands to prepare the compilation:
```
mkdir qt5build; cd qt5build; cmake ../ -DCMAKE_INSTALL_PREFIX=/usr -DQT5BUILD=ON 
```
Start the compilation and installation process:
```
make && sudo make install && (kwin_x11 --replace &)
```

# Using it
This plugin will activate by default once it's installed. You can turn it on and off and change settings in the "Desktop Effects" module inside your system settings.
