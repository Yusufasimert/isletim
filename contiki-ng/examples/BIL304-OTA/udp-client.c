#include "contiki.h"
#include "net/routing/routing.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-debug.h"
#include "net/ipv6/simple-udp.h"
#include "sys/node-id.h"
#include "lib/crc16.h"
#include "firmware_data.h"

#include <stdio.h>
#include <string.h>

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define PKT_TYPE_DATA    0x01
#define PKT_TYPE_ACK     0x02
#define PKT_TYPE_DONE    0x04

#define BLOCK_SIZE       64
#define MAX_RETRIES      5

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
static ota_header_t current_pkt;
static uint16_t current_block = 0;
static uint16_t total_blocks = 0;
static uint8_t retries = 0;
static bool ack_received = false;

PROCESS(udp_client_process, "UDP client process");
AUTOSTART_PROCESSES(&udp_client_process);

/*
 * udp_rx_callback: UDP üzerinden paket (örneğin ACK, NACK) alındığında çalışan geri çağırma (callback) fonksiyonudur.
 * Ne işe yarar:
 * - Gelen paketin boyutunun beklenen ota_ack_t boyutu ile uyumlu olup olmadığını kontrol eder.
 * - Gelen veriyi (data) ota_ack_t yapısına çevirir.
 * - Paketin bir "ACK" olup olmadığını, beklenen blok sırasını taşıyıp taşımadığını ve hata (status) olup olmadığını sınar.
 * - Başarılıysa ack_received bayrağını true yaparak bir sonraki bloğa geçilmesine izin verir.
 */
static void udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen) {
  if(datalen == sizeof(ota_ack_t)) {
    ota_ack_t *ack = (ota_ack_t *)data;
    if(ack->pkt_type == PKT_TYPE_ACK && ack->block_num == current_block && ack->status == 0) {
      ack_received = true;
      printf("ACK received for block %u\n", ack->block_num);
    } else {
      printf("NACK or invalid ACK received for block %u\n", ack->block_num);
    }
  }
}

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
PROCESS_THREAD(udp_client_process, ev, data) {
  static struct etimer periodic_timer;
  static uip_ipaddr_t dest_ipaddr;

  PROCESS_BEGIN();

  if(node_id != 2) {
      printf("I am node %u, not sender (2). Exiting.\n", node_id);
      PROCESS_EXIT();
  }

  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  total_blocks = (sizeof(firmware_data) + BLOCK_SIZE - 1) / BLOCK_SIZE;
  printf("Starting OTA. Total size: %u, Blocks: %u\n", (unsigned int)sizeof(firmware_data), total_blocks);

  etimer_set(&periodic_timer, CLOCK_SECOND * 5);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

  while(current_block < total_blocks) {
    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
        // Root is reachable
    } else {
        printf("Root not reachable yet. Retrying...\n");
        etimer_set(&periodic_timer, CLOCK_SECOND * 2);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
        continue;
    }

    static uint16_t offset;
    static uint16_t len;
    offset = current_block * BLOCK_SIZE;
    len = BLOCK_SIZE;
    if(offset + len > sizeof(firmware_data)) {
        len = sizeof(firmware_data) - offset;
    }

    current_pkt.pkt_type = PKT_TYPE_DATA;
    current_pkt.block_num = current_block;
    current_pkt.data_len = len;
    current_pkt.total_blocks = total_blocks;
    memcpy(current_pkt.data, &firmware_data[offset], len);
    current_pkt.crc16 = crc16_data(current_pkt.data, len, 0);

    retries = 0;
    ack_received = false;

    while(retries < MAX_RETRIES && !ack_received) {
        printf("Sending block %u/%u (len %u, try %u)...\n", current_block, total_blocks, len, retries+1);
        simple_udp_sendto(&udp_conn, &current_pkt, sizeof(ota_header_t) - BLOCK_SIZE + len, &dest_ipaddr);
        
        etimer_set(&periodic_timer, CLOCK_SECOND * 2);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
        if(!ack_received) retries++;
    }

    if(!ack_received) {
        printf("Failed to send block %u. Aborting.\n", current_block);
        PROCESS_EXIT();
    }
    current_block++;
  }

  current_pkt.pkt_type = PKT_TYPE_DONE;
  current_pkt.data_len = 0;
  current_pkt.crc16 = crc16_data(firmware_data, sizeof(firmware_data), 0);
  simple_udp_sendto(&udp_conn, &current_pkt, sizeof(ota_header_t) - BLOCK_SIZE, &dest_ipaddr);
  printf("All blocks sent. DONE packet sent.\n");

  PROCESS_END();
}
