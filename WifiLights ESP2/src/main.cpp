#include <WiFi.h>
#include <PubSubClient.h>
#include <FastLED.h>

const char* ssid = "*********";
const char* password = "******";
const char* mqttServer = "********";
const int mqttPort = 1883; // or 8883 for encrypted communication
const char* clientId = "*******"; //light1

WiFiClient espClient;
PubSubClient client(espClient);

#define numLEDs 7

#define LEDcontrolPin 27
int touchPin = 33;

CRGB leds[numLEDs];

CRGB colours[] = {
  CRGB::Red,
  CRGB::Green,
  CRGB::Blue,
  CRGB::Yellow,
  CRGB::Purple,
  CRGB::Cyan,
  CRGB::White,
  CRGB::Black
};


int currentColourIndex = 0;

void changeColour (int lightNumber)
{
    FastLED.clear();

  for (int i = 0; i < numLEDs; i++) {
    leds[i] = colours[lightNumber];
  }

  FastLED.show();                 
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received in topic: ");
  Serial.println(topic);
  Serial.print("Message:");

  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  

  if (message.indexOf("light") != -1)
  {
      
      Serial.println("CurrentColourIndex: "+ String(currentColourIndex));

      int lightNumber = (message.charAt(5) - '0');
      
      Serial.println("index received to change colour: " + String(lightNumber));


      changeColour(lightNumber);
      currentColourIndex = lightNumber;
      
    
  }

  if (message == "startUp1")
  {
    String m = "light";
    Serial.println("index: " + String(currentColourIndex));
    m.concat(currentColourIndex);
    client.publish("topic", m.c_str());
    Serial.println("Sent message 'light" + String(currentColourIndex));
  }

  
}


void setup() {

Serial.begin(115200);
  pinMode(LEDcontrolPin, OUTPUT); // Set the built-in LED pin as an output

  digitalWrite(touchPin, HIGH);

  FastLED.addLeds<WS2812, LEDcontrolPin, GRB>(leds, numLEDs);
  FastLED.setBrightness(50); 

  FastLED.clear();
  FastLED.show();
  

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi!");
  leds[0] = colours[0];
  FastLED.show();

  // Connect to MQTT broker
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  while (!client.connected()) {
    if (client.connect(clientId)) {
      // Subscribe to a topic
      client.subscribe("topic");

      String message = "startUp2";
      client.publish("topic", message.c_str());
      Serial.println("Sent message 'startUp2'" + String(currentColourIndex));

      Serial.println("Connected to broker!");
      leds[0] = colours[1];
      FastLED.show();
    }
    delay(1000);
  }
}

bool buttonPressed = false; // flag to indicate if the button has been pressed
unsigned long buttonTimer = 0; // timer to reset the flag

void loop() {
  unsigned long currentMillis = millis();


  Serial.println(touchRead(touchPin));
  int touchValue = touchRead(touchPin);
  int threshold = 24;

if (touchValue < threshold && buttonPressed != true)
{
    //Switch colour
    // set the flag to true and start the timer
    buttonPressed = true;
    buttonTimer = currentMillis;

    currentColourIndex = (currentColourIndex + 1) % (sizeof(colours) / sizeof(colours[0]));

    // publish the message
    String message = "light";
    message.concat(currentColourIndex);
    client.publish("topic", message.c_str());
    Serial.println("Sent message 'light" + String(currentColourIndex));
    
}


  delay(25);

  // if the button has been pressed and the timer has expired
  if (buttonPressed && currentMillis - buttonTimer > 400) {
    // reset the flag
    buttonPressed = false;
  }
  

 

  client.loop();

  // other code in the loop
}


