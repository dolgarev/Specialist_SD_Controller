@echo off
cls
del boot.bin >nul
del bios.bin >nul
tasm -gb -b -85 boot.asm boot.bin >errors.txt
if errorlevel 1 goto err
type errors.txt
tasm -gb -b -85 s.asm s.bin >errors.txt
if errorlevel 1 goto err
type errors.txt
tasm -gb -b -85 e.asm e.bin >errors.txt
if errorlevel 1 goto err
type errors.txt
del errors.txt >nul
copy s.bin+boot.bin+e.bin boot.rks
rem copy boot.rks \\192.168.1.105\g\boot\boot.rks
if errorlevel 1 goto err
exit
:err
type errors.txt 
pause