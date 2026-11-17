#pragma once

#include <lwip/sockets.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <esp_timer.h>
#include "esphome/core/component.h"
#include "esphome/core/log.h"

namespace esphome {
namespace wiz_udp {

static const char *const TAG = "wiz_udp";

class WizUdpComponent : public Component {
 public:
  void setup() override {
    this->sock_ = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (this->sock_ < 0) {
      ESP_LOGE(TAG, "Failed to create UDP socket: errno %d", errno);
    }
  }

  void send(const char *ip, const char *payload) {
    int64_t t_start = esp_timer_get_time() / 1000;

    if (this->sock_ < 0) {
      this->sock_ = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      if (this->sock_ < 0) {
        ESP_LOGE(TAG, "[%lld ms] Socket creation failed: errno %d", t_start, errno);
        return;
      }
    }

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(38899);
    if (::inet_pton(AF_INET, ip, &dest_addr.sin_addr.s_addr) <= 0) {
      ESP_LOGE(TAG, "[%lld ms] Invalid IP address: %s", t_start, ip);
      return;
    }

    int sent = ::sendto(this->sock_, payload, strlen(payload), 0,
                        (struct sockaddr *) &dest_addr, sizeof(dest_addr));
    int64_t t_end = esp_timer_get_time() / 1000;

    if (sent < 0) {
      ESP_LOGE(TAG, "[%lld ms] UDP sendto failed to %s: errno %d", t_end, ip, errno);
      ::close(this->sock_);
      this->sock_ = -1;
    } else {
      ESP_LOGD(TAG, "[%lld ms] Sent %d bytes to %s in %lld ms: %s",
               t_end, sent, ip, (t_end - t_start), payload);
    }
  }

 protected:
  int sock_{-1};
};

// Keep global helper so existing YAML lambdas calling send_wiz_udp() work unchanged
inline void send_wiz_udp(const char *ip, const char *payload) {
  static WizUdpComponent fallback;
  fallback.send(ip, payload);
}

}  // namespace wiz_udp
}  // namespace esphome

using esphome::wiz_udp::send_wiz_udp;
