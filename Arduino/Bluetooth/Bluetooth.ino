#define ledPin 7
String Data = "";
void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  Serial.begin(9600); // Default communication rate of the Bluetooth module
}
void loop() {
  while(Serial.available() > 0){ // Checks whether data is comming from the serial port
  char character;
    character = Serial.read(); // Reads the data from the serial port
    Data = Data + character;
    Serial.println(Data);
 }
 if (Data == "OFF") {
  digitalWrite(ledPin, LOW); // Turn LED OFF
  Serial.println("LED: OFF"); // Send back, to the phone, the String "LED: OFF"
  Data = "";
 }else if (Data == "ON") {
  digitalWrite(ledPin, HIGH);
  Serial.println("LED: ON");
  Data = "";
 }else if(Data == "connected"){
  Data = "";
 }
}
