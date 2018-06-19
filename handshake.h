


struct Handshake {
  uint8_t APMAC[6];
  uint8_t clientMAC[6];
  uint8_t ANonce[32];   //from AP in key 2
  uint8_t SNonce[32];   //from client in key 3
  uint8_t SSID[32];
};

