#IF VER < EncodeVer(5,5,0)
  #error Please upgrade your Inno Setup to at least V5.5.0 !!!
#endif

#ifndef UNICODE
  #error Please use **UNICODE** edition of Inno Setup !!!
#endif

#include "..\..\include\splayer-version.h"

;指定是否为64位安装程序
;#define x64

;指定是否只能在 Windows 7 SP1 及更新版本的操作系统上安装
#define Windows7AndNewer

;指定是否要注册相关后缀名
#define RegisteAssociations

;指定是否在安装时轮播图片
;#define ShowSlidePictures

;指定是否为绿色版安装程序（仅释放文件，不写入注册表条目，也不生成卸载程序）
;#define PortableBuild

;指定是否只能安装新版本，而不能用旧版本覆盖新版本
#define OnlyInstallNewVersion

;指定是否使用自定义卸载程序
;#define UseCustomUninstaller
;#define CustomUninstaller ".\{output}\Uninstall.exe"

;若想开启禁止安装旧版本的功能，此处版本号请注意一定要是
;点分十进制的正整数，除数字和英文半角句点以外不允许出现任何其他字符，
;否则程序无法判断版本的高低。
#define MyAppVersion      str(SPLAYER_VERSION_MAJOR) + "." + str(SPLAYER_VERSION_MINOR) + "." + str(SPLAYER_VERSION_PATCH) + "." + str(SPLAYER_VERSION_BUILD)
#define MyAppPublisher    str(SPLAYER_COMPANY_NAME_STR)
#define MyAppPublisherURL str(SPLAYER_COMPANY_URL_STR)
#define MyAppSupportURL   str(SPLAYER_SUPPORT_URL_STR)
#define MyAppUpdatesURL   str(SPLAYER_UPDATE_URL_STR)
#define MyAppComments     str(SPLAYER_COMMENTS_STR)
#define MyAppContact      str(SPLAYER_CONTACT_STR)
#define MyAppSupportPhone str(SPLAYER_SUPPORT_PHONE_STR)
#define MyAppReadmeURL    str(SPLAYER_README_URL_STR)
#define MyAppLicenseURL   str(SPLAYER_LICENSE_URL_STR)
#define MyAppCopyright    str(SPLAYER_COPYRIGHT_STR)
#define MyAppName         str(SPLAYER_APP_NAME_STR)
#ifdef x64
  #define MyAppBinDir     "..\..\bin64\Release"
  #define MyAppID         str(SPLAYER_APP_ID_64_STR)
  #define MyAppExeName    "SPlayer64.exe"
  #define MyAppMutex      str(SPLAYER_APP_MUTEX_64_STR)
  #define MyAppSetupExe   MyAppName + "_" + MyAppVersion + "_x64"
#else
  #define MyAppBinDir     "..\..\bin\Release"
  #define MyAppID         str(SPLAYER_APP_ID_32_STR)
  #define MyAppExeName    "SPlayer.exe"
  #define MyAppMutex      str(SPLAYER_APP_MUTEX_32_STR)
  #define MyAppSetupExe   MyAppName + "_" + MyAppVersion + "_x86"
#endif
#ifdef PortableBuild
  #define MyAppSetupExe   MyAppSetupExe + "_" + "Portable"
#else
  #define MyAppSetupExe   MyAppSetupExe + "_" + "Setup"
#endif

