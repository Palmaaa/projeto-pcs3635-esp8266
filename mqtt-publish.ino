#ifdef ESP8266
 #include <ESP8266WiFi.h>
 #else
 #include <WiFi.h>
#endif

#include <PubSubClient.h>
#include <WiFiClientSecure.h>

/****** WiFi Connection Details *******/
const char* ssid = "";
const char* password = "";

/******* MQTT Broker Connection Details *******/
const char* mqtt_server = "";
const char* mqtt_username = "";
const char* mqtt_password = "";
const int mqtt_port = 8883;

/**** Secure WiFi Connectivity Initialisation *****/
WiFiClientSecure espClient;

/**** MQTT Client Initialisation Using WiFi Connection *****/
PubSubClient client(espClient);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

/****** root certificate *********/

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

/************* Connect to WiFi ***********/
void setup_wifi() {
  delay(10);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";   // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");

      client.subscribe("resposta");   // subscribe the topics here

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");   // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/**** Method for Publishing MQTT Messages **********/
void publishMessage(const char* topic, String payload){
  if (client.publish(topic, payload.c_str(), false))
      Serial.println("Message published ["+String(topic)+"]: "+payload);
}

/***** Call back Method for Receiving MQTT messages ****/

void callback(char* topic, byte* payload, unsigned int length) {
  String incommingMessage = "";
  for (int i = 0; i < length; i++) incommingMessage+=(char)payload[i];

  Serial.println("Message arrived ["+String(topic)+"]: "+incommingMessage);
}

const int acertouPin = 16;
int prevAcertou = 1;
int acertou = 0;

const int ganhouPin = 5;
int prevGanhou = 1;
int ganhou = 0;

const int perdeuPin = 4;
int prevPerdeu = 1;
int perdeu = 0;

const int reiniciarPin = 0;
int prevReiniciar = 1;
int reiniciar = 0;

const int menu0Pin = 14;
int menu0 = 0;
const int menu1Pin = 12;
int menu1 = 0;

const int escolheuMenuPin = 15;
int prevEscolheuMenu = 1;
int escolheuMenu = 0;

/**** Application Initialisation Function ******/
void setup() {
  pinMode(acertouPin, INPUT);
  pinMode(ganhouPin, INPUT);
  pinMode(perdeuPin, INPUT);
  pinMode(reiniciarPin, INPUT);
  pinMode(menu0Pin, INPUT);
  pinMode(menu1Pin, INPUT);
  pinMode(escolheuMenuPin, INPUT);

  Serial.begin(9600);
  while (!Serial) delay(1);
  setup_wifi();

  #ifdef ESP8266
    espClient.setInsecure();
  #else
    espClient.setCACert(root_ca);      // enable this line and the the "certificate" code for secure connection
  #endif

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

/******** Main Function *************/
void loop() {

  if (!client.connected()) reconnect(); // check if client is connected
  client.loop();

  acertou = digitalRead(acertouPin);
  if (!prevAcertou and acertou){
    Serial.println("Acertou (borda de subida)");
    publishMessage("acertou", "1");
  }
  prevAcertou = acertou;

  ganhou = digitalRead(ganhouPin);
  if (!prevGanhou and ganhou){
    Serial.println("Ganhou (borda de subida)");
    publishMessage("ganhou", "1");
  }
  prevGanhou = ganhou;

  perdeu = digitalRead(perdeuPin);
  if (!prevPerdeu and perdeu){
    Serial.println("Perdeu (borda de subida)");
    publishMessage("perdeu", "1");
  }
  prevPerdeu = perdeu;

  reiniciar = digitalRead(reiniciarPin);
  if (!prevReiniciar and reiniciar){
    Serial.println("Reiniciar (borda de subida)");
    publishMessage("reiniciar", "1");
  }
  prevReiniciar = reiniciar;

  escolheuMenu = digitalRead(escolheuMenuPin);
  menu0 = digitalRead(menu0Pin);
  menu1 = digitalRead(menu1Pin);
  if (!prevEscolheuMenu and escolheuMenu){
    Serial.println("Escolheu menu (borda de subida)");
    Serial.print("Menu: ");
    Serial.print(menu0);
    Serial.println(menu1);
    if (menu1 == LOW and menu0 == LOW) {
      publishMessage("menu", "00");
    } else if (menu1 == LOW and menu0 == HIGH) {
      publishMessage("menu", "01");
    } else if (menu1 == HIGH and menu0 == LOW) {
      publishMessage("menu", "10");
    } else if (menu1 == HIGH and menu0 == HIGH) {
      publishMessage("menu", "11");
    }
  }
  prevEscolheuMenu = escolheuMenu;

  // Serial.println("====================SINAIS=======================");

  // acertou = digitalRead(acertouPin);
  // Serial.print("Acertou: ");
  // Serial.println(acertou);

  // ganhou = digitalRead(ganhouPin);
  // Serial.print("Ganhou: ");
  // Serial.println(ganhou);

  // perdeu = digitalRead(perdeuPin);
  // Serial.print("Perdeu: ");
  // Serial.println(perdeu);

  // reiniciar = digitalRead(reiniciarPin);
  // Serial.print("Reiniciar: ");
  // Serial.println(reiniciar);
  
  // menu0 = digitalRead(menu0Pin);
  // Serial.print("Menu(0): ");
  // Serial.println(menu0);

  // menu1 = digitalRead(menu1Pin);
  // Serial.print("Menu(1): ");
  // Serial.println(menu1);

  // escolheuMenu = digitalRead(escolheuMenuPin);
  // Serial.print("Escolheu Menu: ");
  // Serial.println(escolheuMenu);

}
