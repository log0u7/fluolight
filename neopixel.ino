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

/*
 * Non-blocking animation state machine for app events (48-57).
 * Called from loop() via animTick(); throttled internally so the
 * HTTP / event dispatch loop is never blocked by LED effects.
 */
static uint8_t    aType     = 0;   // 0=idle, 1=wipe, 2=fade, 3=toggle
static uint32_t   aColor1   = 0;
static uint32_t   aColor2   = 0;
static char       aChannel  = 'r';
static int        aPixels   = 0;
static int        aStep     = 0;
static int        aTotal    = 0;
static unsigned long aTimer = 0;
static int        aWait     = 0;
static bool       aCont     = false;
static uint8_t    aRepeat   = 0;   // toggle: remaining blink cycles (0 = infinite)

void animStartWipe(uint32_t color, int pixels, int wait) {
  aType   = 1;
  aColor1 = color;
  aPixels = pixels;
  aStep   = 0;
  aTotal  = pixels;
  aWait   = wait;
  aCont   = false;
  aTimer  = millis();
}

void animStartFade(char channel, int pixels, int wait, bool continuous) {
  aType    = 2;
  aChannel = channel;
  aPixels  = pixels;
  aStep    = 0;
  aTotal   = 510;
  aWait    = wait;
  aCont    = continuous;
  aTimer   = millis();
}

// continuous=true : toggle forever (app code 6)
// continuous=false, repeat=N : N blink cycles then stop on base (system events)
void animStartToggle(uint32_t base, uint32_t blink, int wait, bool continuous, uint8_t repeat) {
  aType    = 3;
  aColor1  = base;
  aColor2  = blink;
  aStep    = 0;
  aTotal   = 2;
  aWait    = wait;
  aCont    = continuous;
  aRepeat  = repeat;
  strip.fill(base);
  strip.show();
  aTimer   = millis();
}

void animTick() {
  if (aType == 0) return;

  unsigned long now = millis();
  if (now - aTimer < (unsigned long)aWait) return;
  aTimer = now;

  #if WATCHDOG == 1
  wdt_reset();
  #endif

  switch (aType) {
    case 1:
      if (aStep < aTotal) {
        strip.setPixelColor(aStep, aColor1);
        strip.show();
        aStep++;
      }
      if (aStep >= aTotal) aType = 0;
      break;

    case 2: {
      uint8_t b;
      if (aStep < 255) b = aStep;
      else b = 509 - aStep;
      for (int i = 0; i < aPixels; i++) {
        switch (aChannel) {
          case 'r': strip.setPixelColor(i, 0, b, 0); break;
          case 'g': strip.setPixelColor(i, b, 0, 0); break;
          case 'b': strip.setPixelColor(i, 0, 0, b); break;
        }
      }
      strip.show();
      aStep++;
      if (aStep >= aTotal) {
        if (aCont) aStep = 0;
        else aType = 0;
      }
      break;
    }

    case 3:
      aStep = (aStep + 1) % aTotal;
      strip.fill(aStep == 0 ? aColor1 : aColor2);
      strip.show();
      if (!aCont && aStep == 0) {
        // completed one full blink cycle (base -> flash -> base)
        if (aRepeat > 1) {
          aRepeat--;  // more cycles to go
        } else {
          aType = 0;  // done
        }
      }
      break;
  }
}
