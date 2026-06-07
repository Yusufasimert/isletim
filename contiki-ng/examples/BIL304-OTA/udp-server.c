#include "contiki.h"
#include "net/routing/routing.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-debug.h"
#include "net/ipv6/simple-udp.h"
#include "sys/node-id.h"
#include "lib/crc16.h"
#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"

#include <stdio.h>
#include <string.h>

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define PKT_TYPE_DATA    0x01
#define PKT_TYPE_ACK     0x02
#define PKT_TYPE_DONE    0x04

#define BLOCK_SIZE       64

typedef struct {
  uint8_t  pkt_type;
  uint16_t block_num;
  uint16_t data_len;
  uint16_t total_blocks;
  uint16_t crc16;
  uint8_t  data[BLOCK_SIZE];
} __attribute__((packed)) ota_header_t;

typedef struct {
  uint8_t  pkt_type;
  uint16_t block_num;
  uint8_t  status;
} __attribute__((packed)) ota_ack_t;

static struct simple_udp_connection udp_conn;
static uint16_t expected_block = 0;

PROCESS(udp_server_process, "UDP server process");
AUTOSTART_PROCESSES(&udp_server_process);

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
static void udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen) {
         
  if(datalen < 5) return;
  
  uint8_t pkt_type = data[0];
  
  if(pkt_type == PKT_TYPE_DATA) {
      ota_header_t *pkt = (ota_header_t *)data;
      uint16_t calc_crc = crc16_data(pkt->data, pkt->data_len, 0);
      
      ota_ack_t ack;
      ack.pkt_type = PKT_TYPE_ACK;
      ack.block_num = pkt->block_num;
      
      if(calc_crc == pkt->crc16) {
          ack.status = 0; // OK
          printf("Received valid block %u/%u\n", pkt->block_num, pkt->total_blocks);
          
          int fd;
          if (pkt->block_num == 0) {
              cfs_remove("new-fw.z1");
              expected_block = 0;
          }
          
          if (pkt->block_num == expected_block) {
              expected_block++;
          } else if (pkt->block_num > expected_block) {
              printf("Warning: Missing blocks! Expected %u, got %u\n", expected_block, pkt->block_num);
          }
          fd = cfs_open("new-fw.z1", CFS_WRITE | CFS_APPEND);
          if(fd >= 0) {
              cfs_seek(fd, pkt->block_num * BLOCK_SIZE, CFS_SEEK_SET);
              cfs_write(fd, pkt->data, pkt->data_len);
              cfs_close(fd);
          } else {
              printf("CFS Write Error!\n");
          }
      } else {
          ack.status = 1; // CRC Error
          printf("CRC Error in block %u\n", pkt->block_num);
      }
      
      simple_udp_sendto(&udp_conn, &ack, sizeof(ack), sender_addr);
  } 
  else if (pkt_type == PKT_TYPE_DONE) {
      ota_header_t *pkt = (ota_header_t *)data;
      uint16_t expected_total_crc = pkt->crc16;
      printf("Yüklenmeye hazır yeni firmware alımı tamamlandı.\n");
      printf("Bütünlük doğrulaması (Full Image Verification) başlatılıyor...\n");

      int fd = cfs_open("new-fw.z1", CFS_READ);
      if(fd >= 0) {
          uint16_t calculated_total_crc = 0;
          uint8_t buf[BLOCK_SIZE];
          int bytes_read;
          while((bytes_read = cfs_read(fd, buf, sizeof(buf))) > 0) {
              calculated_total_crc = crc16_data(buf, bytes_read, calculated_total_crc);
          }
          cfs_close(fd);

          if(calculated_total_crc == expected_total_crc) {
              printf("Tüm imaj doğrulandı! CRC Eşleşiyor: 0x%04X. Başarılı.\n", calculated_total_crc);
          } else {
              printf("İmaj doğrulama hatası! Beklenen: 0x%04X, Hesaplanan: 0x%04X\n", expected_total_crc, calculated_total_crc);
          }
      } else {
          printf("Dosya okuma hatası!\n");
      }
  }
}

/*
 * PROCESS_THREAD(udp_server_process, ev, data): Sunucu (Alıcı ve Ağ Kökü - Root) düğümünün çalışma bloğunu temsil eder.
 * Ne işe yarar:
 * - Düğüm 2 (gönderici) yanlışlıkla bu kodu çalıştırırsa engeller (PROCESS_EXIT).
 * - RPL (IPv6 Yönlendirme) ağının kökü (root) olarak kendini kaydettirir. Böylece diğer düğümler buna bağlanabilir.
 * - UDP sunucusunu 5678 portundan dinlemeye başlar (simple_udp_register) ve 8765 portundaki istemcilere açık olduğunu belirtir.
 * - İşlem bitene kadar (sonsuza dek) olaya (event) dayalı olarak bekler (PROCESS_WAIT_EVENT_UNTIL).
 */
PROCESS_THREAD(udp_server_process, ev, data) {
  PROCESS_BEGIN();

  if(node_id == 2) {
      printf("I am node 2 (sender), skipping server code.\n");
      PROCESS_EXIT();
  }

  NETSTACK_ROUTING.root_start();
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);
                      
  printf("UDP server started.\n");

  PROCESS_WAIT_EVENT_UNTIL(0);
  PROCESS_END();
}
