#include <avr/pgmspace.h>

constexpr uint8_t COL_START = 2;
constexpr uint8_t CLOCK_4017 = 10;
constexpr uint8_t RESET_4017 = 11;
constexpr uint8_t BTN_DIR   = A0;
constexpr uint8_t BTN_ESQ = A1;
constexpr uint8_t BTN_CIMA  = A2;
constexpr uint8_t BTN_BAIXO  = A3;

struct Point { int8_t x, y; };

const uint8_t menuIcons[3][8] PROGMEM = {
  {0x00,0x00,0x38,0x28,0x38,0x08,0x08,0x00},
  {0x10,0x38,0x7C,0xEE,0xEE,0x00,0x00,0x00},
  {0x00,0x40,0x40,0xC0,0x40,0x40,0x00,0x00}
};

const uint16_t shapes[7][4] PROGMEM = {
  {0x0F00, 0x4444, 0x0F00, 0x4444},
  {0x0660, 0x0660, 0x0660, 0x0660},
  {0x0E40, 0x4C40, 0x4E00, 0x4640},
  {0x0E80, 0x6440, 0x2E00, 0x44C0},
  {0x0E20, 0x4460, 0x8E00, 0xC440},
  {0x06C0, 0x4620, 0x06C0, 0x4620},
  {0x0C60, 0x2640, 0x0C60, 0x2640}
};

void draw(uint8_t frame[8]) {
  digitalWrite(RESET_4017, HIGH);
  delayMicroseconds(10);
  digitalWrite(RESET_4017, LOW);

  for (uint8_t r = 0; r < 8; r++) {
    for (uint8_t c = 0; c < 8; c++) {
      bool pixel = (frame[r] >> (7 - c)) & 1;
      digitalWrite(COL_START + c, pixel ? LOW : HIGH); 
    }
    delayMicroseconds(1200);
    for (uint8_t i = 0; i < 8; i++) digitalWrite(COL_START + i, HIGH);
    digitalWrite(CLOCK_4017, HIGH);
    delayMicroseconds(10);
    digitalWrite(CLOCK_4017, LOW);
  }
}


void runSnake() {
  Point s[20]; uint8_t len = 3;
  int8_t dx = 1, dy = 0;
  Point food = {random(0,8), random(0,8)};
  s[0] = {4,4}; s[1] = {3,4}; s[2] = {2,4};
  uint32_t t = 0;
  while(true) {
    if (digitalRead(BTN_ESQ) == LOW && dx == 0) { dx=-1; dy=0; }
    if (digitalRead(BTN_DIR) == LOW && dx == 0) { dx=1; dy=0; }
    if (digitalRead(BTN_CIMA) == LOW && dy == 0) { dx=0; dy=-1; }
    if (digitalRead(BTN_BAIXO) == LOW && dy == 0) { dx=0; dy=1; }
    if (millis() - t > 250) {
      Point next = {(int8_t)(s[0].x + dx), (int8_t)(s[0].y + dy)};
      if (next.x<0 || next.x>7 || next.y<0 || next.y>7) return; 
      for(int i=len-1; i>0; i--) {
        s[i] = s[i-1];
        if(next.x == s[i].x && next.y == s[i].y) return;
      }
      s[0] = next;
      if (s[0].x == food.x && s[0].y == food.y) {
        if(len < 20) len++;
        food = {random(0,8), random(0,8)};
      }
      t = millis();
    }
    uint8_t b[8] = {0};
    b[food.y] |= (1 << (7-food.x));
    for(int i=0; i<len; i++) b[s[i].y] |= (1 << (7-s[i].x));
    draw(b);
  }
}

void runSpaceInvaders() {
  int8_t px = 3, ex = 0, edir = 1, ey = 0;
  uint8_t enemies[2] = {0b01111110, 0b01111110};
  int8_t bx = -1, by = -1;
  uint32_t tE = 0, tB = 0;
  while(true) {
    if (digitalRead(BTN_ESQ) == LOW && px > 0) { px--; delay(50); }
    if (digitalRead(BTN_DIR) == LOW && px < 7) { px++; delay(50); }
    if (digitalRead(BTN_CIMA) == LOW && by == -1) { bx = px; by = 6; }
    if (millis() - tE > 800) {
      ex += edir;
      if (ex > 1 || ex < -1) { edir *= -1; ey++; }
      if (ey > 5) return;
      tE = millis();
    }
    if (millis() - tB > 50 && by != -1) {
      by--;
      for (int r=0; r<2; r++) {
        if (by == ey + r) {
          int8_t rel = bx - ex;
          if (rel >= 0 && rel < 8 && (enemies[r] & (1 << (7-rel)))) {
            enemies[r] &= ~(1 << (7-rel)); by = -1;
          }
        }
      }
      tB = millis();
    }
    if (enemies[0] == 0 && enemies[1] == 0) return;
    uint8_t b[8] = {0};
    for(int r=0; r<2; r++) if(ey+r<8) b[ey+r] = (ex>=0) ? (enemies[r] >> ex) : (enemies[r] << -ex);
    if(by != -1) b[by] |= (1 << (7-bx));
    b[7] |= (1 << (7-px));
    draw(b);
  }
}

