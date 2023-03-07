#define IR_L1 4
#define IR_L2 5
#define IR_R1 6
#define IR_R2 7
 
void setup() {
  Serial.begin(9600);
  pinMode(IR_L1, INPUT);
  pinMode(IR_L2, INPUT);
  pinMode(IR_R1, INPUT);
  pinMode(IR_R2, INPUT);
}
 
void loop() {
  delay(50);
  Serial.print(digitalRead(IR_L1));
  Serial.print(" ");
  Serial.print(digitalRead(IR_L2));
  Serial.print(" ");
  Serial.print(digitalRead(IR_R1));
  Serial.print(" ");
  Serial.print(digitalRead(IR_R2));
  Serial.println("");
}