
int randNumber = 0;     //임의로 변수 설정
int interruptPin = 2;
volatile byte state = LOW;

void setup() {
  attachInterrupt(digitalPinToInterrupt(interruptPin), make_random, CHANGE);
  pinMode(interruptPin, INPUT);
  Serial.begin(9600);

}

void loop() {

  Serial.write(randNumber);
  delay(100);
  
}

void make_random() {
  state = !state;
  randNumber = random(3);
  delayMicroseconds(50);
}
