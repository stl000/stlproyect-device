/***************************************
************** LIBRERIAS ***************
****************************************/
#include <Arduino.h>
#include <WiFiManager.h>
#include <DNSServer.h>
#include <Separador.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h> 
#include <DHT.h>
#include "ESP32_MailClient.h"


/***************************************
************* CONFIG WIFI **************
****************************************/

#define WIFI_PIN 17 //pin de reseteo del dispositivo


/***************************************
************* CONFIG DHT11 *************
****************************************/
#define DHT_PIN 27
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

float temp = 0;
float lastTemp;
int hum = 0;
int lastHum;
String temperaturaString = "";
String humedadString = "";

/***************************************
************* CONFIG FAN ************
****************************************/
const int FAN_PIN = 33; // Gobierna el ventilador


/***************************************
*********** CONFIG HC-SR501 ************
****************************************/

#define timeSeconds 10
#define MOTION_PIN 12

unsigned long lastTrigger = 0;
boolean startTimer = false;
bool mov=false;

/***************************************
************* CONFIG SMTP **************
****************************************/
SMTPData datosSMTP;
bool alarma=false;
bool mensaje=false;


/***************************************
************ CONFIG SERVER *************
****************************************/

//estos datos deben estar configurador también en las constantes del panel
const String serial_number = "001";
const String insert_password = "321321";
const String get_data_password = "321321";
const char*  server = "stlproyect.tk";



/***************************************
************* CONFIG MQTT **************
****************************************/
/* Datos Broker: 
TCP: 1883
TCP SSL: 8883
WS: 8093
WSS: 8094
*/

const char *mqtt_server = "stlproyect.tk";
const int mqtt_port = 8883;

//El dispositivo se encargará de averiguar qué usuario y qué contraseña mqtt debe usar.
char mqtt_user[20] = "";
char mqtt_pass[20] = "";

  const char* aws_cert_ca = \
  "-----BEGIN CERTIFICATE-----\n" \
  "MIIEZTCCA02gAwIBAgIQQAF1BIMUpMghjISpDBbN3zANBgkqhkiG9w0BAQsFADA/\n" \
  "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
  "DkRTVCBSb290IENBIFgzMB4XDTIwMTAwNzE5MjE0MFoXDTIxMDkyOTE5MjE0MFow\n" \
  "MjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxCzAJBgNVBAMT\n" \
  "AlIzMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuwIVKMz2oJTTDxLs\n" \
  "jVWSw/iC8ZmmekKIp10mqrUrucVMsa+Oa/l1yKPXD0eUFFU1V4yeqKI5GfWCPEKp\n" \
  "Tm71O8Mu243AsFzzWTjn7c9p8FoLG77AlCQlh/o3cbMT5xys4Zvv2+Q7RVJFlqnB\n" \
  "U840yFLuta7tj95gcOKlVKu2bQ6XpUA0ayvTvGbrZjR8+muLj1cpmfgwF126cm/7\n" \
  "gcWt0oZYPRfH5wm78Sv3htzB2nFd1EbjzK0lwYi8YGd1ZrPxGPeiXOZT/zqItkel\n" \
  "/xMY6pgJdz+dU/nPAeX1pnAXFK9jpP+Zs5Od3FOnBv5IhR2haa4ldbsTzFID9e1R\n" \
  "oYvbFQIDAQABo4IBaDCCAWQwEgYDVR0TAQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8E\n" \
  "BAMCAYYwSwYIKwYBBQUHAQEEPzA9MDsGCCsGAQUFBzAChi9odHRwOi8vYXBwcy5p\n" \
  "ZGVudHJ1c3QuY29tL3Jvb3RzL2RzdHJvb3RjYXgzLnA3YzAfBgNVHSMEGDAWgBTE\n" \
  "p7Gkeyxx+tvhS5B1/8QVYIWJEDBUBgNVHSAETTBLMAgGBmeBDAECATA/BgsrBgEE\n" \
  "AYLfEwEBATAwMC4GCCsGAQUFBwIBFiJodHRwOi8vY3BzLnJvb3QteDEubGV0c2Vu\n" \
  "Y3J5cHQub3JnMDwGA1UdHwQ1MDMwMaAvoC2GK2h0dHA6Ly9jcmwuaWRlbnRydXN0\n" \
  "LmNvbS9EU1RST09UQ0FYM0NSTC5jcmwwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYf\n" \
  "r52LFMLGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjANBgkqhkiG9w0B\n" \
  "AQsFAAOCAQEA2UzgyfWEiDcx27sT4rP8i2tiEmxYt0l+PAK3qB8oYevO4C5z70kH\n" \
  "ejWEHx2taPDY/laBL21/WKZuNTYQHHPD5b1tXgHXbnL7KqC401dk5VvCadTQsvd8\n" \
  "S8MXjohyc9z9/G2948kLjmE6Flh9dDYrVYA9x2O+hEPGOaEOa1eePynBgPayvUfL\n" \
  "qjBstzLhWVQLGAkXXmNs+5ZnPBxzDJOLxhF2JIbeQAcH5H0tZrUlo5ZYyOqA7s9p\n" \
  "O5b85o3AM/OJ+CktFBQtfvBhcJVd9wvlwPsk+uyOy2HI7mNxKKgsBTt375teA2Tw\n" \
  "UdHkhVNcsAKX1H7GNNLOEADksd86wuoXvg==\n" \
  "-----END CERTIFICATE-----\n";

  const char* aws_cert_key = \
  "-----BEGIN RSA PRIVATE KEY-----\n" \
  "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\n" \
  "-----END RSA PRIVATE KEY-----";   //to verify the client

  const char* aws_cert_crt= \
  "-----BEGIN CERTIFICATE-----" \
  "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\n" \
  "p82Anpk/5wz8\n" \
  "-----END CERTIFICATE-----";  //to verify the client

  const char* aws_cert_pem= \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"p82Anpk/5wz8\n" \
