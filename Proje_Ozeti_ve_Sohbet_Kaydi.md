# BİL 304 - İşletim Sistemleri OTA Projesi: Geliştirme ve Sohbet Kaydı Özeti
**Tarih:** 19 Mayıs 2026
**Proje Aşamaları:** Aşama 1 (OTA Geliştirme) ve Aşama 2 (ELF Analizi) Başarıyla Tamamlandı!

## 1. Neler Başardık?
Bu oturumda projenizin en zorlu kısımları olan C programlama, ağ haberleşmesi ve sistem konfigürasyonu gereksinimleri sıfırdan kurularak çalışır duruma getirildi.
* **Altyapı Kuruldu:** WSL ve Docker üzerinden resmi `contiker/contiki-ng` imajı kullanılarak bağımlılık ve derleyici (`msp430-gcc`) sorunları tamamen aşıldı. 
* **Kodlama (Aşama 1):** OTA (Over The Air) bellenim gönderimi için `udp-client.c` (gönderici) ve `udp-server.c` (alıcı) kodları yazıldı. Stop-and-wait, zaman aşımı ve CRC16 kontrolü mekanizmaları kodlandı.
* **Bug Fix (Hata Çözümleri):** 
  * Derlemede çıkan protothread (hafif iplik) bellek tahsisi hatası yerel değişkenler `static` yapılarak çözüldü.
  * Node 1'in (Alıcı) simülasyonda uyanmasını engelleyen ağır `cfs_coffee_format()` komutu kaldırılarak sistem stabilize edildi.
  * Node 2'nin hedefi bulamaması sorunu, manuel IP yerine `NETSTACK_ROUTING.get_root_ipaddr()` (RPL Mesh Ağı Dinamik Yönlendirme) algoritmasına geçilerek çözüldü.
* **Simülasyon Başarısı:** Cooja'da çalıştırılan simülasyon 1. dakika 10. saniyede ağı kurarak blokları hatasız aktardı ve CFS diske yazdı. 

## 2. Üretilen Proje Dosyaları Nerede?
* **Kaynak Kodlar:** `C:\Users\yasimert\Desktop\isletim\contiki-ng\examples\BIL304-OTA` klasöründedir.
* **Cooja Simülasyonu:** Aynı klasördeki `BIL304-OS-Project-1.csc` dosyasıdır.
* **Aşama 1 ve 2 Raporları (Artifacts):**
  Sistem tarafından oluşturulmuş olan detaylı raporlar Antigravity sistem dizininde kayıtlıdır, doğrudan proje raporunuza kopyalayabilirsiniz.
  1. [Asama1_Gelistirme_Raporu.md](file:///C:/Users/yasimert/.gemini/antigravity/brain/a7febb83-4a70-47ea-9ab5-a38ce8cfbf0a/Asama1_Gelistirme_Raporu.md)
  2. [Asama2_Arastirma_Raporu.md](file:///C:/Users/yasimert/.gemini/antigravity/brain/a7febb83-4a70-47ea-9ab5-a38ce8cfbf0a/Asama2_Arastirma_Raporu.md)

## 3. Sonraki (Aşama 3) İçin Not
Aşama 3 olan TI CC1352R donanım uyarlama kısmı için kod yazılmayacak, sadece cihazın bellek haritası (Memory Map) ve Bootloader stratejisi (Flash/RAM kısıtları ve CCFG yönetimi) dokümante edilecektir. Donanım (fiziksel kart) bölümden alındığında veya dokümantasyon yazılmak istendiğinde sohbet penceresinden asistanınızla iletişime geçebilirsiniz.

*Not: Tüm proje sohbeti ve log kayıtları yapay zeka sisteminizin arayüz geçmişinde otomatik olarak kalıcıdır.*
