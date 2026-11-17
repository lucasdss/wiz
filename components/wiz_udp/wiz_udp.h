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
  static int sock = -1;
  int64_t t_start = esp_timer_get_time() / 1000;

  // Create the UDP socket once and reuse it to prevent lwIP ENOMEM (errno 12)
  if (sock < 0) {
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
      ESP_LOGE("wiz_udp", "[%lld ms] Socket creation failed: errno %d", t_start, errno);
      return;
    }
  }

  struct sockaddr_in dest_addr;
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(38899);
  if (inet_pton(AF_INET, ip, &dest_addr.sin_addr.s_addr) <= 0) {
    ESP_LOGE("wiz_udp", "[%lld ms] Invalid IP address: %s", t_start, ip);
    return;
  }

  int sent = sendto(sock, payload, strlen(payload), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
  int64_t t_end = esp_timer_get_time() / 1000;

  if (sent < 0) {
    ESP_LOGE("wiz_udp", "[%lld ms] UDP sendto failed to %s: errno %d", t_end, ip, errno);
    // Reset socket only if it entered a bad state
    close(sock);
    sock = -1;
  } else {
    ESP_LOGI("wiz_udp", "[%lld ms] Sent %d bytes to %s in %lld ms: %s",
             t_end, sent, ip, (t_end - t_start), payload);
  }
}
