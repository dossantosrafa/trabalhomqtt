#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <config.h>

// ----------------------------------------------------------------
//  LED interno do NodeMCU: pino 2, lógica invertida
//  (LOW = aceso, HIGH = apagado)
// ----------------------------------------------------------------
#define PINO_LED LED_BUILTIN

// Intervalo de publicação dos dados do dispositivo (ms)
#define INTERVALO_PUBLICACAO 10000  // 10 segundos

WiFiClient   wifiClient;
PubSubClient mqtt(wifiClient);

unsigned long ultimaPublicacao = 0;

// ----------------------------------------------------------------
//  Conexão Wi-Fi
// ----------------------------------------------------------------
void conectarWifi() {
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ Wi-Fi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// ----------------------------------------------------------------
//  Callback: chamado ao receber mensagem MQTT
// ----------------------------------------------------------------
void aoReceberMensagem(char* topico, byte* payload, unsigned int tamanho) {
  // Converte payload para String
  String mensagem = "";
  for (unsigned int i = 0; i < tamanho; i++) {
    mensagem += (char)payload[i];
  }

  Serial.print("📨 Mensagem recebida | Tópico: ");
  Serial.print(topico);
  Serial.print(" | Payload: ");
  Serial.println(mensagem);

  // Verifica se é o tópico de comando do LED
  if (String(topico) == TOPICO_COMANDO) {
    if (mensagem == "1") {
      digitalWrite(PINO_LED, LOW);   // Liga (lógica invertida)
      Serial.println("💡 LED LIGADO");
      mqtt.publish(TOPICO_CONFIRMA, "LED ligado com sucesso");

    } else if (mensagem == "0") {
      digitalWrite(PINO_LED, HIGH);  // Desliga
      Serial.println("🌑 LED DESLIGADO");
      mqtt.publish(TOPICO_CONFIRMA, "LED desligado com sucesso");

    } else {
      Serial.println("⚠️  Payload desconhecido. Use '1' para ligar ou '0' para desligar.");
      mqtt.publish(TOPICO_CONFIRMA, "Erro: payload invalido. Use 1 ou 0.");
    }
  }
}

// ----------------------------------------------------------------
//  Conexão/reconexão ao Broker MQTT
// ----------------------------------------------------------------
void conectarMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker MQTT...");

    String clientId = "NodeMCU-" + String(ESP.getChipId());

    if (mqtt.connect(clientId.c_str(), MQTT_USUARIO, MQTT_SENHA)) {
      Serial.println(" ✅ Conectado!");

      // Assina o tópico de comandos
      mqtt.subscribe(TOPICO_COMANDO);
      Serial.print("📡 Inscrito em: ");
      Serial.println(TOPICO_COMANDO);

      // Publica status online
      mqtt.publish(TOPICO_STATUS, "online");

    } else {
      Serial.print(" ❌ Falha (rc=");
      Serial.print(mqtt.state());
      Serial.println("). Tentando novamente em 5s...");
      delay(5000);
    }
  }
}

// ----------------------------------------------------------------
//  Publica dados do dispositivo periodicamente
// ----------------------------------------------------------------
void publicarDadosDispositivo() {
  // IP
  String ip = WiFi.localIP().toString();
  mqtt.publish(TOPICO_IP, ip.c_str());

  // Uptime em segundos
  String uptime = String(millis() / 1000);
  mqtt.publish(TOPICO_UPTIME, uptime.c_str());

  // Status
  mqtt.publish(TOPICO_STATUS, "online");

  Serial.println("📤 Dados publicados → IP: " + ip + " | Uptime: " + uptime + "s");
}

// ----------------------------------------------------------------
//  Setup
// ----------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\n====================================");
  Serial.println("  NodeMCU MQTT - Controle de LED");
  Serial.println("====================================");

  pinMode(PINO_LED, OUTPUT);
  digitalWrite(PINO_LED, HIGH);  // Começa desligado

  conectarWifi();

  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(aoReceberMensagem);
}

// ----------------------------------------------------------------
//  Loop principal
// ----------------------------------------------------------------
void loop() {
  // Reconecta ao Wi-Fi se necessário
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️  Wi-Fi perdido. Reconectando...");
    conectarWifi();
  }

  // Reconecta ao broker se necessário
  if (!mqtt.connected()) {
    conectarMQTT();
  }

  // Processa mensagens recebidas
  mqtt.loop();

  // Publica dados do dispositivo a cada INTERVALO_PUBLICACAO ms
  unsigned long agora = millis();
  if (agora - ultimaPublicacao >= INTERVALO_PUBLICACAO) {
    ultimaPublicacao = agora;
    publicarDadosDispositivo();
  }
}
