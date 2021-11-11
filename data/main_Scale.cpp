/*
        When:   20210822
        Who:    Joe Belson
        What:   ESP01 (ESP8266) cpp for Web Server support: html/websocket to display "bath scale" weights
                Bath Scale to HX711 to ESP01 to WebSock to html to phone/tablet/computer

        Note:   Each ESP01 provides an Access Point (AP) "Plane Scale #"
                Each ESP01 publishes a Scale Access web page on 10.0.scaleNo.scaleNo
                Each ESP01 also transmits via broadcast a struct (scaleNo, reading, SCALE, offset,...) using ESPNOW.
                Each ESP01 within each Bath Scale contains the source code & html on its SPI-memory chip
                The ESP01 & HX711 will shutdown after 1 hour. 
*/

//#define dbg // Serial prints and HX711 inputs on gpio0 & gpio2

#ifdef dbg
 #include <ArduinoTrace.h>
#endif

#include <Arduino.h>
#include <espnow.h>
#include <FS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <index.h>

#include <HX711.h>
#include <string.h>

  const int SCALE_NO = 1;       // ************* >>>>>>>>>> SET scaleNo here <<<<<<<<<<<< *****************//
  const int FREQUENCY = 500;    // milliseconds
  const int startup = millis(); // used for shutdown after 1 hour
  const byte DNS_PORT  = 53;
  char AP_Name[] = "Plane Scale  "; // AP name: We append scaleNo;
  
  int notReady=0;       // counter for HX711 not ready loops
  int loopMillis=0;     // delay for HX711 readings

// objects
DNSServer dnsServer;
AsyncWebServer server(80);
AsyncWebSocket webSock("/");
HX711 scale;

typedef struct esp_now_msg_t{   // Scale Information
        int scaleNo = SCALE_NO;
        long reading = 0;       // raw reading: 
        double SCALE = 1;       // amplification factor / calibration value
        long offset  = 0;       // tare() value
        bool isTare  = false;   // Zero weight : offset == -reading
        bool isSave  = false;   // flag: save to SPIFFS

        std::string toStr(){    // make JSON string
            char c[100];
            int n = sprintf(c, "{\"scaleNo\":%i,\"reading\":%li,\"SCALE\":%.2f,\"offset\":%li,\"isTare\":%hi,\"isSave\":%hi}"
            , scaleNo, reading, SCALE, offset, isTare, isSave);
            return std::string(c, n);
        }
} esp_now_msg_t;

/******  PROTOTYPES  *******/
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
void notFound(AsyncWebServerRequest *request);
void AP_Begin();
void initLocalStruct();
void update_sMsgFromString( const std::string &scaleChar, esp_now_msg_t& p);
std::string valFromJson(const std::string &json, const std::string &element);
bool save_ScaleJSON(); // write the local s_Msg struct string to SPIFFS

// ESP_NOW
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus);
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len);

IPAddress  // soft AP IP info
          ip_AP(10,0,SCALE_NO,SCALE_NO)
        , ip_AP_GW(10,0,SCALE_NO,SCALE_NO)
        , ip_subNet(255,255,255,128);

