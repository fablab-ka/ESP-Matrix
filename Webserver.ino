#include "AllIncludes.h"
#include "Webcontent.h"
// Bestandteile der Webseite ==============================================================================//

// Webserver
AsyncWebServer server(80);


boolean pending_reboot = false;


void setupWebserver()
{
  server.addHandler(new SPIFFSEditor(GlobalConfig.myAdminUser,GlobalConfig.myAdminPWD));
  server.on("/contentsave", HTTP_POST|HTTP_GET, handleOnContentSave);
  server.on("/content", HTTP_GET, handleOnContent);
  server.on("/wificonnectAP", HTTP_GET, handleWiFiConnect2AP);
  server.on("/wificonfigAP", HTTP_POST|HTTP_GET, handleConfigAP);
  server.on("/wifiRestartAP", HTTP_POST, handleWiFiRestartAP);
  server.on("/wifiRestartSTA", HTTP_GET, handleWiFiRestartSTA);
  server.on("/wpsconfig", HTTP_GET, handleWPSConfig);
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  
  server.begin();
}

void handleOnContent(AsyncWebServerRequest *request){
  String page = FPSTR(HTML_CONTENT_HEAD);
  page+=FPSTR(HTML_MENU_STYLE);
  page+=FPSTR(HTML_CONTENT_HEADEND);
  page+=FPSTR(HTML_MENU);
  page+=FPSTR(HTML_CONTENT_BODY);
  request->send(200, "text/html", page);
}



void handleConfigAP(AsyncWebServerRequest *request){
  if(!request->authenticate(GlobalConfig.myAdminUser, GlobalConfig.myAdminPWD))
    return request->requestAuthentication();
  Serial.println("Config Start");

  struct T_ConfigStruct tempConf;
  char myAdminPWD2[20];
  
  int params = request->params();
  for(int i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
    if(! p->isFile()) {  // ONLY for GET and POST
      if (! strcmp(p->name().c_str(), "SSID")) {
        strlcpy( tempConf.mySSID, p->value().c_str(), sizeof(tempConf.mySSID));
        Serial.print("SSID: "); Serial.println(tempConf.mySSID);
      } 
      if (! strcmp(p->name().c_str(), "WLANPWD")) {
        strlcpy( tempConf.myPWD, p->value().c_str(), sizeof(tempConf.myPWD));
        Serial.print("WLAN-PWD: "); Serial.println(tempConf.myPWD);
      } 
      if (! strcmp(p->name().c_str(), "Kanal")) {
         tempConf.myChannel = atoi(p->value().c_str());
        Serial.print("Channel: "); Serial.println(tempConf.myChannel);
      } 
      if (! strcmp(p->name().c_str(), "ADMPWD")) {
        strlcpy( tempConf.myAdminPWD, p->value().c_str(), sizeof(tempConf.myAdminPWD));
        Serial.print("ADM: "); Serial.println(tempConf.myAdminPWD);
      } 
      if (! strcmp(p->name().c_str(), "ADMPWD2")) {
        strlcpy( myAdminPWD2, p->value().c_str(), sizeof(myAdminPWD2));
        Serial.print("ADMPWD2: "); Serial.println(myAdminPWD2);
      } 
    }
  }
  String page = FPSTR(HTML_HEAD);
  page.replace("{title}", FPSTR(HTML_TITLE_CONFIG));
  page += FPSTR(HTML_BASIC_STYLES);
  page += FPSTR(HTML_MENU_STYLE);
  page += FPSTR(HTML_BODYSTART);
  page += FPSTR(HTML_MENU);
  page = page+"<h1>"+FPSTR(HTML_TITLE_CONFIG)+"</h1>";
  String form=FPSTR(HTML_GLOBAL_CONF );
  form.replace("{SSID}",GlobalConfig.mySSID);
  form.replace("{WLANPWD}",GlobalConfig.myPWD);
  String ch; ch+=GlobalConfig.myChannel;
  form.replace("{CH}",ch);
  form.replace("{ADMPWD}",GlobalConfig.myAdminPWD);

  if (params >2)  {
    Serial.println("Configuration complete");
    if((strlen(tempConf.myAdminPWD) > 0) and 
       (strlen(myAdminPWD2) >0) and
       (0 == strncmp(tempConf.myAdminPWD, myAdminPWD2, sizeof(myAdminPWD2))))  {
      strlcpy(tempConf.magic, CONF_MAGIC, sizeof(tempConf.magic));
      tempConf.version=CONF_VERSION;
      ConfigSave(&tempConf);
      pending_reboot=true;
      request->send(200, "text/plain", "OK");
      return;
    } else {
      page+=FPSTR(HTML_PWD_WARN);
    }
  }
  
  page+=form;page+="<br>";
  page+=FPSTR(HTML_BODYEND);
  request->send(200, "text/html", page);
}