"-----END CERTIFICATE-----\n" \
"-----BEGIN CERTIFICATE-----\n" \
"MIIEZTCCA02gAwIBAgIQQAF1BIMUpMghjISpDBbN3zANBgkqhkiG9w0BAQsFADA/\n" \
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
"DkRTVCBSb290IENBIFgzMB4XDTIwMTAwNzE5MjE0MFoXDTIxMDkyOTE5MjE0MFow\n" \
"MjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxCzAJBgNVBAMT\n" \
"AlIzMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuwIVKMz2oJTTDxLs\n" \
"jVWSw/iC8ZmmekKIp10mqrUrucVMsa+Oa/l1yKPXD0eUFFU1V4yeqKI5GfWCPEKp\n" \
"Tm71O8Mu243AsFzzWTjn7c9p8FoLG77AlCQlh/o3cbMT5xys4Zvv2+Q7RVJFlqnB\n" \
"U840yFLuta7tj95gcOKlVKu2bQ6XpUA0ayvTvGbrZjR8+muLj1cpmfgwF126cm/7\n" \
"gcWt0oZYPRfH5wm78Sv3htzB2nFd1EbjzK0lwYi8YGd1ZrPxGPeiXOZT/zqItkel\n" \
"/xMY6pgJdz+dU/nPAeX1pnAXFK9jpP+Zs5Od3FOnBv5IhR2haa4ldbsTzFID9e1R\n" \
"oYvbFQIDAQABo4IBaDCCAWQwEgYDVR0TAQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8E\n" \
"BAMCAYYwSwYIKwYBBQUHAQEEPzA9MDsGCCsGAQUFBzAChi9odHRwOi8vYXBwcy5p\n" \
"ZGVudHJ1c3QuY29tL3Jvb3RzL2RzdHJvb3RjYXgzLnA3YzAfBgNVHSMEGDAWgBTE\n" \
"p7Gkeyxx+tvhS5B1/8QVYIWJEDBUBgNVHSAETTBLMAgGBmeBDAECATA/BgsrBgEE\n" \
"AYLfEwEBATAwMC4GCCsGAQUFBwIBFiJodHRwOi8vY3BzLnJvb3QteDEubGV0c2Vu\n" \
"Y3J5cHQub3JnMDwGA1UdHwQ1MDMwMaAvoC2GK2h0dHA6Ly9jcmwuaWRlbnRydXN0\n" \
"LmNvbS9EU1RST09UQ0FYM0NSTC5jcmwwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYf\n" \
"r52LFMLGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjANBgkqhkiG9w0B\n" \
"AQsFAAOCAQEA2UzgyfWEiDcx27sT4rP8i2tiEmxYt0l+PAK3qB8oYevO4C5z70kH\n" \
"ejWEHx2taPDY/laBL21/WKZuNTYQHHPD5b1tXgHXbnL7KqC401dk5VvCadTQsvd8\n" \
"S8MXjohyc9z9/G2948kLjmE6Flh9dDYrVYA9x2O+hEPGOaEOa1eePynBgPayvUfL\n" \
"qjBstzLhWVQLGAkXXmNs+5ZnPBxzDJOLxhF2JIbeQAcH5H0tZrUlo5ZYyOqA7s9p\n" \
"O5b85o3AM/OJ+CktFBQtfvBhcJVd9wvlwPsk+uyOy2HI7mNxKKgsBTt375teA2Tw\n" \
"UdHkhVNcsAKX1H7GNNLOEADksd86wuoXvg==\n" \
"-----END CERTIFICATE-----\n";

