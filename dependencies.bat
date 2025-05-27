@echo off
setlocal

:: === AGILITY SDK ===
set "ASDK_PACKAGE_NAME=Microsoft.Direct3D.D3D12"
set "ASDK_PACKAGE_VERSION=1.717.0-preview"
set "ASDK_DOWNLOAD_URL=https://www.nuget.org/api/v2/package/%ASDK_PACKAGE_NAME%/%ASDK_PACKAGE_VERSION%"
set "ASDK_OUTPUT_DIR=%~dp0nuget_packages\%ASDK_PACKAGE_NAME%"
set "ASDK_PACKAGE_NUPKG=%ASDK_OUTPUT_DIR%\%ASDK_PACKAGE_NAME%.%ASDK_PACKAGE_VERSION%.nupkg"
set "ASDK_PACKAGE_ZIP=%ASDK_OUTPUT_DIR%\%ASDK_PACKAGE_NAME%.%ASDK_PACKAGE_VERSION%.zip"
set "ASDK_EXTRACT_DIR=%ASDK_OUTPUT_DIR%/agility_package"

:: === Create output directory ===
if not exist "%ASDK_OUTPUT_DIR%" (
    mkdir "%ASDK_OUTPUT_DIR%"
)

:: === Download NuGet package ===
echo Downloading %PACKAGE_NAME% version %ASDK_PACKAGE_VERSION%...
powershell -Command "Invoke-WebRequest -Uri '%ASDK_DOWNLOAD_URL%' -OutFile '%ASDK_PACKAGE_NUPKG%'"

if not exist "%ASDK_PACKAGE_NUPKG%" (
    echo Failed to download package.
    exit /b 1
)

:: === Rename .nupkg to .zip ===
copy /Y "%ASDK_PACKAGE_NUPKG%" "%ASDK_PACKAGE_ZIP%" >nul

:: === Extract the ZIP archive ===
echo Extracting package...
powershell -Command "Expand-Archive -Path '%ASDK_PACKAGE_ZIP%' -DestinationPath '%ASDK_EXTRACT_DIR%' -Force"

:: === Copy the include files ===
set "TARGET_DIR=%~dp0\3rd\include"
if not exist "%TARGET_DIR%" (
    mkdir "%TARGET_DIR%"
)

set "SOURCE_INCLUDE=%ASDK_EXTRACT_DIR%\build\native\include\d3d12.h"
copy /Y "%SOURCE_INCLUDE%" "%TARGET_DIR%\"

set "SOURCE_INCLUDE=%ASDK_EXTRACT_DIR%\build\native\include\d3d12compatibility.h"
copy /Y "%SOURCE_INCLUDE%" "%TARGET_DIR%\"

set "SOURCE_INCLUDE=%ASDK_EXTRACT_DIR%\build\native\include\d3d12sdklayers.h"
copy /Y "%SOURCE_INCLUDE%" "%TARGET_DIR%\"

set "SOURCE_INCLUDE=%ASDK_EXTRACT_DIR%\build\native\include\d3d12shader.h"
copy /Y "%SOURCE_INCLUDE%" "%TARGET_DIR%\"

set "SOURCE_INCLUDE=%ASDK_EXTRACT_DIR%\build\native\include\d3d12video.h"
copy /Y "%SOURCE_INCLUDE%" "%TARGET_DIR%\"

set "SOURCE_INCLUDE=%ASDK_EXTRACT_DIR%\build\native\include\d3dcommon.h"
copy /Y "%SOURCE_INCLUDE%" "%TARGET_DIR%\"

set "SOURCE_INCLUDE=%ASDK_EXTRACT_DIR%\build\native\include\dxgiformat.h"
copy /Y "%SOURCE_INCLUDE%" "%TARGET_DIR%\"

