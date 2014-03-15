qmake ..\MMSS.pro -config release
mingw32-make
mkdir MMSS

xcopy /Y /F release\MMSS.exe MMSS\
set QTDIR=C:\Qt\5.2.1\mingw48_32\bin
for %%i in (Qt5Core.dll;Qt5Gui.dll;Qt5Network.dll;Qt5Widgets.dll;icudt51.dll;icuin51.dll;icuuc51.dll;libgcc_s_dw2-1.dll;libstdc++-6.dll;libwinpthread-1.dll) do xcopy /Y /F %QTDIR%\%%i MMSS\

set QTPLUGIN=C:\Qt\5.2.1\mingw48_32\plugins
for %%i in (imageformats;platforms) do xcopy /Y /S /F /I %QTPLUGIN%\%%i MMSS\%%i

xcopy /Y ..\README.md MMSS\
move MMSS\README.md MMSS\README.txt
xcopy /Y ..\LICENSE MMSS\
move MMSS\LICENSE MMSS\LICENSE.txt

rem for %ixx in (Qt5Core.dll;Qt5Gui.dll;Qt5Network.dll;Qt5Widgets.dll;icudt51.dll;icuin51.dll;icuuc51.dll;libgcc_s_dw2-1.dll;libstdc++-6.dll;libwinpthead-1.dll) do echo %ixx

rem for %i in (Qt5Core.dll;Qt5Gui.dll) do echo %i
