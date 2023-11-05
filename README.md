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
## 0 compile
```
cmake -S. -Bbuild
cmake --build build
```
#### 1.libuv __(recommended)__
run ```./build/libuv/main```
#### 2.Boost
run ```./build/libuv/srv```

## 1 run
open [127.0.0.1:8080/index.html](http://127.0.0.1:8080/index.html) in your browse
