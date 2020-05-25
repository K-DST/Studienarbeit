/*
 * music visualisation with MSGEQ7 and remote control via bluetooth app
 * 
 * university project by Steffen Sander & Jannik Brand
 * 
 * 
 * IMPORTANT:   -   In the case new animations will be implemented,
 *                  they need to be added to the commandActions array in the setup function
 *              -   If there is an ongoing animation (which means that the "show"-function is called
 *                  in every interation of the main loop), the "off"-command has to be called before the
 *                  next command. This is a workaround, otherwise the serial input cannot be captured because
 *                  updating the led strip is blocking the interrupts of the serial read. It is not clean, but it 
 *                  works.
 */
#include <FastLED.h>
#include <Adafruit_NeoPixel.h>

#define NUM_LEDS 144
#define LED_DATA_PIN 6
#define MSGEQ7_ANALOG_PIN 0
#define MSGEQ7_STROBE_PIN 2
#define MSGEQ7_RESET_PIN 4

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_DATA_PIN, NEO_GRB + NEO_KHZ800);

CRGB leds[NUM_LEDS];
int spectrumValue[7];

// MSGEQ7 OUT pin produces values around 50-80
// when there is no input, so use this value to
// filter out a lot of the chaff.
int filterValue = 80;

//for wave animation
#define updateLEDS 8 // How many do you want to update every millisecond?

//for bluetooth
String bluetoothData = "";
String lastBluetoothCommand = "";
uint32_t lastColor = "";
String resetCommand = "x";
#define COMMAND_ACTION_ARRAY_SIZE 8


class CommandAction {
    public:
        String bluetoothCommand;
        boolean needsAudioData;
        //for checking whether the animation function should be executed only once after a bluetooth command comes in or in every loop
        boolean callOnce;
        boolean expectsColor;
        //pointer to the function which should be executed when a certain bluetooth command comes in 
        void (*animation_function_pointer)();
        CommandAction(String bluetoothCommand, void (*animation_function_pointer)(), boolean needsAudioData, boolean callOnce, boolean expectsColor) {
            this->animation_function_pointer = animation_function_pointer;
            this->bluetoothCommand = bluetoothCommand;
            this->needsAudioData = needsAudioData;
            this->callOnce = callOnce;
            this->expectsColor = expectsColor;
        }
        CommandAction(){}
};

CommandAction commandActions[COMMAND_ACTION_ARRAY_SIZE];

void setup() { 
    FastLED.addLeds<NEOPIXEL, LED_DATA_PIN>(leds, NUM_LEDS);
    FastLED.setBrightness(100);
    Serial.begin(9600); // Default communication rate of the Bluetooth module
    pinMode(MSGEQ7_ANALOG_PIN, INPUT);
    pinMode(MSGEQ7_STROBE_PIN, OUTPUT);
    pinMode(MSGEQ7_RESET_PIN, OUTPUT);
    // Set analogPin's reference voltage
    analogReference(DEFAULT); // 5V
    // Set startup values for pins
    digitalWrite(MSGEQ7_RESET_PIN, LOW);
    digitalWrite(MSGEQ7_STROBE_PIN, HIGH);

    clearStrip();
    initialAnimation();

    //array of CommandAction objects, 
    //which decide what to do after an incoming bluetooth command 
    //and whether audio values are needed
    //parameters: (command, function, needsAudioData, callOnce, expectsColor)
    commandActions[0] = CommandAction("!", clearStrip, false, true, false);
    commandActions[1] = CommandAction("1", vuMeter1, true, false, false);
    commandActions[2] = CommandAction("2", vuMeter2, true, false, true);
    commandActions[3] = CommandAction("3", seperateStripIn3PiecesWithoutFilter, true, false, false);
    commandActions[4] = CommandAction("4", wave, true, false, false);
    commandActions[5] = CommandAction("5", initialAnimation, false, true, false);//For manually testing initial animation
    commandActions[6] = CommandAction("6", lamp, false, true, true);
    commandActions[7] = CommandAction("7", entireStrip, true, false, true);
    /* 
     *  increment COMMAND_ACTION_ARRAY_SIZE and declare new elements for every new effect
     */
}