WiFiManager wifiManager;
WiFiClientSecure client;
WiFiClientSecure client2;
PubSubClient mqttclient(client);
Separador s;

char device_topic_publish[40];    //Tópico para el panel
char device_topic_publish_t[40];  //Tópico solo temperatura
char device_topic_publish_h[40];  //Tópico solo humedad
char device_topic_subscribe[40];  //Tópico al que suscribirse

const int expected_topic_length = 26; //numero de caracteres esperados para el tópico
bool topic_obteined = false;  //comprobación de tópico
char msg[25]; //Caracteres del mensaje

//Botones del panel(modo auto, switch1, switch2, switch3, barra de refresco y tiempo de refresco)
byte autoAlarm = 0;
byte autoPanel = 0;
byte sw1 = 0;
byte sw2 = 0;
byte sw3 = 0;
int slider = 3;


/***************************************
************** FUNCIONES ***************
****************************************/

void setupWifi();
void setupDHT11();
float getTemperatura();
void publishTemperatura();
float getHumedad();
void publishHumedad();
void setupFan();
void setupPIR();
void IRAM_ATTR detectsMovement();
void setupMQTT();
void callbackMQTT(char* topic, byte* payload, unsigned int length);
void connectMQTT();
bool get_topic(int length);
void send_to_database();
void send_to_gmail(bool alarma, bool mensaje);



/***************************************
**************** SETUP *****************
****************************************/

void setup() {
  randomSeed(analogRead(0));
  Serial.begin(115200);
  setupWifi();
  setupDHT11();
  setupFan();
  setupPIR();

  client2.setCACert(aws_cert_ca);
  client.setCACert(aws_cert_ca);
  PubSubClient mqttclient(client);

  // client.setCertificate(aws_cert_crt); // for client verification
  // client.setPrivateKey(aws_cert_key);	// for client verification
  // client.setPrivateKey(aws_cert_pem);

  while(!topic_obteined){
    topic_obteined = get_topic(expected_topic_length);
    delay(1000);
  }

  setupMQTT();
}



/***************************************
**************** LOOP ******************
****************************************/