[Setup]
AppId                           = {{#MyAppID}
AppName                         = {#MyAppName}
AppVersion                      = {#MyAppVersion}
AppVerName                      = {#MyAppName} {#MyAppVersion}
AppPublisher                    = {#MyAppPublisher}
AppPublisherURL                 = {#MyAppPublisherURL}
AppSupportURL                   = {#MyAppSupportURL}
AppUpdatesURL                   = {#MyAppUpdatesURL}
AppComments                     = {#MyAppComments}
AppContact                      = {#MyAppContact}
AppSupportPhone                 = {#MyAppSupportPhone}
AppReadmeFile                   = {#MyAppReadmeURL}
AppCopyright                    = {#MyAppCopyright}
DefaultGroupName                = {#MyAppName}
VersionInfoDescription          = {#MyAppName} Setup
VersionInfoProductName          = {#MyAppName}
VersionInfoCompany              = {#MyAppPublisher}
VersionInfoCopyright            = {#MyAppCopyright}
VersionInfoProductVersion       = {#MyAppVersion}
VersionInfoProductTextVersion   = {#MyAppVersion}
VersionInfoTextVersion          = {#MyAppVersion}
VersionInfoVersion              = {#MyAppVersion}
SetupIconFile                   = "..\splayer\resources\splayer.ico"
Compression                     = lzma2/ultra64
InternalCompressLevel           = ultra64
SolidCompression                = yes
DisableProgramGroupPage         = yes
DisableDirPage                  = yes
DisableReadyMemo                = yes
DisableReadyPage                = yes
TimeStampsInUTC                 = yes
#IF VER >= EncodeVer(5,5,9)
SetupMutex                      = {{#MyAppID}Setup,Global\{{#MyAppID}Setup
;AppMutex                        = {{#MyAppMutex}
#endif
LanguageDetectionMethod         = uilanguage
ShowLanguageDialog              = no
AllowCancelDuringInstall        = no
#ifdef x64
OutputDir                       = "..\..\bin64\Installer"
ArchitecturesAllowed            = x64
ArchitecturesInstallIn64BitMode = x64
DefaultDirName                  = {pf64}\{#MyAppPublisher}\{#MyAppName}
#else
OutputDir                       = "..\..\bin\Installer"
ArchitecturesAllowed            = x86 x64
DefaultDirName                  = {pf32}\{#MyAppPublisher}\{#MyAppName}
#endif
#ifdef Windows7AndNewer
MinVersion                      = 0,6.1.7600
#else
MinVersion                      = 0,5.1.2600
#endif
#ifdef RegisteAssociations
ChangesAssociations             = yes
#else
ChangesAssociations             = no
#endif
#ifdef PortableBuild
Uninstallable                   = no
PrivilegesRequired              = lowest
#else
Uninstallable                   = yes
PrivilegesRequired              = admin
#ifdef x64
UninstallDisplayName            = {#MyAppName} (64-bit)
#else
UninstallDisplayName            = {#MyAppName}
#endif
UninstallDisplayIcon            = {app}\{#MyAppExeName},0
UninstallFilesDir               = {app}\Uninstaller
#endif
OutputBaseFilename              = {#MyAppSetupExe}

[Languages]
;此段的第一个语言为默认语言，除此之外，语言的名称与顺序都无所谓
Name: "english";             MessagesFile: ".\{lang}\English.isl"
Name: "chinesesimplified";   MessagesFile: ".\{lang}\ChineseSimplified.isl"
Name: "afrikaans";           MessagesFile: ".\{lang}\Afrikaans.isl"
Name: "albanian";            MessagesFile: ".\{lang}\Albanian.isl"
Name: "arabic";              MessagesFile: ".\{lang}\Arabic.isl"
Name: "armenian";            MessagesFile: ".\{lang}\Armenian.islu"
Name: "asturian";            MessagesFile: ".\{lang}\Asturian.isl"
Name: "basque";              MessagesFile: ".\{lang}\Basque.isl"
Name: "belarusian";          MessagesFile: ".\{lang}\Belarusian.isl"
Name: "bengali";             MessagesFile: ".\{lang}\Bengali.islu"
Name: "bosnian";             MessagesFile: ".\{lang}\Bosnian.isl"
Name: "brazilianportuguese"; MessagesFile: ".\{lang}\BrazilianPortuguese.isl"
Name: "bulgarian";           MessagesFile: ".\{lang}\Bulgarian.isl"
Name: "catalan";             MessagesFile: ".\{lang}\Catalan.isl"
Name: "chinesetraditional";  MessagesFile: ".\{lang}\ChineseTraditional.isl"
Name: "corsican";            MessagesFile: ".\{lang}\Corsican.isl"
Name: "croatian";            MessagesFile: ".\{lang}\Croatian.isl"
Name: "czech";               MessagesFile: ".\{lang}\Czech.isl"
Name: "danish";              MessagesFile: ".\{lang}\Danish.isl"
Name: "dutch";               MessagesFile: ".\{lang}\Dutch.isl"
Name: "englishbritish";      MessagesFile: ".\{lang}\EnglishBritish.isl"
Name: "esperanto";           MessagesFile: ".\{lang}\Esperanto.isl"
Name: "estonian";            MessagesFile: ".\{lang}\Estonian.isl"
Name: "farsi";               MessagesFile: ".\{lang}\Farsi.isl"
Name: "finnish";             MessagesFile: ".\{lang}\Finnish.isl"
Name: "french";              MessagesFile: ".\{lang}\French.isl"
Name: "galician";            MessagesFile: ".\{lang}\Galician.isl"
Name: "georgian";            MessagesFile: ".\{lang}\Georgian.islu"
Name: "german";              MessagesFile: ".\{lang}\German.isl"
Name: "greek";               MessagesFile: ".\{lang}\Greek.isl"
Name: "hebrew";              MessagesFile: ".\{lang}\Hebrew.isl"
Name: "hindi";               MessagesFile: ".\{lang}\Hindi.islu"
Name: "hungarian";           MessagesFile: ".\{lang}\Hungarian.isl"
Name: "icelandic";           MessagesFile: ".\{lang}\Icelandic.isl"
Name: "indonesian";          MessagesFile: ".\{lang}\Indonesian.isl"
Name: "italian";             MessagesFile: ".\{lang}\Italian.isl"
Name: "japanese";            MessagesFile: ".\{lang}\Japanese.isl"
Name: "kazakh";              MessagesFile: ".\{lang}\Kazakh.islu"
Name: "korean";              MessagesFile: ".\{lang}\Korean.isl"
Name: "kurdish";             MessagesFile: ".\{lang}\Kurdish.isl"
Name: "latvian";             MessagesFile: ".\{lang}\Latvian.isl"
Name: "ligurian";            MessagesFile: ".\{lang}\Ligurian.isl"
Name: "lithuanian";          MessagesFile: ".\{lang}\Lithuanian.isl"
Name: "luxemburgish";        MessagesFile: ".\{lang}\Luxemburgish.isl"
Name: "macedonian";          MessagesFile: ".\{lang}\Macedonian.isl"
Name: "malaysian";           MessagesFile: ".\{lang}\Malaysian.isl"
Name: "mongolian";           MessagesFile: ".\{lang}\Mongolian.isl"
Name: "montenegrian";        MessagesFile: ".\{lang}\Montenegrian.isl"
Name: "nepali";              MessagesFile: ".\{lang}\Nepali.islu"
Name: "norwegian";           MessagesFile: ".\{lang}\Norwegian.isl"
Name: "norwegiannynorsk";    MessagesFile: ".\{lang}\NorwegianNynorsk.isl"
Name: "occitan";             MessagesFile: ".\{lang}\Occitan.isl"
Name: "polish";              MessagesFile: ".\{lang}\Polish.isl"
Name: "portuguese";          MessagesFile: ".\{lang}\Portuguese.isl"
Name: "romanian";            MessagesFile: ".\{lang}\Romanian.isl"
Name: "russian";             MessagesFile: ".\{lang}\Russian.isl"
Name: "scottishgaelic";      MessagesFile: ".\{lang}\ScottishGaelic.isl"
Name: "serbiancyrillic";     MessagesFile: ".\{lang}\SerbianCyrillic.isl"
Name: "serbianlatin";        MessagesFile: ".\{lang}\SerbianLatin.isl"
Name: "slovak";              MessagesFile: ".\{lang}\Slovak.isl"
Name: "slovenian";           MessagesFile: ".\{lang}\Slovenian.isl"
Name: "spanish";             MessagesFile: ".\{lang}\Spanish.isl"
Name: "swedish";             MessagesFile: ".\{lang}\Swedish.isl"
Name: "tatar";               MessagesFile: ".\{lang}\Tatar.isl"
Name: "thai";                MessagesFile: ".\{lang}\Thai.isl"
Name: "turkish";             MessagesFile: ".\{lang}\Turkish.isl"
Name: "ukrainian";           MessagesFile: ".\{lang}\Ukrainian.isl"
Name: "uzbek";               MessagesFile: ".\{lang}\Uzbek.isl"
Name: "valencian";           MessagesFile: ".\{lang}\Valencian.isl"
Name: "vietnamese";          MessagesFile: ".\{lang}\Vietnamese.isl"

[CustomMessages]
;此段条目在等号后面直接跟具体的值，不能加双引号
;English（默认语言）
english.messagebox_close_title              = {#MyAppName} Setup
english.messagebox_close_text               = Are you sure to abort {#MyAppName} setup?
english.init_setup_outdated_version_warning = You have already installed a newer version of {#MyAppName}, so you are not allowed to continue. Click <OK> to abort.
english.wizardform_title                    = {#MyAppName} V{#MyAppVersion} Setup
english.no_change_destdir_warning           = You are not allowed to change destination folder.
english.installing_label_text               = Installing
;简体中文
chinesesimplified.messagebox_close_title              = {#MyAppName} 安装
chinesesimplified.messagebox_close_text               = 您确定要退出“{#MyAppName}”安装程序吗？
chinesesimplified.init_setup_outdated_version_warning = 您已安装更新版本的“{#MyAppName}”，不允许使用旧版本替换新版本，请单击“确定”按钮退出此安装程序。
chinesesimplified.wizardform_title                    = {#MyAppName} V{#MyAppVersion} 安装
chinesesimplified.no_change_destdir_warning           = 软件已经安装，不允许更换目录。
chinesesimplified.installing_label_text               = 正在安装

[Files]
;包含所有临时资源文件
Source: ".\{tmp}\background_finish.png";        DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
Source: ".\{tmp}\background_installing.png";    DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
Source: ".\{tmp}\background_messagebox.png";    DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
Source: ".\{tmp}\background_welcome.png";       DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
Source: ".\{tmp}\background_welcome_more.png";  DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
Source: ".\{tmp}\botva2.dll";                   DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
Source: ".\{tmp}\button_browse.png";            DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
Source: ".\{tmp}\button_cancel.png";            DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
Source: ".\{tmp}\button_close.png";             DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
Source: ".\{tmp}\button_customize_setup.png";   DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
Source: ".\{tmp}\button_finish.png";            DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
Source: ".\{tmp}\button_license.png";           DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
Source: ".\{tmp}\button_minimize.png";          DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
Source: ".\{tmp}\button_ok.png";                DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
Source: ".\{tmp}\button_setup_or_next.png";     DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
Source: ".\{tmp}\button_uncustomize_setup.png"; DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
Source: ".\{tmp}\checkbox_license.png";         DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
#ifdef RegisteAssociations
Source: ".\{tmp}\checkbox_setdefault.png";      DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
#endif
Source: ".\{tmp}\InnoCallback.dll";             DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
Source: ".\{tmp}\progressbar_background.png";   DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
Source: ".\{tmp}\progressbar_foreground.png";   DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
#ifdef ShowSlidePictures
Source: ".\{tmp}\slides_picture_1.png";         DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
Source: ".\{tmp}\slides_picture_2.png";         DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
Source: ".\{tmp}\slides_picture_3.png";         DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
Source: ".\{tmp}\slides_picture_4.png";         DestDir: "{tmp}"; Flags: dontcopy solidbreak; Attribs: hidden system
#endif
;包含待打包项目的所有文件及文件夹
Source: ".\{#MyAppBinDir}\*";                   DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
#ifndef PortableBuild
#ifdef UseCustomUninstaller
#if FileExists({#CustomUninstaller})
Source: "{#CustomUninstaller}";                 DestDir: "{app}"; Flags: ignoreversion
#endif
#endif
#endif

#ifndef PortableBuild
[Dirs]
;创建一个隐藏的系统文件夹存放卸载程序
Name: "{app}\Uninstaller"; Attribs: hidden system
#endif

[Icons]
#ifdef x64
Name: "{commondesktop}\{#MyAppName} (64-bit)";   Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"
Name: "{commonstartmenu}\{#MyAppName} (64-bit)"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"
#else
Name: "{commondesktop}\{#MyAppName}";            Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"
Name: "{commonstartmenu}\{#MyAppName}";          Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"
#endif

[Run]
#ifdef x64
Filename: "{app}\vcredist_x64.exe";     Parameters: "/install /quiet /norestart"; Flags: 64bit skipifdoesntexist waituntilterminated
Filename: "{app}\SPlayerService64.exe"; Parameters: "-i";                         Flags: 64bit skipifdoesntexist waituntilterminated
#else
Filename: "{app}\vcredist_x86.exe";     Parameters: "/install /quiet /norestart"; Flags: 32bit skipifdoesntexist waituntilterminated
Filename: "{app}\SPlayerService.exe";   Parameters: "-i";                         Flags: 32bit skipifdoesntexist waituntilterminated
#endif

#ifdef RegisteAssociations
[UninstallRun]
;卸载时运行反注册程序
Filename: "{app}\{#MyAppExeName}";      Parameters: "--unregall";                 Flags: waituntilterminated skipifdoesntexist
#endif
#ifdef x64
Filename: "{app}\SPlayerService64.exe"; Parameters: "-u";                         Flags: 64bit skipifdoesntexist waituntilterminated
#else
Filename: "{app}\SPlayerService.exe";   Parameters: "-u";                         Flags: 32bit skipifdoesntexist waituntilterminated
#endif

[InstallDelete]
Type: files; Name: "{app}\*.md"
Type: filesandordirs; Name: "{app}\script-settings"
Type: filesandordirs; Name: "{app}\scripts"

#ifndef PortableBuild
[UninstallDelete]
;卸载时删除安装目录下的所有文件及文件夹
Type: filesandordirs; Name: "{app}"
#endif

#include ".\{code}\code.iss"
