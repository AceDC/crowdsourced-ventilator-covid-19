/*

  This program provides cartesian type graph function

  Revisions
  rev     date        author      description
  1       12-24-2015  kasprzak    initial creation

  Updated by Bodmer to be an example for the library here:
  https://github.com/Bodmer/TFT_HX8357
  
*/
#include <Wire.h>
#include "Adafruit_MPRLS.h"
#include <TFT_HX8357.h> // Hardware-specific library


// You dont *need* a reset and EOC pin for most uses, so we set to -1 and don't connect
#define RESET_PIN  -1  // set to any GPIO pin # to hard-reset on begin()
#define EOC_PIN    -1  // set to any GPIO pin to read end-of-conversion by pin
Adafruit_MPRLS mpr = Adafruit_MPRLS(RESET_PIN, EOC_PIN);

TFT_HX8357 tft = TFT_HX8357();       // Invoke custom library


#define LTBLUE    0xB6DF
#define LTTEAL    0xBF5F
#define LTGREEN   0xBFF7
#define LTCYAN    0xC7FF
#define LTRED     0xFD34
#define LTMAGENTA 0xFD5F
#define LTYELLOW  0xFFF8
#define LTORANGE  0xFE73
#define LTPINK    0xFDDF
#define LTPURPLE  0xCCFF
#define LTGREY    0xE71C

#define BLUE      0x001F
#define TEAL      0x0438
#define GREEN     0x07E0
#define CYAN      0x07FF
#define RED       0xF800
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0
#define ORANGE    0xFC00
//#define PINK      0xF81F
#define PURPLE    0x8010
#define GREY      0xC618
#define WHITE     0xFFFF
#define BLACK     0x0000

#define DKBLUE    0x000D
#define DKTEAL    0x020C
#define DKGREEN   0x03E0
#define DKCYAN    0x03EF
#define DKRED     0x6000
#define DKMAGENTA 0x8008
#define DKYELLOW  0x8400
#define DKORANGE  0x8200
#define DKPINK    0x9009
#define DKPURPLE  0x4010
#define DKGREY    0x4A49

// these are the only external variables used by the graph function
// it's a flag to draw the coordinate system only on the first call to the Graph() function
// and will mimize flicker
// also create some variables to store the old x and y, if you draw 2 graphs on the same display
// you will need to store ox and oy per each display
boolean display1 = true;
boolean update1 = true;

double ox = -999, oy = -999; // Force them to be off screen

double lastx = -999;

/*

  function to draw a cartesian coordinate system and plot whatever data you want
  just pass x and y and the graph will be drawn

  huge arguement list
  &d name of your display object
  x = x data point
  y = y datapont
  gx = x graph location (lower left)
  gy = y graph location (lower left)
  w = width of graph
  h = height of graph
  xlo = lower bound of x axis
  xhi = upper bound of x asis
  xinc = division of x axis (distance not count)
  ylo = lower bound of y axis
  yhi = upper bound of y asis
  yinc = division of y axis (distance not count)
  title = title of graph
  xlabel = x asis label
  ylabel = y asis label
  &redraw = flag to redraw graph on first call only
  color = plotted trace colour
*/


