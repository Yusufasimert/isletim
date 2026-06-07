#!/bin/bash

# =================================================================
# Contiki-NG OTA Project - Docker Çalıştırma Betiği (Linux/WSL2/macOS)
# =================================================================

echo "========================================================="
echo "  Contiki-NG OTA Project - Docker Başlatıcı"
echo "========================================================="

# X11 grafik arayüz yetkilendirmesi (Cooja GUI yönlendirmesi için)
if command -v xhost &> /dev/null; then
    echo "[X11] Grafik arayüz erişim izinleri yapılandırılıyor (xhost)..."
    xhost +local:docker > /dev/null 2>&1 || xhost + > /dev/null 2>&1
fi

# WSL2 (Windows Subsystem for Linux) tespiti
if grep -qEi "(Microsoft|WSL)" /proc/version &> /dev/null; then
    echo "[Sistem] WSL2 ortamı tespit edildi. GUI aktarımı WSLg ile yapılacaktır."
    if [ -z "$DISPLAY" ]; then
        export DISPLAY=:0
    fi
else
    # Normal Linux/macOS
    if [ -z "$DISPLAY" ]; then
        export DISPLAY=:0
    fi
fi

echo "[Docker] Konteyner başlatılıyor..."
echo "  --> Projeyi Derlemek İçin: cd examples/BIL304-OTA && make TARGET=z1"
echo "  --> Cooja Simülatörü İçin: cooja"
echo "  --> Konteynerden Çıkmak İçin: 'exit' yazın"
echo "---------------------------------------------------------"

# Konteyneri etkileşimli modda başlatır ve çıkıldığında otomatik temizler (--rm)
docker-compose run --rm contiki-ota
