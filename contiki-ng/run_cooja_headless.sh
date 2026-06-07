#!/bin/bash
# Xvfb (sanal ekran) ile Cooja headless calistirma betigi

echo "=== Sanal ekran baslatiliyor ==="
export DISPLAY=:99
Xvfb :99 -screen 0 1024x768x24 > /dev/null 2>&1 &
sleep 2
echo "=== Sanal ekran hazir (DISPLAY=:99) ==="

echo "=== Cooja simulasyonu baslatiliyor ==="
cd /home/user/contiki-ng/tools/cooja
sed -i 's/\r$//' gradlew 2>/dev/null || true
./gradlew run --args="--no-gui /home/user/contiki-ng/examples/BIL304-OTA/BIL304-OS-Project-1.csc" 2>&1
echo "=== Simulasyon tamamlandi ==="