// ESP_NOW broadcast
uint8_t peer[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // MAC: MultiCast"

// HX711  (Remember to pull ch_PD/EN pin high) USE 1 & 3 for all three
#ifdef dbg // pins 1 & 3 for serial during debug
                const int LOADCELL_DOUT_PIN = 0;  // ESP01 (pull high for startup)
                const int LOADCELL_SCK_PIN = 2;   // ESP01 (pull high for startup)
#else // production does not need tx/rx
                const int LOADCELL_DOUT_PIN = 1;    // ESP01 (TX)  ===>> USE for SCALEs 1, 2, 3, 4
                const int LOADCELL_SCK_PIN = 3;     // ESP01 (RX)  ===>> USE for SCALEs 1, 2, 3, 4
#endif
// *** ESP DOIT ***
//  const int LOADCELL_DOUT_PIN = 14;
//  const int LOADCELL_SCK_PIN = 13;

const int LOADCELL_GAIN_FACTOR = 128;  // set HX711 Gain Factor : Channel 1 (128) 

// ESP NOW message for local scale
esp_now_msg_t s_Msg;


void setup(){
        system_update_cpu_freq(20);
        AP_Name[12]=(char)48+SCALE_NO;
/*
    1) init HX711
    2) init local Struct s_Msg from JSON in SPIFFS... if exists
    3) init AccessPoint transmission
    4) init esp_NOW; init ESPNOW listener-which WebSock sends any incomming ESPNOW s_Msg)
    5) init WebSock
    6) init DNS - used to 
    4) HTTP: listen on: /, /calib, /html_index, /indexH, /html_calib, /cpp
    6) loop: read HX711, ESPNOW broadcast this s_Msg, WebSock send this JSON
*/

#ifdef dbg
        Serial.begin(115200);
        Serial.setDebugOutput(true);
        TRACE();
#endif

        scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN, LOADCELL_GAIN_FACTOR);
        scale.power_up();

        SPIFFS.begin();

        initLocalStruct(); // update s_Msg using SPIFFS's JSON if available

        AP_Begin();

        esp_now_init();
        esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
        esp_now_register_send_cb(OnDataSent);
        esp_now_register_recv_cb(OnDataRecv);
        esp_now_add_peer(peer, ESP_NOW_ROLE_COMBO, 3, NULL, 0);
 
        webSock.onEvent(onWsEvent);
        server.addHandler(&webSock);

        /* Setup the DNS server redirecting all the domains to the apIP */
        dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
        server.onNotFound(notFound);
        server.on("/"      , HTTP_GET, [](AsyncWebServerRequest *request){request->send_P(200, "text/html", index_html);});
        server.on("/calib" , HTTP_GET, [](AsyncWebServerRequest *request){request->send_P(200, "text/html", calib_html);});

        server.on("/html_index", HTTP_GET, [](AsyncWebServerRequest *request){request->send_P(200, "text/plain", index_html);});
        server.on("/html_calib", HTTP_GET, [](AsyncWebServerRequest *request){request->send_P(200, "text/plain", calib_html);});
 
        server.on("/indexH", HTTP_GET, [](AsyncWebServerRequest *request){request->send(SPIFFS, "/index.h", String(), false);});
        server.on("/cpp", HTTP_GET, [](AsyncWebServerRequest *request){request->send(SPIFFS, "/main_Scale.cpp", String(), false);});
        server.on("/platformio", HTTP_GET, [](AsyncWebServerRequest *request){request->send(SPIFFS, "/platformio.ini", String(), false);});
        server.begin();
}

void loop() { // SELF: get measurement; send esp_+Now; send WS - loop.  Others' sent from WS rcv
        if(millis() > 60*60000){ // timeout/shutdown after 1 hour - incase user forgot to turn off scale
                scale.power_down();
                ESP.deepSleep(999999999*999999999U, WAKE_NO_RFCAL);
        }

        delay(1);
        dnsServer.processNextRequest(); delay(1);
        if(millis()-loopMillis > FREQUENCY){
                if(scale.is_ready()){
                        s_Msg.reading = scale.read_average(25);
                        if(!s_Msg.reading)s_Msg.reading=-1;
                        notReady = 0;
                }else   if(notReady++ >= 15 || !s_Msg.reading)s_Msg.reading = -1;

                esp_now_send(peer, (uint8_t *)&s_Msg, sizeof(s_Msg)); delay(1); // esp_now : send self
#ifdef dbg                
                Serial.printf("NowSent:     %s\n", s_Msg.toStr().c_str());
#endif
                webSock.textAll(s_Msg.toStr().c_str()); delay(1);  // WS : send self
                webSock.cleanupClients();
                loopMillis = millis();
        }
}

void notFound(AsyncWebServerRequest *request){request->send_P(200, "text/html", index_html);}

// WebSock Event Handler: rcv : self - update s_Msg (save/not save); Others' - send via esp_Now
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_DATA){
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    if(info->final && !info->index && info->len == len){
      if(info->opcode == WS_TEXT){
        data[len]=0;
        std::string const s=(char *)data;
        esp_now_msg_t ws_Msg;  // declare struct to hold the WS inbound message

        update_sMsgFromString(s, ws_Msg);
        if(ws_Msg.scaleNo==SCALE_NO){
          update_sMsgFromString(s, s_Msg);
          if(s_Msg.isSave || s_Msg.isTare){
            if(!s_Msg.isTare) save_ScaleJSON();
              s_Msg.isTare=false;
              webSock.textAll(s_Msg.toStr().c_str()); // send back to web : success update s_Msg.  i.e. set isTare = false
              webSock.cleanupClients();
          }
        } else esp_now_send(peer, (uint8_t *)&ws_Msg, sizeof(ws_Msg));
      }
    }
  }
}