void loop() {

 //si el pulsador wifi esta en low, activamos el acces point de configuración
  if (digitalRead(WIFI_PIN) == LOW ) {
    wifiManager.startConfigPortal("Panel de conexión.\n Gestor IoT WIFI-MANAGER ");
    Serial.println("Conectados a WiFi!!! :)");
  }


  if (!client.connected()) {
		connectMQTT();
	}

  if(startTimer && (millis() - lastTrigger > (timeSeconds*1000))) {
    Serial.println("\nMotion stopped...");
    startTimer = false;
  }
  
  if(mqttclient.connected()){
    temp = getTemperatura();
    hum = getHumedad();
    if(alarma){
      if (mov){
        mensaje=true;
        send_to_gmail(alarma,mensaje);
        mensaje=false;
      }
    }
    if(autoPanel==1){
      lastTemp=temp;
      lastHum=hum;
      if(sw3==0){
        Serial.println("Activando sensor AUTO");
        sw3=1;
      }
      if (mov){
        mov=false;
        Serial.println("\nEnviando a base de datos por movimiento detectado.\n");
        send_to_database();
      }
      if (temp < 20){
        Serial.println("\nAVISO: Temperatura BAJA.");
        if(sw1==0){
          Serial.println("\nActivando aire caliente AUTO.\n");
          sw1=1;
        }
        if(sw2==1){
          Serial.println("\nDesactivando aire frío AUTO.\n"); 
          sw2=0;
          digitalWrite(FAN_PIN, LOW);
        }
        Serial.println("\nEnviando a base de datos por temperatura inadecuada.\n");
        send_to_database();
      }else if (temp>25){     
        Serial.println("\nAVISO: Temperatura ALTA.");
        if(sw2==0){
          Serial.println("\nActivando aire frío AUTO.\n"); 
          sw2=1;
          digitalWrite(FAN_PIN, HIGH);
        }
        if(sw1==1){
            Serial.println("\nDesactivando aire caliente AUTO.\n");
            sw1=0;
        }
        Serial.println("\nEnviando a base de datos por temperatura inadecuada.\n");
        send_to_database();
      }else{
        Serial.println("\nTemperatura adecuada.");
        if(sw1==1){
          Serial.println("\nDesactivando aire caliente AUTO.\n");
          sw1=0;
        }else if(sw2==1){
          Serial.println("\nDesactivando aire frío AUTO.\n");
          sw2=0;
          digitalWrite(FAN_PIN, LOW);
        }else{
          Serial.println("\nSin acciones.\n");
        }
      }
    }else{
      if(mov){
        lastTemp=temp;
        lastHum=hum;
        if(sw3==0){
          Serial.println("Activando sensor AUTO por movimiento");
          sw3=1;
        }
        Serial.print("\nEnviando a base de datos por ");

        if(temp > 25){
          Serial.print("temperatura inadecuada.\n");
          Serial.println("\nAVISO: Temperatura ALTA.");
        }else if(temp<20){
          Serial.print("temperatura inadecuada.\n");
          Serial.println("\nAVISO: Temperatura BAJA.");
        }else{
          Serial.print("movimiento detectado.\n");
          Serial.println("\nTemperatura adecuada.");
        }
        send_to_database();
        mov=false;
      }else{
        if(sw3 == 1){
          lastTemp=temp;
          lastHum=hum;
        }
        if(sw1 == 1){
          //el aire caliente está encendido
          lastTemp=temp;
          lastHum=hum;
          if(sw3==0){
            Serial.println("Activando sensor AUTO");
            sw3=1;
          }
          Serial.println("\nEnviando a base de datos por calor encendido.\n");
          send_to_database();
        }
        if(sw2 == (int)1 ){
          //el aire frio está encendido
          lastTemp=temp;
          lastHum=hum;
          Serial.println("\nEnviando a base de datos por frío encendido.\n");
          send_to_database();
        }
      }
    }
    Serial.println("");
    publishTemperatura();
    publishHumedad();
    String to_send = String(lastTemp) + "," + String(lastHum) + "," + String(sw1)+","+ String(sw2)+","+String(sw3)+","+String(autoPanel)+","+String(autoAlarm);
    to_send.toCharArray(msg,20);
    mqttclient.publish(device_topic_publish,msg);
    
    Serial.println("");
    Serial.print("Actualizando Página en: ");
    for(int i=slider; i>0; i--){
      Serial.print(i);
      Serial.print(" ");
      delay(1000);
    }
    Serial.println("");
  }
  mqttclient.loop();
}



//************************************
//*********** FUNCIONES **************
//************************************

void setupWifi(){
  Serial.println("\n\nConfigurando WIFI");
  pinMode(WIFI_PIN,INPUT_PULLUP);  
  //set custom ip for portal
  //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); 

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  //wifiManager.resetSettings(); //En caso de querer que recuerde el ultimo AP, comentar esta linea
  wifiManager.autoConnect("ESP_WIFI_MANAGER");
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  
  //if you get here you have connected to the WiFi
  Serial.println("Conexión a WiFi exitosa!");
}

void setupDHT11(){
  Serial.println("\n\nConfigurando DHT11...");
  dht.begin();
  Serial.println("   Probando funcionamiento DHT11");
  getTemperatura();
  getHumedad();
  Serial.println("DHT11 Configurado");
}

float getTemperatura(){
  float t = dht.readTemperature();
  Serial.println("\n   Calculando Temperatura..");
  if(isnan(t)){
    Serial.println("   Fallo al leer la temperatura");
  }else{
    temperaturaString = "   Temperatura = ";
    temperaturaString+= t;
    temperaturaString+=" C";
    Serial.println(temperaturaString);
  }
  return t;
}

void publishTemperatura(){
  if(mqttclient.publish(device_topic_publish_t ,(char*) temperaturaString.c_str())){
    Serial.println("   Publicacion temperatura OK");
  }else{
    Serial.println("   Publicacion FALLIDA");
  }
}

