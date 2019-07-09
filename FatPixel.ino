//#include <SPI.h>
#include "NeoMatrix.h"
#include <Math.h>

#define WIDTH  10
#define HEIGHT 20

#define PIN_L      2
#define PIN_R      3
#define PIN_U      4
#define PIN_D      5
#define PIN_BTN    6
#define PIN_ESC    7
#define PIN_SCREEN 8
#define PIN_SCORE  10

static bool I[2][16] = {
  {0,0,0,0,
   0,0,0,0,
   1,1,1,1,
   0,0,0,0},
  {0,0,1,0,
   0,0,1,0,
   0,0,1,0,
   0,0,1,0}};

static bool O[2][16] = {
  {0,0,0,0,
   0,1,1,0,
   0,1,1,0,
   0,0,0,0},
  {0,0,0,0,
   0,1,1,0,
   0,1,1,0,
   0,0,0,0}};

static bool T[4][16] = {
  {0,0,0,0,
   0,1,1,1,
   0,0,1,0,
   0,0,0,0},
  {0,0,1,0,
   0,1,1,0,
   0,0,1,0,
   0,0,0,0},
  {0,0,1,0,
   0,1,1,1,
   0,0,0,0,
   0,0,0,0},
  {0,0,1,0,
   0,0,1,1,
   0,0,1,0,
   0,0,0,0}};

static bool S[2][16] = {
  {0,0,0,0,
   0,0,1,1,
   0,1,1,0,
   0,0,0,0},
  {0,1,0,0,
   0,1,1,0,
   0,0,1,0,
   0,0,0,0}};

static bool Z[2][16] = {
  {0,0,0,0,
   0,1,1,0,
   0,0,1,1,
   0,0,0,0},
  {0,0,0,1,
   0,0,1,1,
   0,0,1,0,
   0,0,0,0}};

static bool J[4][16] = {
  {0,0,0,0,
   0,1,1,1,
   0,0,0,1,
   0,0,0,0},
  {0,0,1,0,
   0,0,1,0,
   0,1,1,0,
   0,0,0,0},
  {0,0,0,0,
   0,1,0,0,
   0,1,1,1,
   0,0,0,0},
  {0,0,1,1,
   0,0,1,0,
   0,0,1,0,
   0,0,0,0}};

static bool L[4][16] = {
  {0,0,0,0,
   0,1,1,1,
   0,1,0,0,
   0,0,0,0},
  {0,1,1,0,
   0,0,1,0,
   0,0,1,0,
   0,0,0,0},
  {0,0,0,0,
   0,0,0,1,
   0,1,1,1,
   0,0,0,0},
  {0,0,1,0,
   0,0,1,0,
   0,0,1,1,
   0,0,0,0}};

static byte LEVEL_COLOR[10][3] = {
  {128, 64 , 0},
  {0  , 128, 0},
  {128, 0  , 128},
  {0  , 0  , 128},
  {128, 16 , 32},
  {0  , 128, 64},
  {128, 128, 0},
  {0  , 64 , 128},
  {64 , 0  , 128},
  {128, 128, 128}
};

void tetrisLoop();

bool isCollide(byte* grid, bool* current, short& x, short& y, short& r);
byte checkAndDestroy(byte* grid, short& x);
void plot(byte* grid, bool* current, short& x, short& y, short& r);
void showNextTetromino(bool* next, byte* color_n);

void key_left(byte* grid, bool* current, short& x, short& y, short& r);
void key_right(byte* grid, bool* current, short& x, short& y, short& r);
void key_down(byte* grid, bool* current, short& x, short& y, short& r);
void key_rotate(byte* grid, bool* current, short& x, short& y, short& r);

NeoMatrix matrix(WIDTH, HEIGHT, 4*2);

byte color[3], color_c[3], color_n[3]; //tetromino RGB color
short tetrominoId=0, tetrominoId_c=0, tetrominoId_n=0;

bool interrupt;
unsigned long last_drop = 0;

void setup() {
  pinMode(PIN_L, INPUT_PULLUP);
  pinMode(PIN_R, INPUT_PULLUP);
  pinMode(PIN_U, INPUT_PULLUP);
  pinMode(PIN_D, INPUT_PULLUP);
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_ESC, INPUT_PULLUP);
  
  //Serial.begin(9600);

  //use pin 10 - 13 for SPI
  //SPI.begin();
  //SPI.setBitOrder(LSBFIRST);

  //screenSaverLoop();

  for (int i=0; i<WIDTH; i++) //loading bar
    for (int j=15; j<64; j+=16) {
      matrix.setPixel(i, 10, j, j, j);
      matrix.setPixel(i, 11, j, j, j);
      matrix.pushBuffer(matrix.screen_buffer);
      delay(10);
    }

}

