/*
 * Esse código recebe dados para ligar um LED no esp32
 * a partir do broker: broker.hivemq.com 
 */


// Inclusão das bibliotecas.
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <stdio.h>

// Mapeamento de hardware.
#define LED       2
#define DHTTYPE  DHT11
#define DHTPIN    13

DHT dht(DHTPIN,DHTTYPE);

// Configurações básicas de rede local e broker.
const char* ssid ="";
const char* password = "";
const char* mqtt_server = "broker.hivemq.com";

// Declaração de variáveis independentes.
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;


void setup_wifi() {

  delay(10);
  
  Serial.println();
  Serial.print("Conectando a: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Endereço de IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    if(payload[i] == '1'){
      digitalWrite(LED, HIGH);
    }

    if(payload[i] == '0'){
      digitalWrite(LED, LOW);
    }    
  }
  Serial.println();
}

void reconnect() {
  
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    // Cria um ID aleatório
    String clientId = "";
    clientId += String(random(0xffff), HEX);
    // Tentativa de reconectar
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado");
      client.subscribe("teste");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" Próxima tentativa em 5 segundos.");
      // Espera 5 segundos antes de tentar novamente.
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LED, OUTPUT);     
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  dht.begin();
}

void loop() 
{
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  // Loop da temperatura e Umidade
  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.println(F("ºC"));
  snprintf(msg,MSG_BUFFER_SIZE,"%f", t);
  client.publish("temperatura",msg);

  Serial.print("Umidade: ");
  Serial.print(h);
  Serial.println(F("º%"));
  snprintf(msg,MSG_BUFFER_SIZE,"%f", h);
  client.publish("umidade",msg); 

  delay(5000); // espera 5 segundos para enviar novamente os valores de temperatura e humidade.       

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