float getHumedad(){
  float h = dht.readHumidity();
  Serial.println("\n   Calculando Humedad..");
  if(isnan(h)){
    Serial.println("\n   Fallo al leer la humedad");
  }else{
    humedadString = "   Humedad = ";
    humedadString += h;
    humedadString +=" %";
    Serial.println(humedadString);
  }
  return h;
}

void publishHumedad(){
  if(mqttclient.publish(device_topic_publish_h ,(char*) humedadString.c_str())){
    Serial.println("   Publicacion humedad OK");
  }else{
    Serial.println("   Publicacion FALLIDA");
  }
}

void setupFan(){
  Serial.println("\n\nConfigurando Ventilador...");
  pinMode(FAN_PIN, OUTPUT);
  Serial.println("   Probando funcionamiento de Ventilador");
  digitalWrite(FAN_PIN, HIGH);
  Serial.println("   Ventilador Encendido --> Ok");
  Serial.print("   Apagando Ventilador en: ");
  for(int i=3; i>0; i--){
      Serial.print(i);
      Serial.print(" ");
      delay(1000);
    }
  digitalWrite(FAN_PIN, LOW);
  Serial.println("\n   Ventilador Apagado --> Ok");
  Serial.println("Ventilador Configurado");
}

void setupPIR(){
  Serial.println("\n\nConfigurando PIR...");
  // PIR Motion m mode INPUT_PULLUP
  pinMode(MOTION_PIN, INPUT_PULLUP);
  // Set motion pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(MOTION_PIN), detectsMovement, RISING);
  Serial.println("PIR Configurado");
}

// Checks if motion was detected, sets LED HIGH and starts a timer
void IRAM_ATTR detectsMovement() {
  Serial.println("\nMOTION DETECTED!!!");
  startTimer = true;
  lastTrigger = millis();
  mov=true;
  if(alarma){
    mensaje=true;
  }
}

//función para obtener el tópico perteneciente a este dispositivo
bool get_topic(int length){

  Serial.println("\n\nIniciando conexión segura para obtener tópico raíz...");

  if (!client2.connect(server, 443)) {
    Serial.println("   Falló conexión!");
    return false;
  }else {
    Serial.println("   Conectados a servidor para obtener tópico - ok");
    // Make a HTTP request:
    String data = "gdp="+get_data_password+"&sn="+serial_number+"\r\n";
    client2.print(String("POST ") + "/app/getdata/gettopics" + " HTTP/1.1\r\n" +\
                 "Host: " + server + "\r\n" +\
                 "Content-Type: application/x-www-form-urlencoded"+ "\r\n" +\
                 "Content-Length: " + String (data.length()) + "\r\n\r\n" +\
                 data +\
                 "Connection: close\r\n\r\n");

    Serial.println("   Solicitud enviada - ok");

    while (client2.connected()) {
      String line = client2.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("   Headers recibidos - ok");
        break;
      }
    }

    String line;
    while(client2.available()){
      line += client2.readStringUntil('\n');
    }
    Serial.println("\n"+line);

    String temporal_topic = s.separa(line,'#',1);
    String temporal_user = s.separa(line,'#',2);
    String temporal_password = s.separa(line,'#',3);



    Serial.println("   El tópico es: " + temporal_topic);
    Serial.println("   El user MQTT es: " + temporal_user);
    Serial.println("   La pass MQTT es: " + temporal_password);
    Serial.println("   La cuenta del tópico obtenido es: " + String(temporal_topic.length()));

    if (temporal_topic.length()==length){
      Serial.println("   El largo del tópico es el esperado: " + String(temporal_topic.length()));

      String temporal_topic_subscribe = temporal_topic + "/actions/#";
      temporal_topic_subscribe.toCharArray(device_topic_subscribe,40);
      Serial.println("   Suscripción a: "+String(device_topic_subscribe)+" --> Ok");
      String temporal_topic_publish = temporal_topic + "/data";
      temporal_topic_publish.toCharArray(device_topic_publish,40);
      temporal_topic_publish = temporal_topic + "/temperatura";
      temporal_topic_publish.toCharArray(device_topic_publish_t,40);
      temporal_topic_publish = temporal_topic + "/humedad";
      temporal_topic_publish.toCharArray(device_topic_publish_h,40);
      temporal_user.toCharArray(mqtt_user,20);
      temporal_password.toCharArray(mqtt_pass,20);

      client2.stop();
      return true;
    }else{
      client2.stop();
      return false;
    }
  }
}