void handleWiFiConnect2AP(AsyncWebServerRequest *request){
  Serial.println("WiFiConfig Start");
  int16_t numWLANs=  WiFi.scanComplete();
  if (numWLANs == -2) { // es gab bislang noch keinen WLAN-Scan
    WiFi.scanNetworks(true);
    Serial.print("Scan start at: ");Serial.println(millis());
  }
  String page = FPSTR(HTML_HEAD);
  page.replace("{title}", FPSTR(HTML_TITLE_APCONNECT));
  page += FPSTR(HTML_SELECTSCRIPT);
  page += FPSTR(HTML_BASIC_STYLES);
  page += FPSTR(HTML_MENU_STYLE);
  page += FPSTR(HTML_BODYSTART);
  page += FPSTR(HTML_MENU);
  
  page = page+"<h1>"+FPSTR(HTML_TITLE_APCONNECT)+"</h1>";
  if (numWLANs == -1) { //scan still in progress
    // warte, bis der Scan fertig ist
    while(numWLANs == -1){ yield(); numWLANs=  WiFi.scanComplete();}
  }
  if (numWLANs == 0) {
    page += FPSTR(HTML_NO_WLAN_FOUND);  
  } else {
    page += FPSTR(HTML_DIVTABLESTART);
    for (uint16_t i=0; i<numWLANs; i++) {
      page += FPSTR(HTML_ROWTABLESTART);
      Serial.print("List Network no: ");
      Serial.println(i);
      String wlan    = FPSTR(HTML_ACCESSPOINT_INFO);
      String quality; quality+=WiFi.RSSI(i);
      wlan.replace("{wlan}",WiFi.SSID(i));
      wlan.replace("{quality}",quality);
      switch (WiFi.encryptionType(i)) {
        case 2: wlan.replace("{security}","WPA2 (TKIP)");break;
        case 4: wlan.replace("{security}","WPA (CCMP)");break;
        case 5: wlan.replace("{security}","WEP :-(");break;
        case 7: wlan.replace("{security}","ungesichert");break;
        case 8: wlan.replace("{security}","AUTO");break;
        default: wlan.replace("{security}","unknown");break;
      }
      wlan.replace("{mac}",WiFi.BSSIDstr(i));
      String channel; channel+=WiFi.channel(i);
      wlan.replace("{ch}",channel);
      page+=wlan;
      page += FPSTR(HTML_DIVEND);  //row
    }
    page += FPSTR(HTML_DIVEND);
    
  }
  page+="<br><br>";
  
  page+=FPSTR(HTML_FORM_SSID_PWD );page+="<br>";
  page+=FPSTR(HTML_LINK_SCAN_CONFIG);page+="<br>";
  page+=FPSTR(HTML_LINK_AP_CONFIG);page+="<br>";
  //page+=FPSTR(HTML_LINK_WPS);page+="<br>";
  page+=FPSTR(HTML_BODYEND);
  request->send(200, "text/html", page);
  WiFi.scanNetworks(true);  //Starte nächsten WLAN-Scan (nach dem Requestende des Servers!!)
}







void handleWiFiRestartAP(AsyncWebServerRequest *request){
  handleWiFiRestart(request, true);
}
void handleWiFiRestartSTA(AsyncWebServerRequest *request){
  handleWiFiRestart(request, false);
}

void handleWiFiRestart(AsyncWebServerRequest *request, boolean asAP){
  char  clientSSID[32] = "";
  char  clientPWD[64]  = "";  
  Serial.println("WiFiSave start");
  int params = request->params();
  String page;
  if (asAP) {
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(! p->isFile()) {  // ONLY for GET and POST
        if (! strcmp(p->name().c_str(), "s")) {
          strlcpy(  clientSSID, p->value().c_str(), sizeof( clientSSID));
          Serial.println("SSID received!");
        }
        if (! strcmp(p->name().c_str(), "p")) {
          strlcpy(  clientPWD, p->value().c_str(), sizeof( clientPWD));
          Serial.println("PWD received!");
        }
      }
    }
    page += FPSTR(HTML_HEAD);
    page.replace("{title}", "Versuche zu WLAN zu verbinden");
  } else {  
    page += FPSTR(HTML_HEAD);
    page.replace("{title}", "Restart als Accesspoint");
  }
  page += FPSTR(HTML_SELECTSCRIPT);
  page += FPSTR(HTML_BASIC_STYLES);
  page += FPSTR(HTML_MENU_STYLE);
  page += FPSTR(HTML_BODYSTART);
  page += FPSTR(HTML_MENU);
  String line;
  if (asAP) {
    line = FPSTR(HTML_MSG_RESTART_STA);
  } else {
    line = FPSTR(HTML_MSG_RESTART_AP);  
  }
  line.replace("{SSID}",GlobalConfig.mySSID );
  line.replace("{PWD}", GlobalConfig.myPWD);
  page+=line; 
  page+=FPSTR(HTML_LINK_SCAN_CONFIG);page+="<br/>";
  page+=FPSTR(HTML_LINK_AP_CONFIG);page+="<br/>";
  //page+=FPSTR(HTML_LINK_WPS);page+="<br/>";
  page+=FPSTR(HTML_BODYEND);
  request->send(200, "text/html", page);
  Serial.println("Request done");
  struct station_config newConf;  memset(&newConf, 0, sizeof(newConf));
  strlcpy((char *)newConf.ssid, clientSSID, strlen(clientSSID)+1);
  strlcpy((char *)newConf.password, clientPWD, strlen(clientPWD)+1);
  ETS_UART_INTR_DISABLE();
  wifi_station_set_config(&newConf);
  ETS_UART_INTR_ENABLE();
  Serial.println("New config is written");
  pending_reboot=true; //make sure, ESP gets rebooted
}

