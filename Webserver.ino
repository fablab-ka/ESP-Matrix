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

void printFreeHeap(int pagesize) {
  Serial.print("Free Heap: ");
  Serial.print(system_get_free_heap_size());
  Serial.print("  Lenght page: ");
  Serial.println(pagesize);
}

void handleOnContent(AsyncWebServerRequest *request){
  String page = FPSTR(HTML_CONTENT_HEAD);
  page+=F("<SCRIPT>");
  page+=FPSTR(JS_SORT_BY_KEY);
  page+=FPSTR(JS_DO_GET_REQUEST);  
  page+=FPSTR(JS_POPULATE_SELECTION);
  page+=FPSTR(JS_INIT_AFTER_LOAD);
  page+=F("</SCRIPT>");
  page+=FPSTR(HTML_CONTENT_HEADEND);
  File menuFile = SPIFFS.open("/.menu.html", "r");
  while (menuFile.available()){
    page += menuFile.readStringUntil('\n');
  }
  menuFile.close();
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
  page += FPSTR(HTML_BODYSTART);
  File menuFile = SPIFFS.open("/.menu.html", "r");
  while (menuFile.available()){
    page += menuFile.readStringUntil('\n');
  }
  menuFile.close();
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
  page += FPSTR(HTML_BODYSTART);
  File menuFile = SPIFFS.open("/.menu.html", "r");
  while (menuFile.available()){
    page += menuFile.readStringUntil('\n');
  }
  menuFile.close();
  
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
  Serial.println("ContentSave");
  if (request->hasArg("content")) {
    AsyncWebParameter* p = request->getParam("content", true);
    Serial.println("request content ");
    uint8_t myContent=strtol(p->value().c_str(), NULL, 10);
    if (myContent > 5) {myContent=4;}
    switch (myContent) {
      case 0:  p = request->getParam("text", true);
               Serial.println("content text");Serial.println(p->value().c_str());
               myMatrix.setMode(0, p->value().c_str());
               break;
      case 1:
      case 2:
      case 3:  myMatrix.setMode(myContent, "");          
               break;
      case 5:  p = request->getParam("graphics", true);
               Serial.print("graphics ");//Serial.println(p->value().c_str());
               myMatrix.setMode(5, p->value().c_str());
               break;
      default: myMatrix.setMode(myContent, "");                     
               break;
    }
  }  
  if (request->hasArg("speed")) {
    AsyncWebParameter* p = request->getParam("speed", true);
    Serial.println("request speed ");
    myMatrix.setSpeed(strtol(p->value().c_str(), NULL, 10));
  }
  if (request->hasArg("brightness")) {
    AsyncWebParameter* p = request->getParam("brightness", true);
    Serial.println("request brightness ");
    myMatrix.setBright(strtol(p->value().c_str(), NULL, 10));
  }
  request->send(204, "text/html");
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
    delay(50);
    ESP.restart();
  }  
  
}


void setupTime()
{
  memset(NTPpacket, 0, sizeof(NTPpacket));
  NTPpacket[0]=0b11100011;
  NTPpacket[1]=0;
  NTPpacket[2]=6;
  NTPpacket[3]=0xEC;
  NTPpacket[12]=49;
  NTPpacket[13]=0x4E;
  NTPpacket[14]=49;
  NTPpacket[15]=52;
  DNSClient dns;
  dns.begin(WiFi.dnsIP());
  Serial.println("dns started");
  WiFi.hostByName(TimeServerName,timeServer);
  if(ntpClient.connect(timeServer, TimeServerPort)) {
    Serial.print("NTP UDP connected: "); Serial.println(timeServer);
    ntpClient.onPacket([](AsyncUDPPacket packet) {
        unsigned long highWord = word(packet.data()[40], packet.data()[41]);
        unsigned long lowWord = word(packet.data()[42], packet.data()[43]);
        time_t UnixUTCtime = (highWord << 16 | lowWord)-2208988800UL;
        setTime(UnixUTCtime);
        Serial.println("received NTP packet!");
      });
    } else {Serial.println("No connection to NTP Server possible");} 
  ntpClient.write(NTPpacket, sizeof(NTPpacket));  
}




