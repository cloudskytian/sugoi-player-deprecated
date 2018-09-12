#ifndef SUGOIENGINE_H
#define SUGOIENGINE_H

#include <QObject>
#include <QHash>

class QTranslator;

class MainWindow;
class MpvWidget;
class OverlayHandler;
class DimDialog;

class SugoiEngine : public QObject
{
    Q_OBJECT
public:
    explicit SugoiEngine(QObject *parent = nullptr);
    ~SugoiEngine() override;

    MainWindow     *window = nullptr;
    MpvWidget      *mpv = nullptr;
    OverlayHandler *overlay = nullptr;
    DimDialog      *dimDialog = nullptr;

    QTranslator     *translator = nullptr;
    QTranslator     *qtTranslator = nullptr;

    // input hash-table provides O(1) input-command lookups
    QHash<QString, QPair<QString, QString>> input; // [shortcut] = QPair<command, comment>

    // the following are the default input bindings
    // they are loaded into the input before parsing the settings file
    // when saving the settings file we don't save things that appear here
    const QHash<QString, QPair<QString, QString>> default_input = {
        {"Ctrl++",          {"mpv add sub-scale +0.1",              tr("Increase sub size")}},
        {"Ctrl+-",          {"mpv add sub-scale -0.1",              tr("Decrease sub size")}},
        {"Ctrl+W",          {"mpv cycle sub-visibility",            tr("Toggle subtitle visibility")}},
        {"Ctrl+R",          {"mpv set time-pos 0",                  tr("Restart playback")}},
        {"PgDown",          {"mpv add chapter +1",                  tr("Go to next chapter")}},
        {"PgUp",            {"mpv add chapter -1",                  tr("Go to previous chapter")}},
        {"Right",           {"mpv seek +5",                         tr("Seek forwards by %0 sec").arg("5")}},
        {"Left",            {"mpv seek -5",                         tr("Seek backwards by %0 sec").arg("5")}},
        {"Shift+Left",      {"mpv frame_back_step",                 tr("Frame step backwards")}},
        {"Shift+Right",     {"mpv frame_step",                      tr("Frame step")}},
        {"Ctrl+M",          {"mute",                                tr("Toggle mute audio")}},
        {"Ctrl+T",          {"screenshot subtitles",                tr("Take screenshot with subtitles")}},
        {"Ctrl+Shift+T",    {"screenshot",                          tr("Take screenshot without subtitles")}},
        {"Ctrl+Down",       {"volume -5",                           tr("Decrease volume")}},
        {"Ctrl+Up",         {"volume +5",                           tr("Increase volume")}},
        {"Ctrl+Shift+Up",   {"speed +0.1",                          tr("Increase playback speed by %0").arg("10")}},
        {"Ctrl+Shift+Down", {"speed -0.1",                          tr("Decrease playback speed by %0").arg("10")}},
        {"Ctrl+Shift+R",    {"speed 1.0",                           tr("Reset speed")}},
        {"Alt+Return",      {"fullscreen",                          tr("Toggle fullscreen")}},
        {"Ctrl+H",          {"hide_all_controls",                   tr("Toggle hide all controls mode")}},
        {"Ctrl+D",          {"dim",                                 tr("Dim lights")}},
        {"Ctrl+E",          {"show_in_folder",                      tr("Show the file in its folder")}},
        {"Tab",             {"media_info",                          tr("View media information")}},
        {"Ctrl+J",          {"jump",                                tr("Show jump to time dialog")}},
        {"Ctrl+N",          {"new",                                 tr("Open a new window")}},
        {"Ctrl+O",          {"open",                                tr("Show open file dialog")}},
        {"Ctrl+Q",          {"quit",                                tr("Quit")}},
        {"Ctrl+Right",      {"playlist play +1",                    tr("Play next file")}},
        {"Ctrl+Left",       {"playlist play -1",                    tr("Play previous file")}},
        {"Ctrl+S",          {"stop",                                tr("Stop playback")}},
        {"Ctrl+U",          {"open_location",                       tr("Show location dialog")}},
        {"Ctrl+V",          {"open_clipboard",                      tr("Open clipboard location")}},
        {"Ctrl+F",          {"playlist toggle",                     tr("Toggle playlist visibility")}},
        {"Ctrl+Z",          {"open_recent 0",                       tr("Open the last played file")}},
        {"Ctrl+`",          {"output",                              tr("Access command-line")}},
        {"F1",              {"online_help",                         tr("Launch online help")}},
        {"Space",           {"play_pause",                          tr("Play/Pause")}},
        {"Alt+1",           {"fitwindow",                           tr("Fit the window to the video")}},
        {"Alt+2",           {"fitwindow 50",                        tr("Fit window to %0%").arg("50")}},
        {"Alt+3",           {"fitwindow 75",                        tr("Fit window to %0%").arg("75")}},
        {"Alt+4",           {"fitwindow 100",                       tr("Fit window to %0%").arg("100")}},
        {"Alt+5",           {"fitwindow 150",                       tr("Fit window to %0%").arg("150")}},
        {"Alt+6",           {"fitwindow 200",                       tr("Fit window to %0%").arg("200")}},
        {"Esc",             {"boss",                                tr("Boss key")}},
        {"Down",            {"playlist select +1",                  tr("Select next file on playlist")}},
        {"Up",              {"playlist select -1",                  tr("Select previous file on playlist")}},
        {"Return",          {"playlist play",                       tr("Play selected file on playlist")}},
        {"Del",             {"playlist remove",                     tr("Remove selected file from playlist")}}
    };

public slots:
    void LoadSettings();
    void SaveSettings();