void handleWPSConfig(AsyncWebServerRequest *request){
  Serial.println("wps config done");
  String message="<html><body><h1>WPSCONFIG NOT YET IMPLEMENTED</h1></body></html>";
  request->send(200, "text/html", message);
  
}

void handleApConfig(AsyncWebServerRequest *request){
  Serial.println("AP config done");
  String message="<html><body><h1>AP CONFIG NOT YET IMPLEMENTED</h1></body></html>";
  request->send(200, "text/html", message);
  
}




void handleOnContentSave(AsyncWebServerRequest *request){
  int params = request->params();
  for(int i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
    if(! p->isFile()) {  // ONLY for GET and POST
      if (! strcmp(p->name().c_str(), "text")) {
        strlcpy( tape, p->value().c_str(), sizeof(tape));
        for( uint16_t j=0; j<=strlen(tape); j++){
          // map certain latin1 characters to extended ascii cp437
          switch(tape[j]) {
            case 0xA7: tape[j]=0x15; break; // §
            case 0xB5: tape[j]=0xE5; break; // µ
            case 0xB0: tape[j]=0xF7; break; // °
            case 0xB2: tape[j]=0xFC; break; // ²
            case 0xFC: tape[j]=0x81; break; // ü
            case 0xE4: tape[j]=0x84; break; // ä
            case 0xEB: tape[j]=0x89; break; // ë
            case 0xC4: tape[j]=0x8E; break; // Ä
            case 0xCB: tape[j]=0x45; break; // Ë->E
            case 0xD8: tape[j]=0xEC; break; // 
            case 0xF6: tape[j]=0x94; break; // ö
            case 0xD6: tape[j]=0x99; break; // Ö
            case 0xDF: tape[j]=0xE1; break; // ß
            case 0xDC: tape[j]=0x9A; break; // Ü
            case 0xF7: tape[j]=0xF5; break; // 
          }
          
        }
        Serial.print("Text received: ");
        if(0 == strlen(tape)) {strlcpy(tape, "-", sizeof(tape));}
        Serial.println(tape);
      }
      if (! strcmp(p->name().c_str(), "speed")) {
        speedval=strtol(p->value().c_str(), NULL, 10);
        if (speedval > 200) {speedval = 200;}
        Serial.printf("Speed: %d\n", speedval);
      }
      if (! strcmp(p->name().c_str(), "brightness")) {
        brightval=strtol(p->value().c_str(), NULL, 10);
        if (brightval > 15) {brightval=15;}
        Serial.printf("Brightness: %d\n", brightval);
      }
      if (! strcmp(p->name().c_str(), "content")) {
        myContent=strtol(p->value().c_str(), NULL, 10);
        if (myContent > 4) {myContent=4;}
        Serial.printf("Content: %d\n", myContent);
      }
    }
  }
  String page =FPSTR(HTML_REDIRECT_CONTENT);
  request->send(200, "text/html", page);
}


void do_pending_Webserver_Actions(void){
  // do all things, which need to be done after an asynchronous request
  if (pending_reboot) {
    Serial.println("Restarting system, hold on");
    delay(1000);
    // Sicherstellen, dass die GPIO-Pins dem Bootloader das erneute Starten des Programmes erlauben!
    pinMode( 0,OUTPUT);
    pinMode( 2,OUTPUT);
    pinMode(15,OUTPUT);
    digitalWrite(15, LOW);
    digitalWrite( 0,HIGH);
    digitalWrite( 2,HIGH);
    delay(100);
    ESP.restart();
  }  
  
}



