#include <Arduboy2.h>
#include <qrcode.h>

// Deary-Liewald Task

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

Arduboy2 arduboy;
QRExporter qrexporter(arduboy);

void setup() {
  arduboy.begin();
  arduboy.setFrameRate(60);
  arduboy.display();
}

void loop() {
  if (!arduboy.nextFrame()) {
    return;
  }
  qrexporter.execute();
}
