@echo off
setlocal

set MODE=%1
if "%MODE%"=="" set MODE=setup

where dotnet >nul 2>nul
if errorlevel 1 (
    echo [Bun]  .NET 8 SDK not found.
    echo          Install it from https://dotnet.microsoft.com/download/dotnet/8.0
    exit /b 1
)

echo [Bun] Building build tool...
dotnet publish .\Build\BunBuildTool.csproj -c Release -r win-x64 --self-contained ^
    -p:PublishSingleFile=true -v q --nologo -o .\Build\bin\win-x64
if errorlevel 1 (
    echo [Bun] Failed to build the build tool.
    exit /b 1
)

.\Build\bin\win-x64\BunBuildTool.exe %MODE%