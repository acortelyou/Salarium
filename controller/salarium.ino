#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include <FastLED.h>
#include <ezTime.h>

#define WIFI_SSID        "WIFI"
#define WIFI_PSK         "thepasswordispassword"
#define HOSTNAME         "salarium"
#define OTA_PW           "salarium"

#define TZ               "America/Los_Angeles"

#define WEB_PORT         80

#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define DATA_PIN    15
#define BRIGHTNESS  255
#define FPS         500

#define BORDER      1
#define WIDTH       15
#define HEIGHT      10
#define NUM_LEDS    (WIDTH*HEIGHT)


Timezone tz;

WebServer server(WEB_PORT);

CRGB leds[NUM_LEDS];


uint16_t XY( uint8_t x, uint8_t y) {
  return (y * WIDTH) + x;
}



void blur1d( CRGB* leds, uint16_t numLeds, uint8_t keep, uint8_t seep)
{
  CRGB carryover = CRGB::Black;
  for ( uint16_t i = 0; i < numLeds; i++) {
    CRGB cur = leds[i];
    CRGB part = cur;
    part.nscale8( seep);
    cur.nscale8( keep);
    cur += carryover;
    if ( i) leds[i - 1] += part;
    leds[i] = cur;
    carryover = part;
  }
}

void blurColumns(CRGB* leds, uint8_t width, uint8_t height, uint8_t keep, uint8_t seep) {
  for ( uint8_t col = 0; col < width; col++) {
    CRGB carryover = CRGB::Black;
    for ( uint8_t i = 0; i < height; i++) {
      CRGB cur = leds[XY(col, i)];
      CRGB part = cur;
      part.nscale8( seep);
      cur.nscale8( keep);
      cur += carryover;
      if ( i) leds[XY(col, i - 1)] += part;
      leds[XY(col, i)] = cur;
      carryover = part;
    }
  }
}

void blurHelix( CRGB* leds, uint8_t width, uint8_t height, uint8_t keep, uint8_t seep)
{
  blur1d(leds, width * height, keep, seep);
  blurColumns(leds, width, height, keep, seep);
}

DEFINE_GRADIENT_PALETTE( sunrise_gp ) {
  0,  44,   64, 150,
 64,  199,  57, 103,
128,  250,  97, 100,  
192,  253, 212,  34,
255,  139, 190, 248};

DEFINE_GRADIENT_PALETTE( sunset_gp ) {
  0,  139, 190, 248,
 42,  62,   88, 121,
 84,  155, 165, 174,  
126,  220, 182, 151,
168,  252, 112,   1,
210,  221, 114,  60,
255,  173,  74,  40};

DEFINE_GRADIENT_PALETTE( bedtime_gp ) {
  0,   173, 74,  40,
 64,   72,  69, 154,
128,   53,  50, 131,  
192,   37,  53, 105,
255,   30,  43,  88};

double theta = 0;
uint8_t brightness = 0;
CRGB color;

void clockMode() {
  tmElements_t t;
  breakTime(tz.now(), t);

  double day =
    ((double)t.Hour) +
    ((double)t.Minute) * ((double)1 / 60) +
    ((double)t.Second) * ((double)1 / 3600);

  double sunrise = 6.5;
  double sunset  = 17;
  double bedtime = 21;

  double duration = 1;

  double day_brightness = 64;
  double evening_brightness = 8;
  double night_brightness = 0;

  if (day < sunrise) {
    //Predawn

    theta = 0;

    color = CRGB::Black;
    
    brightness = night_brightness;
    
  } else if (day < sunrise + duration) {
    //Sunrise

    theta = (day - sunrise) / duration;
    
    CRGBPalette16 palette = sunrise_gp;

    color = ColorFromPalette(palette, theta * 240);

    brightness = theta * day_brightness;

  } else if (day < sunset) {
    //Day

    theta = 1;
    
    CRGBPalette16 palette = sunrise_gp;

    color = ColorFromPalette(palette, 240);

    brightness = day_brightness;

  } else if (day < sunset + duration) {
    //Sunset    

    theta = (day - sunset) / duration;
    
    CRGBPalette16 palette = sunset_gp;

    color = ColorFromPalette(palette, theta * 240);

    brightness = day_brightness - (theta * (day_brightness - evening_brightness));

  } else if (day < bedtime) {
    //Evening

    theta = 1;
    
    CRGBPalette16 palette = sunset_gp;

    color = ColorFromPalette(palette, 240);

    brightness = evening_brightness;

  } else if (day < bedtime + duration) {
    //Twilight

    theta = (day - bedtime) / duration;
    
    CRGBPalette16 palette = bedtime_gp;

    color = ColorFromPalette(palette, theta * 240);

    brightness = evening_brightness - (theta * (evening_brightness - night_brightness));

  } else {
    //Night

    theta = 1;

    color = CRGB::Black;

    brightness = night_brightness;

  }
      
  fill_solid(leds, NUM_LEDS, color);
  FastLED.setBrightness(brightness);
  FastLED.show();
}



uint8_t hue;

void demo()
{
  EVERY_N_MILLISECONDS( 20 ) {
    hue++;
  }

  //  fill_solid(leds, NUM_LEDS, CHSV( hue, 255, 255 ));

  //  fill_rainbow(leds, NUM_LEDS, hue, 16);

  //  FastLED.show();
  //  quadwave8
  //  return;

  uint8_t blurAmount = beatsin8( 2, 10, 255);

  blurHelix( leds, WIDTH, HEIGHT, 127, hue >> 2);

  uint8_t  i = beatsin8( 41, 0, 3 * WIDTH) % WIDTH;
  uint8_t  j = beatsin8( 27, BORDER, HEIGHT - BORDER);

  uint16_t ms = millis();

  leds[XY( i, j)] += CHSV( ms / 11, 200, 255 );

  FastLED.show();
}



void handleRoot() {
  server.send(200, "text/html", "I &#10084;&#65039; LAMP");
}

void handleStatus() {
  String status = "<h1>Status</h1>\n";
  status += "<h2>" + tz.dateTime() + "</h2>\n";
  status += String(theta) + "<br>\n";
  status += String(brightness) + "<br>\n";
  status += String(color.r) + "," + String(color.g) + "," + String(color.b) + "<br>\n";
  server.send(200, "text/html", status);
}

void handleNotFound() {
  server.send(404, "text/plain", "404");
}



void setup() {

  //LEDs
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  delay(100);
  FastLED.show();
  delay(100);

  //Network
  Serial.print("\nConnecting to network..");
  WiFi.setHostname(HOSTNAME);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.print(".");
    WiFi.begin(WIFI_SSID, WIFI_PSK);
  }
  Serial.print(" ");
  Serial.println(WiFi.localIP());
  delay(100);

  //OTA
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.setPassword(OTA_PW);
  ArduinoOTA.onStart([]() {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    Serial.println("OTA Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("OTA Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("OTA Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("OTA Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("OTA Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("OTA End Failed");
  });
  ArduinoOTA.begin();
  delay(100);

  //Time
  waitForSync();
  tz.setLocation(TZ);
  delay(100);

  //Web
  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.onNotFound(handleNotFound);
  server.begin();
  delay(100);

  //Ready
  Serial.println("Ready.\n");
  delay(100);
}

void loop()
{
  ArduinoOTA.handle();
  server.handleClient();

  clockMode();

  FastLED.delay(100);
}
