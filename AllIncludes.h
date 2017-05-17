// gemeinsame Includes
#ifndef LEDMatrixCommonIncludes
#define LEDMatrixCommonIncludes  
  // Workaround nach dem Split in Einzeldateien.
  // Verwendete Bibliotheken   =========================================================================//
  
  #include "ESP8266WiFi.h"        // http://esp8266.github.io/Arduino/versions/2.3.0/doc/installing.html 
  #include "ESP8266WiFiGeneric.h" // http://esp8266.github.io/Arduino/versions/2.3.0/doc/installing.html 
  #include <Dns.h>                // http://esp8266.github.io/Arduino/versions/2.3.0/doc/installing.html 
  
  #include "ESPAsyncUDP.h"        // http://github.com/me-no-dev/ESPAsyncUDP
  #include <ESPAsyncTCP.h>        // http://github.com/me-no-dev/ESPAsyncTCP
  #include <ESPAsyncWebServer.h>  // http://github.com/me-no-dev/ESPAsyncWebServer
  #include <SPIFFSEditor.h>       // http://github.com/me-no-dev/ESPAsyncWebServer
  
  #include <TimeLib.h>            // http://github.com/PaulStoffregen/Time
  #include <Timezone.h>           // http://github.com/JChristensen/Timezone
  #include <Adafruit_GFX.h>       // via Library-Manager; Voraussetzung für Max72xxPanel
  #include <Max72xxPanel.h>       // https://github.com/markruys/arduino-Max72xxPanel.git
  
  
  #include "FS.h"
  #include <EEPROM.h>
  #include <SPI.h>           
  #include <memory>
  #include "display.h"

  extern "C" {
    #include "user_interface.h"
  }
  
  
  // Basiskonfigurationen ==============================================================================//
  
  // Webserver und WiFi Konfigurationen
  #define CONF_VERSION 1    // 1..255 incremented, each time the configuration struct changes
  #define CONF_MAGIC "flka" // unique identifier for project. Please change for forks of this code
  #define CONF_DEFAULT_SSID "ESPMATRIX"
  #define CONF_DEFAULT_PWD "ABCdef123456"
  #define CONF_HOSTNAME "ESPTESTWMK"
  #define CONF_ADMIN_USER "admin"
  #define CONF_ADMIN_PWD "admin"
  #define CONF_DEFAULT_CHANNEL 4

  // Zeitserver etc.
  #define TimeServerName "de.pool.ntp.org"
  #define TimeServerPort 123


  enum showContent {longtext, longdate, shortdate, shorttime, ownIP};

  
  uint8_t myContent = ownIP;  // at StartUp show own IP address
  uint8_t speedval  = 100;    // Speed at Startup
  uint8_t brightval = 1;      // Brightness at startup





#endif
