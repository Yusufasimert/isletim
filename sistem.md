# [cite_start]Ondokuz Mayıs Üniversitesi Bilgisayar Mühendisliği Bölümü [cite: 2]

## [cite_start]ÇALIŞMA ŞARTNAMESİ: CC1352R Platformlarında Contiki-NG Telsiz Donanım Yazılımının Güncellemesi [cite: 3, 4]
[cite_start]**Proje Kodu:** BİL 304-TW-OTA-2026 [cite: 6]
[cite_start]**Ders:** BİL 304 İşletim Sistemleri - Bahar 2025/2026 [cite: 9, 11, 12, 13]
[cite_start]**Konu Kapsamı:** OTA / ELF ANALİZİ / CC1352R [cite: 5]

[cite_start]Bu belge, Contiki-NG'de kontrollü telsiz firmware (bellenim) aktarım sistemi için dönem çalışması gereksinimlerini tanımlamaktadır[cite: 7]. [cite_start]Çalışma; parçalı firmware dağıtımı, sağlama fonksiyonu ve yeniden iletim stratejisi, alıcı düğümde kalıcı depolama, ELF denetimi ve TI CC1352R Sensortag/Launchpad için donanım uyarlama analizini kapsamaktadır[cite: 8].

---

### [cite_start]1. Geliştirme Süreci (OTA - Over The Air / Havadan Güncelleme) [cite: 14, 16]

[cite_start]Bu ödev, Telsiz Duyarga Ağları (Wireless Sensor Networks - WSNs) görevi icra eden gömülü sistem cihazları arasında güncellenmiş bir firmware (bellenim) dosyasının denetimli olarak gönderilmesi ve gönderim sonrası yeni firmware ile başlatılması (reboot) için gereken bir takım araştırma ve geliştirme süreçlerini içermektedir[cite: 15]. [cite_start]Cooja simülatörü kullanarak Contiki-NG işletim sistemi çalıştıran iki cihaz arasında, OTA (Over-the-Air) prensibiyle firmware aktarım mekanizması geliştirilmesini kapsamaktadır[cite: 17].

* [cite_start]**Gönderici düğüm:** Kendisine önceden verilmiş güncel firmware dosyasını parçalar halinde alıcı düğüme iletmelidir[cite: 18].
* [cite_start]**Alıcı düğüm:** Bu parçalı veriyi almalı, tutarlılık doğrulamasından geçirmeli, kendi kalıcı depolama alanına yazmalı ve kaydettiği firmware imajını daha sonra etkinleştirilebilecek bir güncelleme olarak birleştirip saklaması gerekmektedir[cite: 19].

[cite_start]Geliştirme sürecinde, gerçek donanımlardaki bootloader zincirinin birebir gerçeklenmesi beklenmemektedir[cite: 20]. [cite_start]Bunun yerine, Cooja ve Contiki-NG'nin elverdiği ölçüde bir kablosuz firmware gönderiminin denetimli ve kontrollü iletilmesi beklenmektedir[cite: 20]. [cite_start]Dolayısıyla geliştirme sürecinin temel odağı, yeni firmware imajının ağ üzerinden güvenilir biçimde iletilmesi, alıcı düğümde de dosya veya flash benzeri kalıcı bir alana yazılması, imajın bütünlük kontrolünün yapılması yeterlidir[cite: 21].

#### [cite_start]Verilenler [cite: 22]
[cite_start]Geliştirme için gereken şablon verilmiştir[cite: 23]. [cite_start]Sıkıştırılmış olarak verilen klasör açılıp makinenizdeki `contiki-ng/examples/` dizini altına kopyalanmalıdır[cite: 23]. Bu klasör altında:
* [cite_start]`udp-server.c`: Gönderici düğüm için başlangıç uygulama kodu [cite: 24]
* [cite_start]`udp-server.z1`: Gönderici düğüm için derlenmiş başlangıç uygulaması [cite: 25]
* [cite_start]`udp-client.c`: Alıcı düğüm için başlangıç uygulama kodu [cite: 26]
* [cite_start]`udp-client.z1`: Alıcı düğüm için derlenmiş başlangıç uygulaması [cite: 27]
* [cite_start]`new-firmware.z1`: Kablosuz olarak aktarılacak güncel firmware imajı [cite: 28]
* [cite_start]`BIL304-OS-Project-1.csc`: Cooja senaryosu ve simülasyon altyapısı [cite: 28]

