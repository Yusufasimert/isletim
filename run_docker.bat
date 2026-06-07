@echo off
cd /d "%~dp0"
rem =================================================================
rem Contiki-NG OTA Project - Windows Docker Çalıştırma Betiği
rem =================================================================

echo =========================================================
echo   Contiki-NG OTA Project - Windows Docker Baslatici
echo =========================================================
echo.
echo [GRAFIK ARAYUZ BILGISI]
echo Cooja GUI simulatorunu Windows uzerinde goruntuleyebilmek icin 
echo VcXsrv, Xming veya MobaXterm gibi bir X11 sunucusunun Windows
echo arka planinda calisiyor olmasi gerekmektedir.
echo.
echo * Oneri: VcXsrv (Xlaunch) baslatirken "Multiple windows", 
echo          "Start no client", "Disable access control" (Kritik!) 
echo          seceneklerini isaretleyerek calistirin.
echo.
echo ---------------------------------------------------------
echo [Docker] Konteyner baslatiliyor...
echo   --^> Projeyi derlemek icin: cd examples/BIL304-OTA ^&^& make TARGET=z1
echo   --^> Cooja Simulatoru icin: cooja
echo   --^> Konteynerden cikmak icin: 'exit' yazin
echo ---------------------------------------------------------
echo.

docker-compose run --rm contiki-ota

if %errorlevel% neq 0 (
    echo.
    echo [HATA] Konteyner baslatilamadi. Docker Desktop'in calistigindan emin olun.
)

pause
