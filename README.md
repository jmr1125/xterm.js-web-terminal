# xterm.js-web-terminal
----
a terminal can be used in your browse
it includes ws server and HTTP server
using xterm.js and libuv or Boost
```
+------+   websocket     +-------+          +-----+
|browse| <===========>   |program| <======> | pty | <==> [real terminal]
+------+                 +-------+          +-----+
```
#0 compile
```
cmake -S. -Bbuild
cmake --build build
```
#### 1.libuv __(recommended)__
run ```./build/libuv/main```
#### 2.Boost
run ```./build/libuv/srv```