void runTetris() {
  uint8_t grid[8] = {0};
  int8_t px = 2, py = -1, pt = random(0, 7), pr = 0;
  uint32_t t = 0;

  auto collide = [&](int8_t nx, int8_t ny, int8_t nr) {
    uint16_t shape = pgm_read_word(&(shapes[pt][nr]));
    for (int8_t i = 0; i < 4; i++) {
      for (int8_t j = 0; j < 4; j++) {
        if ((shape >> (15 - (i * 4 + j))) & 1) {
          int8_t gx = nx + j;
          int8_t gy = ny + i;
          if (gx < 0 || gx > 7 || gy > 7) return true;
          if (gy >= 0 && (grid[gy] & (1 << (7 - gx)))) return true;
        }
      }
    }
    return false;
  };

  while (true) {
    if (digitalRead(BTN_ESQ) == LOW) { if (!collide(px - 1, py, pr)) px--; delay(120); }
    if (digitalRead(BTN_DIR) == LOW) { if (!collide(px + 1, py, pr)) px++; delay(120); }
    if (digitalRead(BTN_CIMA) == LOW) { 
      int8_t nextR = (pr + 1) % 4;
      if (!collide(px, py, nextR)) pr = nextR; 
      delay(200); 
    }
    
    uint16_t currentDrop = (digitalRead(BTN_BAIXO) == LOW) ? 70 : 500;

    if (millis() - t > currentDrop) {
      if (!collide(px, py + 1, pr)) {
        py++;
      } else {
        uint16_t shape = pgm_read_word(&(shapes[pt][pr]));
        for (int8_t i = 0; i < 4; i++) {
          for (int8_t j = 0; j < 4; j++) {
            if ((shape >> (15 - (i * 4 + j))) & 1) {
              if (py + i < 0) return;
              grid[py + i] |= (1 << (7 - (px + j)));
            }
          }
        }
        for (int8_t i = 0; i < 8; i++) {
          if (grid[i] == 0xFF) {
            for (int8_t k = i; k > 0; k--) grid[k] = grid[k - 1];
            grid[0] = 0;
          }
        }
        pt = random(0, 7); pr = 0; py = -2; px = 2;
      }
      t = millis();
    }

    uint8_t b[8];
    for (int i = 0; i < 8; i++) b[i] = grid[i];

    uint16_t shape = pgm_read_word(&(shapes[pt][pr]));
    for (int8_t i = 0; i < 4; i++) {
      for (int8_t j = 0; j < 4; j++) {
        if ((shape >> (15 - (i * 4 + j))) & 1) {
          int8_t gx = px + j;
          int8_t gy = py + i;
          if (gx >= 0 && gx < 8 && gy >= 0 && gy < 8) {
            b[gy] |= (1 << (7 - gx));
          }
        }
      }
    }
    draw(b);
  }
}

int8_t sel = 0;

void setup() {
  for (uint8_t i = 2; i <= 11; i++) pinMode(i, OUTPUT);
  pinMode(BTN_ESQ, INPUT_PULLUP);
  pinMode(BTN_DIR, INPUT_PULLUP);
  pinMode(BTN_CIMA, INPUT_PULLUP);
  pinMode(BTN_BAIXO, INPUT_PULLUP);
  randomSeed(analogRead(A5));
}

void loop() {
  if (digitalRead(BTN_ESQ) == LOW) { sel--; if(sel < 0) sel = 2; delay(200); }
  if (digitalRead(BTN_DIR) == LOW) { sel++; if(sel > 2) sel = 0; delay(200); }

  uint8_t icon[8];
  for(int i=0; i<8; i++) icon[i] = pgm_read_byte(&(menuIcons[sel][i]));
  draw(icon);

  if (digitalRead(BTN_CIMA) == LOW) {
    delay(500);
    if (sel == 0) runSnake();
    else if (sel == 1) runSpaceInvaders();
    else if (sel == 2) runTetris();
    delay(500); 
  }
}
