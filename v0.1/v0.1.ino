volatile bool zeroCrossed = false;
unsigned long lastFiredTime = 0;
const short TRIAC_PIN = 7;
const short ZERO_CROSS_PIN = 2;
const short POT = A0;
unsigned long firingDuration = 0;  

// Krajnje vrijednosti za impuls motoru
const unsigned long int maxFiringDuration = 10; // 10ms za poluperiod AC na 50Hz
const unsigned long int minFiringDuration = 1;


void setup() {
    pinMode(TRIAC_PIN, OUTPUT);
    pinMode(ZERO_CROSS_PIN, INPUT);
    pinMode(POT, INPUT);

    // Zakači interrupt na zero-cross pin
    attachInterrupt(digitalPinToInterrupt(ZERO_CROSS_PIN), &zeroCrossRoutine, RISING);

    //Serial.begin(9600); // za debugging, ako zatreba

}

// Kad AC prođe kroz nulu, prebaci na true
void zeroCrossRoutine() {
  zeroCrossed = true;
}


// Otvori TRIAC onoliko dugo koliko treba
volatile void fireTRIACNonBlocking(unsigned long duration) {
  unsigned long currentMillis = millis();

  digitalWrite(TRIAC_PIN, HIGH);
  if(currentMillis - lastFiredTime >= duration) {
    digitalWrite(TRIAC_PIN, LOW);
    lastFiredTime = currentMillis;
  }
}

void adjustPower() {
  int potValue = analogRead(POT);
  int motorPower = map(potValue, 0, 1023, 0, 100);
  firingDuration = map(motorPower, 0, 100, minFiringDuration, maxFiringDuration);

  /*
  Serial.print("Motor Power: ");
  Serial.print(motorPower);
  Serial.print("%, Firing Duration: ");
  Serial.print(firingDuration);
  Serial.println(" ms");
  */

}

// Main loop
void loop() {
  adjustPower();

  if(zeroCrossed) {
    zeroCrossed = false;                    // Vrati na false za iduće okidanje
    fireTRIACNonBlocking(firingDuration);   // Otvori TRIAC za firingDuration milisekundi  
  }
}