:: === Copy the binary files ===
set "TARGET_DIR=%~dp0\3rd\bin\D3D12"
if not exist "%TARGET_DIR%" (
    mkdir "%TARGET_DIR%"
)
set "SOURCE_DLL=%ASDK_EXTRACT_DIR%\build\native\bin\x64\d3d12SDKLayers.dll"
copy /Y "%SOURCE_DLL%" "%TARGET_DIR%\"

set "SOURCE_DLL=%ASDK_EXTRACT_DIR%\build\native\bin\x64\D3D12Core.dll"
copy /Y "%SOURCE_DLL%" "%TARGET_DIR%\"

:: === DXC Compiler ===
set "DXC_PACKAGE_NAME=Microsoft.Direct3D.DXC"
set "DXC_PACKAGE_VERSION=1.8.2505.28"
set "DXC_DOWNLOAD_URL=https://www.nuget.org/api/v2/package/%DXC_PACKAGE_NAME%/%DXC_PACKAGE_VERSION%"
set "DXC_OUTPUT_DIR=%~dp0nuget_packages\%DXC_PACKAGE_NAME%"
set "DXC_PACKAGE_NUPKG=%DXC_OUTPUT_DIR%\%DXC_PACKAGE_NAME%.%DXC_PACKAGE_VERSION%.nupkg"
set "DXC_PACKAGE_ZIP=%DXC_OUTPUT_DIR%\%DXC_PACKAGE_NAME%.%DXC_PACKAGE_VERSION%.zip"
set "DXC_EXTRACT_DIR=%DXC_OUTPUT_DIR%/dxc_package"

:: === Create output directory ===
if not exist "%DXC_OUTPUT_DIR%" (
    mkdir "%DXC_OUTPUT_DIR%"
)

:: === Download NuGet package ===
echo Downloading %PACKAGE_NAME% version %DXC_PACKAGE_VERSION%...
powershell -Command "Invoke-WebRequest -Uri '%DXC_DOWNLOAD_URL%' -OutFile '%DXC_PACKAGE_NUPKG%'"

if not exist "%DXC_PACKAGE_NUPKG%" (
    echo Failed to download package.
    exit /b 1
)

:: === Rename .nupkg to .zip ===
copy /Y "%DXC_PACKAGE_NUPKG%" "%DXC_PACKAGE_ZIP%" >nul

:: === Extract the ZIP archive ===
echo Extracting package...
powershell -Command "Expand-Archive -Path '%DXC_PACKAGE_ZIP%' -DestinationPath '%DXC_EXTRACT_DIR%' -Force"

:: === Copy the binary files ===
set "TARGET_DIR=%~dp0\3rd\bin"
if not exist "%TARGET_DIR%" (
    mkdir "%TARGET_DIR%"
)
set "SOURCE_DLL=%DXC_EXTRACT_DIR%\build\native\bin\x64\dxcompiler.dll"
copy /Y "%SOURCE_DLL%" "%TARGET_DIR%\"

set "SOURCE_DLL=%DXC_EXTRACT_DIR%\build\native\bin\x64\dxil.dll"
copy /Y "%SOURCE_DLL%" "%TARGET_DIR%\"

:: === Copy the library files ===
set "TARGET_DIR=%~dp0\3rd\lib"
if not exist "%TARGET_DIR%" (
    mkdir "%TARGET_DIR%"
)
set "SOURCE_LIB=%DXC_EXTRACT_DIR%\build\native\lib\x64\dxcompiler.lib"
copy /Y "%SOURCE_LIB%" "%TARGET_DIR%\"

set "SOURCE_LIB=%DXC_EXTRACT_DIR%\build\native\lib\x64\dxil.lib"
copy /Y "%SOURCE_LIB%" "%TARGET_DIR%\"

set "SOURCE_FILE=%DXC_EXTRACT_DIR%\build\native\include\hlsl\dx\linalg.h"
copy /Y "%SOURCE_FILE%" "%~dp0\shaders\shader_lib\"

if exist "%~dp0nuget_packages%" (
    rmdir /s /q "%~dp0nuget_packages%"
)

echo Dependencies have been downloaded.
exit /b 0