void loop() {
  randomSeed(millis() - 1234);
  interrupt = false;

  tetrisLoop();
}

bool* pickTetromino() {
  tetrominoId = random(0, 7);
  switch (tetrominoId) {
    case 0:
      color[0] = 16; //cyan
      color[1] = 127;
      color[2] = 127;
      return I[0];

    case 1:
      color[0] = 127; //yellow
      color[1] = 127;
      color[2] = 0;
      return O[0];

    case 2:
      color[0] = 127; //magenta
      color[1] = 0;
      color[2] = 127;
      return T[0];

    case 3:
      color[0] = 0; //green
      color[1] = 127;
      color[2] = 0;
      return S[0];

    case 4:
      color[0] = 127; //red
      color[1] = 0;
      color[2] = 0;
      return Z[0];

    case 5:
      color[0] = 0; //blue
      color[1] = 0;
      color[2] = 127;
      return J[0];

    case 6:
      color[0] = 127; //orange
      color[1] = 32;
      color[2] = 0;
      return L[0];
  }
}

bool isCollide(byte* grid, bool* current, short& x, short& y, short& r) {
  for (int i=0; i<4; i++) //check for collision
    for (int j=0; j<4; j++)
      if (current[(16*r) + i + j*4] == 1  &&  y+j > -2) {
        if (y+j+1 > 20) return true;
        if (matrix.isNotBlack(grid, x+i, y+j+1)) return true;
      }

  return false;
}

byte checkAndDestroy(byte* grid, short& x) {
  byte lines = 0;
  for (int j=0; j<20; j++)
    if (matrix.isLineFull(grid, j)) {
      lines++;

      //clear line animation
      for (int i=0; i<5; i++) {
        matrix.setPixel(i, j, 255,255,255);
        matrix.setPixel(9-i, j, 255,255,255);
        matrix.refresh();
        delay(10);
      }
      for (int i=0; i<5; i++) {
        matrix.setPixel(i, j, 0,0,0);
        matrix.setPixel(9-i, j, 0,0,0);
        matrix.refresh();
        delay(10);
      }

      for (int k=j; k>0; k--) //clear and rearrange grid
        for (int i=0; i<10; i++) {
          byte* pixel = matrix.getPixel(grid, i, k-1);
          matrix.setPixel(grid, i, k, pixel[0], pixel[1], pixel[2]);
          free(pixel);
        }

      short temp = 0;
      plot(grid, O[0], temp, temp, temp);
    }

    return lines;
}

void plot(byte* grid, bool* current, short& x, short& y, short& r) {
  memcpy(matrix.screen_buffer, grid, 200*3);

  for (int i=0; i<4; i++) //copy current to screen buffer
    for (int j=0; j<4; j++)
      if (current[(16*r) + i + j*4] == 1  &&  y+j > -1)
        matrix.setPixel(x+i, y+j, color_c[0] / 1.3, color_c[1] / 1.3, color_c[2] / 1.3);
          
  matrix.refresh();
}

void showNextTetromino(bool* next, byte* color_n) {
  for  (int j=0; j<2; j++) //push next tetromino
    for (int i=0; i<4; i++) {
      int index = WIDTH*HEIGHT*3 + ((j%2 == 0)? (4*j + i)*3 : (4*j + 4 - i - 1)*3);
      if (next[i + (j+1) * 4] == 1) {
        matrix.screen_buffer[index]     = color_n[1] / 1.8;
        matrix.screen_buffer[index + 1] = color_n[0] / 1.8;
        matrix.screen_buffer[index + 2] = color_n[2] / 1.8;
      } else {
        matrix.screen_buffer[index]     = 0;
        matrix.screen_buffer[index + 1] = 0;
        matrix.screen_buffer[index + 2] = 0;
      }
  }
}


void key_left(byte* grid, bool* current, short& x, short& y, short& r) {
  for (int i=0; i<4; i++) //check for collision
    for (int j=0; j<4; j++)
      if (current[(16*r) + i + j*4] == 1  &&  y+j > -2) {
        if (x+i <= 0) return;
        if (matrix.isNotBlack(grid, x+i-1, y+j)) return;
      }

  x--;
  plot(grid, current, x, y, r);
}

