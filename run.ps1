$env:PATH += ";C:\opencv\build\x64\vc16\bin"

cmake --build build --config Release


if ($LASTEXITCODE -eq 0) {
    .\build\Release\OpenCV.exe
}