void send_to_database(){

  Serial.println("\n\nIniciando conexión segura para enviar a base de datos...");

  if (!client2.connect(server, 443)) {
    Serial.println("   Falló conexión!");
  }else {
    Serial.println("   Conectados a servidor para insertar en db - ok");
    // Make a HTTP request:
    String data = "idp="+insert_password+"&sn="+serial_number+"&temp="+String(temp)+"&hum="+String(hum)+"\r\n";
    client2.print(String("POST ") + "/app/insertdata/insert" + " HTTP/1.1\r\n" +\
                 "Host: " + server + "\r\n" +\
                 "Content-Type: application/x-www-form-urlencoded"+ "\r\n" +\
                 "Content-Length: " + String (data.length()) + "\r\n\r\n" +\
                 data +\
                 "Connection: close\r\n\r\n");

    Serial.println("   Solicitud enviada - ok");

    while (client2.connected()) {
      String line = client2.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("   Headers recibidos - ok");
        break;
      }
    }
    String line;
    while(client2.available()){
      line += client2.readStringUntil('\n');
    }
    Serial.println("\n"+line);
    client2.stop();

  }

}

void send_to_gmail(bool alarma, bool mensaje){
  Serial.println("\n\nIniciando sesión en Gmail...");
  datosSMTP.setLogin("smtp.gmail.com", 465, "jbenagtfg@gmail.com", "javier00tfg");
  Serial.println("Escribiendo mensaje...");
  // Establecer el nombre del remitente y el correo electrónico
  datosSMTP.setSender("GestorIoT", "jbenagtfg@gmail.com");
  if(mensaje==false && alarma==true){
    // Establezca la prioridad o importancia del correo electrónico High, Normal, Low o 1 a 5 (1 es el más alto)
    datosSMTP.setPriority("Normal");
    // Establecer el asunto
    datosSMTP.setSubject("Información sobre la alarma");
    // Establece el mensaje de correo electrónico en formato de texto (sin formato)
    datosSMTP.setMessage("Alarma activada", false);
    // Agregar destinatarios, se puede agregar más de un destinatario
    datosSMTP.addRecipient("jbenagtfg@gmail.com");
  }else if(mensaje==false && alarma==false){
    // Establezca la prioridad o importancia del correo electrónico High, Normal, Low o 1 a 5 (1 es el más alto)
    datosSMTP.setPriority("Normal");
    // Establecer el asunto
    datosSMTP.setSubject("Información sobre la alarma");
    // Establece el mensaje de correo electrónico en formato de texto (sin formato)
    datosSMTP.setMessage("Alarma desactivada", false);
    // Agregar destinatarios, se puede agregar más de un destinatario
    datosSMTP.addRecipient("jbenagtfg@gmail.com");
  }else if(mensaje==true && alarma==true){
    // Establezca la prioridad o importancia del correo electrónico High, Normal, Low o 1 a 5 (1 es el más alto)
    datosSMTP.setPriority("High");
    // Establecer el asunto
    datosSMTP.setSubject("¡¡¡ALARMA!!!");
    // Establece el mensaje de correo electrónico en formato de texto (sin formato)
    datosSMTP.setMessage("La alarma ha detectado un intruso!!", false);
    // Agregar destinatarios, se puede agregar más de un destinatario
    datosSMTP.addRecipient("jbenagtfg@gmail.com");
  }

  //Comience a enviar correo electrónico.
  if (!MailClient.sendMail(datosSMTP)){
    Serial.println("Error enviando el correo, " + MailClient.smtpErrorReason());
    delay(1000);
    send_to_gmail(alarma,mensaje);
  }
  Serial.println("Mensaje enviado con éxito!!");

  //Borrar todos los datos del objeto datosSMTP para liberar memoria
  datosSMTP.empty();
}

void setupMQTT(){
  Serial.println("\n\nConfigurando broker...");
  mqttclient.setServer(mqtt_server, mqtt_port);
  mqttclient.setCallback(callbackMQTT);
  Serial.println("   Dispositivo escuchando mensajes del servidor MQTT");
  Serial.println("Servidor MQTT configurado");
}

