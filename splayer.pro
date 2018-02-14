TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS += \
    src/iconlib \
    src/splayer

splayer.depends = iconlib
