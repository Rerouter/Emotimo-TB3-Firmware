void init_external_triggering()
{
  pinMode(IO_3, INPUT);
  digitalWrite(IO_3, HIGH);
  attachInterrupt(1, cam_change, CHANGE);
}


void cam_change()
{
  changehappened = true;
  state = digitalRead(3);
  if (DEBUG) Serial.print("i");

}
