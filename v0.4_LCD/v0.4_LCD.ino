#include <HX710B.h>
#include <PID_v1.h>
#include <LiquidCrystal_I2C.h>

// Motor control
volatile bool zeroCrossed = false;
unsigned long lastFiredTime = 0;
const short TRIAC_PIN = 7;
const short ZERO_CROSS_PIN = 2;
const short POT = A0;
unsigned long firingDuration = 0;
unsigned short motorPower = 0;

// Senzor pritiska
const short SENS_OUT = 4;
const short SENS_CLOCK = 3;
HX710B pressureSensor;

// PID varijable
double PIDsetPoint, PIDinput, PIDoutput;
double Kp = 30.0, Ki = 100.0, Kd = 100.0;
PID myPID(&PIDinput, &PIDoutput, &PIDsetPoint, Kp, Ki, Kd, REVERSE);

// LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  //Serial.begin(9600);
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
  pressureSensor.set_offset(-7222000);

  PIDsetPoint = 5;
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(4300, 11000);

  lcd.init();
  lcd.backlight();
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
  delayMicroseconds(10);
  digitalWrite(TRIAC_PIN, LOW);
  
  // Zaustavi timer1 do idućeg zero-crossa
  TCCR1B = 0;
}

void adjustPower() {
  //int potValue = analogRead(POT);
  //int pressureReading = pressureSensor.pascal();
  //firingDuration = map(pressureReading, 0, 6, 4200, 12000);
  //firingDuration = double(potValue / 1023.0) * 10000;
  PIDsetPoint = map(analogRead(POT), 0, 1023, 1, 10);
  PIDinput = pressureSensor.pascal();
  myPID.Compute();
  firingDuration = PIDoutput;
  motorPower = map(firingDuration, 4300, 11000, 100, 0);
}

volatile void lcdRefresh() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Setpoint: ");
  lcd.print(PIDsetPoint);
  lcd.print(" Pa");
  lcd.setCursor(0, 1);
  lcd.print("Pressure: ");
  lcd.print(PIDinput);
  lcd.print(" Pa");
  lcd.setCursor(0, 2);
  lcd.print("Fan Speed: ");
  lcd.print(motorPower);
  lcd.print(" %");
}

void loop() {
  adjustPower();
  lcdRefresh();
  if(zeroCrossed)
    zeroCrossed = false;
}
