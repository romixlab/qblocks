QT       -= gui
QT       += quick widgets
CONFIG   += c++11
TARGET    = blocks
TEMPLATE  = lib
DEFINES  += BLOCKS_LIBRARY
VERSION   = 0.0.0

release: DESTDIR = $$TOP_SRCDIR/build/release
debug:   DESTDIR = $$TOP_SRCDIR/build/debug
OBJECTS_DIR = $$DESTDIR/obj
MOC_DIR = $$DESTDIR/moc

HEADERS += \
    blocks_global.h \
    block.h \
    blocksapplication.h \
    blockinfo.h \
    blockinfo_p.h \
    block_p.h

SOURCES += \
    block.cpp \
    blocksapplication.cpp \
    blockinfo.cpp


HEADERS_INSTALL = \
    blocks_global.h \
    block.h \
    blocksapplication.h \
    Block \
    Application

unix {
    header_files.files = $$HEADERS_INSTALL
    header_files.path = /usr/include/blocks
    target.path = /usr/lib
    INSTALLS += header_files target
}

