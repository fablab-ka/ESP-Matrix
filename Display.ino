#include "AllIncludes.h"

LEDmatrix::LEDmatrix(uint8_t numHoriz, uint8_t spacer) : _matrix(pinCS, numHoriz, spacer) {
  _numHoriz    = numHoriz;
  _spacer      = spacer;
  _width       = _spacer+FONT_W;
  _screenStart = -1 * FONT_W;
  _screenEnd   = (_numHoriz*8)-1;
  _lastRun     = millis();
  _currPos     = _screenEnd;
  _contentPt   = 0;
  _contentMode = 4;
  _wait        = 100;
  _brightness  = 2;
  for (uint8_t i=0; i<_numHoriz; i++) {
    _matrix.setPosition(i, i, 0);
    _matrix.setRotation(i, 1);
  }
  _matrix.fillScreen(LOW);
  _matrix.setIntensity(_brightness);
  _matrix.write();
}

void LEDmatrix::setBright(uint8_t brightness){
  _brightness=_min(15, brightness);
}

void LEDmatrix::setSpeed(uint8_t speedval){
  _wait = (450/(_min(200,speedval)+1))+48;
}

void LEDmatrix::setMode(uint8_t mode, const char *content){
  //ntpClient.write(NTPpacket, sizeof(NTPpacket));
  _matrix.fillScreen(LOW);_matrix.write();
  strlcpy( _content, content, sizeof(_content));
  if(0 == strlen(_content)) { strlcpy(_content, "no text! ", sizeof(_content)); }
  _contentMode = mode;
  _currPos     = _screenEnd;
  _contentPt   = 0;
  if (0 == _contentMode) {  // normal Text
    for( uint16_t j=0; j<=strlen(_content); j++){
      // map certain latin1 characters to extended ascii cp437
      switch(_content[j]) {
        case 0xA7: _content[j]=0x15; break; // §
        case 0xB5: _content[j]=0xE5; break; // µ
        case 0xB0: _content[j]=0xF7; break; // °
        case 0xB2: _content[j]=0xFC; break; // ²
        case 0xFC: _content[j]=0x81; break; // ü
        case 0xE4: _content[j]=0x84; break; // ä
        case 0xEB: _content[j]=0x89; break; // ë
        case 0xC4: _content[j]=0x8E; break; // Ä
        case 0xCB: _content[j]=0x45; break; // Ë->E
        case 0xD8: _content[j]=0xEC; break; // 
        case 0xF6: _content[j]=0x94; break; // ö
        case 0xD6: _content[j]=0x99; break; // Ö
        case 0xDF: _content[j]=0xE1; break; // ß
        case 0xDC: _content[j]=0x9A; break; // Ü
        case 0xF7: _content[j]=0xF5; break; // 
      }
    }
  }
  if (5 == _contentMode) {  // Graphics
    if (_graphFile.available()) { _graphFile.close(); }
    _graphFile = SPIFFS.open(_content, "r");
  }  
}

void LEDmatrix::loop() {
  _matrix.setIntensity(_brightness);
  switch( _contentMode) {
    case  0: _loopText(true); break;
    case  1:
    case  2:
    case  3: _loopDateTime(); break;
    case  5: _loopGraphics(); break;
    default:  strlcpy(_content, myIPaddress, sizeof(_content)); strlcat(_content, "  ", sizeof(_content));  _loopText(true); 
  }
  
}

void LEDmatrix::_loopText(boolean doLoop){
  _matrix.drawChar(_currPos, 0,  _content[_contentPt], HIGH, LOW, 1);
  int16_t nextPos    = _currPos + _width;
  uint8_t nextCtPt = _contentPt;
  while (nextPos <= _screenEnd) {
    nextCtPt = (nextCtPt+1)% strlen(_content);
    _matrix.drawChar(nextPos, 0, _content[nextCtPt], HIGH, LOW, 1);
    nextPos    += _width;
  }
  if (doLoop) {
    if (_currPos <= _screenStart) {
      _currPos =0; _contentPt = (_contentPt+1)% strlen(_content);
    } else {
      _currPos--;
    }  
  }   
  _matrix.write();
  while ((_lastRun+_wait) > millis()) {yield();};
  _lastRun=millis();
}

void LEDmatrix::_drawLine(int16_t x, uint8_t matrixElem, uint8_t color, uint8_t bg) {
  uint8_t bitPos = 1;
  for(int8_t i=0; i<8; i++ ) {
    _matrix.drawPixel(x, i, ((matrixElem & bitPos)?color:bg));
    bitPos = bitPos << 1;
  }
}