void initialAnimation(){
    for(int i = 0; i < (NUM_LEDS/2); i++) { 
        leds[i*2] = CRGB::Blue;
        FastLED.show();
        delay(10);
    }
    int startValue;
    if(NUM_LEDS % 2 == 0){
        startValue = NUM_LEDS -1;
    }else{
        startValue = NUM_LEDS;
    }
    for(int i = NUM_LEDS; i >= (NUM_LEDS/2); i--) { 
        leds[i*2 - startValue] = CRGB::Red;
        FastLED.show();
    }
    delay(500);
    clearStrip();
}

void loop() {
    readBluetoothData(); //collect HC-05 data (can be testet via serial monitor)
    processBluetoothData(); //decide what todo when a new command is sent
}

void lamp(){
    for(int i = 0; i < NUM_LEDS; i++){
        strip.setPixelColor(i, lastColor);
    }
    strip.setBrightness(225);
    strip.show();
}

//beginning from first pixel; turn blue for bass hit
void vuMeter1(){
    int avg = 0;
    for (int i = 0; i < 7; i++){
        avg += spectrumValue[i];
    }
    avg = avg/7;

    //Clear out the NeoPixel String
    for(int i = 0; i < NUM_LEDS; i++){
        strip.setPixelColor(i, strip.Color(0, 0, 0));
    }

    int start = NUM_LEDS / 2; //72
    if(NUM_LEDS % 2 == 1){
        start++;
        strip.setPixelColor((int) (NUM_LEDS / 2), strip.Color(0*4, 60 - 0, map(spectrumValue[0], 0, 1023, 0, 60)));
    }
    //TODO: 60 anpassen auf Pixellänge
    for(int i=0; i < map(avg, 0, 1023, 0, 60); i++){
        strip.setPixelColor(start + i, strip.Color(i*4, 60 - i, map(spectrumValue[0], 0, 1023, 0, 60))); //Added blue flash for bass hit
        strip.setPixelColor(start - 1 - i, strip.Color(i*4, 60 - i, map(spectrumValue[0], 0, 1023, 0, 60)));
        strip.setPixelColor(i, strip.Color(i*4, 60 - i, map(spectrumValue[0], 0, 1023, 0, 60))); //Added blue flash for bass hit
        strip.setPixelColor(NUM_LEDS - 1 - i, strip.Color(i*4, 60 - i, map(spectrumValue[0], 0, 1023, 0, 60))); //Added blue flash for bass hit
    }
    strip.setBrightness(225);
    strip.show();
    
    //Draw the meter on the NeoPixel string
//    for(int i = 0; i < map(avg, 0, 1023, 0, 60); i++){
//        Serial.println(map(avg, 0, 1023, 0, 60));
//        strip.setPixelColor(i, strip.Color(i*4, 60 - i, map(spectrumValue[0], 0, 1023, 0, 60))); //Added blue flash for bass hit
//        //strip.setPixelColor(i, strip.Color(i*4, 60 - i, 0)); //Without blue flash
//    }
//    strip.setBrightness(225);
//    strip.show();
}

void vuMeter2(){
    int intensity = 0;
    for(int i=0; i<7; i++){
        int intensity_temp = map(spectrumValue[i], filterValue, 1023, 0, NUM_LEDS);
        if(intensity_temp > intensity){
            intensity = intensity_temp;
        }
    }
    strip.clear();
    for(int i=0; i<intensity; i++){
        strip.setPixelColor(i, lastColor);
    }  
    strip.show();
    
    //fill_solid(leds, intensity, CRGB(0, 0, 250));
    //FastLED.show();
}
void wave(){
     // Shift all LEDs to the right by updateLEDS number each time
    for(int i = NUM_LEDS - 1; i >= updateLEDS; i--) {
        leds[i] = leds[i - updateLEDS];
    }

    int r = 0;
    int g = 0;
    int b = 0;
    int avg_low = (spectrumValue[0] + spectrumValue[1] + spectrumValue[2]) / 3;
    int avg_high = (spectrumValue[4] + spectrumValue[5] + spectrumValue[6]) / 3;

   
    if(avg_high > 500){
        r = 80;
        g = 169;
        b = 178;
    }
    if(avg_low > 700){
        r = 0;
        g = 200;
        b = 0;
    }
    // Set the left most updateLEDs with the new color
    for(int i = 0; i < updateLEDS; i++) {
        leds[i] = CRGB(r, g, b);
    }
    FastLED.show();
    
    //printColor(nc);
    delay(10);
}

