
void setup() 
{ 
  Serial.begin(115200);
  Serial1.begin(9600);

} 
 
void loop() 
{ 
  if (Serial1.available()) {
    int inByte = Serial1.read();
    Serial.write(inByte);
  }
}