
// Enable debug prints
#define MY_DEBUG

// Enable and select radio type attached 
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69
//#define MY_RS485


#include <SPI.h>
#include <MySensors.h>  
#include <DHT.h>

// Set this to the pin you connected the DHT's data pin to
#define DHT_DATA_PIN 3

// Set this offset if the sensor has a permanent small offset to the real temperatures
#define SENSOR_TEMP_OFFSET 0

// Sleep time between sensor updates (in milliseconds)
// Must be >1000ms for DHT22 and >2000ms for DHT11
static const uint64_t UPDATE_INTERVAL = 30000;

// Force sending an update of the temperature after n sensor reads, so a controller showing the
// timestamp of the last update doesn't show something like 3 hours in the unlikely case, that
// the value didn't change since;
// i.e. the sensor would force sending an update every UPDATE_INTERVAL*FORCE_UPDATE_N_READS [ms]
static const uint8_t FORCE_UPDATE_N_READS = 10;


#define RELAY_PIN 4  // Arduino Digital I/O pin number for relay
#define NUMBER_OF_RELAYS 1 // Total number of attached relays
#define RELAY_ON 1  // GPIO value to write to turn on attached relay
#define RELAY_OFF 0 // GPIO value to write to turn off attached relay

#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define CHILD_ID_RELAY 2



float lastTemp;
float lastHum;
uint8_t nNoUpdatesTemp;
uint8_t nNoUpdatesHum;
bool metric = true;
bool state;
unsigned long CHECK_TIME = millis();

MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgRelay(CHILD_ID_RELAY,V_STATUS);

DHT dht;

void before()
{
  //for (int sensor=1, pin=RELAY_1; sensor<=NUMBER_OF_RELAYS; sensor++, pin++) {
    // Then set relay pins in output mode
  //  pinMode(RELAY_1, OUTPUT);
    // Set relay to last known state (using eeprom storage)
  // state = loadState(sensor)?RELAY_ON:RELAY_OFF;
  // digitalWrite(RELAY_1, !state);
  //}
  
}

void presentation()  
{ 
  // Send the sketch version information to the gateway
  sendSketchInfo("Thermostat Test", "1.1");

  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID_HUM, S_HUM);
  present(CHILD_ID_TEMP, S_TEMP);
  present(CHILD_ID_RELAY, S_BINARY);
  
  

  metric = getControllerConfig().isMetric;
}


void setup()
{
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF);
    // Set relay to last known state (using eeprom storage) 
  state = loadState(CHILD_ID_RELAY);
  digitalWrite(RELAY_PIN, state?RELAY_ON:RELAY_OFF);
  Serial.println("Sending relay state");
  send(msgRelay.set(state));
  
  dht.setup(DHT_DATA_PIN); // set data pin of DHT sensor
  if (UPDATE_INTERVAL <= dht.getMinimumSamplingPeriod()) {
    Serial.println("Warning: UPDATE_INTERVAL is smaller than supported by the sensor!");
  }
  // Sleep for the time of the minimum sampling period to give the sensor time to power up
  // (otherwise, timeout errors might occure for the first reading)
  delay(dht.getMinimumSamplingPeriod());
}


void loop()      
{  
  // Force reading sensor, so it works also after sleep()
 unsigned long NOW_TIME = millis();
  if(NOW_TIME - CHECK_TIME >= UPDATE_INTERVAL)
  {
  dht.readSensor(true);
  readdata();
    CHECK_TIME = NOW_TIME;
  }
  
  
 

  // Sleep for a while to save energy
  //delay(UPDATE_INTERVAL); 
}

void readdata()
{
 // Get temperature from DHT library
  float temperature = dht.getTemperature();
  if (isnan(temperature)) {
    Serial.println("Failed reading temperature from DHT!");
  } else if (temperature != lastTemp || nNoUpdatesTemp == FORCE_UPDATE_N_READS) {
    // Only send temperature if it changed since the last measurement or if we didn't send an update for n times
    lastTemp = temperature;
    if (!metric) {
      temperature = dht.toFahrenheit(temperature);
    }
    // Reset no updates counter
    nNoUpdatesTemp = 0;
    temperature += SENSOR_TEMP_OFFSET;
    send(msgTemp.set(temperature, 1));

    #ifdef MY_DEBUG
    Serial.print("T: ");
    Serial.println(temperature);
    #endif
  } else {
    // Increase no update counter if the temperature stayed the same
    nNoUpdatesTemp++;
  }

  // Get humidity from DHT library
  float humidity = dht.getHumidity();
  if (isnan(humidity)) {
    Serial.println("Failed reading humidity from DHT");
  } else if (humidity != lastHum || nNoUpdatesHum == FORCE_UPDATE_N_READS) {
    // Only send humidity if it changed since the last measurement or if we didn't send an update for n times
    lastHum = humidity;
    // Reset no updates counter
    nNoUpdatesHum = 0;
    send(msgHum.set(humidity, 1));

    #ifdef MY_DEBUG
    Serial.print("H: ");
    Serial.println(humidity);
    #endif
  } else {
    // Increase no update counter if the humidity stayed the same
    nNoUpdatesHum++;
  }
  
}

void receive(const MyMessage &message)
{
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type==V_STATUS) {
    // Change relay state
    digitalWrite(RELAY_PIN, !message.getBool()?RELAY_ON:RELAY_OFF);
    // Store state in eeprom
    state=message.getBool();
    saveState(CHILD_ID_RELAY, state);
    
    // Write some debug info
    //lcd.setCursor(0,0);
    //lcd.print("Incoming change!");
    Serial.print("Incoming change for sensor:");
    Serial.print(message.sensor);
    Serial.print(", New status: ");
    Serial.println(message.getBool());
    send(msgRelay.set(message.getBool()));
    //lcd.setCursor(0, 1);
    //lcd.print("New status: ");
    //lcd.print(message.getBool());
    
  }
}

