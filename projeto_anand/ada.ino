#include "esp_camera.h"
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>


Adafruit_NeoPixel leds = Adafruit_NeoPixel(8,14,NEO_GRB+NEO_KHZ800);

// ===========================
// Select camera model in board_config.h
// ===========================
#include "board_config.h"

// ===========================
// Enter your WiFi credentials
// ===========================
const char *ssid = "iPhone de Gabriel";
const char *password = "jabes123";

WiFiUDP udp;

unsigned int localUdpPort = 4210; //porta pra receber pacotes
char pacote[255]; //buffer de entrada

void startCameraServer();
void setupLedFlash();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  
 //leds
  leds.begin();

  // ===== Camera config =====
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG; 
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

#if defined(LED_GPIO_NUM)
  setupLedFlash();
#endif

  // ===== WiFi =====
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect"); 

  //==== config udp ===

  udp.begin(localUdpPort);
  Serial.printf("Esperando pacotes na porta %d", localUdpPort);
}

// funçoes

void acenderRegiao  (String regiao){
  if(regiao == "FARLEFT") {
    leds.setPixelColor(0, leds.Color(180, 180, 180));
  }
  else if(regiao == "LEFT") {
    leds.setPixelColor(1, leds.Color(180, 180, 180));
    leds.setPixelColor(2, leds.Color(180, 180, 180));
    }
    else if(regiao == "CENTER") {
      leds.setPixelColor(3, leds.Color(180, 180, 180));
      leds.setPixelColor(4, leds.Color(180, 180, 180));
      }
      else if(regiao == "RIGHT") {
        leds.setPixelColor(5, leds.Color(180, 180, 180));
        leds.setPixelColor(6, leds.Color(180, 180, 180));
       }
        else if (regiao == "FARRIGHT") {
        leds.setPixelColor(7, leds.Color(180, 180, 180));
        }
  leds.show();
}


void apagarRegiao (String regiao){
  if(regiao == "FARLEFT") {
    leds.setPixelColor(0, leds.Color(0, 0, 0));
  }
  else if(regiao == "LEFT") {
    leds.setPixelColor(1, leds.Color(0, 0, 0));
    leds.setPixelColor(2, leds.Color(0, 0, 0));
    }
    else if(regiao == "CENTER") {
      leds.setPixelColor(3, leds.Color(0, 0, 0));
      leds.setPixelColor(4, leds.Color(0, 0, 0));
      }
      else if(regiao == "RIGHT") {
        leds.setPixelColor(5, leds.Color(0, 0, 0));
        leds.setPixelColor(6, leds.Color(0, 0, 0));
       }
        else if (regiao == "FARRIGHT") {
        leds.setPixelColor(7, leds.Color(0, 0, 0));
        }
  leds.show();
}

void loop() {
  int tam = udp.parsePacket();
  if (tam != 0) {
    int i = udp.read(pacote, 255); //retorna quantos bytes foram lidos
    if(i > 0) {
      pacote[i] = '\0'; //coloca o terminador da string
    }
    String str = String(pacote);
    Serial.print("Recebi: ");
    Serial.println(pacote);

// se a str no buffer de entrada tem "ON" ou "OFF"
  if(str.endsWith("ON")){
    str.replace("ON", "");
    acenderRegiao(str);
  }
    else if(str.endsWith("OFF")){
      str.replace("OFF", "");
      apagarRegiao(str);
    }
  }
} 