TARGET = opengw
TYPE = ps-exe

SRCS = \
src/main.cpp \
src/mathutils.cpp \
src/render.cpp \
src/grid.cpp \
src/model.cpp \
src/models.cpp \
src/entity.cpp \
src/player.cpp \
src/controls.cpp \
src/world.cpp \
src/enemies.cpp \
src/spawner.cpp \
src/game.cpp \
src/particles.cpp \
src/font.cpp \
src/font_data.cpp \
src/bomb.cpp \
src/pointdisplay.cpp \

CXXFLAGS = -std=c++20 -Isrc

include third_party/nugget/psyqo/psyqo.mk
