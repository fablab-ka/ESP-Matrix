#include "AllIncludes.h"


void setupMatrix()
{
  matrix.setIntensity(BRIGHTNESS); // Use a value between 0 and 15 for brightness
  matrix.setPosition(0, 0, 0); // The first display is at <0, 0>
  matrix.setPosition(1, 1, 0); // The second display is at <1, 0>
  matrix.setPosition(2, 2, 0); // The third display is at <2, 0>
  matrix.setPosition(3, 3, 0); // And the last display is at <3, 0>
  matrix.setRotation(0, 1);    // The first display is position upside down
  matrix.setRotation(1, 1);    
  matrix.setRotation(2, 1);    
  matrix.setRotation(3, 1);    // The same hold for the last display
  matrix.fillScreen(LOW);
}


void MatrixLoopText(){
  MatrixLoopText(WAIT);
}

void MatrixLoopText(uint16_t wait)
{
  matrix.drawChar(pos, y,  tape[letter], HIGH, LOW, 1);
  int16_t nextPos    = pos + width;
  uint8_t nextLetter = letter;
  while (nextPos <= screenEnd) {
    nextLetter = (nextLetter+1)% strlen(tape);
    matrix.drawChar(nextPos, y, tape[nextLetter], HIGH, LOW, 1);
    nextPos    = nextPos+width;
  }
  if (pos<=screenStart) {
    pos =0; letter = (letter+1) % strlen(tape) ;
  } else {
    pos--;
  }   
  matrix.write();
  while ((lastrun+wait) > millis()) {};
  lastrun=millis();

}

void MatrixLoopDateTime()
{
  MatrixLoopDateTime(WAIT);
}

void MatrixLoopDateTime(uint16_t wait)
{
  localTime = CE.toLocal(now(), &tcr);
  switch (myContent) {
    case 1: loadLongTimeDate(localTime); break;
    case 2: loadShortTimeDate(localTime); break;
    default: break;
  }    
  MatrixLoopText(wait);
  if((minute(now()) % 20) == 0){
    // sent NTP-Query every 20 minutes
    ntpClient.write(NTPpacket, sizeof(NTPpacket));
  }
}

void MatrixShowText()
{
  MatrixShowText(WAIT);
}

void MatrixShowText(uint16_t wait)
{
  pos = 1;
  matrix.fillScreen(LOW);
  matrix.drawChar(pos, y,  tape[0], HIGH, LOW, 1);
  int16_t nextPos    = pos + width;
  uint8_t nextLetter = 0;
  while (nextPos <= screenEnd) {
    nextLetter = (nextLetter+1)% strlen(tape);
    matrix.drawChar(nextPos, y, tape[nextLetter], HIGH, LOW, 1);
    nextPos    = nextPos+width;
  }
  matrix.write();
  while ((lastrun+wait) > millis()) {};
  lastrun=millis();
  if((minute(now()) % 20) == 0){
    // sent NTP-Query every 20 minutes
    ntpClient.write(NTPpacket, sizeof(NTPpacket));
  }

}


