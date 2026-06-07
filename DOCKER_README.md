# Docker ile Contiki-NG OTA Projesi Kullanım Kılavuzu

Bu dizin, dönem ödevinizin (BİL 304 İşletim Sistemleri OTA Projesi) derleyici ve simülasyon araç zincirleriyle birlikte sıfır kurulumla, taşınabilir bir biçimde çalıştırılması ve paylaşılması için gereken Docker ortamını barındırmaktadır.

---

## 1. Sistem Gereksinimleri

* Bilgisayarınızda **Docker Desktop** (Windows/macOS) veya **Docker Engine** (Linux) kurulu ve çalışır durumda olmalıdır.
* Windows üzerinde **WSL2 (Windows Subsystem for Linux)** etkinleştirilmiş olması tavsiye edilir.

---

## 2. Hızlı Başlangıç (Konteyneri Çalıştırma)

İşletim sisteminize göre aşağıdaki başlatma betiklerini kullanabilirsiniz. Bu betikler, konteyneri grafik arayüz yönlendirmeleriyle birlikte etkileşimli modda başlatır:

* **Linux / macOS / WSL2:**
  Terminal açıp proje kök dizinindeyken aşağıdaki komutu çalıştırın:
  ```bash
  chmod +x run_docker.sh
  ./run_docker.sh
  ```
  *(WSL2 üzerinde WSLg kuruluysa Cooja arayüzü doğrudan Windows masaüstünüze gelecektir).*

* **Windows (Docker Desktop ile):**
  Proje kök dizininde bulunan **`run_docker.bat`** dosyasına çift tıklayın veya komut satırından çalıştırın.

Konteyner başarıyla başlatıldığında, sizi `/home/user/contiki-ng` dizininde karşılayan etkileşimli bir Bash kabuğu açılacaktır.

---

## 3. Cooja Simülatörü için Grafik Arayüz (GUI) Yönlendirmesi

Konteyner içinden `cooja` komutunu verdiğinizde Java tabanlı grafik ekranın bilgisayarınızda açılabilmesi için konak (host) sistemde bir **X11 Sunucusu** kurulu olmalıdır.

### Windows (VcXsrv / XLaunch Kullanımı)
Windows üzerinde grafik arayüzü almak için en kararlı araç **VcXsrv**'dir:
1. [VcXsrv'yi indirin ve kurun](https://sourceforge.net/projects/vcxsrv/).
2. Arama çubuğuna **XLaunch** yazıp çalıştırın.
3. Açılan sihirbazda şu ayarları seçin:
   * **Display settings:** *Multiple windows* -> Next
   * **Client startup:** *Start no client* -> Next
   * **Extra settings:** **"Disable access control"** seçeneğini mutlaka **işaretleyin** (Docker bağlantısına izin vermek için gereklidir) -> Next
   * Son ekranda *Save configuration* diyerek ayarlarınızı kaydedip *Finish*'e tıklayın.
4. XLaunch arka planda çalışırken `run_docker.bat` veya `run_docker.sh` ile konteyneri başlatın.

### macOS (XQuartz Kullanımı)
macOS üzerinde grafik arayüz almak için:
1. `brew install --cask xquartz` ile XQuartz'ı kurun.
2. XQuartz ayarlarından *"Allow connections from network clients"* seçeneğini etkinleştirin.
3. Terminalde `xhost + 127.0.0.1` komutunu çalıştırıp konteyneri `./run_docker.sh` ile açın.

---

## 4. Konteyner İçi Komutlar

Konteyner açıldıktan sonra aşağıdaki komutları kullanarak ödevinizi derleyebilir ve simüle edebilirsiniz:

### Projeyi Derleme
```bash
# Proje dizinine gidin
cd examples/BIL304-OTA

# MSP430 tabanlı z1 hedefi için derleme yapın
make TARGET=z1
```

### Simülasyonu (Cooja) Çalıştırma
Grafik arayüz üzerinden hazır simülasyonu açmak için:
```bash
cooja examples/BIL304-OTA/BIL304-OS-Project-1.csc
```

---

## 5. Projeyi Paylaşma ve Dağıtma

Bu Docker altyapısı sayesinde projenizi arkadaşlarınızla veya hocalarınızla iki farklı yolla paylaşabilirsiniz:

### Yöntem A: Kaynak Kod Arşivi (Önerilen & En Kolay)
Tüm proje klasörünü (Dockerfile, docker-compose.yml, run_docker betikleri ve contiki-ng klasörünü) bir `.zip` veya `.tar.gz` dosyası haline getirip paylaşın.
* Alıcı taraf sadece zip dosyasını açıp Docker çalışır durumdayken `./run_docker.sh` veya `run_docker.bat` betiğini çalıştıracaktır. Konteyner ilk açılışta imajı otomatik olarak derleyecek ve kodları kullanıma hazır hale getirecektir.

### Yöntem B: Hazır Docker İmajı Paylaşımı (Offline / İnternetsiz Ortam)
Eğer alıcı tarafın internet üzerinden `contiker/contiki-ng` imajını indirmesini istemiyorsanız, derlenmiş projenizi içeren hazır imajınızı bir dosyaya paketleyebilirsiniz:

1. **İmajı Kaydetme (Gönderici Tarafında):**
   ```bash
   docker save ota-project:latest | gzip > ota_projesi_docker.tar.gz
   ```
   *Oluşan `ota_projesi_docker.tar.gz` dosyasını (yaklaşık 1-1.5 GB boyutundadır) paylaşabilirsiniz.*

2. **İmajı Yükleme (Alıcı Tarafında):**
   Alıcı taraf terminalden veya komut satırından dosyanın olduğu dizinde şu komutu çalıştırır:
   ```bash
   docker load < ota_projesi_docker.tar.gz
   ```
   Yükleme tamamlandıktan sonra doğrudan `./run_docker.sh` veya `run_docker.bat` ile sistemi anında kullanmaya başlayabilirler.
