README
=====

Jambi - Lazy PE infector
------------------------

Jambi is a PE infector that will prepend a shellcode to a binary, thus preloading the shellcode.
At the execution stage, both shellcode and binary will be run simultaneously.


Compile with Cygwin
-------------------

The compiler used is x86_64-w64-mingw32-gcc (That is, migw32 with TARGET=HOST=amd64). Edit it to suits your needs.


Prefer running on cmd.exe
-------------------------

Due to Cygwin specificities, running the infected binary can sometimes fail on Cygwin.
If repeating the process 3 or 4 times always result in s failure, try cmd.exe.