void callbackMQTT(char* topic, byte* payload, unsigned int length) {
  String incoming= "";
  Serial.println("\n\n***************************************");
  Serial.println("Mensaje recibido desde servidor MQTT.");
  Serial.print("Tópico: ");
  Serial.print(topic);
	for (int i = 0; i < length; i++) {
		incoming += (char)payload[i];
	}
	incoming.trim();
	Serial.println("\nMensaje -> " + incoming);
  Serial.println("***************************************");

  String str_topic = String(topic);
  String command = s.separa(str_topic,'/',3);

  if(command=="ala"){
    Serial.println("Alarma pasa manualmente a estado " + incoming);
    autoAlarm = incoming.toInt();
    Serial.print("Alarma: ");
    if(autoAlarm == int(1)){
      Serial.println("ACTIVADO.");
      alarma=true;
      send_to_gmail(alarma,mensaje);
    }else if(autoAlarm ==int(0)){
      Serial.println("DESACTIVADO.");
      alarma=false;
      send_to_gmail(alarma,mensaje);
    }
  }

  if(command=="aut"){
    Serial.println("AutoPanel pasa manualmente a estado " + incoming);
    autoPanel = incoming.toInt(); 
    Serial.print("Modo automatico: ");
    if(autoPanel == int(1)){
      Serial.println("ACTIVADO.");
    }else if(autoPanel ==int(0)){
      Serial.println("DESACTIVADO.");
    }
  }

  if(command=="sw1"){
    if(autoPanel==1){
      Serial.println("Modo automatico: DESACTIVADO.");
      autoPanel=0;
    }
    Serial.println("Sw1 pasa manualmente a estado " + incoming);
    sw1 = incoming.toInt();
    Serial.print("Aire caliente: ");
    if(sw1 == int(1)) {
      Serial.println("ACTIVADO");
      if(sw2==int(1)){
        Serial.println("Desactivando aire frío AUTO ");
        sw2=0;
        digitalWrite(FAN_PIN, LOW);
      }
      if(sw3==int(0)){
        Serial.println("Activando sensor AUTO");
        sw3=1;
      }
    }else if (sw1 == int(0)) {
      Serial.println("DESACTIVADO");
    }
  }

  if(command=="sw2"){
    if(autoPanel==1){
      Serial.println("Modo automatico: DESACTIVADO.");
      autoPanel=0;
    }
    Serial.println("Sw2 pasa manualmente a estado " + incoming);
    sw2 = incoming.toInt();
    Serial.print("Aire frio: ");
    if(sw2 == int(1)){
      Serial.println("ACTIVADO");
      digitalWrite(FAN_PIN, HIGH);
      if(sw1==int(1)){
        Serial.println("Desactivando aire caliente AUTO.");
        sw1=0;
      }
      if(sw3==int(0)){
        Serial.println("Activando sensor AUTO");
        sw3=1;
      }
    }else if (sw2 == int(0)) {
      Serial.println("DESACTIVADO");
      digitalWrite(FAN_PIN, LOW);
    }
  }
  if(command=="sw3"){
    Serial.println("Sw3 pasa manualmente a estado " + incoming);
    sw3 = incoming.toInt();
    Serial.print("Sensor de temperatura y humedad: ");
    if(sw3 == int(1)){
      Serial.println("ACTIVADO.");
    }else if(sw3==int(0)){
      Serial.println("DESACTIVADO.");
    }
  }
//no pasar de 10 segundos
  if(command=="slider"){
    Serial.println("Slider pasa a estado " + incoming);
    slider = incoming.toInt();
    if(slider < int(3)){
      Serial.println("El tiempo mínimo es 3 Segundos.");
      slider = 3;
    }
    Serial.print("Tiempo de refresco de la página: ");
    Serial.print(slider);
    Serial.println(" Segundos.");
  }
}

void connectMQTT(){
  while(!mqttclient.connected()){
    //Creo un idClient aleatorio
    String clientId = "stL_esp32_";
          clientId += String(random(0xffff), HEX);
          //intentamos conectar
    Serial.println("");
    Serial.println("Connecting client to : "+String(mqtt_server));
    if(mqttclient.connect(clientId.c_str(), mqtt_user, mqtt_pass)){
      Serial.println("Connected --> Ok");
      if(mqttclient.subscribe(device_topic_subscribe)){
        Serial.print("Suscripcion a topico: ");
        Serial.print(device_topic_subscribe);
        Serial.println(" --> Ok");
      }else{
        Serial.println("Suscripcion fallida");
      }
    }else{
        Serial.print("ERROR: ");
        Serial.println(mqttclient.state());
      	Serial.println(" Intentamos de nuevo en 4 segundos");
        delay(4000);
    }
  }
}