// ESP NOW  sent call back(cb)
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
#ifdef dbg        
  if (sendStatus == 0)Serial.printf("\nNOW: Delivery success from: %i\n", SCALE_NO);
  else Serial.printf("NOW: Delivery fail from: %i\n", SCALE_NO);
#endif
}

//ESP NOW  call back(cb) OnDataReceive
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {// if scale==local updates_Msg else send to web
        esp_now_msg_t now_Msg;  // needed to land inbound binary data;
        memcpy(&now_Msg, incomingData, sizeof(now_Msg));  // maybe need len or sizeof incomming data?? (TBD)
        if(now_Msg.scaleNo==SCALE_NO){
                memcpy(&s_Msg, &now_Msg, sizeof(now_Msg));
 
                if(s_Msg.isSave || s_Msg.isTare){
                        if(!s_Msg.isTare) save_ScaleJSON();
                        s_Msg.isTare=false;
                        esp_now_send(peer, (uint8_t *)&s_Msg, sizeof(s_Msg)); delay(1); // esp_now : send out self: success update s_Msg
                }
        }

#ifdef dbg
        Serial.printf("NOW-in: %i: %s\n", now_Msg.scaleNo, now_Msg.toStr().c_str());Serial.flush();
#endif
     webSock.textAll(now_Msg.toStr().c_str());
}

void AP_Begin(){
        WiFi.disconnect();
        WiFi.softAPdisconnect();

        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(ip_AP, ip_AP_GW, ip_subNet);
        WiFi.softAP(AP_Name, "");
        WiFi.begin();
        
#ifdef dbg
        Serial.printf("\n** STA:  %s\tAP: %s\n", WiFi.localIP().toString().c_str(), WiFi.softAPIP().toString().c_str()); Serial.flush();
#endif

}
// load SPIFFS copy of JSON into the local scale's s_Msg
void initLocalStruct(){
        File f = SPIFFS.open("/s_Msg.json", "r");
        if(f){
                std::string s = f.readString().c_str();
                f.close();
                s_Msg.SCALE=::atof(valFromJson(s, "SCALE").c_str());
                s_Msg.offset=::atol(valFromJson(s, "offset").c_str());
        }
#ifdef dbg
        else Serial.printf("\nFailed init from SPIFFS: %s\n", s_Msg.toStr().c_str());Serial.flush();
#endif
}
// save the local s_Msg data as JSON string to SPIFFS
bool save_ScaleJSON(){

        File f = SPIFFS.open(F("/s_Msg.json"), "w");
        if(f){
                s_Msg.isSave=false;
                if(f.print(s_Msg.toStr().c_str()) != s_Msg.toStr().length())
                        s_Msg.isSave=true;
                f.close();
                return true;
        }
        else{return false;}
}
// process inbound websock json (calibrate)
void update_sMsgFromString(const std::string& scaleChar, esp_now_msg_t& p){
		p.scaleNo=::atoi(valFromJson(scaleChar, "scaleNo").c_str());
                p.SCALE=::atof(valFromJson(scaleChar, "SCALE").c_str());
                p.offset=::atol(valFromJson(scaleChar, "offset").c_str());

                p.reading=::atol(valFromJson(scaleChar, "reading").c_str());
                p.isTare=::atoi(valFromJson(scaleChar, "isTare").c_str());
                p.isSave=::atoi(valFromJson(scaleChar, "isSave").c_str());
}

// return a value from scaleJSON as std::string.  NOTE: Stack Dumps when trying to use <ArduinoJson.h>
std::string valFromJson(const std::string &json, const std::string &element){
  size_t start, end;

  start = json.find(element.c_str());
  start = json.find(":", start)+1;
  end  = json.find_first_of(",}", start); // commented "{"

  return(json.substr(start, end-start));
}

/*
// Update value in JSON string.  NOTE: Stack Dumps when trying to use <ArduinoJson.h>
void update_JSON(std::string& json, const std::string &element, const std::string &value){
  size_t start, end;

  start = json.find(element.c_str());
  start = json.find(":", start)+1;
  end  = json.find_first_of(",}", start); // commented "{"
  json = json.substr(0,start) + value + json.substr(end);
}
*/