[cite_start]Verilen `rpl-udp` uygulaması, başlangıç bir UDP haberleşme örneğidir[cite: 29]. [cite_start]Projede geliştirme sürecinde beklenen, bu iskeleti kullanarak kilobaytlar büyüklüğünde diskte yer kaplayan bir OTA firmware gönderme/alma ve saklama sistemine dönüştürülmesidir[cite: 29]. [cite_start]Gönderilecek `new-firmware.z1` dosyası ham veri değil, ELF biçiminde bir MSP430 sistemlerde koşulabilir bir dosyadır ve boyutu yaklaşık 129760 bayt düzeyindedir[cite: 30]. [cite_start]Dolayısıyla bu imaj tek paket halinde taşınamamaktadır; parçalama, yeniden gönderim ve bütünlük denetimi (checksum) zorunlu olarak ele alınmalıdır[cite: 31].

#### [cite_start]Problem Tanımı [cite: 32]
[cite_start]Cooja simülatörü açıldığında üst menüden File > Open Simulation > Browse menüsünden `BIL304-OS-Project-1.csc` hazırlanmış simülasyon dosyası seçildiğinde bir gönderici, bir komşu iletici ve bir de alıcı düğümlü senaryo otomatik olarak açılacaktır[cite: 33].
* [cite_start]**2 ve 3 ID değerli düğümlere:** `udp-client.z1` yüklenmiştir[cite: 34].
* [cite_start]**1 ID değerli düğüme:** `udp-server.z1` yüklenmiştir[cite: 35].

[cite_start]Varsayılan hali ile simülasyon çalıştırıldığında, 2 ID'li düğüm tarafından her adımda artan bir tamsayı değeri `tx_count`, "Merhaba" mesajı ile birleştirilip gönderilmektedir (Örn: "Merhaba 232")[cite: 35]. [cite_start]3 numaralı düğüm, 2 numaralı düğüm tarafından 1 numaralı düğüme gönderilmek istenen tüm mesajları ileten aracı komşu görevi görmektedir[cite: 36].

> [cite_start]**Soru:** 2 ve 3 numaralı düğüme aynı firmware yüklenmesine rağmen gönderim işlemi neden sadece 2 numaralı düğüm tarafından yapılmaktadır, 3 numaralı düğüm de mesaj göndermesi gerekmez mi? [cite: 37]
> [cite_start]**Cevap:** Her 2 cihaza da aynı kaynak kodunun derlenmiş hali yüklenmesine rağmen kod içerisinde `node-id` kütüphanesi ve `if` blokları kullanılarak; sistem, içerisine yüklendiği cihazın ID değerini donanıma sormakta, eğer bu değer "2" ise bir takım kod bloklarını, değilse bir başka kod bloğunu koşmaktadır[cite: 38, 39, 40]. [cite_start]`node-id` ve `if` blokları sayesinde her cihaz için farklı firmware hazırlama zahmetinden kurtulmuş olunmaktadır[cite: 41].

[cite_start]Gönderici düğüm (ID:2), `new-firmware.z1` dosyasını bloklara ayırarak alıcı düğüme göndermelidir[cite: 73]. [cite_start]Hali hazırda yapılan "Merhaba ..." dizge (string) mesajı gönderimi, bayt dizisi göndericisine dönüştürülmelidir[cite: 74]. [cite_start]Alıcı düğüm (ID:1), aldığı bayt bloklarını hatalı bit kontrolü yapıp, doğru sırayla birleştirerek kalıcı depolama alanına kaydetmelidir[cite: 75]. [cite_start]Ardından "Yüklenmeye hazır yeni firmware alımı tamamlandı." mesajı ekrana yazdırılmalıdır[cite: 76]. [cite_start]3 numaralı komşu düğüm için `udp-client.c` içerisinde herhangi bir geliştirmeye gerek yoktur, komşu rolünü icra edebilecek durumdadır[cite: 77].

