#pragma once
#include <lwip/sockets.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <esp_timer.h>
#include "esphome/core/log.h"

inline void send_wiz_udp(const char* ip, const char* payload) {
  int64_t t_start = esp_timer_get_time() / 1000;

  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 0) {
    ESP_LOGE("wiz_udp", "[%lld ms] Socket creation failed: errno %d", t_start, errno);
    return;
  }

  struct sockaddr_in dest_addr;
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(38899);
  if (inet_pton(AF_INET, ip, &dest_addr.sin_addr.s_addr) <= 0) {
    ESP_LOGE("wiz_udp", "[%lld ms] Invalid IP address: %s", t_start, ip);
    close(sock);
    return;
  }

  int sent1 = sendto(sock, payload, strlen(payload), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
  int sent2 = sendto(sock, payload, strlen(payload), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));

  close(sock);
  int64_t t_end = esp_timer_get_time() / 1000;

  if (sent1 < 0 || sent2 < 0) {
    ESP_LOGE("wiz_udp", "[%lld ms] UDP sendto failed to %s: errno %d", t_end, ip, errno);
  } else {
    ESP_LOGI("wiz_udp", "[%lld ms] Sent %d bytes to %s in %lld ms: %s",
             t_end, sent1, ip, (t_end - t_start), payload);
  }
}
