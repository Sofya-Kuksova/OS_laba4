@echo off
setlocal

set R_COUNT=6
set W_COUNT=6
set LOG_DIR=%~dp0..\analysis\logs
set BIN_DIR=%~dp0..\build

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%"
del /F /Q "%LOG_DIR%\reader_*.log" 2>nul
del /F /Q "%LOG_DIR%\writer_*.log" 2>nul

echo Launching all readers and writers 
for /L %%i in (1,1,%R_COUNT%) do (
    start "" /d "%LOG_DIR%" "%BIN_DIR%\reader.exe" %%i
    start "" /d "%LOG_DIR%" "%BIN_DIR%\writer.exe" %%i
)

echo Done.
endlocal