void key_right(byte* grid, bool* current, short& x, short& y, short& r) {
  for (int i=0; i<4; i++) //check for collision
    for (int j=0; j<4; j++)
      if (current[(16*r) + i + j*4] == 1  &&  y+j > -2) {
        if (x+i+2 > 10) return;
        if (matrix.isNotBlack(grid, x+i+1, y+j)) return;
      }
  
  x++;
  plot(grid, current, x, y, r);
}

void key_down(byte* grid, bool* current, short& x, short& y, short& r) {
  if (!isCollide(grid, current, x, y, r)) {
    y++;
  }
  plot(grid, current, x, y, r);
}

void key_rotate(byte* grid, bool* current, short& x, short& y, short& r) {
  short next = r;

  if (tetrominoId_c == 1) {
     next = 0;

  } else if (tetrominoId_c == 0 || tetrominoId_c == 3 || tetrominoId_c == 4) {
    next++;
    next %= 2;

  } else { //2, 5, 6
    next++;
    next %= 4;
  }

  bool collide = false;

  for (int i=0; i<4; i++) //check for collision
    for (int j=0; j<4; j++)
      if (current[(16*next) + i + j*4] == 1  &&  y+j > -2) 
        if (y+j > 19 ||
            x+i < 0 ||
            x+i > 9 ||
            matrix.isNotBlack(grid, x+i, y+j)) {
              
          collide = true;
          break;
        }
  

  if (!collide) {
    //TODO: collision
    r = next;
    plot(grid, current, x, y, r);
  }

}

#define GAP  40
#define HOLD 300
void tetrisLoop() {
  //initialize
  byte* grid = (byte*) calloc(WIDTH * HEIGHT * 3, 1);

  bool* current = pickTetromino();
  memcpy(color_c, color, 3);
  tetrominoId_c = tetrominoId;

  bool* next = pickTetromino();
  memcpy(color_n, color, 3);
  tetrominoId_n = tetrominoId;

  showNextTetromino(next, color_n);

  short x=3, y=-3, r=0;
  byte lineCount = 0;
  int speed = 450;
  int level = 0;

  unsigned long last_drop = 0;
  bool          l_0, r_0, u_0, d_0, b_0, e_0;
  unsigned long l_t, r_t, u_t, d_t, b_t, e_t;
  l_0=false; r_0=false; u_0=false; d_0=false; b_0=false; e_0=false;
  l_t=0; r_t=0; u_t=0; d_t=0; b_t=0; e_t=0;

  while (!interrupt) {
    // *** INPUT ***
    if (digitalRead(PIN_L) == LOW) {
      if (l_0 == false  &&  l_t + GAP < millis()) { //press
        l_0 = true;
        l_t = millis();
        key_left(grid, current, x, y, r);
      } else if (l_t + HOLD < millis()) { //hold
        l_t = millis + HOLD*2;
        key_left(grid, current, x, y, r);
      }
    } else {
      if (l_t + GAP < millis()) l_0 = false; //up
    }

    if (digitalRead(PIN_R) == LOW) {
      if (r_0 == false  &&  r_t + GAP < millis()) { //press
        r_0 = true;
        r_t = millis();
        key_right(grid, current, x, y, r);
      } else if (r_t + HOLD < millis()) { //hold
        r_t = millis + HOLD*2;
        key_right(grid, current, x, y, r);
      }
    } else {
      if (r_t + GAP < millis()) r_0 = false; //up
    }

    if (digitalRead(PIN_D) == LOW) {
      if (d_0 == false  &&  d_t + GAP < millis()) { //press
        d_0 = true;
        d_t = millis();
        key_down(grid, current, x, y, r);
      } else if (d_t + HOLD < millis()) { //hold
        d_t = millis + HOLD*2;
        key_down(grid, current, x, y, r);
      }
    } else {
      if (d_t + GAP < millis()) d_0 = false; //up
    }

    if (digitalRead(PIN_BTN) == LOW) {
      if (b_0 == false  &&  b_t + GAP*2 < millis()) { //press
        b_0 = true;
        b_t = millis();
        key_rotate(grid, current, x, y, r);
      }
    } else {
      if (b_t + GAP < millis()) b_0 = false; //up
    }

    // *** CHECK ***
    if (last_drop + speed < millis()) {
      last_drop = millis();

      if (isCollide(grid, current, x, y, r)) { //NEXT PIECE
        for (int i=0; i<4; i++) //move current to grid
          for (int j=0; j<4; j++)
            if (current[(16*r) + i + j*4] == 1  &&  y+j > -1)
              matrix.setPixel(grid, x+i, y+j, color_c[0], color_c[1], color_c[2]);

        current = next;
        tetrominoId_c = tetrominoId_n;
        memcpy(color_c, color_n, 3);

        next = pickTetromino();
        tetrominoId_n = tetrominoId;
        memcpy(color_n, color, 3);

        lineCount += checkAndDestroy(grid, x);

        if (lineCount >= 10) { //classic is 30!
          level++;
          lineCount %= 10;
          speed = speed * 2 / 3;
          
          for (int j=0; j<20; j++)
            for (int i=0; i<10; i++)
              if (matrix.isNotBlack(grid, i, j))
                matrix.setPixel(grid, i, j, LEVEL_COLOR[level%10][0], LEVEL_COLOR[level%10][1], LEVEL_COLOR[level%10][2]);
        }

        if (y < 0) //check if game is over
          for (int i=0; i<4; i++)
            for (int j=0; j<4; j++)
              if (current[i + j*4] == 1  &&  y+j < 0) { //game over
                //TODO: score
                delay(2500);
                free(grid);

                for (y=0; y<20; y++) { //clean screen
                  for (x=0; x<10; x++) matrix.setPixel(x, y, 0, 0, 0);
                  delay(33);
                  matrix.refresh();
                }

                //screen saver
                if (b_t + 60000 * 3 < millis()) screenSaverLoop();

                for (y=0; y<20; y++) { //clean screen
                  for (x=0; x<10; x++) matrix.setPixel(x, y, 0, 0, 0);
                  delay(33);
                  matrix.refresh();
                }

                return;
              }

        x=3; y=-3; r=0;

        showNextTetromino(next, color_n);
        matrix.refresh();

      } else { //KEEP DROPING
        key_down(grid, current, x, y, r);         
      }
    }

  } //end loop
}