void Graph(TFT_HX8357 &tft, double x, double y, byte dp,
                           double gx, double gy, double w, double h,
                           double xlo, double xhi, double xinc,
                           double ylo, double yhi, double yinc,
                           char *title, char *xlabel, char *ylabel,
                           boolean &redraw, unsigned int color) {

  double ydiv, xdiv;
  double i;
  double temp;
  int rot, newrot;

  // gcolor = graph grid colors
  // acolor = axes line colors
  // pcolor = color of your plotted data
  // tcolor = text color
  // bcolor = background color
  unsigned int gcolor = DKBLUE;
  unsigned int acolor = RED;
  unsigned int pcolor = color;
  unsigned int tcolor = WHITE;
  unsigned int bcolor = BLACK;

  if (redraw == true) {

    redraw = false;
    // initialize old x and old y in order to draw the first point of the graph
    // but save the transformed value
    // note my transform funcition is the same as the map function, except the map uses long and we need doubles
    //ox = (x - xlo) * ( w) / (xhi - xlo) + gx;
    //oy = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;

    tft.setTextDatum(MR_DATUM);

    // draw y scale
    for ( i = ylo; i <= yhi; i += yinc) {
      // compute the transform
      temp =  (i - ylo) * (gy - h - gy) / (yhi - ylo) + gy;

      if (i == 0) {
        tft.drawLine(gx, temp, gx + w, temp, acolor);
        tft.setTextColor(acolor, bcolor);
        tft.drawString(xlabel, (int)(gx + w) , (int)temp, 2);
      }
      else {
        tft.drawLine(gx, temp, gx + w, temp, gcolor);
      }
      // draw the axis labels
      tft.setTextColor(tcolor, bcolor);
      // precision is default Arduino--this could really use some format control
      tft.drawFloat(i, dp, gx - 4, temp, 1);
    }

    // draw x scale
    for (i = xlo; i <= xhi; i += xinc) {

      // compute the transform
      temp =  (i - xlo) * ( w) / (xhi - xlo) + gx;
      if (i == 0) {
        tft.drawLine(temp, gy, temp, gy - h, acolor);
        tft.setTextColor(acolor, bcolor);
        tft.setTextDatum(BC_DATUM);
        tft.drawString(ylabel, (int)temp, (int)(gy - h - 8) , 2);
      }
      else {
        tft.drawLine(temp, gy, temp, gy - h, gcolor);
      }
      // draw the axis labels
      tft.setTextColor(tcolor, bcolor);
      tft.setTextDatum(TC_DATUM);
      // precision is default Arduino--this could really use some format control
      tft.drawFloat(i, dp, temp, gy + 7, 1);
    }

    //now draw the graph labels
    tft.setTextColor(tcolor, bcolor);
    tft.drawString(title, (int)(gx + w / 2) , (int)(gy - h - 30), 4);
  }

  // the coordinates are now drawn, plot the data
  // the entire plotting code are these few lines...
  // recall that ox and oy are initialized above
  //x =  (x - xlo) * ( w) / (xhi - xlo) + gx;
  //y =  (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
  //tft.drawLine(ox, oy, x, y, pcolor);
  // it's up to you but drawing 2 more lines to give the graph some thickness
  //tft.drawLine(ox, oy + 1, x, y + 1, pcolor);
  //tft.drawLine(ox, oy - 1, x, y - 1, pcolor);
  //ox = x;
  //oy = y;

}

