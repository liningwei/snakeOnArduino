#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

#define sw 8 // joystick input sw, x, y
#define Vx A0
#define Vy A1
#define alarmOut 12
#define alarmIn 11

#define FPS 5

int pixel[8][8];
int snake[8][8];
int face_direction;
int headX;
int headY;
int treatX;
int treatY;
int arrX[64], arrY[64];

int reach4 = 0;
int enabled = 1;

//in the snake matrix, 1 is body, 2 is head, 3 is treat, 0 is empty
//in direction, 1, 2, 3, 4 is up down left right

Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();

void setup() {
  Serial.begin(9600);
  Serial.println("8x8 LED Matrix Test");

  pinMode(sw, INPUT);
  pinMode(Vx, INPUT);
  pinMode(Vy, INPUT);
  pinMode(alarmOut, OUTPUT);

  reset();

  matrix.begin(0x70);  // pass in the address
  matrix.setBrightness(5);

  testSequence();

  enabled = 1;
}

void reset() {
  headX = 3;
  headY = 3;
  initializeArray();
  //snake[headX][headY] = 2;
  face_direction = 2;
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      pixel[i][j] = 0;
      snake[i][j] = 0;
    }
  }
}

void flash()
{
  matrix.clear();
  matrix.drawRect(0, 0, 8, 8, LED_RED);
  matrix.writeDisplay();
  delay(500);
  matrix.drawRect(0, 0, 8, 8, LED_RED);
  matrix.drawRect(1, 1, 6, 6, LED_YELLOW);
  matrix.writeDisplay();
  delay(500);
  matrix.drawRect(0, 0, 8, 8, LED_RED);
  matrix.drawRect(1, 1, 6, 6, LED_YELLOW);
  matrix.drawRect(2, 2, 4, 4, LED_GREEN);
  matrix.writeDisplay();
  delay(500);
  matrix.clear();


}
void loop() {
  readAlarm();
  if (!enabled) {
    if (!digitalRead(sw)) {
      enabled = 1;
      digitalWrite(alarmOut, LOW);
      reset();
      delay(1000);
      flash();
      return;
    }
    return;
  }





  matrix.clear();
  getDirection();
  if (!treatPresent())
  {
    generateTreat();
  }
  //getDirection();
  //delay(10);
  //getDirection();
  check4();
  if (checkLose())
  {

    if (!reach4)
    {
      reset();
      delay(1000);
      flash();
      return;
    }
    else {
      enabled = 0;
      return;
    }
  }

  updateHead();
  updateArray();
  displayFrame();



  matrix.writeDisplay();

  delay(1000 / FPS);



  //  matrix.drawBitmap(0, 0, smile_bmp, 8, 8, LED_GREEN);
  //  matrix.drawPixel(0, 0, LED_GREEN);
  //  matrix.writeDisplay();  // write the changes we just made to the display
  //  matrix.drawRect(0, 0, 8, 8, LED_RED);
  //  matrix.fillRect(2, 2, 4, 4, LED_GREEN);
  //  matrix.writeDisplay();  // write the changes we just made to the display
  //  matrix.drawCircle(3, 3, 3, LED_YELLOW);
  //  matrix.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  //  matrix.setTextSize(1);
  //  matrix.setTextColor(LED_GREEN);
  //  for (int8_t x = 7; x >= -36; x--) {
  //    matrix.clear();
  //    matrix.setCursor(x, 0);
  //    matrix.print("Hello");
  //    matrix.writeDisplay();
  //    delay(100);
  //  }
  //  matrix.setRotation(3);
  //  matrix.setTextColor(LED_RED);
  //  for (int8_t x = 7; x >= -36; x--) {
  //    matrix.clear();
  //    matrix.setCursor(x, 0);
  //    matrix.print("World");
  //    matrix.writeDisplay();
  //    delay(100);
  //  }
  //  matrix.setRotation(0);
}

void testSequence()
{
  matrix.clear();
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      matrix.clear();
      matrix.drawPixel(j, i, LED_GREEN);
      matrix.writeDisplay();
      delay(20);
    }

  }
  //Serial.println("Tested sequence");
}



boolean isOccupied(int x, int y)
{
  if (snake[x][y] != 0) return true;
  //Serial.println("Is occupied");
  return false;
}

void getDirection()
{
  int x_pos = map(analogRead(Vx), 0, 1023, 0, 7);
  int y_pos = map(analogRead(Vy), 0, 1023, 0, 7);

  if (y_pos >= 6 && face_direction != 3) face_direction = 1;
  else if (y_pos <= 1 && face_direction != 1) face_direction =  3;
  else if (x_pos <= 1 && face_direction != 2) face_direction =  4;
  else if (x_pos >= 6 && face_direction != 4) face_direction =  2;
  //Serial.println("\nGot direction:");
  //Serial.print(face_direction);
}

