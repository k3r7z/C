# Build and link static library

+ Compile the code
```bash
gcc -c tui.c -o tui.o
```

(-c flag tells the compiler to not link)

+ Archive the object file into a static library

```bash
ar rcs libtui.a tui.o
```

The `ar` command creates the archive. The _lib_ prefix is mandatory
for Unix-like systems.

+ Compile main program and link the static library

```bash
gcc src/main.c -ITUI -L. -ltui -o main.o
```

The `-ITUI` adds the directory to the header search path so `#include
"tui.h"` works in the main program. You must provide the relative location depending where you're executing the compilation command, so the flag would end up in `-I../TUI`, or `-I../../TUI` for example.
The `-L.` flag tells the compiler
to look in the current directory (.) for library files. The same applies here regarding the relative directory of the library. And finally, the `-ltui` tells the compiler to link the library (note that
including the lib prefix is not needed).







