# Fichier pour automake
INCLUDES= -I../include -I../resources/ -I/usr/include/ffmpeg `xslt-config --cflags`
bin_PROGRAMS = shotdetect
shotdetect_LDADD = -lgd -lz -lswscale -lavformat -lavcodec -lavutil
shotdetect_LDFLAGS = `xslt-config --libs`
shotdetect_SOURCES = \
graph.cpp \
film.cpp   \
image.cpp  \
xml.cpp \
shot.cpp \
main.cpp
