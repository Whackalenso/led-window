#include <FastLED.h>

#define LED_PIN     A0
#define POT_PIN     A1
#define NUM_LEDS    64  // Change this if your matrix is bigger
#define BRIGHTNESS  80  // Set low (0-255) to save power/eyes
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

// 28 pixels on edge
void indexToEdgePos(int index) {
  int x;
  int y;

  if (index < 8) {
    x = index;
    y = 0;
  }
}

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  pinMode(POT_PIN, INPUT);
  FastLED.clear(true);
}

void drawPixel(int x, int y, CRGB color) {
  // Simple safety check to prevent crashing if x or y is out of bounds
  if (x >= 0 && x < 8 && y >= 0 && y < 8) {
    leds[getPixelIndex(x, y)] = color;
  }
  FastLED.show();
}

uint16_t getPixelIndex(uint8_t x, uint8_t y) {
  uint16_t index;
  // If your matrix is 8x8, width is 8
  const uint8_t width = 8;

  if (y % 2 == 0) {
    // Even rows (0, 2, 4...) go left to right
    index = (y * width) + x;
  } else {
    // Odd rows (1, 3, 5...) go right to left
    index = (y * width) + (width - 1 - x);
  }
  return index;
}

// Draw a 2x2 sun at top-left (sx, sy). Only draws pixels that are on the 8x8 grid.
void drawSun(int sx, int sy, CRGB color) {
  for (int dy = 0; dy < 2; dy++) {
    for (int dx = 0; dx < 2; dx++) {
      const int x = sx + dx;
      const int y = sy + dy;
      if (x >= 0 && x < 8 && y >= 0 && y < 8) {
        leds[getPixelIndex(x, y)] = color;
      }
    }
  }
}

// Draw a 3-pixel moon: top, middle (shifted right), bottom. Only draws pixels on the 8x8 grid.
void drawMoon(int mx, int my, CRGB color) {
  const int pixels[3][2] = {{0, 0}, {-1, 1}, {0, 2}};  // (dx, dy) relative to top-left
  for (int i = 0; i < 3; i++) {
    const int x = mx + pixels[i][0];
    const int y = my + pixels[i][1];
    if (x >= 0 && x < 8 && y >= 0 && y < 8) {
      leds[getPixelIndex(x, y)] = color;
    }
  }
}

