# ESPHome WiZ Direct UDP Component (`wiz_udp`)

Lightweight ESPHome external component for **ESP-IDF** (`esp32`, `esp32-c3`, `esp32-c6`, `esp32-s3`) that sends direct local UDP JSON packets (port `38899`) to Philips WiZ smart bulbs.

By triggering WiZ bulbs directly from the ESP32 on sensor state changes (such as an LD2450 mmWave radar), light activation drops to **~15–20ms** without waiting for a Home Assistant automation round-trip, while Home Assistant continues to track both device states normally.

---

## Repository Structure

```text
wiz/
├── README.md
└── components/
    └── wiz_udp/
        ├── __init__.py
        └── wiz_udp.h
```

### `components/wiz_udp/__init__.py`

```python
import esphome.codegen as cg
import esphome.config_validation as cv

CONFIG_SCHEMA = cv.Schema({})

async def to_code(config):
    cg.add_global(cg.RawStatement('#include "wiz_udp.h"'))
```

### `components/wiz_udp/wiz_udp.h`

```cpp
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

  // Send twice back-to-back to prevent occasional UDP packet drops
  sendto(sock, payload, strlen(payload), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
  sendto(sock, payload, strlen(payload), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));

  close(sock);
}
```

---

## Prerequisites & Low-Latency Setup

1. **Assign Static IPs (Router DHCP Reservation):**
   * Bind each WiZ bulb's MAC address to a fixed IP in your router's DHCP settings so the target IP never changes after a reboot or power outage.
   * *(Optional)* Assign a `manual_ip` to your ESP32 in ESPHome to skip DHCP discovery on reconnects.
2. **Enable Local Communication (WiZ V2 App):**
   * Open **WiZ V2** → **Settings** → **Security** → toggle **Allow local communication** **ON**.
3. **Set Fade-In / Fade-Out to `0s` (WiZ V2 App):**
   * Open **WiZ V2** → **Settings** → **Lights** → select your bulb → set **Fade-in** and **Fade-out** to **0s** so the bulb snaps on immediately instead of ramping brightness over 0.5–1s.
4. **Use ESP-IDF Framework:**
   * This component uses native `lwip/sockets.h` and requires `framework: type: esp-idf` in your ESPHome configuration.

---

## ESPHome Configuration Example

Add `external_components` and `wiz_udp:` to your ESPHome YAML, then call `send_wiz_udp(ip, json_payload)` inside any `lambda`:

```yaml
esphome:
  name: elevator-hall-sensors
  friendly_name: elevator-hall-sensors

esp32:
  board: esp32-c6-devkitc-1
  framework:
    type: esp-idf

logger:
  level: WARN  # Reduces CPU & serial overhead during rapid sensor updates

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
  power_save_mode: none  # Keeps Wi-Fi radio constantly awake for lowest latency
  fast_connect: true

external_components:
  - source:
      type: git
      url: [https://github.com/lucasdss/wiz](https://github.com/lucasdss/wiz)
      ref: main
    refresh: 1d  # Use 0s while testing changes to the repo

wiz_udp:

binary_sensor:
  - platform: ld2450
    ld2450_id: ld2450_radar
    has_target:
      name: Presence
      on_press:
        then:
          - lambda: |-
              send_wiz_udp("192.168.1.163", "{\"id\":1,\"method\":\"setState\",\"params\":{\"state\":true,\"dimming\":100}}");
      on_release:
        then:
          - lambda: |-
              send_wiz_udp("192.168.1.163", "{\"id\":1,\"method\":\"setState\",\"params\":{\"state\":false}}");
```

---

## Common WiZ UDP Payloads (Port `38899`)

| Action | JSON Payload | Escaped String for ESPHome Lambda |
| :--- | :--- | :--- |
| **Turn ON (100%)** | `{"id":1,"method":"setState","params":{"state":true,"dimming":100}}` | `"{\"id\":1,\"method\":\"setState\",\"params\":{\"state\":true,\"dimming\":100}}"` |
| **Warm White (3000K)** | `{"id":1,"method":"setState","params":{"state":true,"dimming":100,"temp":3000}}` | `"{\"id\":1,\"method\":\"setState\",\"params\":{\"state\":true,\"dimming\":100,\"temp\":3000}}"` |
| **Cool White (5000K)** | `{"id":1,"method":"setState","params":{"state":true,"dimming":100,"temp":5000}}` | `"{\"id\":1,\"method\":\"setState\",\"params\":{\"state\":true,\"dimming\":100,\"temp\":5000}}"` |
| **RGB Color (Orange)** | `{"id":1,"method":"setState","params":{"state":true,"dimming":100,"r":255,"g":120,"b":0}}` | `"{\"id\":1,\"method\":\"setState\",\"params\":{\"state\":true,\"dimming\":100,\"r\":255,\"g\":120,\"b\":0}}"` |
| **Turn OFF** | `{"id":1,"method":"setState","params":{"state":false}}` | `"{\"id\":1,\"method\":\"setState\",\"params\":{\"state\":false}}"` |

---

## Best Practices & Notes

* **Multiple Bulbs:** Call `send_wiz_udp(...)` sequentially for each bulb's static IP inside the same `lambda` block—each UDP transmission takes less than 1ms.
