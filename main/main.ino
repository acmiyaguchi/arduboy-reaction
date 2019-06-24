#include <Arduboy2.h>
#include <qrcode.h>

Arduboy2 arduboy;

class QR {
  private:
    QRCode _encoder;
    static const uint8_t _scale = 2;
    enum Position { left, right, center };
    
    void draw(const uint8_t* data, Position position) {
      // side length is `4 * version + 17`
      uint8_t version = 3;
      uint8_t _data[qrcode_getBufferSize(version)];
      qrcode_initText(&_encoder, _data, version, ECC_MEDIUM, data);
      
      for (uint8_t y = 0; y < _encoder.size; y++) {
        for (uint8_t x = 0; x < _encoder.size; x++) {
            bool color = qrcode_getModule(&_encoder, x, y);
            arduboy.drawRect(x*_scale, y*_scale, _scale, _scale, color);
        }
      }
    }
    
  public:
    void draw_center(const uint8_t* data);
    void draw_left(const uint8_t* data) {
      draw(data, Position::left);
    }
    void draw_right(const uint8_t* data);
};

QR qr;

void setup() {
  arduboy.begin();
  qr.draw_left("hello world");
  arduboy.display();
}

void loop() {
  // put your main code here, to run repeatedly:

}
