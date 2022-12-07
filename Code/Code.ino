#include <ESP8266WiFi.h> //เรียกใช้ไลเบอรี่ WIFI ของ ESP8266
#include <PubSubClient.h> //เรียกใช้ไลเบอรี่ PubSubClient พวก MQTT หรือ https
#include <Wire.h> //เรียกใช้ไลเบอรี่ Wire สำหรับการอ่านขา I2C หรืออืนๆ
#include <BH1750.h> //เรียกใช้ไลเบอรี่ เซนเซอร์รับแสง
#include <DHT.h> //เรียกใช้ไช้ไลเบอรี่ เซนเซอร์อุณหภูมิ

/*
#define คือ การประการตัวแปรตัวแปรแต่ไม่ใช่ตัวชื่อเรียกตัวแปร
WIFI_STA_NAME หรืออืนๆ คือ ชื่อตัวแปร
หลังจากชื่อตัวแปรมันคือ ข้อมูลที่เก็บเข้าในตัวแปร
ส่วนมากใช้ เก็บ ข้อความ
*/
//WiFi Setting
#define WIFI_STA_NAME "รหัสไวไฟ" 
#define WIFI_STA_PASS  "พาส"
//MQTT Setting
#define MQTT_SERVER   "URLหรือIP เซิฟเวอร์ MQTT"
#define MQTT_PORT     1883
#define MQTT_USERNAME "ชื่อผู้ใช้"
#define MQTT_PASSWORD "รหัสผ่าน"
#define MQTT_NAME     "..."
//Pin Setting
/*
การประกาศขา จะไม่มี เครื่องหมาย "" แต่เป็นการใส่ชื่อตรงๆ
*/
#define LED_LIGHT D0
#define LED_PUMP D1
#define LED_AIR D2
#define SoilDelight A0
#define DHTPIN D5
//DHT Setting
/*
เป็นการประกาศชนิดของ DHT
*/
#define DHTTYPE DHT22 
//Val Setting
/*
(50) คือการประกาศขนาดของข้อมูล
*/
#define MSG_BUFFER_SIZE (50)

/*
char คือชนิดข้อมูล ในที่นี้คือข้อมูลตัวอักษร
ตามด้วยชือ
[MSG_BUFFER_SIZE] ขนาดของ char 
*/
char numRandom[MSG_BUFFER_SIZE];
char usepump[MSG_BUFFER_SIZE];
char inputLux[MSG_BUFFER_SIZE];
char inputDHT[MSG_BUFFER_SIZE];
int valSoil = 0;
float valLux,valDht = 0;
String RandomString = "";
//Code Setting
//การสร้าง OBJ ของ WiFiClinet ให้ชือ client
WiFiClient client;
//ตั้งค่าการเชื่อมต่อของ matt โดยใช้ clinet เชื่อมต่อก็คือ obj ของ WiFi  และสร้าง obj ของ PubSubClient ชือ mqtt
PubSubClient mqtt(client);
//สร้าง obj ของ BH1750 ชื่อ lightMeter
BH1750 lightMeter;
//สร้าง obj ของ DHT ชือ dht และกำหนดชนิดของ dht และ ขา
DHT dht(DHTPIN, DHTTYPE);
//MQTT Void
void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String topic_str = topic, payload_str = (char*)payload;
  Serial.println("[" + topic_str + "]: " + payload_str);


  if(topic_str == "/pumptree/status") //1
  {
    if(payload_str == "true")
    {
      digitalWrite(LED_PUMP,HIGH);
      Serial.println("pumptree On");
    }else if (payload_str == "false"){
      digitalWrite(LED_PUMP,LOW);
      Serial.println("pumptree Off");
    }
  }else if(topic_str == "/air/status") //2
  {
    if(payload_str == "true")
    {
      digitalWrite(LED_AIR,HIGH);
      Serial.println("Air On");
    }else if (payload_str == "false"){
      digitalWrite(LED_AIR,LOW);
      Serial.println("Air Off");
    }
  }else if(topic_str == "/lux/status") //3
  {
    if(payload_str == "true")
    {
      digitalWrite(LED_LIGHT,HIGH);
      Serial.println("lux On");
    }else if (payload_str == "false"){
      digitalWrite(LED_LIGHT,LOW);
      Serial.println("lux Off");
    }
  }
}

void setup() {
  Serial.begin(115200);
  //WiFiSetting
  Serial.print("Connecting to ");
  Serial.println(WIFI_STA_NAME);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_STA_NAME, WIFI_STA_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

/*
 .begin คือกระกาศว่าให้มันทำงานใน() คือการกำหนดค่าต่างๆ
*/
  Wire.begin(D4,D3); //กำหนด ขา Wire.h (I2C)
  lightMeter.begin();
  dht.begin();
  pinMode(SoilDelight,INPUT);
  pinMode(LED_AIR,OUTPUT);
  digitalWrite(LED_AIR,LOW);
  pinMode(LED_LIGHT,OUTPUT);
  digitalWrite(LED_LIGHT,LOW);
  pinMode(LED_PUMP,OUTPUT);
  digitalWrite(LED_PUMP,LOW);
  //Mqtt Setting
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(callback);
}

void loop() {
  //เปิดเครื่อง ESP ครั้งแรกมันยังไม่มีการเชื่อมต่อ mqtt.connected จึงเท่ากับ  false
  if (mqtt.connected() == false) {
    Serial.print("MQTT connection... ");
    if (mqtt.connect(MQTT_NAME, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("connected");
      //subscribe คือการรับสมัครข้อมูล ของ mqtt
      mqtt.subscribe("/pumptree/status");
      mqtt.subscribe("/air/status");
      mqtt.subscribe("/lux/status");
    } else {
      Serial.println("failed");
      delay(1000);
    }
  } else {
    //หลังจากเชื่อมต่อแล้วมันทำ งานคำสั่งนี้
    mqtt.loop();
  }
  int numrandom_1 = random(0,100);
  //Soil Check
  valSoil = analogRead(SoilDelight); 
  //Lux Check
  valLux = lightMeter.readLightLevel(); //readLightLevel คือฟังชั่นที่ใช้ในการอ่านค่า
  //DHT22 Check
  valDht = dht.readTemperature(); //readTemperature คือฟังชั่นที่ใช้ในการอ่านค่า
   if (isnan(valDht)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  //snprintf หมายความว่า แปลงข้อมูลมาเก็บอีกตัวแปรหนึาง แบบ BuFFER size 
  snprintf (numRandom, MSG_BUFFER_SIZE,"%d", numrandom_1);
  snprintf (usepump, MSG_BUFFER_SIZE,"%d", valSoil);
  snprintf (inputLux,MSG_BUFFER_SIZE,"%.2f",valLux);
  snprintf (inputDHT,MSG_BUFFER_SIZE,"%.2f",valDht);
  //mqtt.publish ส่งค่ากลับไปที่ node red (ใช้ MQTTส่ง)
  mqtt.publish("esp/status",numRandom);
  mqtt.publish("/pumptree/use",usepump);
  mqtt.publish("/lux/use",inputLux);
  mqtt.publish("/air/use",inputDHT);
  delay(1000);
}