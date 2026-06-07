# BİL304 İşletim Sistemleri Dönem Projesi - OTA (Over-The-Air) Firmware Güncelleme

## 1. Proje Ne İşe Yarıyor?
Bu proje, Contiki-NG işletim sistemi üzerinde çalışan Z1 mote'ları (düğümleri) arasında kablosuz ağ (OTA - Over-The-Air) üzerinden donanım yazılımı (firmware) güncellemesi yapılmasını simüle etmektedir.
- **Parçalama (Fragmentation):** Gönderici düğüm (Node 2), güncellenecek donanım yazılımı verisini (firmware) küçük paketlere (64 byte) böler.
- **Ağ Üzerinden Gönderim:** IPv6/RPL tabanlı bir kablosuz ağ üzerinden UDP protokolü kullanılarak alıcı düğüme (Node 1) gönderilir.
- **Hata Kontrolü (CRC):** Gönderilen her paket bir CRC-16 (Döngüsel Artıklık Denetimi) değeri ile korunur. Alıcı, paketi aldığında CRC'yi yeniden hesaplayarak veri bozulması olup olmadığını anlar. Eğer bozulma yoksa "ACK" (Onay), varsa "NACK" döner.
- **Kalıcı Depolama (CFS):** Alıcı, gelen her doğru paketi Contiki Dosya Sistemine (CFS) sırasıyla yazar.
- **Tüm İmaj Doğrulaması (Full Image Verification):** Dosyanın tamamı alıcıya ulaştığında, alıcı tüm dosyayı okuyarak genel bir CRC kontrolü daha yapar ve güncellemenin güvenilirliğini garanti altına alır.

---

## 2. Derlenmiş ELF Dosyasının Analizi (udp-server.z1)

Cooja simülatöründe çalıştırılmak üzere derlenen `.z1` uzantılı dosya aslında bir **ELF (Executable and Linkable Format)** dosyasıdır. Aşağıdaki bilgiler `msp430-readelf -h -S` komutuyla elde edilmiştir.

