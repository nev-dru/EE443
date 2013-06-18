/* touchTest.ino
   David Hurlbut
   Andrew Neville
   EE 443
   June 12, 2013
   
   Reads horizontal and vertical coordinates
   from touchscreen and writes them to serial port
*/

#define MAXVAL 1023

int y1 = A0;
int x2 = A1;
int y2 = A2;
int x1 = A3;


int newX, oldX;
int newY, oldY;

void setup()
{
  newX = 0;
  newY = 0;
  oldX = 0;
  oldY = 0;
  Serial.begin(9600);
}

int readX()
{
  pinMode(y1, INPUT);
  pinMode(x2, OUTPUT);
  pinMode(y2, INPUT);
  pinMode(x1, OUTPUT);
  digitalWrite(x2, LOW);
  digitalWrite(x1, HIGH);
  delay(5);
  return analogRead(y1);
}

int readY()
{
  pinMode(y1, OUTPUT);
  pinMode(x2, INPUT);
  pinMode(y2, OUTPUT);
  pinMode(x1, INPUT);
  digitalWrite(y1, LOW);
  digitalWrite(y2, HIGH);
  delay(5);
  return analogRead(x1);
}

void loop()
{
  newX = readX();
  newY = readY();

  // place limits on x
  if (newX < 0) newX = 0;
  else if (newX >= MAXVAL) newX = oldX;
  else oldX = newX;

  // place limits on y
  if (newY < 0) newY = 0;
  else if (newY >= MAXVAL) newY = oldY;
  else oldY = newY;

  Serial.print(MAXVAL-newX);
  Serial.print(" ");
  Serial.print(newY);
  Serial.print("\n");

  delay(100);
}