[cite_start]Kişisel bilgisayarlardaki "disk" ifadesi, gömülü sistemlerde kalıcı saklama alanı (Flash, EEPROM vb.) olarak yorumlanmalıdır[cite: 78, 79]. [cite_start]Contiki-NG işletim sistemi tarafından sunulan dosya sistemi Coffee'dir[cite: 79]. [cite_start]İsteyenler, başka depolama yapıları, CFS arayüzü, harici flash benzetimi veya uygun gördükleri başka bir kalıcı saklama yaklaşımını da kullanabilirler[cite: 80].

#### Geliştirilecek Çözümün Sağlaması Gereken Temel Özellikler:
1. [cite_start]Gönderici düğüm, `new-firmware.z1` isimli imajı belleğe almalı veya uygun biçimde okumalıdır[cite: 105].
2. [cite_start]Firmware imajı, sabit boyutlu paketlere veya bloklara bölünmelidir[cite: 106].
3. [cite_start]Her blok için en azından blok numarası, veri uzunluğu ve gerekli kontrol bilgisi taşınmalıdır[cite: 107].
4. [cite_start]Alıcı düğüm, gelen blokları toplamalı ve kalıcı depolama alanına yazmalıdır[cite: 108].
5. [cite_start]Alıcı düğüm, varsa eksik veya bozuk blokları tespit edebilmelidir[cite: 109].
6. [cite_start]Hatalı iletim gerçekleşmişse, tespit sonrası ilgili bloğun yeniden gönderimi tasarlanmalıdır[cite: 110].
7. [cite_start]Aktarım sonunda da tüm imaj için bir bütünlük doğrulaması yapılmalıdır[cite: 111].
8. [cite_start]Alıcı düğüm, kendi diskine birleştirilmiş imajı kaydetmelidir[cite: 112].
9. [cite_start]Kayıt sonrası güncellemenin başarıyla alındığı ve saklandığı gösterilmelidir[cite: 113].
10. [cite_start]Cooja aracında alıcı düğüm tarafından, diskine sakladığı bu makine kodununun boot edilecek başlangıç adresi olarak kaydedilemeyeceği ve reboot edilemeyeceği için ödevin 1. aşamasında reboot beklenmemektedir[cite: 114, 115].

#### İş Parçacıkları:
1. [cite_start]**Parçalama:** Firmware imajının uygun büyüklükte bloklara ayrılması[cite: 117].
2. [cite_start]**Sıralama:** Her blok için blok numarası veya offset bilgisinin taşınması[cite: 118].
3. [cite_start]**Parça doğrulama:** Her blok için checksum, CRC veya benzeri bir doğrulama yapılması[cite: 119].
4. [cite_start]**Tüm-imaj doğrulama:** Aktarım sonunda tüm dosya için hash, CRC32 veya benzeri bir bütünlük mekanizması çalıştırılması[cite: 120].
5. [cite_start]**Yeniden gönderim:** Bozuk, kayıp veya zaman aşımına uğrayan blokların tekrar istenebilmesi[cite: 121].
6. [cite_start]**Tekrarlı gönderim veya pencereleme:** Stop-and-wait, sliding window, seçmeli tekrar gönderim veya benzeri bir stratejinin gerçeklenmesi[cite: 122].
7. [cite_start]**Durum yönetimi:** Alıcı düğüm, hangi blokların geldiğini ve hangilerinin eksik olduğunu izleyebilmesi[cite: 123].