boolean treatPresent()
{
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      if (snake[i][j] == 3) return true;
    }
  }
  //Serial.println("Treat present");
  return false;
}

void generateTreat()
{
  treatX = random(0, 8);
  treatY = random(0, 8);
  while (isOccupied(treatX, treatY))
  {
    treatX = random(0, 8);
    treatY = random(0, 8);
  }
  snake[treatX][treatY] = 3;

  //Serial.println("Generated treat");
}


void displayFrame()
{
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      switch (snake[i][j]) {
        case 1://body is green
          matrix.drawPixel(i, j, LED_GREEN);
          break;
        case 2://head is yellow
          matrix.drawPixel(i, j, LED_YELLOW);
          break;
        case 3://treat is red
          matrix.drawPixel(i, j, LED_RED);
          break;
      }
    }
  }
  //Serial.println("Set the frame");
}

boolean checkLose()
{

  if (headX == 0 && face_direction == 4) return true;
  else if (headX == 7 && face_direction == 2) return true;
  else if (headY == 0 && face_direction == 1) return true;
  else if (headY == 7 && face_direction == 3) return true;


  if (snake[headX - 1][headY] == 1 && face_direction == 4) return true;
  else if (snake[headX + 1][headY] == 1 && face_direction == 2)return true;
  else if (snake[headX][headY - 1] == 1 && face_direction == 1)return true;
  else if (snake[headX][headY + 1] == 1 && face_direction == 3) return true;

  //Serial.println("Checked lose");
  return false;
}

void initializeArray() {
  arrX[0] = 3;
  arrY[0] = 3;
  for (int i = 1; i < 64; i++)
  {
    arrX[i] = -1;
    arrY[i] = -1;
  }
  //Serial.println("Initialized array");
}

void updateArray()
{
  int tempX[64];
  int tempY[64];
  for (int i = 1; i < 63; i++)
  {
    tempX[i] = arrX[i - 1];
    tempY[i] = arrY[i - 1];
  }
  tempX[0] = headX;
  tempY[0] = headY;

  for (int i = 0; i < 64; i++)
  {
    arrX[i] = tempX[i];
    arrY[i] = tempY[i];
    if (!treatEaten() && arrX[i] == -1 && arrY[i] == -1)
    {
      arrX[i - 1] = -1;
      arrY[i - 1] = -1;
    }
  }

  updateMatrix();
  //Serial.println();
  //Serial.println("Updated array first ele is");
  //Serial.print(arrX[0]);
  //Serial.print("  ");
  //Serial.print(arrY[0]);
  //Serial.println();
  //Serial.print(arrX[1]);
  //Serial.print("  ");
  //Serial.print(arrY[1]);
}

void updateHead()
{
  switch (face_direction) {
    case 1:
      if (headY >= 1) headY--;
      break;
    case 2:
      if (headX <= 6)headX++;
      break;
    case 3:
      if (headY <= 6)headY++;
      break;
    case 4:
      if (headX >= 1)headX--;
      break;
  }
  //Serial.println("Updated head");
}

void updateMatrix() {
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {

      snake[i][j] = 0;
    }
  }
  snake[treatX][treatY] = 3;
  snake[arrX[0]][arrY[0]] = 2;
  for (int i = 1; i < 64; i++) {
    if (arrX[i] != -1 && arrY[i] != -1)
    {
      snake[arrX[i]][arrY[i]] = 1;



    }

  }
}

boolean treatEaten() {
  //if (snake[headX - 1][headY] == 3 && face_direction == 4) return true;
  //else if (snake[headX + 1][headY] == 3 && face_direction == 2)return true;
  //else if (snake[headX][headY - 1] == 3 && face_direction == 1)return true;
  //else if (snake[headX][headY + 1] == 3 && face_direction == 3) return true;
  if (snake[headX][headY] == 3) return true;
  return false;

}

void check4()
{
  int len = 0;
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {

      if (snake[i][j] == 1) len++;
    }
  }
  if (len >= 4)
  {
    reach4 = 1;
    digitalWrite(alarmOut, HIGH);
  }
  else reach4 = 0;
}

void readAlarm()
{
  int alarm = digitalRead(alarmIn);
  if (alarm &&digitalRead(alarmOut))
  {
    digitalWrite(alarmOut, LOW);
    reset();
    delay(1000);
    flash();
    enabled =1;

  }

}

