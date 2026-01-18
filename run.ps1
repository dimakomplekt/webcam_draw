# Добавляем OpenCV в PATH
$env:PATH += ";C:\opencv\build\x64\vc16\bin"

# Если папки build нет — создаём и конфигурируем проект
if (-not (Test-Path "build")) {
    Write-Host "build отсуствует  — создаем и запускаем сборку."
    mkdir build
    cd build
    cmake ..
}
else {
    cd build
}

# Собираем проект
cmake --build . --config Release

# Если сборка успешна — запускаем программу
if ($LASTEXITCODE -eq 0) {
    Write-Host "Запуск программы!"
    .\Release\OpenCV.exe
}
else {
    Write-Host "Ошибка сборки!"
}