    void Command(const QString& command);

protected slots:
    // Utility functions
    void Print(const QString& what, const QString& who = QStringLiteral("Sugoi"));
    void PrintLn(const QString& what, const QString& who = QStringLiteral("Sugoi"));
    void InvalidCommand(const QString&);
    void InvalidParameter(const QString&);
    void RequiresParameters(const QString&);

signals:


private:
    // This is a Sugoi-command hashtable initialized below
    //  by using a hash-table -> function pointer we acheive O(1) function lookups
    // Format: void SugoiCommand(QStringList args)
    // See Sugoicommands.cpp for function definitions

    // todo: write advanced information about commands
    typedef void(SugoiEngine::*SugoiCommandFPtr)(QStringList&);
    const QHash<QString, QPair<SugoiCommandFPtr, QStringList>> SugoiCommandMap = {
        {"mpv", // command
         {&SugoiEngine::SugoiMpv,
          {
           // params     description
           QString(), tr("executes mpv command"),
           QString() // advanced
          }
         }
        },
        {"sh",
         {&SugoiEngine::SugoiSh,
          {
           QString(), tr("executes system shell command"),
           QString()
          }
         }
        },
        {"new",
         {&SugoiEngine::SugoiNew, // function pointer to command functionality
          {
           // params     description
           QString(), tr("creates a new instance of sugoi player"),
           QString()
          }
         }
        },
        {"open_location",
         {&SugoiEngine::SugoiOpenLocation,
          {
           QString(),
           tr("shows the open location dialog"),
           QString()
          }
         }
        },
        {"open_clipboard",
         {&SugoiEngine::SugoiOpenClipboard,
          {
           QString(),
           tr("opens the clipboard"),
           QString()
          }
         }
        },
        {"show_in_folder",
         {&SugoiEngine::SugoiShowInFolder,
          {
           QString(),
           tr("shows the current file in folder"),
           QString()
          }
         }
        },
        {"add_subtitles",
         {&SugoiEngine::SugoiAddSubtitles,
          {
           QString(),
           tr("add subtitles dialog"),
           QString()
          }
         }
        },
        {"add_audio",
         {&SugoiEngine::SugoiAddAudio,
          {
           QString(),
           tr("add audio track dialog"),
           QString()
          }
         }
        },
        {"screenshot",
         {&SugoiEngine::SugoiScreenshot,
          {
           tr("[subs]"),
           tr("take a screenshot (with subtitles if specified)"),
           QString()
          }
         }
        },
        {"media_info",
         {&SugoiEngine::SugoiMediaInfo,
          {
           QString(),
           tr("toggles media info display"),
           QString()
          }
         }
        },
        {"stop",
         {&SugoiEngine::SugoiStop,
          {
           QString(),
           tr("stops the current playback"),
           QString()
          }
         }
        },
        {"playlist",
         {&SugoiEngine::SugoiPlaylist,
          {
           "[...]",
           tr("playlist options"),
           QString()
          }
         }
        },
        {"jump",
         {&SugoiEngine::SugoiJump,
          {
           QString(),
           tr("opens jump dialog"),
           QString()
          }
         }
        },
        {"dim",
         {&SugoiEngine::SugoiDim,
          {
           QString(),
           tr("toggles dim desktop"),
           QString()
          }
         }
        },
        {"output",
         {&SugoiEngine::SugoiOutput,
          {
           QString(),
           tr("toggles output textbox"),
           QString()
          }
         }
        },
        {"preferences",
         {&SugoiEngine::SugoiPreferences,
          {
           QString(),
           tr("opens preferences dialog"),
           QString()
          }
         }
        },
        {"online_help",
         {&SugoiEngine::SugoiOnlineHelp,
          {
           QString(),
           tr("launches online help"),
           QString()
          }
         }
        },
        {"update",
         {&SugoiEngine::SugoiUpdate,
          {
           QString(),
           tr("opens the update dialog or updates youtube-dl"),
           QString()
          }
         }
        },
        {"open",
         {&SugoiEngine::SugoiOpen,
          {
           tr("[file]"),
           tr("opens the open file dialog or the file specified"),
           QString()
          }
         }
        },
        {"play_pause",
         {&SugoiEngine::SugoiPlayPause,
          {
           QString(),
           tr("toggle play/pause state"),
           QString()
          }
         }
        },
        {"fitwindow",
         {&SugoiEngine::SugoiFitWindow,
          {
           tr("[percent]"),
           tr("fit the window"),
           QString()
          }
         }
        },
        {"deinterlace",
         {&SugoiEngine::SugoiDeinterlace,
          {
           QString(),
           tr("toggle deinterlace"),
           QString()
          }
         }
        },
        {"interpolate",
         {&SugoiEngine::SugoiInterpolate,
          {
           QString(),
           tr("toggle motion interpolation"),
           QString()
          }
         }
        },
        {"mute",
         {&SugoiEngine::SugoiMute,
          {
           QString(),
           tr("mutes the audio"),
           QString()
          },
         }
        },
        {"volume",
         {&SugoiEngine::SugoiVolume,
          {
           tr("[level]"),
           tr("adjusts the volume"),
           QString()
          }
         }
        },
        {"speed",
         {&SugoiEngine::SugoiSpeed,
          {
           tr("[ratio]"),
           tr("adjusts the speed"),
           QString()
          }
         }
        },
        {"fullscreen",
         {&SugoiEngine::SugoiFullScreen,
          {
           QString(),
           tr("toggles fullscreen state"),
           QString()
          }
         }
        },
        {"hide_all_controls",
         {&SugoiEngine::SugoiHideAllControls,
          {
           QString(),
           tr("toggles hide all controls state"),
           QString()
          }
         }
        },
        {"boss",
         {&SugoiEngine::SugoiBoss,
          {
           QString(),
           tr("pause and hide the window"),
           QString()
          }
         }
        },
        {"clear",
         {&SugoiEngine::SugoiClear,
          {
           QString(),
           tr("clears the output textbox"),
           QString()
          }
         }
        },
        {"help",
         {&SugoiEngine::SugoiHelp,
          {
           tr("[command]"),
           tr("internal help menu"),
           QString()
          }
         }
        },
        {"bug_report",
         {&SugoiEngine::SugoiBugReport,
          {
           QString(),
           tr("report bugs to the developers"),
           QString()
          }
         }
        },
        {"sys_info",
         {&SugoiEngine::SugoiSysInfo,
          {
           QString(),
           tr("show system information"),
           QString()
          }
         }
        },
        {"about",
         {&SugoiEngine::SugoiAbout,
          {
           tr("[qt]"),
           tr("open about dialog"),
           QString()
          }
         }
        },
        {"msg_level",
         {&SugoiEngine::SugoiMsgLevel,
          {
           tr("[level]"),
           tr("set mpv msg-level"),
           QString()
          }
         }
        },
        {"quit",
         {&SugoiEngine::SugoiQuit,
          {
           QString(),
           tr("quit sugoi player"),
           QString()
          }
         }
        }
    };
    // Sugoi Command Functions
    void SugoiMpv(QStringList&);
    void SugoiSh(QStringList&);
    void SugoiNew(QStringList&);
    void SugoiOpenLocation(QStringList&);
    void SugoiOpenClipboard(QStringList&);
    void SugoiShowInFolder(QStringList&);
    void SugoiAddSubtitles(QStringList&);
    void SugoiAddAudio(QStringList&);
    void SugoiScreenshot(QStringList&);
    void SugoiMediaInfo(QStringList&);
    void SugoiStop(QStringList&);
    void SugoiPlaylist(QStringList&);
    void SugoiJump(QStringList&);
    void SugoiDim(QStringList&);
    void SugoiOutput(QStringList&);
    void SugoiPreferences(QStringList&);
    void SugoiOnlineHelp(QStringList&);
    void SugoiUpdate(QStringList&);
    void SugoiOpen(QStringList&);
    void SugoiPlayPause(QStringList&);
    void SugoiFitWindow(QStringList&);
    void SugoiAspect(QStringList&);
    void SugoiDeinterlace(QStringList&);
    void SugoiInterpolate(QStringList&);
    void SugoiMute(QStringList&);
    void SugoiVolume(QStringList&);
    void SugoiSpeed(QStringList&);
    void SugoiFullScreen(QStringList&);
    void SugoiHideAllControls(QStringList&);
    void SugoiBoss(QStringList&);
    void SugoiClear(QStringList&);
    void SugoiHelp(QStringList&);
    void SugoiAbout(QStringList&);
    void SugoiMsgLevel(QStringList&);
    void SugoiQuit(QStringList&);
    void SugoiBugReport(QStringList&);
    void SugoiSysInfo(QStringList&);
public:
    void Open();
    void OpenLocation();
    void Screenshot(bool subs);
    void MediaInfo(bool show);
    void PlayPause();
    void Jump();
    void FitWindow(int percent = 0, bool msg = true);
    void Dim(bool dim);
    void About(const QString& what = QString());
    void Quit();
};

#endif // SUGOIENGINE_H
