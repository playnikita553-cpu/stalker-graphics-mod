@echo off
chcp 65001 >nul
title S.T.A.L.K.E.R. Realistic Graphics Mod — Удаление
echo.
echo ================================================================
echo  S.T.A.L.K.E.R.: Тень Чернобыля — Realistic Graphics Mod
echo  Удаление мода
echo ================================================================
echo.

set "DEFAULT_PATH=C:\Program Files (x86)\Steam\steamapps\common\S.T.A.L.K.E.R. Shadow of Chernobyl"

if exist "%DEFAULT_PATH%\fsgame.ltx" (
    echo Найдена установка игры: %DEFAULT_PATH%
    set "GAME_PATH=%DEFAULT_PATH%"
) else (
    echo Введите путь к папке с игрой:
    set /p GAME_PATH="Путь: "
)

echo.
set /p CONFIRM="Вы уверены, что хотите удалить мод? (Y/N): "
if /i not "%CONFIRM%"=="Y" (
    echo Отменено.
    pause
    exit /b 0
)

echo.
echo Удаляю файлы мода...

:: Remove mod shader files
if exist "%GAME_PATH%\gamedata\shaders\r2\bloom_build.h" del "%GAME_PATH%\gamedata\shaders\r2\bloom_build.h"
if exist "%GAME_PATH%\gamedata\shaders\r2\bloom_filter.h" del "%GAME_PATH%\gamedata\shaders\r2\bloom_filter.h"
if exist "%GAME_PATH%\gamedata\shaders\r2\dof.h" del "%GAME_PATH%\gamedata\shaders\r2\dof.h"
if exist "%GAME_PATH%\gamedata\shaders\r2\sun_shafts.h" del "%GAME_PATH%\gamedata\shaders\r2\sun_shafts.h"
if exist "%GAME_PATH%\gamedata\shaders\r2\color_grading.h" del "%GAME_PATH%\gamedata\shaders\r2\color_grading.h"
if exist "%GAME_PATH%\gamedata\shaders\r2\enhanced_shadows.h" del "%GAME_PATH%\gamedata\shaders\r2\enhanced_shadows.h"
if exist "%GAME_PATH%\gamedata\shaders\r2\combine_postprocess.h" del "%GAME_PATH%\gamedata\shaders\r2\combine_postprocess.h"

:: Remove mod weather files
if exist "%GAME_PATH%\gamedata\config\environment\weathers\weather_default.ltx" del "%GAME_PATH%\gamedata\config\environment\weathers\weather_default.ltx"
if exist "%GAME_PATH%\gamedata\config\environment\weathers\weather_rain.ltx" del "%GAME_PATH%\gamedata\config\environment\weathers\weather_rain.ltx"
if exist "%GAME_PATH%\gamedata\config\environment\weathers\weather_foggy.ltx" del "%GAME_PATH%\gamedata\config\environment\weathers\weather_foggy.ltx"

:: Remove recommended settings
if exist "%GAME_PATH%\recommended_settings.ltx" del "%GAME_PATH%\recommended_settings.ltx"

:: Restore fsgame.ltx backup
if exist "%GAME_PATH%\fsgame.ltx.backup" (
    echo Восстанавливаю fsgame.ltx из резервной копии...
    copy /Y "%GAME_PATH%\fsgame.ltx.backup" "%GAME_PATH%\fsgame.ltx" >nul
    del "%GAME_PATH%\fsgame.ltx.backup"
) else (
    echo Резервная копия fsgame.ltx не найдена.
    echo Вручную измените $game_data$ = true на $game_data$ = false в fsgame.ltx
)

echo.
echo ================================================================
echo  Мод успешно удалён!
echo ================================================================
echo.
echo  Также рекомендуется проверить целостность файлов через Steam:
echo  ПКМ по игре - Свойства - Установленные файлы - Проверить целостность
echo.
pause
