# Controle de LED com botão push-button via MQTT (utilizando ESP32)

Projeto desenvolvido para a disciplina de Sistemas Embarcados / IoT  
Controle instantâneo de LED em uma placa ESP32 utilizando protocolo MQTT.

## Descrição do Projeto

Uma placa ESP32 possue:
- 1 LED conectado ao GPIO 13  
  1 Botão conectado ao GPIO 14 (com pull-up interno)

Ao pressionar o botão, o LED da inserido acende ao manter pressionado, e apaga ao soltar.  
O sistema funciona com apenas **uma ESP32 + um app MQTT no celular/PC**.

Broker público utilizado: `test.mosquitto.org` (não precisa de cadastro)

### Membros da Equipe
| Nome                     | Matrícula     |
|--------------------------|---------------|
| José Otávio Gurgel Souto            | 20222ADSGR0376     |
## Funcionamento

- Tópico `home/led/command`  → usado para enviar comandos (ON / OFF)
- Tópico `home/led/status`   → usado para publicar o estado atual do LED
- No ESP32:
  - Publica no `command` quando o botão é pressionado
  - Publica o estado atual do led no `status` ao pressionar/manter pressionado

## Hardware Necessário

- 1 placas ESP32
- 1 LED + resistor
- 1 botão push-button
- Conexões:
LED    → GPIO 13
Botão  → GPIO 14 (um lado no GND, outro no GPIO 14)
text## Como Usar

### 1. Configurar Wi-Fi e Broker
Altere se necessário:

WIFI_SSID → sua rede
WIFI_PASS → sua senha
(O broker já está fixo como mqtt://test.mosquitto.org)

### 2. Compilar e gravar
Utilizar a opção Build, Flash and Monitor da extensão ESP-IDF
