#pragma once
#include <lwip/sockets.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

inline void send_wiz_udp(const char* ip, const char* payload) {
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 0) return;

  struct sockaddr_in dest_addr;
  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(38899);
  inet_pton(AF_INET, ip, &dest_addr.sin_addr.s_addr);

  sendto(sock, payload, strlen(payload), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
  sendto(sock, payload, strlen(payload), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));

  close(sock);
}
