#include "AllIncludes.h"

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
        Serial.print("Unix UTC time is " );
        Serial.println(UnixUTCtime);
        setTime(UnixUTCtime);
      });
    } else {Serial.println("No connection to NTP Server possible");} 
  ntpClient.write(NTPpacket, sizeof(NTPpacket));  
}


char* byte2text(uint16_t value) 
{
  memset(convbuffer, 0, sizeof(convbuffer));
  if (value < 10) {
    convbuffer[0]=0x30;
    convbuffer[1]=value+0x30;
  } else 
  itoa(value,convbuffer, 10);
  return convbuffer;
}

void loadLongTimeDate(time_t t)
{   memset(tape, 0, sizeof(tape));
    strlcat(tape, dayStr(weekday(t)),  sizeof(tape));
    strlcat(tape, " ",  sizeof(tape));
    strlcat(tape, byte2text(day(t)),  sizeof(tape));
    strlcat(tape, ". ",  sizeof(tape));
    strlcat(tape, monthStr(month(t)),  sizeof(tape));
    strlcat(tape, " ",  sizeof(tape));
    strlcat(tape, byte2text(year(t)),  sizeof(tape));
    strlcat(tape, " ",  sizeof(tape));
    strlcat(tape, byte2text(hour(t)),  sizeof(tape));
    strlcat(tape, ":",  sizeof(tape));
    strlcat(tape, byte2text(minute(t)),  sizeof(tape));
    strlcat(tape, ":",  sizeof(tape));
    strlcat(tape, byte2text(second(t)),  sizeof(tape));
    strlcat(tape, " ",  sizeof(tape));
  
}

void loadShortTimeDate(time_t t){
    memset(tape, 0, sizeof(tape));
    strlcat(tape, byte2text(day(t)),  sizeof(tape));
    strlcat(tape, "-",  sizeof(tape));
    strlcat(tape, byte2text(month(t)),  sizeof(tape));
    strlcat(tape, "-",  sizeof(tape));
    strlcat(tape, byte2text(year(t)),  sizeof(tape));
    strlcat(tape, " ",  sizeof(tape));
    strlcat(tape, byte2text(hour(t)),  sizeof(tape));
    strlcat(tape, ":",  sizeof(tape));
    strlcat(tape, byte2text(minute(t)),  sizeof(tape));
    strlcat(tape, ":",  sizeof(tape));
    strlcat(tape, byte2text(second(t)),  sizeof(tape));
    strlcat(tape, " ",  sizeof(tape));
}

void loadLocalTime(void){
    localTime = CE.toLocal(now(), &tcr);
    memset(tape, 0, sizeof(tape));
    strlcat(tape, byte2text(hour(localTime)),  sizeof(tape));
    strlcat(tape, ":",  sizeof(tape));
    strlcat(tape, byte2text(minute(localTime)),  sizeof(tape));
    strlcat(tape, " ",  sizeof(tape));
}