### 2.1. Dosyanın ELF Sınıfı, Mimarisi ve Giriş Adresi
- **ELF Sınıfı:** `ELF32` (32-bit format yapısında, adreslemeler genelde 16/32 bit aralığındadır).
- **Mimari (Machine):** `Texas Instruments msp430 microcontroller` (Z1 mote'larının kullandığı mikrodenetleyici mimarisi).
- **Giriş Adresi (Entry Point):** `0x3100`. (İşlemcinin reset sonrası veya programı başlatırken ilk komutu okuyacağı bellek adresi).

### 2.2. Temel Bölümler (Sections) ve Anlamları
Programın RAM ve ROM üzerindeki dağılımı ELF içindeki bölümler (sections) ile belirlenir:
- **`.text` (0x3100):** Programın çalıştırılabilir makine kodlarını (fonksiyonlar, döngüler vb.) içerir. Salt okunurdur ve genellikle Flash belleğe (ROM) yazılır.
- **`.rodata` (0xe8a8):** Salt okunur verileri (Read-Only Data) tutar. Programdaki sabit dizgiler (örneğin `printf` içindeki metinler) ve `const` değişkenler buradadır.
- **`.data` (0x1100):** Başlangıç değeri atanmış global ve statik değişkenleri tutar. Flash'ta yer kaplar ve sistem başlarken RAM'e kopyalanır.
- **`.bss` (0x125e):** Başlangıç değeri atanmamış (veya sıfır atanmış) global ve statik değişkenleri barındırır. Disk üzerinde yer kaplamaz (NOBITS), sadece çalışma anında RAM'de (0 ile doldurularak) yer ayrılır.
- **`.vectors` (0xffc0):** Mikrodenetleyicinin kesme (interrupt) vektör tablosunu içerir.

### 2.3. Kod ve Veri Boyutları (Size Bilgileri)
- **Kod Boyutu (`.text`):** ~47 KB (`0xb7a8` hex). Programın yazılım mantığının ne kadar yer tuttuğunu gösterir.
- **Salt Okunur Veri Boyutu (`.rodata`):** ~1.7 KB (`0x06cb`).
- **Veri (RAM) Kullanımı:** `.data` (350 byte) + `.bss` (~6 KB). Bu durum Z1 mote'un sınırlı RAM (8 KB) kapasitesinin ne kadarının çalışma anında global değişkenler (örneğin IPv6 tamponları, uip_buf) tarafından işgal edildiğini gösterir.

### 2.4. Sembol Tablosu (.symtab) ve Anlamlı Semboller
ELF dosyası `.symtab` adlı bir tablo barındırır. Bu tablo, koddaki fonksiyon isimlerini (örneğin `udp_rx_callback`, `main`, `simple_udp_sendto`) ve değişken adlarını bellek adresleriyle eşleştirir.
**Ne Anlama Gelir?** Sembol tablosu, işletim sisteminin veya hata ayıklayıcının (debugger) `0x4A12` gibi anlamsız bir adresin aslında `udp_rx_callback` fonksiyonuna ait olduğunu anlamasını sağlar. Cooja simülatörü de "Mote Output" ekranında bu semboller yardımıyla hangi fonksiyonun çalıştığını takip edebilir.

### 2.5. Kesme Vektörleri (.vectors) ve Başlangıç Adresi
MSP430 mimarisinde bellek sonundaki özel bir alan (`0xFFE0 - 0xFFFF` civarı) donanımsal kesmeler (zamanlayıcı, radyo sinyali gelmesi vb.) için ayrılır. `.vectors` bölümü bu işlevi görür.
Giriş adresi (`0x3100`), sistemin `Reset` vektörü tetiklendiğinde `main` veya başlatma kodlarına atlamak için kullandığı başlangıç konumudur.

### 2.6. Dosya Neden "Ham Binary (Raw)" Değil de "ELF Executable"?
Ham binary (`.bin`) dosyaları sadece ve sadece belleğe yazılacak olan baytları (0 ve 1'leri) sırasıyla içerir. Nerenin `.text`, nerenin `.data` olduğu belli değildir.
**ELF (Executable and Linkable Format)** formatı ise bir "zarf" gibidir:
1. Başlık bilgileri barındırır (Mimari MSP430'dur, giriş adresi 0x3100'dür).
2. Değişkenlerin, sabitlerin ve kodların bellekte tam olarak **nereye** yerleştirilmesi gerektiğini (`Addr`) söyler.
3. Hata ayıklama (`.debug`) ve sembol (`.symtab`) bilgilerini içerdiği için Cooja simülatörünün bellek izlemesi ve ağ analizi yapabilmesine olanak tanır.
Bu nedenlerle Contiki projelerinde, doğrudan donanıma atılmadan önce simülasyon ve debug kolaylığı için kodlar zengin yapıdaki **ELF** formatında derlenir.

---

## 3. Kod İçerisindeki Temel Fonksiyonlar ve Görevleri

Kullanıcının isteği üzerine, proje içerisindeki C dosyalarında kullanılan temel fonksiyonların her birinin ne işe yaradığı kendi bloklarının yanına/altına eklenmiştir.

### 3.1. İstemci (Gönderici) Kodları (`udp-client.c`)

```c
/*
 * udp_rx_callback: UDP üzerinden paket (örneğin ACK, NACK) alındığında çalışan geri çağırma (callback) fonksiyonudur.
 * Ne işe yarar:
 * - Gelen paketin boyutunun beklenen ota_ack_t boyutu ile uyumlu olup olmadığını kontrol eder.
 * - Gelen veriyi (data) ota_ack_t yapısına çevirir.
 * - Paketin bir "ACK" olup olmadığını, beklenen blok sırasını taşıyıp taşımadığını ve hata (status) olup olmadığını sınar.
 * - Başarılıysa ack_received bayrağını true yaparak bir sonraki bloğa geçilmesine izin verir.
 */
static void udp_rx_callback(struct simple_udp_connection *c, ...)
```

```c
/*
 * PROCESS_THREAD(udp_client_process, ev, data): İstemci düğümünün (Gönderici) ana çalışma bloğunu temsil eder.
 * Ne işe yarar:
 * - Düğüm numarasının (node_id) 2 olup olmadığını kontrol eder. 2 değilse gönderici değildir, işlemi sonlandırır.
 * - Sunucuya UDP bağlantısı açar (simple_udp_register).
 * - Gönderilecek donanım yazılımının (firmware) toplam kaç blok tutacağını hesaplar.
 * - Hedef düğümün (Root) ağda erişilebilir (reachable) olmasını bekler.
 * - Firmware verisini blok blok okur, veri uzunluğunu hesaplar.
 * - Gelen bloğun hata kontrolü için CRC (Cyclic Redundancy Check) değerini hesaplayıp pakete ekler.
 * - Bloğu ağa gönderir ve sunucudan ACK gelene kadar (maksimum MAX_RETRIES kez) tekrar dener.
 * - Tüm bloklar başarılı şekilde gittiğinde (current_block == total_blocks), "PKT_TYPE_DONE" isimli bitirme mesajını yollar ve tüm imajın doğrulanması için imajın tam CRC16 değerini sunucuya bildirir.
 */
PROCESS_THREAD(udp_client_process, ev, data) { ... }
```

### 3.2. Sunucu (Alıcı) Kodları (`udp-server.c`)

```c
/*
 * udp_rx_callback: Alıcı düğümde (Server) UDP üzerinden paket geldiğinde tetiklenen callback (geri çağırma) fonksiyonu.
 * Ne işe yarar:
 * - Gelen paketin tipini (DATA veya DONE) okur.
 * - PKT_TYPE_DATA ise:
 *    1. Gelen verinin CRC16 değerini yeniden hesaplar ve paketteki değerle karşılaştırır.
 *    2. Doğruysa, Contiki Dosya Sistemine (CFS) bu bloğu yazar. Eksik veya atlanmış blok var mı diye kontrol eder.
 *    3. Başarı durumunda karşı tarafa ACK, CRC hatalıysa NACK gönderir.
 * - PKT_TYPE_DONE ise:
 *    1. Gönderimin bittiği sinyalini alır.
 *    2. "Full Image Verification" (Tüm-İmaj Doğrulaması) sürecini başlatır.
 *    3. Dosya sistemine yazılan "new-fw.z1" isimli dosyayı baştan sona okuyarak toplam CRC16'yı hesaplar.
 *    4. Hesaplanan değer ile göndericinin gönderdiği orijinal imajın CRC'sini karşılaştırarak işlemin başarıyla bittiğini doğrular.
 */
static void udp_rx_callback(struct simple_udp_connection *c, ...)
```

```c
/*
 * PROCESS_THREAD(udp_server_process, ev, data): Sunucu (Alıcı ve Ağ Kökü - Root) düğümünün çalışma bloğunu temsil eder.
 * Ne işe yarar:
 * - Düğüm 2 (gönderici) yanlışlıkla bu kodu çalıştırırsa engeller (PROCESS_EXIT).
 * - RPL (IPv6 Yönlendirme) ağının kökü (root) olarak kendini kaydettirir. Böylece diğer düğümler buna bağlanabilir.
 * - UDP sunucusunu 5678 portundan dinlemeye başlar (simple_udp_register) ve 8765 portundaki istemcilere açık olduğunu belirtir.
 * - İşlem bitene kadar (sonsuza dek) olaya (event) dayalı olarak bekler (PROCESS_WAIT_EVENT_UNTIL).
 */
PROCESS_THREAD(udp_server_process, ev, data) { ... }
```
