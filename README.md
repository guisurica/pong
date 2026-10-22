emcc -o pong.html source/main.c -Os -Wall ./web/libraylib.a -I./include -L./web -lraylib -s USE_GLFW=3 -s ASYNCIFY -DPLATFORM_WEB