void seperateStripIn7Pieces(){
    int l = NUM_LEDS / 7;
    int i0 = 0;
    int i1 = 0;
    int i2 = 0;
    int i3 = 0;
    int i4 = 0;
    int i5 = 0;
    int i6 = 0;
    if(spectrumValue[0] > 50){
        i0 = spectrumValue[0];
    }
    if(spectrumValue[1] > 50){
        i1 = spectrumValue[1];
    }
    if(spectrumValue[2] > 50){
        i2 = spectrumValue[2];
    }
    if(spectrumValue[3] > 50){
        i3 = spectrumValue[3];
    }
    if(spectrumValue[4] > 50){
        i4 = spectrumValue[4];
    }
    if(spectrumValue[5] > 50){
        i5 = spectrumValue[5];
    }
    if(spectrumValue[6] > 50){
        i6 = spectrumValue[6];
    }
    fill_gradient_RGB(leds, 0, CRGB(i0, 0, 0), l-1, CRGB(i0, 0, 0));
    fill_gradient_RGB(leds, l, CRGB(i1, 0, i1), 2*l-1, CRGB(i1, 0, i1));
    fill_gradient_RGB(leds, 2*l, CRGB(0, 0, i2), 3*l-1, CRGB(0, 0, i2));
    fill_gradient_RGB(leds, 3*l, CRGB(0, i3, i3), 4*l-1, CRGB(0, i3, i3));
    fill_gradient_RGB(leds, 4*l, CRGB(0, i4, 0), 5*l-1, CRGB(0, i4, 0));
    fill_gradient_RGB(leds, 5*l, CRGB(i5, i5, 0), 6*l-1, CRGB(i5, i5, 0));
    fill_gradient_RGB(leds, 6*l, CRGB(i6, i6, i6), 7*l-1, CRGB(i6, i6, i6));
    FastLED.show();
}

void seperateStripIn3Pieces(){
    //clearStrip();
    int l = NUM_LEDS / 3;
    int low = 0;
    int mid = 0;
    int high = 0;
    if(spectrumValue[1] >= 130){
        low = spectrumValue[1];
        low = map(low, 130, 255, 0, 255);
    }
    if(spectrumValue[4] >= 70){
        mid = spectrumValue[4];
        mid = map(mid, 70, 255, 0, 255);
    }
    if(spectrumValue[6] >= 70){
        high = spectrumValue[6];
        high = map(high, 70, 255, 0, 255);
    }
    
    
    
    fill_gradient_RGB(leds, 0, CRGB(low, 0, 0), l-1, CRGB(low, 0, 0));
    fill_gradient_RGB(leds, l, CRGB(0, mid, 0), 2*l-1, CRGB(0, mid, 0));
    fill_gradient_RGB(leds, l+l, CRGB(0, 0, high), 3*l-1, CRGB(0, 0, high));
    FastLED.show();
}

void seperateStripIn3PiecesWithoutFilter(){
    int l = NUM_LEDS / 3;
    fill_gradient_RGB(leds, 0, CRGB(spectrumValue[1], 0, 0), l-1, CRGB(spectrumValue[1], 0, 0));
    fill_gradient_RGB(leds, l, CRGB(0, spectrumValue[4], 0), 2*l-1, CRGB(0, spectrumValue[4], 0));
    fill_gradient_RGB(leds, l+l, CRGB(0, 0, spectrumValue[6]), 3*l-1, CRGB(0, 0, spectrumValue[6]));
    FastLED.show();
}

void seperateStripIn7PiecesWithoutFilter(){
    int l = NUM_LEDS / 7;
    fill_gradient_RGB(leds, 0, CRGB(spectrumValue[0], 0, 0), l-1, CRGB(spectrumValue[0], 0, 0));
    fill_gradient_RGB(leds, l, CRGB(spectrumValue[1], 0, spectrumValue[1]), 2*l-1, CRGB(spectrumValue[1], 0, spectrumValue[1]));
    fill_gradient_RGB(leds, 2*l, CRGB(0, 0, spectrumValue[2]), 3*l-1, CRGB(0, 0, spectrumValue[2]));
    fill_gradient_RGB(leds, 3*l, CRGB(0, spectrumValue[3], spectrumValue[3]), 4*l-1, CRGB(0, spectrumValue[3], spectrumValue[3]));
    fill_gradient_RGB(leds, 4*l, CRGB(0, spectrumValue[4], 0), 5*l-1, CRGB(0, spectrumValue[4], 0));
    fill_gradient_RGB(leds, 5*l, CRGB(spectrumValue[5], spectrumValue[5], 0), 6*l-1, CRGB(spectrumValue[5], spectrumValue[5], 0));
    fill_gradient_RGB(leds, 6*l, CRGB(spectrumValue[6], spectrumValue[6], spectrumValue[6]), 7*l-1, CRGB(spectrumValue[6], spectrumValue[6], spectrumValue[6]));
    FastLED.show();
}

