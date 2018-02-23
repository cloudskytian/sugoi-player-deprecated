TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS += \
    src/iconlib \
    src/guard \
    src/player

player.depends = iconlib guard
