@echo off

:::::::: get git commit hash ::::::::
for /F "tokens=* USEBACKQ" %%a in (`git log -1 --format^="%%h"`) do (set revision=%%a)
:::::::::::::::::::::::::::::::::::::

copy .\Objects\NBD_OSD.hex .\Objects\osd_vtx_nbd_%revision%.hex
copy .\Objects\NBD_OSD.bin .\Objects\osd_vtx_nbd_%revision%.bin
