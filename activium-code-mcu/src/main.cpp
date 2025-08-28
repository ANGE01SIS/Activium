#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "data.cpp"

#define BUTTON_PIN_ON 21
#define BUTTON_PIN_OFF 19
#define LED_PIN 2

bool is_led_on = false;
WiFiClientSecure espClient;
PubSubClient client(espClient);

// Función que maneja los mensajes recibidos
void callback(char* topic, byte* payload, unsigned int length) {
  String payload_str = "";

  for (unsigned int i = 0; i < length; i++) {
    payload_str += (char)payload[i];
  }
  if (strcmp(topic, "led/state") == 0 && payload_str == "ON") {
    is_led_on = true;
    digitalWrite(LED_PIN, HIGH); // Enciende el LED
  } else if (strcmp(topic, "led/state") == 0 && payload_str == "OFF") {
    is_led_on = false;
    digitalWrite(LED_PIN, LOW); // Apaga el LED
  } else{
    Serial.print("Mensaje no reconocido:  ");
    Serial.print("Mensaje recibido en topic: ");
    Serial.print(topic);
    Serial.print(" - Mensaje: ");
    Serial.print(payload_str);
    Serial.println();
  }
}

void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
      Serial.println("Conectado al broker MQTT");
      client.subscribe("led/state"); // Subscripcion al topic
    } else {
      Serial.print("Falló conexión, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  setup_wifi();
  //TODO Quitar este setIsecure
  espClient.setInsecure(); // solo para pruebas sin certificado
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  pinMode(BUTTON_PIN_ON, INPUT_PULLUP);
  pinMode(BUTTON_PIN_OFF, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Evitar, que si el le esta encendido se haga la solicitud al broker, y viceversa
  if (digitalRead(BUTTON_PIN_ON) == LOW) {
    delay(100);
    is_led_on = true;
    client.publish("led/state", "ON");
  }
  if (digitalRead(BUTTON_PIN_OFF) == LOW) {
    delay(100);
    is_led_on = false;
    client.publish("led/state", "OFF");
  }
}
