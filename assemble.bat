@echo off
set INC=%INCLUDE%
set INCLUDE=fasmw17332\INCLUDE\;%INCLUDE%
fasmw17332\FASM.EXE %1 %2 %3 %4 %5 %6 %7 %8 %9
set INCLUDE=%INC%
set INC=    