#include <HX710B.h>

// Motor control
volatile bool zeroCrossed = false;
unsigned long lastFiredTime = 0;
const short TRIAC_PIN = 7;
const short ZERO_CROSS_PIN = 2;
const short POT = A0;
unsigned long firingDuration = 0;

// Pressure sensor
const short SENS_OUT = 4;
const short SENS_CLOCK = 3;

HX710B pressureSensor;

void setup() {
  Serial.begin(9600);
  pinMode(TRIAC_PIN, OUTPUT);
  pinMode(ZERO_CROSS_PIN, INPUT);
  pinMode(POT, INPUT);

  // Zakači interrupt na zero-cross pin
  attachInterrupt(digitalPinToInterrupt(ZERO_CROSS_PIN), &zeroCrossRoutine, RISING);

  // Inicijaliziraj timer1 (16-bit) za preciznu kontrolu
  TCCR1A = 0;               // Normal mode
  TCCR1B = 0;               // Bez prescalera u startu
  TIMSK1 = (1 << OCIE1A);   // timer1 Interrupt Mask Register podešen da okine 
                            // interrupt kada timer1 dostigne vrijednost u OCIE1A registru

  pressureSensor.begin(SENS_OUT, SENS_CLOCK);
  pressureSensor.set_offset(-7252300);
}

void zeroCrossRoutine() {
  zeroCrossed = true;

  // Resetuj timer1 da broji od nule
  TCNT1 = 0;

  // Izračunaj delay na osnovu firingDuration (us)
  // Pretvoreno u tickove timera1 (1 tick = 1/16 µs na 16MHz)  
  unsigned long delayTime = firingDuration * 2; 

  // Postavi poredbenu vrijednost na osnovu firingDuration
  OCR1A = delayTime;  

  // Startaj timer1 sa prescalerom 8 za precizan delay (1 tick = 0.5 us)
  TCCR1B = (1 << WGM12) | (1 << CS11);  // CTC mode, prescaler = 8 
}

// Interrupt rutina za okidanje TRIAC-a
ISR(TIMER1_COMPA_vect) {
  
  // Okidanje TRIAC-a impulsom
  digitalWrite(TRIAC_PIN, HIGH);
  delayMicroseconds(100);
  digitalWrite(TRIAC_PIN, LOW);
  
  // Zaustavi timer1 do idućeg zero-crossa
  TCCR1B = 0;
}



void adjustPower() {
  //int potValue = analogRead(POT);
  int pressureReading = pressureSensor.pascal();
  firingDuration = map(pressureReading, 0, 6, 4200, 12000);
  //firingDuration = double(potValue / 1023.0) * 10000;
}

void loop() {
  adjustPower();
  Serial.println(pressureSensor.pascal());

  if(zeroCrossed)
    zeroCrossed = false;
}