void Trace(TFT_HX8357 &tft, double x,  double y,  byte dp,
           double gx, double gy,
           double w, double h,
           double xlo, double xhi, double xinc,
           double ylo, double yhi, double yinc,
           char *title, char *xlabel, char *ylabel,
           boolean &update1, unsigned int color)
{
  double ydiv, xdiv;
  double i;
  double temp;
  int rot, newrot;

  //unsigned int gcolor = DKBLUE;   // gcolor = graph grid color
  unsigned int acolor = RED;        // acolor = main axes and label color
  unsigned int pcolor = color;      // pcolor = color of your plotted data
  unsigned int tcolor = WHITE;      // tcolor = text color
  unsigned int bcolor = BLACK;      // bcolor = background color

  // initialize old x and old y in order to draw the first point of the graph
  // but save the transformed value
  // note my transform funcition is the same as the map function, except the map uses long and we need doubles
  if (update1) {
    update1 = false;
    
    ox = (x - xlo) * ( w) / (xhi - xlo) + gx;
    oy = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;

    if ((ox < gx) || (ox > gx+w)) {update1 = true; return;}
    if ((oy < gy-h) || (oy > gy)) {update1 = true; return;}
    

    tft.setTextDatum(MR_DATUM);

    // draw y scale
    for ( i = ylo; i <= yhi; i += yinc) {
      // compute the transform
      temp =  (i - ylo) * (gy - h - gy) / (yhi - ylo) + gy;

      if (i == 0) {
        tft.setTextColor(acolor, bcolor);
        tft.drawString(xlabel, (int)(gx + w) , (int)temp, 2);
      }
      // draw the axis labels
      tft.setTextColor(tcolor, bcolor);
      // precision is default Arduino--this could really use some format control
      tft.drawFloat(i, dp, gx - 4, temp, 1);
    }

    // draw x scale
    for (i = xlo; i <= xhi; i += xinc) {

      // compute the transform
      temp =  (i - xlo) * ( w) / (xhi - xlo) + gx;
      if (i == 0) {
        tft.setTextColor(acolor, bcolor);
        tft.setTextDatum(BC_DATUM);
        tft.drawString(ylabel, (int)temp, (int)(gy - h - 8) , 2);
      }

      // draw the axis labels
      tft.setTextColor(tcolor, bcolor);
      tft.setTextDatum(TC_DATUM);
      // precision is default Arduino--this could really use some format control
      tft.drawFloat(i, dp, temp, gy + 7, 1);
    }

    //now draw the graph labels
    tft.setTextColor(tcolor, bcolor);
    tft.drawString(title, (int)(gx + w / 2) , (int)(gy - h - 30), 4);
  }

  // the coordinates are now drawn, plot the data
  // the entire plotting code are these few lines...
  // recall that ox and oy are initialized above
  x =  (x - xlo) * ( w) / (xhi - xlo) + gx;
  y =  (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;

  if ((x < gx) || (x > gx+w)) {update1 = true; return;}
  if ((y < gy-h) || (y > gy)) {update1 = true; return;}
    
    
  tft.drawLine(ox, oy, x, y, pcolor);
  // it's up to you but drawing 2 more lines to give the graph some thickness
  //tft.drawLine(ox, oy + 1, x, y + 1, pcolor);
  //tft.drawLine(ox, oy - 1, x, y - 1, pcolor);
  ox = x;
  oy = y;

}

/*

  End of graphing function

*/

/*
 * rr = respiratory rate in cycles per minute
 * ie = expiratory time over inspiratory time
 * peep = post end epiratory pressure
 * cmpt = time to compress bag
 */

double readPressure(int rr, double ie, int peep, double cmpt) {
  
  int period = 4000;
  double dt = (millis() % period) / 1000.0;
  double inspTime = period / 1000.0 / (ie + 1.0);
  int K1 = -3;
  int K2 = -10;
  int K3 = -5;

  double y1 = peep + (40 - peep) * (1 - exp(K1 * cmpt));
  double y2 = y1 - 2 * (1 - exp(K2 * (inspTime - cmpt)));

  if (dt <= cmpt) {
    return peep + (40 - peep) * ( 1 - exp(K1 * dt ));
  } else if (dt <= inspTime) {
    return y1 - 2 * (1 - exp(K2 * (dt - cmpt)));
  } else {
    return y2 - (y2 - peep) * (1 - exp(K3 * (dt - inspTime)));
  }
  return 0;
}



void setup() {
  double x, y, lastx;
  boolean redraw = false;
  
  mpr.begin();
  tft.begin();
  tft.fillScreen(BLACK);
  tft.setRotation(1);

  tft.setCursor(0,0,4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println("RR: 15bpm  I/E: 1.0");
  tft.println("PpeakAlarm: >40  PplatAlarm: >30");  
  tft.println("Ppeak: 38  Pplat: 36  PEEP: 12");

  Graph(tft, x, y, 1, 60, 290, 390, 180, 0, 10, 1, 0, 45, 5, "", "", "", display1, YELLOW);

}


void loop(void) {
  double pressure_hPa = mpr.readPressure();
  double pressure_cmh2o = pressure_hPa * 1.01974428892 - 1030.22;
  //double pressure_cmh2o = 10;
  double x = (millis() % 10000) / 1000.0;
  //double y = readPressure(15, 1, 12, 1);
  double y = pressure_cmh2o;
  boolean redraw = true;
  
  if (lastx > x) {
    ox = x + 60;
    tft.fillRect(1, 100, 480, 200, BLACK);
    Graph(tft, x, y, 1, 60, 290, 390, 180, 0, 10, 1, 0, 45, 5, "Pressure", "Time(s)", "cmH2o", redraw, YELLOW);
  }
  Trace(tft, x, y, 1, 60, 290, 390, 180, 0, 10, 1, 0, 45, 5, "Pressure", "Time(s)", "cmH2o", update1, YELLOW);
  lastx = x;
  delay(50);
}
