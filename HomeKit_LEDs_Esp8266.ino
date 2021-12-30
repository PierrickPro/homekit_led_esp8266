#include <Arduino.h>
#include <arduino_homekit_server.h>
#include <FastLED.h>
#include <WiFiManager.h> 

#define DATA_PIN    D4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    64
#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

CRGB leds[NUM_LEDS];
bool plain_strip_is_on = false;
float current_brightness =  100.0;
float current_sat = 0.0;
float current_hue = 0.0;
int rgb_colors[3];
static uint32_t next_heap_millis = 0;

extern "C" homekit_server_config_t accessory_config;
extern "C" homekit_characteristic_t cha_on;
extern "C" homekit_characteristic_t cha_bright;
extern "C" homekit_characteristic_t cha_sat;
extern "C" homekit_characteristic_t cha_hue;
extern "C" homekit_characteristic_t target_temperature;

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  WiFi.mode(WIFI_STA);
  //WiFi.disconnect(true);
  //homekit_storage_reset();

  //Use WiFiManager library to setup WiFi
  WiFiManager wm;
  bool res;
  res = wm.autoConnect("ESP8266 Setup"); // anonymous ap
  //res = wm.autoConnect("AutoConnectAP","password"); // password protected ap
    if(!res) {
      Serial.println("Failed to connect");
      // ESP.restart();
  } 
  else {
      //if you get here you have connected to the WiFi    
      Serial.println("connected to \'" + WiFi.SSID() + "\'");
  }

  delay(1000);
  rgb_colors[0] = 255;
  rgb_colors[1] = 255;
  rgb_colors[2] = 255;

  cha_on.setter = setOn;
  cha_bright.setter = setBright;
  cha_sat.setter = setSat;
  cha_hue.setter = setHue;
  target_temperature.setter = setPattern;

  arduino_homekit_setup(&accessory_config);  
}

void loop() {
  arduino_homekit_loop();

  // show heap info every 5 seconds
  const uint32_t t = millis();
  if (t > next_heap_millis) {
    next_heap_millis = t + 5 * 1000;
  }

  if (!plain_strip_is_on)
  {
    demoReel100Loop();
  }

  delay(10);
}

void setOn(const homekit_value_t v) {
  bool on = v.bool_value;
  Serial.println("setOn " + on);
  cha_on.value.bool_value = on; //sync the value

  if (on) {
    plain_strip_is_on = true;
  } else  {
    plain_strip_is_on = false;
  }

  updatePlainLedStrip();
}

void setHue(const homekit_value_t v) {
  float hue = v.float_value;
  Serial.println("setHue");
  cha_hue.value.float_value = hue; //sync the value

  current_hue = hue;
  updatePlainLedStrip();
}

void setSat(const homekit_value_t v) {
  float sat = v.float_value;
  Serial.println("setSat");
  cha_sat.value.float_value = sat; //sync the value

  current_sat = sat;
  updatePlainLedStrip();
}

void setBright(const homekit_value_t v) {
  int bright = v.int_value;
  Serial.println("setBright " + bright);
  cha_bright.value.int_value = bright; //sync the value

  current_brightness = bright;
  updatePlainLedStrip();
}

void updatePlainLedStrip()
{
  Serial.println("PlainStrip " + plain_strip_is_on);

  if (plain_strip_is_on) {
    HSV2RGB(current_hue, current_sat, current_brightness);

    for (int i = 0; i < NUM_LEDS; ++i)
    {
      leds[i] = CRGB(rgb_colors[0], rgb_colors[1], rgb_colors[2]);
    }
  }

  else if (!plain_strip_is_on) //lamp - switch to off
  {
    FastLED.clear();
  }
  FastLED.show();
}

void HSV2RGB(float h, float s, float v) {

  int i;
  float m, n, f;

  s /= 100;
  v /= 100;

  if (s == 0) {
    rgb_colors[0] = rgb_colors[1] = rgb_colors[2] = round(v * 255);
    return;
  }

  h /= 60;
  i = floor(h);
  f = h - i;

  if (!(i & 1)) {
    f = 1 - f;
  }

  m = v * (1 - s);
  n = v * (1 - s * f);

  switch (i) {

    case 0: case 6:
      rgb_colors[0] = round(v * 255);
      rgb_colors[1] = round(n * 255);
      rgb_colors[2] = round(m * 255);
      break;

    case 1:
      rgb_colors[0] = round(n * 255);
      rgb_colors[1] = round(v * 255);
      rgb_colors[2] = round(m * 255);
      break;

    case 2:
      rgb_colors[0] = round(m * 255);
      rgb_colors[1] = round(v * 255);
      rgb_colors[2] = round(n * 255);
      break;

    case 3:
      rgb_colors[0] = round(m * 255);
      rgb_colors[1] = round(n * 255);
      rgb_colors[2] = round(v * 255);
      break;

    case 4:
      rgb_colors[0] = round(n * 255);
      rgb_colors[1] = round(m * 255);
      rgb_colors[2] = round(v * 255);
      break;

    case 5:
      rgb_colors[0] = round(v * 255);
      rgb_colors[1] = round(m * 255);
      rgb_colors[2] = round(n * 255);
      break;
  }
}


typedef void (*SimplePatternList[])(); 
SimplePatternList gPatterns = { clearStrip, rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void demoReel100Loop()
{
  gPatterns[gCurrentPatternNumber]();

  
  FastLED.show(); // send the 'leds' array out to the actual LED strip
  FastLED.delay(1000 / FRAMES_PER_SECOND); // insert a delay to keep the framerate modest

  EVERY_N_MILLISECONDS( 20 ) {
    gHue++;  // slowly cycle the "base color" through the rainbow
  }
}

void setPattern(const homekit_value_t v)
{
  float temp = v.float_value;
  target_temperature.value.float_value = temp; //sync the value

  int pattern_index = (int) (temp / 0.5) - 20;
  Serial.println("setPattern " + pattern_index);

  gCurrentPatternNumber = pattern_index % ARRAY_SIZE( gPatterns);
}


void clearStrip()
{
  // FastLED's built-in rainbow generator
  FastLED.clear();
}

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS - 1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16( i + 7, 0, NUM_LEDS - 1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
