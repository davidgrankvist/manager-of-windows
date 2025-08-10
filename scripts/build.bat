@echo off

mkdir bin > NUL 2>&1

cl src\*.c ^
   /Fe: bin\mow.exe ^
   /Fo: bin\ ^
   /Fd: bin\ ^
   /Zi ^
   /I"src" ^
   /link ^
   	 user32.lib ^
   /nologo
