<h1 align="center">
  <a href="https://github.com/wangwenx190/sugoi-player-deprecated"><img src="src/sugoi/resources/player.svg" alt="Sugoi Player" width="256" height="256"/></a><br>
  <a href="https://github.com/wangwenx190/sugoi-player-deprecated">Sugoi Player</a>
</h1>

<p align="center"><em>(forked from "Baka-MPlayer")</em></p>

<h4 align="center">A free multimedia player for Windows based on libmpv and Qt</h4>

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
* [Donation](#donation)


## External links

* [Wiki](https://github.com/wangwenx190/Sugoi-Player/wiki)
* [FAQ](https://github.com/wangwenx190/Sugoi-Player/wiki/FAQ)
* [MPV Manual](http://mpv.io/manual/master/)


## Overview

Sugoi Player is a free and open source multimedia player for Windows 7+ based on **libmpv** and **Qt**.

People should know that Sugoi Player is a fork of [**Baka MPlayer**](https://github.com/u8sand/Baka-MPlayer). Baka MPlayer is a free and open source, cross-platform, libmpv based multimedia player. Its simple design reflects the idea for an uncluttered, simple, and enjoyable environment for watching tv shows. Thanks to the great work of Baka MPlayer's original developers. Without their hard work, there won't be Sugoi Player anymore. I really appreciate **godly-devotion** (Creator/UX Designer/Programmer), [**u8sand**](https://github.com/u8sand) (Lead Programmer/Website Host) and their team.

**IMPORTANT NOTES**

Sugoi Player is just an improved edition of Baka MPlayer. It does not compete with any other media players, including Baka MPlayer. It just offers an aditional option to choose from. Just choose what you like.

Baka MPlayer is a great media player. It is beautiful and powerful. I really like it. However, it can still be improved. Because it cannot meet everyone's needs. No media player can meet everyone's needs. And I want to improve it. So there is Sugoi Player. There are no accurate goals of Sugoi Player, I just improve it to meet my personal needs. I still have many great ideas about it, but sadly, due to the limitation of my technique, I cannot achieve them all. I really need some help. Pull requests are welcome. I also need some translators to help me add more translations. Any suggestions are welcome as well. Anyway, any contribution will be greatly appreciated.

There are many mpv front-ends in the world, some of them are truly great, I hope you can find the one you really like.


## Snapshots

![mainwindow](/doc/images/mainwindow.png)

![playing](/doc/images/playing.png)


## Features

- Smart playlist
- Dim Desktop
- Hardware accelerated playback (vdpau, vaapi, vda)
- Youtube playback support ([and others](http://rg3.github.io/youtube-dl/supportedsites.html))
- Multilingual support (we are looking for translators!)
- Beautiful and modern interface
- High DPI support
- Support playing almost all kinds of video and audio files
- Much more to be found :)


## System requirements

- Microsoft Windows 7 or later (64 bit os is highly recommended).
- A somewhat capable CPU. Hardware decoding might help if the CPU is too slow to
  decode video in realtime, but must be explicitly enabled with the `--hwdec`
  option.
- A not too crappy GPU. Sugoi Player is not intended to be used with bad GPUs. There are
  many caveats with drivers or system compositors causing tearing, stutter,
  etc. On Windows, you might want to make sure the graphics drivers are
  current. In some cases, ancient fallback video output methods can help
  (such as `--vo=xv` on Linux), but this use is not recommended or supported.

**NOTE**

Sugoi Player is only tested on the latest 64 bit Windows 10 Pro stable edition (that's my work environment). I am very sorry that I don't known whether Sugoi Player works well or not on other platforms. You have to compile Sugoi Player yourself to get the correct binary files for Linux and macOS. The source code is some sort of compatible, but you need to change a little bit.


## Downloads

For official builds and third-party packages please see
https://github.com/wangwenx190/Sugoi-Player/releases.


## Changelog

See [**doc/CHANGELOG.md**](/doc/CHANGELOG.md) for more information.


## Compilation


### Part A: Requirements

- [Visual Studio 2017](https://www.visualstudio.com/thank-you-downloading-visual-studio/?sku=Community&rel=15) (Only the build tools are needed, but you can also install the full IDE. The community edition is also fine)
  - [**Windows 10 SDK**](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk) (I recommend download the ISO image from this website instead of installing it from the VS installer. Remember to install the CDB debugger while you are installing the Win10 SDK if you want to use Qt Creator)
- [**libmpv**](https://sourceforge.net/projects/mpv-player-windows/files/libmpv/)
- [**Qt5**](http://download.qt.io/archive/qt/)
- [YouTube-dl](https://yt-dl.org/latest/youtube-dl.exe) (Optional, for streaming youtube videos)
- [Inno Setup 5 Unicode](http://jrsoftware.org/isdl.php) (Optional, for building the installer)

**NOTE**

Using the **latest** version is highly recommended.


### Part B: Prepare the environment

Create a file named **build.user.bat** in **`C:\Sugoi-Player\`** (just for example, it could be placed in anywhere you like) containing the following entries, adapted for your system:
```bat
@ECHO OFF
SET "_QT_DIR_32=C:\Qt\Qt5.12.0\5.12.0\msvc2017"
SET "_VC_BAT_PATH==C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat"
SET "_JOM_DIR=C:\Qt\Qt5.12.0\tools\qtcreator\bin"
```


### Part C: Download and compile the source code

1. Use Git to clone Sugoi Player's repository to **`C:\Sugoi-Player\`** (just for example, it could be anywhere you like).
   1. Download Git from https://git-for-windows.github.io/ and install.
   2. Open Git bash and run:
      ```text
      git clone https://github.com/wangwenx190/Sugoi-Player.git
      ```
2. Open the solution file **sugoi-player.pro**. Change the solution's configuration to **Release** (in the toolbar).
3. Press <kbd>Ctrl</kbd> + <kbd>B</kbd> or click **Build** to build the solution.
4. You now have **Sugoi[64].exe** and **iconlib.dll** under **"bin[64]/Release"**.

**NOTES**

- Alternatively, you can use **build.bat** that can build everything for you.
- How to generate Visual Studio solution file through qmake:
   ```bat
   REM Open cmd, call qtenv2.bat in your Qt bin dir
   REM For example:
   CALL "C:\Qt\Qt5.12.0\5.12.0\msvc2017\bin\qtenv2.bat"
   REM Then call vcvarsall.bat x64 or vcvarsall.bat x86 in your VC build dir
   REM For example:
   CALL "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
   REM If your project TEMPLATE is "subdirs", use this:
   REM qmake -r -tp vc xxx.pro
   REM Other templates use this:
   REM qmake -tp vc xxx.pro
   REM xxx.pro is your .pro file
   REM For example:
   qmake -r -tp vc sugoi-player.pro
   REM Now you have the .sln file. Open it using VS.
   ```


## Bug reports & Feature requests

Please use the [**issues tracker**](https://github.com/wangwenx190/Sugoi-Player/issues) provided by GitHub to send us bug reports or feature requests. Follow the template's instructions or the issue
will likely be ignored or closed as invalid.


## Contributors

See [**doc/contributors.md**](/doc/contributors.md) for more information.


## Contributing

See [**doc/contributing.md**](/doc/contributing.md) for more information.


## License

![GPLv3](http://www.gnu.org/graphics/gpl-v3-logo.svg)

See [**LICENSE.md**](/LICENSE.md) for more information.


## Contact

e-mail: chymxuan@foxmail.com

For Chinese users and developers:

QQ group number: 643590794

**NOTE**

Do **NOT** contact me through my private QQ number. Contact me through e-mail and QQ group **ONLY**. All messages from my private QQ are ignored.


## Donation

Thanks for your concern, but I don't need any kinds of donations.

Sugoi Player will always be free and open-source, and there will never be any kinds of advertisements in it.
