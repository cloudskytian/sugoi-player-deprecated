![logo](/src/splayer/resources/splayer.svg)

## SPlayer

[![AppVeyor](https://ci.appveyor.com/api/projects/status/github/wangwenx190/SPlayer?branch=master&svg=true)](https://ci.appveyor.com/project/wangwenx190/splayer)
[![Coverity Scan Build Status](https://img.shields.io/coverity/scan/15146.svg)](https://scan.coverity.com/projects/splayer)
[![Github All Releases](https://img.shields.io/github/downloads/wangwenx190/SPlayer/total.svg)](https://github.com/wangwenx190/SPlayer/releases/latest)
[![GitHub release](https://img.shields.io/github/release/wangwenx190/SPlayer.svg)](https://github.com/wangwenx190/SPlayer/releases/latest)
[![Github commits (since latest release)](https://img.shields.io/github/commits-since/wangwenx190/SPlayer/latest.svg)](https://github.com/wangwenx190/SPlayer/releases/latest)
[![GitHub Release Date](https://img.shields.io/github/release-date/wangwenx190/SPlayer.svg)](https://github.com/wangwenx190/SPlayer/releases/latest)

---

* [External links](#external-links)

* [Overview](#overview)

* [Snapshots](#snapshots)

* [Features](#features)

* [System requirements](#system-requirements)

* [Downloads](#downloads)

* [Changelog](#changelog)

* [Compilation](#compilation)

* [Bug reports & Feature requests](#bug-reports--feature-requests)

* [Contributors](#contributors)

* [Contributing](#contributing)

* [License](#license)

* [Contact](#contact)

* [Donate](#donate)


## External links

* [Wiki](https://github.com/wangwenx190/SPlayer/wiki)

* [FAQ](https://github.com/wangwenx190/SPlayer/wiki/FAQ)

* [MPV Manual](http://mpv.io/manual/master/)


## Overview

SPlayer is a free and open source multimedia player for Windows 7+ based on **libmpv** and **Qt**.

People should know that SPlayer is a fork of [**Baka MPlayer**](https://github.com/u8sand/Baka-MPlayer). Baka MPlayer is a free and open source, cross-platform, libmpv based multimedia player. Its simple design reflects the idea for an uncluttered, simple, and enjoyable environment for watching tv shows. Thanks to the great work of Baka MPlayer's original developers. Without their hard work, there won't be SPlayer anymore. I really appreciate [**u8sand**](https://github.com/u8sand) and his team.


## Snapshots

![mainwindow](/doc/images/mainwindow.png)

![playing](/doc/images/playing.png)


## Features

- Beautiful and modern interface

- High DPI support

- Internationalization support

- Play online media streams

- Download online media streams from websites such as YouTube

- Support play almost all kinds of video and audio files

- High performance

- Much more to be found :)


## System requirements

- A not too ancient Linux, Windows 7 or later, or OSX 10.8 or later.

- A somewhat capable CPU. Hardware decoding might help if the CPU is too slow to
  decode video in realtime, but must be explicitly enabled with the `--hwdec`
  option.

- A not too crappy GPU. SPlayer is not intended to be used with bad GPUs. There are
  many caveats with drivers or system compositors causing tearing, stutter,
  etc. On Windows, you might want to make sure the graphics drivers are
  current. In some cases, ancient fallback video output methods can help
  (such as `--vo=xv` on Linux), but this use is not recommended or supported.

**NOTE**

SPlayer is only tested on the latest 64 bit Windows 10 Pro stable edition (that's my work environment). I am very sorry that I don't known wether SPlayer works well or not on other platforms. You have to compile SPlayer yourself to get the correct binary files for Linux and macOS. The source code is some sort of compatible, you need to change a little bit, but not much I guess.


## Downloads

For official builds and third-party packages please see
https://github.com/wangwenx190/SPlayer/releases.


## Changelog

See [**./doc/CHANGELOG.md**](/doc/CHANGELOG.md) for more information.


## Compilation


### Part A: Requirements

- [Visual Studio 2017](https://www.visualstudio.com/thank-you-downloading-visual-studio/?sku=Community&rel=15) (Only the build tools are needed, but you can also install the full IDE. The community edition is also fine)

  - [**Windows 10 SDK**](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk) (I recommend download the ISO image from this website instead of installing it from the VS installer. Remember to install the CDB debugger while you are installing the Win10 SDK if you want to use Qt Creator)

- [Qt Creator](http://download.qt.io/official_releases/qtcreator/) (Optional, install this software in case of you don't want to install VS. Remember to install the CDB support component if you want to debug)

- [**libmpv**](https://mpv.srsfckn.biz/)

- [**Qt5**](http://download.qt.io/official_releases/qt/)

- [YouTube-dl](https://yt-dl.org/latest/youtube-dl.exe) (Optional, for streaming youtube videos)

- [Inno Setup 5 Unicode](http://jrsoftware.org/isdl.php) (Optional, for building the installer)

**NOTE**

Using the **latest** version is highly recommended.


### Part B: Prepare the environment

Create a file named **build.user.bat** in **`C:\SPlayer\`** (just for example, it could be placed in anywhere you like) containing the following entries, adapted for your system:

```bat
@ECHO OFF
SET "_QT_DIR=C:\Qt\Qt5.10.1\5.10.1\msvc2017_64"
SET "_VC_BUILD_DIR=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build"
SET "_INNO_SETUP_DIR=C:\Program Files (x86)\Inno Setup 5"
```


### Part C: Download and compile the source code

1. Use Git to clone SPlayer's repository to **`C:\SPlayer\`** (just for example, it could be anywhere you like).

   1. Download Git from https://git-for-windows.github.io/ and install.

   2. Open Git bash and run:

      ```text
      git clone https://github.com/wangwenx190/SPlayer.git
      ```

2. Open the solution file **splayer.pro**. Change the solution's configuration to **Release** (in the toolbar).

3. Press <kbd>Ctrl</kbd> + <kbd>B</kbd> or click **Build** to build the solution.

4. You now have **SPlayer[64].exe** and **iconlib.dll** under **"./bin[64]/Release"**.

**NOTES**

- Alternatively, you can use **build.bat** that can build everything for you.

- How to generate Visual Studio solution file through qmake:

   ```bat
   REM Open cmd, call qtenv2.bat in your Qt bin dir
   REM For example:
   CALL "C:\Qt\Qt5.10.1\5.10.1\msvc2017_64\bin\qtenv2.bat"
   REM Then call vcvarsall.bat x64 or vcvarsall.bat x86 in your VC build dir
   REM For example:
   CALL "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
   REM If your project TEMPLATE is "subdirs", use this:
   REM qmake -r -tp vc xxx.pro
   REM Other templates use this:
   REM qmake -tp vc xxx.pro
   REM xxx.pro is your .pro file
   REM For example:
   qmake -r -tp vc splayer.pro
   REM Now you have the .sln file. Open it using VS.
   ```


## Bug reports & Feature requests

Please use the [**issues tracker**](https://github.com/wangwenx190/SPlayer/issues) provided by GitHub to send us bug reports or feature requests. Follow the template's instructions or the issue
will likely be ignored or closed as invalid.


## Contributors

See [**./doc/CONTRIBUTORS.md**](/doc/CONTRIBUTORS.md) for more information.


## Contributing

See [**./doc/CONTRIBUTING.md**](/doc/CONTRIBUTING.md) for more information.


## License

![GPLv3](http://www.gnu.org/graphics/gpl-v3-logo.svg)

See [**LICENSE.md**](/LICENSE.md) for more information.


## Contact

e-mail: chymxuan@foxmail.com

For Chinese users and developers:

QQ group number: 643590794

**NOTE**

Do **NOT** contact me through my private QQ number. Contact me through e-mail and QQ group **ONLY**. All messages from my private QQ are ignored.


## Donate

I am quite lack of money in fact, but I don't need any kinds of donations.

SPlayer will always be free and open-source, and there will never be any kinds of advertisements in it.