> [cite_start]**Güncelleme (18.05.2026):** İmaj dosyasını hex formatında `.h` dosyasına kaydedip gönderebilirsiniz (Örnek firmware'i `contiki-ng/examples` klasörünüze çıkarınız ve `firmware_data.h` dosyasını inceleyiniz)[cite: 124, 125].

---

### [cite_start]2. Araştırma Süreci [cite: 126]

[cite_start]Projenin ikinci bölümü olan araştırma sürecinde; yüklenecek firmware dosyasının geliştirici araç zinciri (toolchain) kullanılarak analiz edilmesini ve Texas Instruments CC1352R Sensortag veya Launchpad SoC cihazlarda OTA'nın gerçeklenebilmesi için gereken yükleyici (loader) implementasyonu ve raporlanmasını kapsamaktadır[cite: 128].

#### [cite_start]ELF Analizi [cite: 129]
[cite_start]ELF analiz bölümü, farklı görevler için geliştirilmiş firmware imajlarının yapısal olarak icra edilmesinin yanında, bellek ve disk uzayına oturma stratejilerinin (linker analizi) anlaşılması ve izah edilmesini kapsamaktadır[cite: 130]. [cite_start]Öğrenciler farklı firmware dosyalarını analiz etmelidir[cite: 131].

Analizde en az aşağıdaki başlıklar yer almalıdır:
1. [cite_start]Dosyanın ELF sınıfı, mimarisi ve giriş adresi[cite: 133].
2. [cite_start]`.text`, `.data`, `.bss`, `.rodata`, `.vectors` gibi temel bölümlerin varlığı ve ifade ettiği anlam[cite: 134].
3. [cite_start]Kod ve veri boyutları (Bunların ne anlama geldiği)[cite: 135].
4. [cite_start]Sembol tablosu ve anlamlı semboller (Bunların ne anlama geldiği)[cite: 136].
5. [cite_start]Kesme vektörleri veya başlangıç adresi ile ilişkili bilgiler (Bunların ne anlama geldiği)[cite: 137].
6. [cite_start]Dosyanın neden "ham binary" değil de "ELF executable" olarak değerlendirildiği[cite: 138].

[cite_start]Öğrenciler kurulum sonrası gelen araçlardan uygun olanlarını kullanmalıdır; bu araç zincirleri `msp430` c derleyicisinin (msp430-gcc) de bulunduğu `bin/` dizini altındadır[cite: 139]:
* [cite_start]`file x-firmware.z1` [cite: 140]
* [cite_start]`msp430-readelf -h x-firmware.z1` [cite: 141]
* [cite_start]`msp430-readelf -S x-firmware.z1` [cite: 142]
* [cite_start]`msp430-readelf -l x-firmware.z1` [cite: 143, 144]
* [cite_start]`msp430-readelf -s x-firmware.z1` [cite: 145, 146]
* [cite_start]`msp430-objdump -x x-firmware.z1` [cite: 147, 148]
* [cite_start]`msp430-objdump -h x-firmware.z1` [cite: 149]
* [cite_start]`msp430-objdump -d x-firmware.z1` [cite: 150]
* [cite_start]`msp430-nm -n x-firmware.z1` [cite: 151]
* [cite_start]`msp430-size x-firmware.z1` [cite: 152]
* [cite_start]`msp430-strings x-firmware.z1` [cite: 153]
* [cite_start]Ve dizinde bulunan diğer araç zincirleri (`msp430-addr2line`, `msp430-ld`, `msp430-strip`, `msp430mcu-config`...)[cite: 154].

[cite_start]Rapor bölümünde yalnızca komut çıktısını kopyalamak yeterli değildir; araç zincirinin kullanım amacı ve çıktıların anlamı yorumlanmalı, firmware imajının rolüne ilişkin sonuçlar açıklanmalıdır[cite: 155]. [cite_start]Araç zinciri ile üretilen disk ve bellek uzayına ait sonuçlar, CC1352R SoC Teknik Dokümantasyonunda verilen disk ve bellek uzaylarına uygun olarak görselleştirilmelidir[cite: 156].

**Paylaşılan SoC Dokümanları:**
* [cite_start]`TI-Docs/CC1352R SimpleLink High-Performance Multi-Band Wireless MCU datasheet (Rev. I).pdf` [cite: 161]
* [cite_start]`TI-Docs/CC13x2, CC26x2 SimpleLink Wireless MCU Technical Reference Manual (Rev. G).pdf` [cite: 161]
* [cite_start]`TI-Docs/CC1352R SimpleLink™ Wireless MCU Device Revision E Silicon Errata (Rev. C).pdf` [cite: 161]

---

### [cite_start]3. CC1352R Gerçekleme ve Donanım Uyarlama Görevi [cite: 162]

[cite_start]Bu görevde OTA işleminin 3. bileşeni olan gerçek cihaz üzerinde, cihazın diskinde data olarak oturan yeni firmware ile reboot edilmesi problemini kapsamaktadır[cite: 163]. [cite_start]Bölüm envanterinde bulunan TI CC1352R LaunchPad ve Sensortag cihazlarında geliştirme yapılabilmesi için cihaza uygun araç zincirleri yüklenmelidir[cite: 164].

Ödevin ilk 2 iş parçacığı `msp430` çekirdek mimarisi üzerinde iken; 3. [cite_start]iş parçacığı ise CC1352R SoC çekirdeğini oluşturan ARM mimarisi üzerindedir[cite: 165, 166]. [cite_start]Dolayısıyla Contiki-NG işletim sistemini ARM platformlarında koşan programların üretilmesi için ARM araç zincirleri yüklenmelidir[cite: 167]. [cite_start]Bu iş parçacığı tek cihaz üzerinde çalışacak şekilde tasarlanmıştır ve reboot edilecek yeni firmware için gönderim işlemi yapılmayacaktır[cite: 171]. [cite_start]Tek cihaza eski firmware flash-lanırken, yeni firmware de cihazın diskine kaydedilecek data olarak yüklenmelidir[cite: 172].

#### [cite_start]CC1352R Launchpad Üzerinde Yeni Firmware ile Reboot İçin Örnek Araştırma Konuları: [cite: 174]
1. [cite_start]CC1352R'nin Flash, SRAM, cache/GPRAM ve ROM kapasitesi[cite: 175].
2. [cite_start]Uygulama kodunun hangi bellekte tutulduğu[cite: 176].
3. [cite_start]Flash aşamasında yeni imajın hangi fiziksel veya mantıksal bölgeye yerleştirileceği[cite: 177].
4. [cite_start]Güncelleme için tek imaj, çift imaj veya geçici staging alanı gerekip gerekmediği[cite: 178].
5. [cite_start]Boot sürecinde ROM bootloader ve CCFG alanının rolü[cite: 179].
6. [cite_start]Reset sonrası hangi koşullarda hangi imajın seçileceği[cite: 180].
7. [cite_start]Flash erase/write işlemlerinin neden dikkatli planlanması gerektiği[cite: 181].
8. [cite_start]RAM kısıtları nedeniyle aktarım tamponlarının nasıl boyutlandırılması gerektiği[cite: 182].

#### [cite_start]Dokümantasyonda Beklenen Çıktılar: [cite: 183]
1. [cite_start]Flash ve RAM kullanımını gösteren bir bellek yerleşim tablosu[cite: 184].
2. [cite_start]Boot zincirini gösteren kısa bir akış diyagramı[cite: 185].
3. [cite_start]CCFG alanının neden kritik olduğunu açıklayan kısa teknik not[cite: 186].
4. [cite_start]Diske 2. firmware kayıt için önerilen yerleşim stratejisi[cite: 187].
5. [cite_start]Eğer tam firmware değiştirme yapılacaksa, bunun neden bootloader gerektirdiğinin açıklanması[cite: 188].

#### [cite_start]Peşinden Gidilecek Sorular: [cite: 193]
* [cite_start]Uygulamanın çalışan ana imajı nereye yerleşecek? [cite: 194]
* [cite_start]Diske kaydedilecek yeni imaj hangi alana yazılacak? [cite: 195]
* [cite_start]Aynı anda iki tam imaj saklanabiliyor mu? [cite: 196]
* [cite_start]Sadece staging alanı varsa aktivasyon nasıl yapılacak? [cite: 197]
* [cite_start]Flash sektör silme ve yazma işlemleri mevcut çalışan imajı nasıl etkileyebilir? [cite: 198]

---

### [cite_start]Puanlama [cite: 199]
* [cite_start]**1- Geliştirme İş Parçacığı:** 0-50 puan [cite: 200]
* [cite_start]**2- Araştırma İş Parçacığı:** 0-30 puan [cite: 201]
* [cite_start]**3- CC1352R Gerçekleme ve Donanım Uyarlama İş Parçacığı:** 0-100 puan [cite: 202]

[cite_start]**Ödev Grupları Girişi:** [cite: 205]
* [cite_start]Gruplardaki üye sayısı en az 1, en fazla 4 olmalıdır[cite: 206].
* [cite_start]**Ödev grubu giriş son tarih: 3 Mayıs!** [cite: 206]

[cite_start]Başarılar, [cite: 207]
Doç. [cite_start]Dr. Sercan DEMİRCİ [cite: 208]
Arş. Gör. [cite_start]İsmail Hakkı TURAN [cite: 209]