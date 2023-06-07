int pin_list[] = {2,3,4,5,6,7,8,9};

int segment_led_count = 8;
int in_0 = 10;
int in_1 = 11;
int in_2 = 12;
int data = 0;


int segment_led_list[10][8] = {
  {0, 0, 0, 0, 0, 0, 1, 1}, // 0
  {1, 0, 0, 1, 1, 1, 1, 1}, // 1
  {0, 0, 1, 0, 0, 1, 0, 1}, // 2
  {0, 0, 0, 0, 1, 1, 0, 1}, // 3
  {1, 0, 0, 1, 1, 0, 0, 1}, // 4
  {0, 1, 0, 0, 1, 0, 0, 1}, // 5
  {0, 1, 0, 0, 0, 0, 0, 1}, // 6
  {0, 0, 0, 1, 1, 1, 1, 1}, // 7
  {0, 0, 0, 0, 0, 0, 0, 1}, // 8
  {0, 0, 0, 0, 1, 0, 0, 1}  // 9
};


void setup() {
  
  Serial.begin(9600);
  // 세그먼트 각각 LED에 연결된 핀을 출력으로 설정
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  
  
  pinMode(in_0, INPUT);
  pinMode(in_1, INPUT);
  pinMode(in_2, INPUT);

  //INPUT핀도 설정할거임

}

void loop() {

  if(digitalRead(in_0) == HIGH){
    Serial.println("zero");
    data = 0;
  }
  else if(digitalRead(in_1) == HIGH){
    Serial.println("one");
    data = 1;
  }
  else if(digitalRead(in_2) == HIGH) {
    Serial.println("two");
    data = 2;
  }
  else{
    data = 0;
    Serial.println("nothing");
  }

 for (int j = 0; j < segment_led_count; j++) {
      digitalWrite(pin_list[j], segment_led_list[data][j]);     //data is receiving data
    }
  
}
  
