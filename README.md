Attempt to capture 4 way handshake with ESP.
Used ernacktobs esp8266_wifi_raw half reverse engineered SDK as a base.

current status:
- Can pick up and identify all 4 handshake packets and dump them into serial. This can be analysed with wireshark.
- I estimate around 40% of packages are not picked up by ESP. I have no solution for this, I assume building your own full stack on one of the NOSDK builds ([Cnlohr's](https://github.com/cnlohr/nosdk8266), [ba0sh1's](https://github.com/pvvx/SDKnoWiFi)) is the only way around it.
- I don't remember if I managed to get it to send deauth packets. But they started blocking deauth packets in some version of the SDK. There is already enough information about that on the internet so I won't go more indepth here.
- The plan was to store the handshake in the EEPROM for later analysis. But i stopped working on it because of the reliability problems.

Promiscous mode does not capture full packets, only the first 128 - x bytes. This makes it not suitable for capture for handshake capture. I managed to get around this by setting the sender IP to the IP of the access point, and the destination IP to the target IP. I'm not 100% sure why the ESP captures its 'own' packets and triggers the underlying functions. But it works. :D

To compile this program, please refer to the upstream git. I have used the arduino compiled version of this because I'm lazy. I have mirrored the readme in README_OLD.md.

If there are any questions, feel free to get in touch with me by making an issue.
