## Settings

On Windows, the config files are saved in the user-specific persistent application data directory (%APPDATA%\..\Local\wangwenx190\Sugoi Player) in an ini file `config.ini`.

    # Below are the keys and comments specify setting details
    {
      "autoFit": n,                # autoFit percentage (0 = no autofit)
      "debug": b,                  # debugging enabled (output box)
      "hideAllControls": b,        # enable/disable hide all control mode
      "hidePopup": b,              # hide tray icon notifications
      "input": {                   # input macros
        "Ctrl+Q": [                # key-binding
          "sugoi quit",             # sugoi command (for command information see Commands section)
          "Quit"                   # label
        ],
        ...
      },
      "lang": "",                  # the language used by the program (auto selects from locale)
      "maxRecent": n,              # the maximum files saved in recent
      "mpv": {                     # mpv specific options
        "screenshot-format": "",   # format of mpv's screenshots
        "speed": f                 # speed of the video playback
        "vf": ""                   # video filters
        "volume": n,               # volume
        "msg-level": s,            # set mpv message level
        ...
      },
      "onTop": "b",                # on top setting (always, never, or playing)
      "recent": [                  # recent file history
        {
          "path": "/file/path",
          "title": "The Title in case of Url!",
          "time": n
        },
        ...
      ],
      "remaining": b,              # display remaining time or duration time label
      "resume": b,                 # enable/disable auto resume from last time feature
      "screenshotDialog": b,       # always show the screenshot dialog when taking screenshots
      "splitter": n,               # the normal splitter position (playlist size)
      "trayIcon": b,               # should we display the trayIcon
    }

Note that the `[mpv]` section is using mpv's options. See mpv's manual for a list of valid options.
