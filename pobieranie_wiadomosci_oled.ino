#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =====================================================
// WiFi
// =====================================================

const char* ssid = "KCKU_LM";
const char* password = "banan1999";

// =====================================================
// MQTT
// =====================================================

const char* mqtt_server = "broker.mqtt.cool";
const int mqtt_port = 1883;

const char* topicWaluty = "projekt-wsb-waluty";
const char* topicPogoda = "projekt-wsb-pogoda";
const char* topicGdynia = "projekt-wsb-gdynia";

// =====================================================
// OLED
// =====================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

WiFiClient espClient;
PubSubClient client(espClient);

// =====================================================
// Ostatnie wiadomości
// =====================================================

String waluty = "Brak danych";
String pogoda = "Brak danych";
String gdynia = "Brak danych";

// =====================================================
// Skracanie tekstu
// =====================================================

String shorten(String text, int maxLength) {

  if (text.length() <= maxLength) {
    return text;
  }

  return text.substring(0, maxLength - 3) + "...";
}

// =====================================================
// OLED
// =====================================================

void showData() {

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  // ===================================================
  // WALUTY
  // ===================================================

  display.setCursor(0, 0);
  display.println("WALUTY");

  display.drawFastHLine(
    0,
    8,
    128,
    SSD1306_WHITE
  );

  display.setCursor(0, 10);

  display.println(
    shorten(waluty, 21)
  );

  // ===================================================
  // POGODA
  // ===================================================

  display.setCursor(0, 22);
  display.println("POGODA");

  display.drawFastHLine(
    0,
    30,
    128,
    SSD1306_WHITE
  );

  display.setCursor(0, 32);

  display.println(
    shorten(pogoda, 21)
  );

  // ===================================================
  // GDYNIA
  // ===================================================

  display.setCursor(0, 44);
  display.println("GDYNIA");

  display.drawFastHLine(
    0,
    52,
    128,
    SSD1306_WHITE
  );

  display.setCursor(0, 54);

  display.println(
    shorten(gdynia, 21)
  );

  display.display();
}

// =====================================================
// MQTT CALLBACK
// =====================================================

void callback(
  char* topic,
  byte* payload,
  unsigned int length
) {

  String message = "";

  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println();
  Serial.print("Odebrano z: ");
  Serial.println(topic);

  Serial.print("Wiadomosc: ");
  Serial.println(message);

  // ===================================================
  // WALUTY
  // ===================================================

  if (String(topic) == topicWaluty) {

    waluty = message;

    Serial.println("-> Zapisano WALUTY");
  }

  // ===================================================
  // POGODA
  // ===================================================

  else if (String(topic) == topicPogoda) {

    pogoda = message;

    Serial.println("-> Zapisano POGODE");
  }

  // ===================================================
  // GDYNIA
  // ===================================================

  else if (String(topic) == topicGdynia) {

    gdynia = message;

    Serial.println("-> Zapisano GDYNIE");
  }

  // Od razu odśwież OLED
  showData();
}

// =====================================================
// WIFI
// =====================================================

void setup_wifi() {

  Serial.println();
  Serial.print("Laczenie z WiFi: ");
  Serial.println(ssid);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.println("Laczenie WiFi...");
  display.println(ssid);

  display.display();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");

    display.print(".");
    display.display();
  }

  Serial.println();
  Serial.println("WiFi OK");

  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  display.clearDisplay();
  display.setCursor(0, 0);

  display.println("WIFI OK");
  display.println();

  display.print("IP: ");
  display.println(WiFi.localIP());

  display.display();

  delay(1500);
}

// =====================================================
// MQTT RECONNECT
// =====================================================

void reconnect() {

  while (!client.connected()) {

    Serial.print("Laczenie z MQTT...");

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);

    display.println("Laczenie MQTT...");
    display.display();

    String clientId = "ESP32-OLED-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {

      Serial.println("OK");

      // =================================================
      // SUBSKRYPCJA 3 KANAŁÓW
      // =================================================

      client.subscribe(topicWaluty);
      client.subscribe(topicPogoda);
      client.subscribe(topicGdynia);

      Serial.println("Subskrypcje:");

      Serial.println(topicWaluty);
      Serial.println(topicPogoda);
      Serial.println(topicGdynia);

      display.clearDisplay();
      display.setCursor(0, 0);

      display.println("MQTT OK");
      display.println();
      display.println("3 kanaly");
      display.println("oczekiwanie...");

      display.display();

      delay(1000);

      // Pokazujemy aktualny stan
      showData();

    } else {

      Serial.print("Blad rc=");
      Serial.println(client.state());

      display.clearDisplay();
      display.setCursor(0, 0);

      display.println("BLAD MQTT");

      display.print("RC: ");
      display.println(client.state());

      display.println();
      display.println("Ponawiam...");

      display.display();

      delay(5000);
    }
  }
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  // ===================================================
  // I2C OLED
  // ===================================================

  Wire.begin(21, 22);

  // ===================================================
  // OLED
  // ===================================================

  if (
    !display.begin(
      SSD1306_SWITCHCAPVCC,
      OLED_ADDR
    )
  ) {

    Serial.println(
      "Nie znaleziono OLED!"
    );

    while (true) {
      delay(1000);
    }
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 0);

  display.println("MQTT");
  display.println("RECEIVER");

  display.display();

  delay(1500);

  // ===================================================
  // WIFI
  // ===================================================

  setup_wifi();

  // ===================================================
  // MQTT
  // ===================================================

  client.setServer(
    mqtt_server,
    mqtt_port
  );

  client.setCallback(callback);

  // ===================================================
  // MQTT CONNECT
  // ===================================================

  reconnect();
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  if (WiFi.status() != WL_CONNECTED) {

    setup_wifi();
  }

  if (!client.connected()) {

    reconnect();
  }

  // Odbieranie MQTT
  client.loop();
}
