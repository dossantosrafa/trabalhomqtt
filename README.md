# Controle de LED via MQTT — NodeMCU ESP8266

Projeto embarcado para controle remoto de LED usando MQTT.
ALUNOS: RAFAEL FONTANA DOS SANTOS, JHONATAN NEGRI, GABRIEL SIMIONATO

## Configuração

Crie o arquivo `src/config.h` com suas credenciais:

```cpp
#define WIFI_SSID     "sua_rede"
#define WIFI_PASSWORD "sua_senha"

#define MQTT_HOST    "endereco_do_broker"
#define MQTT_PORT    1884
#define MQTT_USUARIO "seu_usuario"
#define MQTT_SENHA   "sua_senha"
```

Depois abra no PlatformIO e clique em **Upload**.

## Uso

Publique no tópico `casa/led`:
- `1` → Liga o LED
- `0` → Desliga o LED

## Tópicos

| Tópico | Descrição |
|---|---|
| `casa/led` | Comando liga/desliga |
| `casa/led/confirmacao` | Confirmação do comando |
| `casa/nodemcu/ip` | IP do dispositivo |
| `casa/nodemcu/uptime` | Tempo ligado (segundos) |
| `casa/nodemcu/status` | Status online |