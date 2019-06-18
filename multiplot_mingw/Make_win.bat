@rem *******
@rem *******    change under to your own compiler path

@SET var=D:\mingw-w64-5.3.0\mingw32\

@rem *******    change above to your own compiler path
@rem *******

@echo off

@REM Set PATH's for this session.  

SET PATH=%var%bin;
SET MINGW_INCLUDE=%var%include
SET MINGW_LIB=%var%

cd /d %~dp0

echo.
echo  Path to compiler  : %var%
echo.
echo.

IF EXIST "makefile" ( goto :build)
echo.
echo.
echo   Not found Makefile!  Sorry nothing done. 
echo.
goto :end

:build
echo.
echo launching makefile 
echo.

mingw32-make  %1 %2 %3 %4 %5 %6 %7 %8 %9

echo.
echo    Done!
echo.
echo.

:end

pause