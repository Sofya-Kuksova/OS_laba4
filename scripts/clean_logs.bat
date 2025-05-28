@echo off
REM Удаляет все логи reader_*.log и writer_*.log из папки analysis
setlocal

REM Получаем абсолютный путь к analysis
set LOG_DIR=%~dp0..\analysis\logs

if not exist "%LOG_DIR%" (
    echo Directory "%LOG_DIR%" does not exist. Creating it...
    mkdir "%LOG_DIR%"
)

del /F /Q "%LOG_DIR%\reader_*.log" 2>nul
del /F /Q "%LOG_DIR%\writer_*.log" 2>nul

echo Done cleaning logs.
endlocal
