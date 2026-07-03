/*
 *
 * NEO PIXEL FUNCTION
 *
 **/
 
/*
 * 
 * 
 */
void colorWipe(uint32_t color, int pixel, int wait) {
  for(int i=0; i< pixel; i++) {               // For each pixels between first and int pixel
    strip.setPixelColor(i, color);            // Set pixel's color (in RAM)
    strip.show();                             // Update strip to match
    delay(wait);                              // Pause for a moment
    #if WATCHDOG == 1
    wdt_reset();
    #endif
  }
}

/*
 * 
 * 
 */
void colorBlink(uint32_t color,int pixel, int wait) {
  uint32_t lastColor = strip.getPixelColor(0);
  strip.fill(color);
  strip.show();
  delay(wait);
  strip.fill(lastColor);
  strip.show();
}

/*
 * Fade a single color channel in then out across all pixels.
 *
 * Color order: the strip is wired GRB. setPixelColor() arguments are
 * interpreted as (pixel, G, R, B) by the NEO_RGB driver, NOT (R, G, B).
 * The mapping below is intentional and correct - do not "fix" it:
 *   'r' -> red   channel: setPixelColor(i, 0, j, 0)  -> arg2 = R
 *   'g' -> green channel: setPixelColor(i, j, 0, 0)  -> arg1 = G
 *   'b' -> blue  channel: setPixelColor(i, 0, 0, j)  -> arg3 = B
 */
void colorFade(char color, int pixel, int wait) {
  uint32_t lastColor = strip.getPixelColor(pixel);
  uint16_t i, j;
  for (j = 0; j < 255; j++) { //fadein
    for (i = 0; i < pixel; i++) {
      switch (color) {
        case 'r': { strip.setPixelColor(i, 0, j, 0);     break; } // G=0, R=j, B=0
        case 'g': { strip.setPixelColor(i, j, 0, 0);     break; } // G=j, R=0, B=0
        case 'b': { strip.setPixelColor(i, 0, 0, j);     break; } // G=0, R=0, B=j
      }
    }
    strip.show();
    delay(wait);
    #if WATCHDOG == 1
    wdt_reset();
    #endif
  }
  for (j = 255; j > 0; j--) { //fadeout
    for (i = 0; i < strip.numPixels(); i++) {
      switch (color) {
        case 'r': { strip.setPixelColor(i, 0, j, 0);     break; } // G=0, R=j, B=0
        case 'g': { strip.setPixelColor(i, j, 0, 0);     break; } // G=j, R=0, B=0
        case 'b': { strip.setPixelColor(i, 0, 0, j);     break; } // G=0, R=0, B=j
      }
    }
    strip.show();
    delay(wait);
    #if WATCHDOG == 1
    wdt_reset();
    #endif
  }
  strip.fill(lastColor);
  strip.show(); 
}
