#include <ArduinoBLE.h>
#include <Arduino_HTS221.h>
#include <Arduino_LPS22HB.h>

BLEService ClimateSensors ("db2d0d1f-68d4-4bd1-8692-d4cb7986b3c1");
BLEDescriptor uartServiceDescriptor("2901", "UART");

BLECharCharacteristic tempCharacteristic("9475d433-6f83-460d-8fcc-eefd6758db50",BLERead | BLENotify );
BLECharCharacteristic humidityCharacteristic("0d53afd9-c0a5-4de4-971f-30ef87bdb046", BLERead | BLENotify);
BLECharCharacteristic pressureCharacteristic("d770192c-a0dc-4e3b-854f-72a47247930b", BLERead | BLENotify);

BLEByteCharacteristic rxCharacteristic("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLEWrite);
BLEDescriptor rxServiceDescriptor("2901", "RX - Receive Data (Write)");


BLEByteCharacteristic txCharacteristic("6E400003-B5A3-F393-E0A9-E50E24DCCA9E", BLENotify);
BLEDescriptor txServiceDescriptor("2901", "TX - Transfer Data (Notify)");


long previousMillis = 0;

void setup() {
 
  Serial.begin(9600);
  while (!Serial);
 

pinMode(LED_BUILTIN, OUTPUT);
digitalWrite (LED_BUILTIN, LOW);

  if (!HTS.begin()) {
     Serial.println("Failed to initialize temperature sensor!");
     while (1);
  }

  if (!BLE.begin()) {
    Serial.println("Starting BLE Failed!");
    while (1);
  }
  if (!BARO.begin()) {
    Serial.println("Failed to initialize pressure sensor!");
    while(1);
  }

    BLE.setLocalName("RoverTeam2");
    BLE.setAdvertisedService(ClimateSensors);
    ClimateSensors.addCharacteristic(tempCharacteristic);
    ClimateSensors.addCharacteristic(humidityCharacteristic);
    ClimateSensors.addCharacteristic(pressureCharacteristic);

    
  // add the characteristics to the service
  ClimateSensors.addCharacteristic(rxCharacteristic);
  ClimateSensors.addCharacteristic(txCharacteristic);

  rxCharacteristic.addDescriptor(rxServiceDescriptor);
  txCharacteristic.addDescriptor(txServiceDescriptor);
  
    BLE.addService(ClimateSensors);
    tempCharacteristic.writeValue(0);
    humidityCharacteristic.writeValue(0);
    pressureCharacteristic.writeValue(0);
    
  BLE.advertise();

  Serial.println("Bluetooth is Active, Waiting for Bluetooth Connection..");
 
}

void loop() {

  BLEDevice central = BLE.central();
  
  // poll for BLE events
  BLE.poll();

  if (rxCharacteristic.valueUpdated()) {
    byte testByte = 0; 
    Serial.println(rxCharacteristic.readValue(testByte));
  }

  if(central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, HIGH);
  }

  while (central.connected()) {
    long currentMillis = millis();
    if (currentMillis - previousMillis >= 5000) {
      previousMillis = currentMillis;
     
      updateSensors();
     
    }
  }
}

void updateSensors() {

  float temperature = HTS.readTemperature(FAHRENHEIT);
  float humidity = HTS.readHumidity();
  float pressure = BARO.readPressure(PSI);
  
// print the Temp sensor value
  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" °F");
  
// print the Humidity sensor value
  Serial.print("Humidity = ");
  Serial.print(humidity);
  Serial.println(" %");

// print the Pressure sensor value
  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println(" PSI");

  Serial.println();

  tempCharacteristic.writeValue(temperature);
  humidityCharacteristic.writeValue(humidity);
  pressureCharacteristic.writeValue(pressure);

}
