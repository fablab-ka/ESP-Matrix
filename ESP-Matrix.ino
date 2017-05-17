/****************************************************************************************************
*  LED-Matrix  1.1   
*  Wolfgang Kraft, Fablab Karlsruhe 2017                                                            *
*  Modul um eine Reihe LED-Matrizen (8x8) mittels ESP8266 als Display zu verwenden                  *
*  Erste Features:                                                                                  *  
*    - ständige Erreichbarkeit des Webservers zur Steuerung der Applikation                         *
*      und Konfiguration des WiFi-Teils (AP oder STA Modus)                                         *
*    - Anzeige der aktuellen IP-Adresse nach Start                                                  *
*    - integrierter HTML Editor für SPIFFS-Dateien auch ohne Internet-Verbindung                    *
*  ToDos:                                                                                           *
*    - Refactoring, Zusammenfassen der Webserverfunktionen in eine eigene Bibliothek als            *
*      Webapplication-Basisklasse                                                                   *
*      * mehrstufige Benutzerauthentifizierung und -verwaltung (z.B. Konfiguration und Anwendung    *
*        der Applikation                                                                            *
*      * Ausbau der IP-Konfiguration um WPS, feste IP-Adressen, DHCP-Ranges, Nameserver,            *
*        Mailgateway ...                                                                            *
*      * Einsatz von https                                                                          *
*    - Durchreichen verschiedener ESP-Funktionen an Javascript (vgl. Firmata)                       *    
*                                                                                                   *
*****************************************************************************************************/

#include "AllIncludes.h"

// globale Variablen ==============================================================================//

// Konfiguration für Flash
struct T_ConfigStruct { 
  char    magic[5] = CONF_MAGIC;
  uint8_t version  = CONF_VERSION;
  char    mySSID[32] = CONF_DEFAULT_SSID;
  char    myPWD[64]  = CONF_DEFAULT_PWD;
  char    myHostname[20] = CONF_HOSTNAME;
  char    myAdminUser[20] = CONF_ADMIN_USER;
  char    myAdminPWD[20] = CONF_ADMIN_PWD;
  
  uint8_t myChannel  = CONF_DEFAULT_CHANNEL;
  
} GlobalConfig;

void ConfigSave( struct T_ConfigStruct* conf) {
   EEPROM.begin(512);
   uint8_t* confAsArray = (uint8_t *) conf;
   for (uint16_t i=0; i<sizeof(T_ConfigStruct); i++) {
     EEPROM.write(i,confAsArray[i]);
   }
   EEPROM.commit();
   EEPROM.end();
}


void ConfigLoad( struct T_ConfigStruct* conf) {
   EEPROM.begin(512);
   uint8_t* confAsArray = (uint8_t*) conf;
   for (uint16_t i=0; i<sizeof(T_ConfigStruct); i++) {
     confAsArray[i]=EEPROM.read(i);
   }
   EEPROM.commit();
   EEPROM.end();
}

void LoadAndCheckConfiguration( void) {
  struct T_ConfigStruct newConf;
  ConfigLoad(&newConf);
  if (strncmp(newConf.magic, GlobalConfig.magic, sizeof(newConf.magic)) == 0){
    Serial.println("Magic string from configuration is ok, overload configuration from eeprom");
    ConfigLoad(&GlobalConfig);
  } else {
    Serial.println("Magic string mismatch, keeping with default configuration");
  }
}


char myIPaddress[18]="000.000.000.000  ";





// Zeitserver und Zeithandling
IPAddress timeServer(0, 0, 0, 0);  // wird per NTP ermittelt
AsyncUDP ntpClient;                // asynchroner UDP Socket für NTP-Abfragen
uint8_t NTPpacket[ 48];            // Anfragepaket für NTP-Anfragen
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Standard Time
TimeChangeRule *tcr;  // Zeiger auf aktuelle Zeitzone
Timezone CE(CEST, CET);
time_t localTime;

LEDmatrix myMatrix(4,1);

void setup(){
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  LoadAndCheckConfiguration();
  WiFi.hostname(GlobalConfig.myHostname);
  ETS_UART_INTR_DISABLE();
  wifi_station_disconnect();
  ETS_UART_INTR_ENABLE();
  SPIFFS.begin();
  WiFi.setOutputPower(20.5); // möglicher Range: 0.0f ... 20.5f
  WiFi.mode(WIFI_STA);
  Serial.println("Start in Client Mode!");
  WiFi.begin();
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Start in Access Point Mode!");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(GlobalConfig.mySSID, GlobalConfig.myPWD, GlobalConfig.myChannel);
    strlcpy(myIPaddress, WiFi.softAPIP().toString().c_str(), sizeof(myIPaddress));
    Serial.println(GlobalConfig.mySSID);
  } else {
    WiFi.setAutoReconnect(true);
    strlcpy(myIPaddress, WiFi.localIP().toString().c_str(), sizeof(myIPaddress));
  }
   
  int16_t numWLANs=  WiFi.scanComplete();
  if (numWLANs == -2) { // es gab bislang noch keinen WLAN-Scan
    WiFi.scanNetworks(true);
  }
  setupTime();
  setupWebserver();
}



void loop() {
  myMatrix.loop();
  do_pending_Webserver_Actions();
}


