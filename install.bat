@echo off
chcp 65001 >nul
title S.T.A.L.K.E.R. Realistic Graphics Mod — Установка
echo.
echo ================================================================
echo  S.T.A.L.K.E.R.: Тень Чернобыля — Realistic Graphics Mod
echo  Автоматическая установка
echo ================================================================
echo.

:: Default Steam path
set "DEFAULT_PATH=C:\Program Files (x86)\Steam\steamapps\common\S.T.A.L.K.E.R. Shadow of Chernobyl"

:: Check default path
if exist "%DEFAULT_PATH%\fsgame.ltx" (
    echo Найдена установка игры: %DEFAULT_PATH%
    echo.
    set /p CONFIRM="Установить мод сюда? (Y/N): "
    if /i "%CONFIRM%"=="Y" (
        set "GAME_PATH=%DEFAULT_PATH%"
        goto install
    )
)

:: Ask for custom path
echo.
echo Введите путь к папке с игрой S.T.A.L.K.E.R. Shadow of Chernobyl:
echo (Например: D:\Games\STALKER Shadow of Chernobyl)
echo.
set /p GAME_PATH="Путь: "

:: Validate path
if not exist "%GAME_PATH%\fsgame.ltx" (
    echo.
    echo ОШИБКА: Файл fsgame.ltx не найден в указанной папке!
    echo Убедитесь, что путь указывает на корневую директорию игры.
    echo.
    pause
    exit /b 1
)

:install
echo.
echo ================================================================
echo  Начинаю установку...
echo ================================================================
echo.

:: Backup fsgame.ltx
if not exist "%GAME_PATH%\fsgame.ltx.backup" (
    echo [1/4] Создаю резервную копию fsgame.ltx...
    copy "%GAME_PATH%\fsgame.ltx" "%GAME_PATH%\fsgame.ltx.backup" >nul
    echo       Резервная копия: fsgame.ltx.backup
) else (
    echo [1/4] Резервная копия fsgame.ltx уже существует, пропускаю...
)

:: Copy gamedata
echo [2/4] Копирую файлы мода...
xcopy /E /I /Y "%~dp0gamedata" "%GAME_PATH%\gamedata" >nul
echo       Файлы скопированы в %GAME_PATH%\gamedata

:: Modify fsgame.ltx
echo [3/4] Настраиваю fsgame.ltx...
powershell -Command "(Get-Content '%GAME_PATH%\fsgame.ltx') -replace '\$game_data\$\s*=\s*false', '$game_data$ = true' | Set-Content '%GAME_PATH%\fsgame.ltx'"
echo       fsgame.ltx обновлён: $game_data$ = true

:: Copy recommended settings info
echo [4/4] Копирую рекомендованные настройки...
copy "%~dp0recommended_settings.ltx" "%GAME_PATH%\recommended_settings.ltx" >nul
echo       Файл recommended_settings.ltx скопирован

echo.
echo ================================================================
echo  Установка завершена!
echo ================================================================
echo.
echo  Рекомендации:
echo  1. Запустите игру с рендерером R2 (DirectX 9 Enhanced)
echo  2. Скопируйте настройки из recommended_settings.ltx в user.ltx
echo     (Файл user.ltx находится в: %%USERPROFILE%%\Documents\stalker-shoc\)
echo  3. Для удаления мода удалите папку gamedata и восстановите
echo     fsgame.ltx из резервной копии (fsgame.ltx.backup)
echo.
pause
