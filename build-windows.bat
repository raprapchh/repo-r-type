@echo off

:: Setup vcpkg if not present
if not exist vcpkg (
    echo Cloning vcpkg...
    git clone https://github.com/microsoft/vcpkg.git
    call .\vcpkg\bootstrap-vcpkg.bat
)

:: Configure
cmake -S . -B build ^
    -DCMAKE_TOOLCHAIN_FILE=%CD%\vcpkg\scripts\buildsystems\vcpkg.cmake ^
    -DCMAKE_BUILD_TYPE=Release

:: Build
cmake --build build --config Release

:: Copy assets to bin/windows
if not exist bin\windows\client mkdir bin\windows\client
if not exist bin\windows\server mkdir bin\windows\server
if not exist bin\windows\config mkdir bin\windows\config

xcopy /E /I /Y client\assets bin\windows\client\assets
xcopy /E /I /Y client\fonts bin\windows\client\fonts
xcopy /E /I /Y client\sprites bin\windows\client\sprites
xcopy /E /I /Y server\assets bin\windows\server\assets
xcopy /E /I /Y config bin\windows\config

:: Copy DLLs from vcpkg
echo Copying dependencies DLLs...
if exist build\vcpkg_installed\x64-windows\bin (
    copy /Y build\vcpkg_installed\x64-windows\bin\*.dll bin\windows\
)

:: Handle OpenAL specifically (rename soft_oal.dll to OpenAL32.dll if needed)
if exist bin\windows\soft_oal.dll (
    if not exist bin\windows\OpenAL32.dll (
        ren bin\windows\soft_oal.dll OpenAL32.dll
    )
)

echo Build complete. Binaries and dependencies are in bin/windows/
pause
