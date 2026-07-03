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
 * 
 * 
 */
void colorFade(char color, int pixel, int wait) {
  uint32_t lastColor = strip.getPixelColor(pixel);
  uint16_t i, j;
  for (j = 0; j < 255; j++) { //fadein
    for (i = 0; i < pixel; i++) {
      switch (color) {
        case 'r': { strip.setPixelColor(i, 0, j, 0);     break; }
        case 'g': { strip.setPixelColor(i, j, 0, 0);     break; }
        case 'b': { strip.setPixelColor(i, 0, 0, j);     break; }
      }
    }
    strip.show();
    delay(wait);
  }
  for (j = 255; j > 0; j--) { //fadeout
    for (i = 0; i < strip.numPixels(); i++) {
      switch (color) {
        case 'r': { strip.setPixelColor(i, 0, j, 0);     break; }
        case 'g': { strip.setPixelColor(i, j, 0, 0);     break; }
        case 'b': { strip.setPixelColor(i, 0, 0, j);     break; }
      }
    }
    strip.show();
    delay(wait);
  }
  strip.fill(lastColor);
  strip.show(); 
}