void entireStrip(){
    int low = 0;
    int mid = 0;
    int high = 0;
    
    if(spectrumValue[1] >= 130){
        low = spectrumValue[1];
        low = map(low, 130, 255, 0, 255);
    }
    if(spectrumValue[4] >= 70){
        mid = spectrumValue[4];
        mid = map(mid, 70, 255, 0, 255);
    }
    if(spectrumValue[6] >= 70){
        high = spectrumValue[6];
        high = map(high, 70, 255, 0, 255);
    }
    fill_solid( leds, NUM_LEDS, CRGB(low, mid, high));
    FastLED.show();
}

void collectAudio(){
    // Set reset pin low to enable strobe
    digitalWrite(MSGEQ7_RESET_PIN, HIGH);
    digitalWrite(MSGEQ7_RESET_PIN, LOW);
    // Get all 7 spectrum values from the MSGEQ7
    for (int i = 0; i < 7; i++){
        digitalWrite(MSGEQ7_STROBE_PIN, LOW);
        delayMicroseconds(40); // Allow output to settle
         spectrumValue[i] = analogRead(MSGEQ7_ANALOG_PIN);
        //Serial.print(spectrumValue[i]);
        // Constrain any value above 1023 or below filterValue
        spectrumValue[i] = constrain(spectrumValue[i], filterValue, 1023);
        // Remove serial stuff after debugging
//        Serial.print(spectrumValue[i]);
//        Serial.print(" ");
        digitalWrite(MSGEQ7_STROBE_PIN, HIGH);
    }
//    Serial.println();
}

void readBluetoothData(){
        while(Serial.available() > 0){ // Checks whether data is comming from the serial port
            char character;
            character = Serial.read(); // Reads the data from the serial port
            bluetoothData = bluetoothData + character;
            //Serial.println(bluetoothData);
        }
        //Serial.println(bluetoothData);
        if(bluetoothData.endsWith(resetCommand)){
            bluetoothData = "";
        }
}

void processBluetoothData(){
    for(int i=0; i<COMMAND_ACTION_ARRAY_SIZE; i++){
        CommandAction a = commandActions[i];
        if(bluetoothData.startsWith(a.bluetoothCommand)){
            if(a.expectsColor == true){
                if(!bluetoothData.endsWith("/")){ //if color command doesn't end with "/" skip iteration
                    continue;
                }
            }
            lastBluetoothCommand = bluetoothData;
            bluetoothData = "";
            Serial.println(lastBluetoothCommand);
            if(lastBluetoothCommand.length() > a.bluetoothCommand.length()){
                lastColor = getColorFromCommand(a.bluetoothCommand);
            }
            if(a.callOnce == true){
                a.animation_function_pointer();
            }
        }
        if (lastBluetoothCommand.startsWith(a.bluetoothCommand) && a.callOnce == false){
            if(a.needsAudioData == true){
                collectAudio();//collect MSGEQ7 data
            }
            a.animation_function_pointer();
        }
    }
}

uint32_t getColorFromCommand(String command){
    //incoming command contains not the color, but the lastBluetoothCommand does
    String c = lastBluetoothCommand.substring(command.length());
    char color[lastBluetoothCommand.length()+1];
    strcpy(color, c.c_str()); 
    char delimiter[] = ",/";
    char *ptr;
    int rgb[3];
    
    // initialisieren und ersten Abschnitt erstellen
    ptr = strtok(color, delimiter);

    //Serial.println(command);
    //Serial.println(c);
    //Serial.println(color);

    int i = 0;
    while(ptr != NULL && i<3) {
        rgb[i] = atoi(ptr);
        Serial.println(ptr);
        // naechsten Abschnitt erstellen
        ptr = strtok(NULL, delimiter);
        i++;
    }
    //Serial.println(rgb[0]);
    //Serial.println(rgb[1]);
    //Serial.println(rgb[2]);
    return strip.Color(rgb[0], rgb[1], rgb[2]);
}

void clearStrip(){
    fill_solid( leds, NUM_LEDS, CRGB(0, 0, 0));
    FastLED.show();
}
