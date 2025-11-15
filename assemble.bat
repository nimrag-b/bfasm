@echo off
set INC=%INCLUDE%
set INCLUDE=fasm\INCLUDE\;%INCLUDE%
fasm\FASM.EXE %1 %2 %3 %4 %5 %6 %7 %8 %9
set INCLUDE=%INC%
set INC=    