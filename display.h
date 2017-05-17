#ifndef display_h
#define display_h

#define pinCS     12  // MISO  aka D6 zu CS des Displays verbinden
#define pinDIN    13  // MOSI  aka D7 zu Data IN des Displays
#define pinCLOCK  14  // CLOCK aka D5 zu Clock des Displays
#define FONT_W     5  // Breite der Zeichen in Pixel

#include <Max72xxPanel.h>       // https://github.com/markruys/arduino-Max72xxPanel.git

class LEDmatrix {
  public:
    LEDmatrix(uint8_t numHoriz, uint8_t spacer);
    void loop();
    void setBright(uint8_t brightness);
    void setSpeed(uint8_t speedval);
    void setMode(uint8_t mode, const char *content);

  private:
    Max72xxPanel _matrix;
    uint8_t   _pinCS;
    uint8_t   _numHoriz;  //number of horizontal elements in matrix
    uint8_t   _width;     // width of character incl. spacers
    uint8_t   _brightness;
    uint8_t   _spacer;    // distance between characters
    uint16_t  _wait;      // time to wait between single frames, calculated from speed
    int16_t   _screenStart;
    int16_t   _screenEnd;
    uint32_t  _lastRun;
    int16_t   _currPos;
    uint8_t   _contentMode;
    uint16_t  _contentPt;
    char      _content[200];
    File      _graphFile; 
    int16_t   _nextGraphPos;
    
    void      _loopDateTime();
    void      _loopTime();
    void      _loopText(boolean doLoop);
    void      _loopGraphics();
    void      _loopIP();
    void      _drawLine(int16_t x, uint8_t matrixElem, uint8_t color, uint8_t bg);

};



#endif //display_h