void screenSaverLoop() {

  for  (int j=0; j<2; j++) //clear next tetromino
    for (int i=0; i<4; i++) {
      int index = WIDTH*HEIGHT*3 + ((j%2 == 0)? (4*j + i)*3 : (4*j + 4 - i - 1)*3);
      matrix.screen_buffer[index]     = 0;
      matrix.screen_buffer[index + 1] = 0;
      matrix.screen_buffer[index + 2] = 0;
    }
  matrix.refresh();

  byte count = 0;

  double R  = 2;
  byte px   = 5;
  byte py   = 10;
  int  sx   = 1;
  int  sy   = 1;

  int fr, fg, fb;
  int ar, ag, ab;

  fr = 255;
  fg = 128;
  fb = 0;

  ar=-1;
  ag=-2;
  ab=1;

  while (!interrupt) {
    //check input
    if (digitalRead(PIN_BTN) == LOW) return;

    matrix.subtract(1);

    //render
    for (int x=-R; x<R; x++)
      for (int y=-R; y<R; y++) {
        if (px+x > 9 || px+x < 0) break;
        if (py+y > 19 || py+y < 0) break;

        double d = sqrt(x*x + y*y);
        double a = R - sqrt(x*x + y*y);

        if (a > 0)
          if (a > 1) {
            matrix.setPixel(px+x,py+y, fr, fg, fb);
          } else {
            matrix.setPixel(px+x,py+y, a*fr/2, a*fg/2, a*fb/2);
          }
      }

    matrix.setPixel(px+sx, py+sy, fr, fg, fb);

    matrix.refresh();
    delay(33);
    
    px += sx;
    py += sy;
    if (px > WIDTH-R)  sx = -1;
    if (px < 0+R)      sx = 1;
    if (py > HEIGHT-R) sy = -1;
    if (py < 0+R)      sy = 1;

    fr += ar;
    fg += ag;
    fb += ab;
    if (fr >= 255) ar = -random(2, 4);
    if (fr <= 0)   ar =  random(2, 4);
    if (fg >= 255) ag = -random(2, 4);
    if (fg <= 0)   ag =  random(2, 4);
    if (fb >= 255) ab = -random(2, 4);
    if (fb <= 0)   ab =  random(2, 4);
  }
}