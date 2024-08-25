$ErrorActionPreference = "Stop"
$WarningPreference = "SilentlyContinue"
$VerbosePreference = "Continue"
if (Test-Path -Path "output" -PathType Container) {
    Remove-Item -Path "output" -Recurse -Force
} else {
    New-Item -ItemType Directory -Path "output"
}
if (Test-Path -Path "build" -PathType Container) {
    Remove-Item -Path "build" -Recurse -Force
}
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=output
cmake --build build --parallel (Get-CimInstance -Class Win32_ComputerSystem).NumberOfLogicalProcessors --target install  --config Release