void loop() {
  // Exponential smoothing to reduce pot jitter
  static float smoothedPot = 512.0f;
  smoothedPot = smoothedPot * 0.88f + analogRead(POT_PIN) * 0.12f;
  const uint16_t pot = (uint16_t)smoothedPot;

  // Cycle toggle: when pot reaches either end, switch sun<->moon for next sweep
  static bool showSun = true;
  static bool wasAtEnd = false;
  static bool firstRun = true;
  if (firstRun) {
    firstRun = false;
    uint16_t potStart = analogRead(POT_PIN);
    showSun = (potStart >= 100);  // left = moon, middle/right = sun
    wasAtEnd = (potStart < 30 || potStart > 993);
  }
  const bool atEnd = (pot < 30 || pot > 993);
  if (atEnd) {
    if (!wasAtEnd) {
      showSun = !showSun;
    }
    wasAtEnd = true;
  } else {
    wasAtEnd = false;
  }

  // Dark blue <-> sky blue: pot 0 = dark, 512 = sky, 1023 = dark
  const CRGB darkBlue(0, 0, 139);
  const CRGB skyBlue(95, 166, 210);
  const CRGB yellow(255, 255, 0);
  const CRGB orangishRed(255, 69, 0);  // orange-red at warm end, yellow transitions to/from it
  const CRGB white(255, 255, 255);

  uint8_t colorAmt;
  if (pot < 512) {
    colorAmt = (uint32_t)pot * 255 / 512;  // dark -> sky
  } else {
    colorAmt = 255 - (uint32_t)(pot - 512) * 255 / 511;  // sky -> dark
  }
  colorAmt = ease8InOutQuad(colorAmt);
  const CRGB skyColor = blend(darkBlue, skyBlue, colorAmt);

  // Sun arc: phase 0..1 synced with pot. Off -> bottom right -> middle -> bottom left -> off
  const float phase = pot / 1023.0f;
  int sunX, sunY;

  if (phase < 0.12f || phase >= 0.88f) {
    sunX = -2;
    sunY = -2;  // off screen
  } else if (phase < 0.28f) {
    // Entering at bottom right
    float t = (phase - 0.12f) / 0.16f;
    t = ease8InOutQuad((uint8_t)(t * 255)) / 255.0f;
    sunX = 8 - (int)(2 * t + 0.5f);   // 8 -> 6
    sunY = 6;
  } else if (phase < 0.5f) {
    // Arc up: bottom right -> middle
    float t = (phase - 0.28f) / 0.22f;
    t = ease8InOutQuad((uint8_t)(t * 255)) / 255.0f;
    sunX = 6 - (int)(3 * t + 0.5f);   // 6 -> 3
    sunY = 6 - (int)(3 * t + 0.5f);   // 6 -> 3
  } else if (phase < 0.72f) {
    // Arc down: middle -> bottom left
    float t = (phase - 0.5f) / 0.22f;
    t = ease8InOutQuad((uint8_t)(t * 255)) / 255.0f;
    sunX = 3 - (int)(3 * t + 0.5f);   // 3 -> 0
    sunY = 3 + (int)(3 * t + 0.5f);   // 3 -> 6
  } else {
    // Exiting at bottom left
    float t = (phase - 0.72f) / 0.16f;
    t = ease8InOutQuad((uint8_t)(t * 255)) / 255.0f;
    sunX = 0 - (int)(2 * t + 0.5f);   // 0 -> -2
    sunY = 6;
  }

  const bool objectVisible = (sunX >= -1 && sunX <= 6 && sunY >= -1 && sunY <= 6);
  const bool sunOffZone = (phase < 0.12f || phase >= 0.88f);

  if (showSun) {
  if (sunOffZone && !objectVisible) {
    // Transition driven by pot: how far into the sun-off zone (0.88-1.0 or 0-0.12)
    float shiftProgress;
    if (phase >= 0.88f) {
      shiftProgress = (phase - 0.88f) / 0.12f;  // 0 at 0.88, 1 at 1.0
    } else {
      shiftProgress = (0.12f - phase) / 0.12f;  // 0 at 0.12, 1 at 0
    }
    int shiftCount = min(8, (int)(shiftProgress * 9));  // 0..8 shifts
    if (atEnd) shiftCount = 8;  // Match moon gradient at toggle to avoid brightness jump

    // Base gradient (top=skyColor, mid=yellow, bottom=orangishRed)
    CRGB rows[8];
    for (int y = 0; y < 8; y++) {
      const uint8_t t = (y * 255) / 7;
      rows[y] = (t <= 128) ? blend(skyColor, yellow, (t * 255) / 128)
                           : blend(yellow, orangishRed, ((t - 128) * 255) / 127);
    }
    // Apply shiftCount "shifts": each row gets color from row above; new top is a shade darker each time
    for (int s = 0; s < shiftCount; s++) {
      for (int y = 7; y >= 1; y--) {
        rows[y] = rows[y - 1];
      }
      rows[0].nscale8_video(175);  // new top row = one shade darker than current
    }

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        leds[getPixelIndex(x, y)] = rows[y];
      }
    }
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].nscale8_video(128);
    }
  } else {
    // Normal: gradient when sun at corners, solid when sun in middle
    const float gradientStrength = (sunY >= 3 && sunY <= 6)
      ? (sunY - 3) / 3.0f
      : 0.0f;

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        const uint8_t gradientBlend = (uint8_t)(gradientStrength * (y * 255 / 7));
        const CRGB color = (gradientBlend <= 128)
          ? blend(skyColor, yellow, (gradientBlend * 255) / 128)
          : blend(yellow, orangishRed, ((gradientBlend - 128) * 255) / 127);
        leds[getPixelIndex(x, y)] = color;
      }
    }
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].nscale8_video(128);
    }
    if (objectVisible) {
      drawSun(sunX, sunY, white);
    }
  }
  } else {
    // Moon cycle: same dark blue gradient the sun cycle ends with (8 shifts)
    CRGB rows[8];
    for (int y = 0; y < 8; y++) {
      const uint8_t t = (y * 255) / 7;
      rows[y] = (t <= 128) ? blend(darkBlue, yellow, (t * 255) / 128)
                           : blend(yellow, orangishRed, ((t - 128) * 255) / 127);
    }
    for (int s = 0; s < 8; s++) {
      for (int y = 7; y >= 1; y--) {
        rows[y] = rows[y - 1];
      }
      rows[0].nscale8_video(175);
    }
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        leds[getPixelIndex(x, y)] = rows[y];
      }
    }
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].nscale8_video(128);
    }
    if (objectVisible) {
      drawMoon(sunX, sunY, white);
    }
  }
  FastLED.show();
  delay(10);
}