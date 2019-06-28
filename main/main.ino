#include <Arduboy2.h>
#include <qrcode.h>

// Deary-Liewald Task
// https://www.psytoolkit.org/experiment-library/deary_liewald.html

class QR {
 private:
  QRCode _encoder;
  Arduboy2& _arduboy;
  // The size of the pixel for QR code
  const uint8_t _scale = 2;
  // side length is `4 * version + 17`, or 29
  const uint8_t _version = 3;

  enum Position { left, right, center };

  void draw(const char* data, Position position) {
    uint8_t _data[qrcode_getBufferSize(_version)];
    qrcode_initText(&_encoder, _data, _version, ECC_MEDIUM, data);

    uint8_t x_off = 0;
    uint8_t y_off = 3;  // (64 - 58) / 2
    switch (position) {
      // offset by the left border
      case Position::left:
        x_off = 3;
        break;
      // offset by the borders and width of QR code
      case Position::right:
        x_off = 3 * 2 + 58;
        break;
      // offset from the center by half the width of the QR code
      case Position::center:
        x_off = 64 - 29;
        break;
    }

    for (uint8_t y = 0; y < _encoder.size; y++) {
      for (uint8_t x = 0; x < _encoder.size; x++) {
        bool color = qrcode_getModule(&_encoder, x, y);
        _arduboy.drawRect(x * _scale + x_off, y * _scale + y_off, _scale,
                          _scale, color);
      }
    }
  }

 public:
  QR(Arduboy2& arduboy) : _arduboy(arduboy) {}
  void draw_center(const char* data) { draw(data, Position::center); }
  void draw_left(const char* data) { draw(data, Position::left); }
  void draw_right(const char* data) { draw(data, Position::right); }
};

class QRExporter {
 private:
  Arduboy2& arduboy;
  QR qr;
  static const uint8_t MAX_DATA = 5;
  char const* data[MAX_DATA] = {
      "one fish", "two fish", "red fish", "blue fish", "fin",
  };
  uint8_t page = 0;
  bool dirty = true;

 public:
  QRExporter(Arduboy2& arduboy) : arduboy(arduboy), qr(arduboy) {}
  void execute() {
    arduboy.pollButtons();
    if (arduboy.justPressed(LEFT_BUTTON)) {
      if (page > 0) {
        page--;
      }
      dirty = true;
    } else if (arduboy.justPressed(RIGHT_BUTTON)) {
      if (page < MAX_DATA / 2) {
        page++;
      }
      dirty = true;
    }

    if (dirty) {
      arduboy.clear();
      if (page == 2) {
        qr.draw_center(data[2 * page]);
      } else {
        qr.draw_left(data[2 * page]);
        qr.draw_right(data[2 * page + 1]);
      }
      arduboy.display();
      dirty = false;
    }
  }
};

class Block {
 private:
  Arduboy2& _arduboy;
  bool _is_target;

 public:
  Block(Arduboy2& arduboy, bool is_target = false)
      : _arduboy(arduboy), _is_target(is_target) {}
  bool set(bool state) { _is_target = state; }
  bool get() { return _is_target; }
  void draw(int x, int y, int len) {
    _arduboy.fillRect(x, y, len, len, WHITE);
    if (_is_target) {
      _arduboy.drawLine(x, y, x + len, y + len, BLACK);
      _arduboy.drawLine(x + len, y, x, y + len, BLACK);
    }
  }
};

struct Data {
  uint16_t elapsed;
};

class Timer {
 private:
  Arduboy2& arduboy;
  Block** blocks;
  const uint8_t LEN = 25;
  uint8_t pos;
  uint8_t size;

 public:
  Timer(Arduboy2& arduboy, uint8_t size) : arduboy(arduboy), size(size) {
    blocks = new Block*[size];
    for (int i = 0; i < size; i++) {
      blocks[i] = new Block(arduboy);
    }
    pos = 0;
  }

  ~Timer() {
    for (int i = 0; i < size; i++) {
      delete blocks[i];
    }
    delete blocks;
  }

  void clear() { blocks[pos]->set(false); }

  void reset() {
    clear();
    pos = random(0, size);
    blocks[pos]->set(true);
  }

  void draw() {
    // half the size of the border
    int offset = (WIDTH - 32 * size) >> 1;
    int border = 32 - LEN;
    for (int i = 0; i < size; i++) {
      blocks[i]->draw(offset + (32 * i), 32 - border, LEN);
    }
    arduboy.display();
  }

  Data execute() {
    Data result;

    clear();
    draw();
    delay(random(0, 3000));
    reset();
    draw();

    unsigned long start = millis();
    do {
      if (arduboy.pressed(B_BUTTON)) {
        result.elapsed = millis() - start;
        break;
      }
    } while (true);

    arduboy.clear();
    clear();
    draw();
    return result;
  }
};

Arduboy2 arduboy;
QRExporter qrexporter(arduboy);
Timer timer(arduboy, 4);
uint8_t step = 0;
int sum = 0;
char buffer[20];

void setup() {
  arduboy.begin();
  arduboy.setFrameRate(200);
}

void loop() {
  if (!arduboy.nextFrame()) {
    return;
  }

  if (step == 0) {
    arduboy.clear();
    arduboy.print("press A to randomize");
    timer.draw();
    arduboy.display();
    arduboy.pollButtons();
    if (arduboy.justPressed(B_BUTTON)) {
      step++;
      arduboy.clear();
      arduboy.display();
    }
  } else if (step == 1) {
    for (int trial = 0; trial < 5; trial++) {
      auto data = timer.execute();
      sum += data.elapsed;
      arduboy.setCursor(0, 0);
      memset(buffer, 0, 20);
      snprintf(buffer, 20, "reaction: %i ms", data.elapsed);
      arduboy.print(buffer);
      arduboy.display();
    }

    arduboy.clear();
    arduboy.setCursor(0, 0);
    arduboy.println("avg. reaction time:");
    memset(buffer, 0, 20);
    snprintf(buffer, 20, "%i ms", sum / 5);
    arduboy.print("    ");
    arduboy.println(buffer);
    arduboy.println("");
    arduboy.println("press A to reset");
    arduboy.println("press B to export");
    arduboy.display();
    step++;
  } else if (step == 2) {
    arduboy.pollButtons();
    if (arduboy.justPressed(A_BUTTON)) {
      arduboy.clear();
      QR qr(arduboy);
      qr.draw_center(buffer);
      arduboy.display();
    } else if (arduboy.justPressed(B_BUTTON)) {
      sum = 0;
      step = 0;
    }
  }
}
