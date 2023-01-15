A clean starting point for a GUI.

## Build

```
$ make -B
g++ -std=c++20 ... src/main.cpp -o build/main `pkg-config --libs sdl2`
```

## Run

```
$ ./build/main
```

- opens a transparent window
    - resize the window
    - notice how the center area changes with window size
- `q` to quit

## Develop

```
$ make tags         # Create a tags file for hopping and omnicompletion
$ make tags-stdout  # Print human readable tags to stdout
```

## Hints

Print hints on building, running, and generating tags:

```
$ make how

--- Build and Run ---

             Vim shortcut    Vim command line (approximate)
             ------------    ------------------------------
Build        ;<Space>        :make -B  <if(error)> :copen
Run          ;r<Space>       :!./build/main
Run in Vim   ;w<Space>       :!./build/main <args> &
Make tags    ;t<Space>       :make tags
Print tags   ;ts<Space>      :make tags-stdout
```
