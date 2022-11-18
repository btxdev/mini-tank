void setup()
{
  Serial.begin(115200);
  pinMode(4, INPUT);
}


void loop()
{
  Serial.println(analogRead(4));

  delay(500);
}