void LEDmatrix::_loopGraphics(){
  uint8_t graphLine[1];
  _graphFile.seek(_contentPt,SeekSet);
  _graphFile.read(graphLine, sizeof(graphLine));
  _drawLine(_currPos, graphLine[0], HIGH, LOW); 
  int16_t nextPos  = _currPos + 1;
  uint8_t nextCtPt = _contentPt;
  
  while (nextPos <= _screenEnd) {
    nextCtPt = ((nextCtPt+1)% _graphFile.size());
    _graphFile.seek(nextCtPt,SeekSet);
    _graphFile.read(graphLine, sizeof(graphLine));
  
    _drawLine(nextPos, graphLine[0], HIGH, LOW); 
    nextPos++;
  }
  if (_currPos <= 0) {
    _currPos =0; _contentPt = ((_contentPt+1) % _graphFile.size());
  } else {
    _currPos--;
  }
  _matrix.write();  
  while ((_lastRun+(_wait)) > millis()) {yield();};
  _lastRun=millis();
//  Serial.print("After Loop| Len: " );Serial.print(len);Serial.print(" _graphBufLen: " );Serial.print(_graphBufLen);Serial.print(" _nextGraphPos: " );Serial.print(_nextGraphPos);
//  Serial.print(" _currPos: " );Serial.print(_currPos);Serial.print(" _contentPt: " );Serial.println(_contentPt);
}

char* byte2text(uint16_t value){
  static char convbuffer[6];
  memset(convbuffer, 0, sizeof(convbuffer));
  if (value < 10) {
    convbuffer[0]=0x30;
    convbuffer[1]=value+0x30;
  } else 
  itoa(value,convbuffer, 10);
  return convbuffer;
}



void LEDmatrix::_loopDateTime(){
  time_t t = CE.toLocal(now(), &tcr);
  memset(_content, 0, sizeof(_content));
  switch (_contentMode) {
    case 1: 
        _matrix.fillScreen(LOW);
        strlcat(_content, dayStr(weekday(t)),  sizeof(_content));
        strlcat(_content, " ",  sizeof(_content));
        strlcat(_content, byte2text(day(t)),  sizeof(_content));
        strlcat(_content, ". ",  sizeof(_content));
        strlcat(_content, monthStr(month(t)),  sizeof(_content));
        strlcat(_content, " ",  sizeof(_content));
        strlcat(_content, byte2text(year(t)),  sizeof(_content));
        strlcat(_content, " ",  sizeof(_content));
        strlcat(_content, byte2text(hour(t)),  sizeof(_content));
        strlcat(_content, ":",  sizeof(_content));
        strlcat(_content, byte2text(minute(t)),  sizeof(_content));
        strlcat(_content, ":",  sizeof(_content));
        strlcat(_content, byte2text(second(t)),  sizeof(_content));
        strlcat(_content, " ",  sizeof(_content));
        _loopText(true);
        break;
    case 2:
        _matrix.fillScreen(LOW);
        strlcat(_content, byte2text(day(t)),  sizeof(_content));
        strlcat(_content, "-",  sizeof(_content));
        strlcat(_content, byte2text(month(t)),  sizeof(_content));
        strlcat(_content, "-",  sizeof(_content));
        strlcat(_content, byte2text(year(t)),  sizeof(_content));
        strlcat(_content, " ",  sizeof(_content));
        strlcat(_content, byte2text(hour(t)),  sizeof(_content));
        strlcat(_content, ":",  sizeof(_content));
        strlcat(_content, byte2text(minute(t)),  sizeof(_content));
        strlcat(_content, ":",  sizeof(_content));
        strlcat(_content, byte2text(second(t)),  sizeof(_content));
        strlcat(_content, " ",  sizeof(_content));
        _loopText(true);
        break;
    case 3: 
        _currPos=2;
         _matrix.fillScreen(LOW);
        strlcat(_content, byte2text(hour(t)),  sizeof(_content));
        strlcat(_content, ":",  sizeof(_content));
        strlcat(_content, byte2text(minute(t)),  sizeof(_content));
        _loopText(false);
        break;
    default: strlcpy(_content,  "no valid timeformat ", sizeof(_content)); break;
  }    
  if(((minute(t)) % 20) == 0){
    Serial.println("NTP query send!");
    ntpClient.write(NTPpacket, sizeof(NTPpacket));
  }
}




