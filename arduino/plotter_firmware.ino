#include <Servo.h>

#define PENSERVOPIN 6
#define YMICROSW    9
#define XMICROSW    8
#define YDIR        5
#define YSTEP       4
#define XDIR        3
#define XSTEP       2
#define STEPDEALY   1

#define PENDELAY    7
#define PENUP       190
#define PENDOWN     30

#define MM_STEP 0.005

enum com {RESET, FOUND_G, FOUND_M, G_MOVE, FOUND_X, FOUND_Y, FOUND_Z};

typedef struct {
  double x, y;
}vec2D;

Servo penServo; 
vec2D currPos;

void setup() {
  currPos.x = 0;
  currPos.y = 0;

  penServo.attach(PENSERVOPIN);
  
  pinMode (YMICROSW, INPUT_PULLUP);
  pinMode (XMICROSW, INPUT_PULLUP);
  pinMode (YSTEP, OUTPUT);
  pinMode (YDIR, OUTPUT);
  pinMode (XDIR, OUTPUT);
  pinMode (XSTEP, OUTPUT);
  
  Serial.begin(9600);
}

void steps (int pin, int n) {
  while(n--) {
    digitalWrite(pin,HIGH);
    delay(STEPDEALY);
    digitalWrite(pin, LOW);
    delay(STEPDEALY);
  }
}

void reset() {
  Serial.print("re\n");
  goTo(PENUP);
  digitalWrite(XDIR, LOW);
  while(digitalRead(XMICROSW)) {
    steps(XSTEP, 200);          
  }
  digitalWrite(YDIR, LOW);
  while(digitalRead(YMICROSW)) {
    steps(YSTEP, 200);          
  }
}

void move (vec2D newPos) {

  newPos.x = fabs(newPos.x);
  newPos.y = fabs(newPos.y);
  
  vec2D move;
  move.x = (newPos.x-currPos.x);
  move.y = (newPos.y-currPos.y);
  

  if(move.x>0)
    digitalWrite(XDIR, HIGH);
  else
    digitalWrite(XDIR, LOW);

  if(move.y>0)
    digitalWrite(YDIR, HIGH);
  else
    digitalWrite(YDIR, LOW);


  long xsteps = abs(move.x/MM_STEP);
  long ysteps = abs(move.y/MM_STEP);
/*
  Serial.println((String)"Muovo in: " + newPos.x + " " + newPos.y);
  Serial.println((String)"Spostamento: " + move.x + " " + move.y);
  Serial.println((String)"Steps: " + xsteps + " " + ysteps + "\n");
*/
  long tmp = 0;

  if(xsteps>ysteps) {
    for(int i=0; i<xsteps; i++) {
      steps(XSTEP, 1);
      tmp+=ysteps;
      if(tmp>=xsteps) {
        tmp-=xsteps;
        steps(YSTEP, 1);
      }
    }
  } else {
    for(int i=0; i<ysteps; i++) {
      steps(YSTEP, 1);
      tmp+=xsteps;
      if(tmp>=ysteps) {
        tmp-=ysteps;
        steps(XSTEP, 1);
      }
    }
  }

  currPos = newPos;
}

int pos = 0;
void goTo(int endAngle) {
  int step = (endAngle > pos) ? 1 : -1;
  for (; pos != endAngle; pos += step) {
    penServo.write(pos);
    delay(PENDELAY);
  }
}

void sendOk() {
  Serial.print("ok\n");
}

void clearInput() {
  //Serial.println("CLEAR_IN");
  /*
  while(Serial.available() > 0) {
    char t = Serial.read();
  }
  */
  byte w = 0;

for (int i = 0; i < 10; i++)
{
while (Serial.available() > 0)
{
char k = Serial.read();
w++;
delay(1);
}
delay(1);
}
}

void loop() {
  
  bool done = false;
  bool posAbsolute = true;

  String num;

  com c = RESET;

  int opNumber;

  int numDecimals = 0;
  int sign = 1;

  vec2D nextPosition = {0, 0};

  reset();
  
  while(!done) {
    if(Serial.available()>0) { 
      char ch = Serial.read();
      switch(c) {
        case RESET:
          //Serial.print("RESET ");
          //Serial.println(ch);
          opNumber = 0;
          if(ch=='G') {
            c = FOUND_G;
          } else if(ch=='M') {
            c = FOUND_M;
          }
          break;
  
        case FOUND_G:
          //Serial.print("FOUND_G ");
          //Serial.println(ch);
          if(isDigit(ch)) {
            opNumber = opNumber*10 + (ch-'0');
          }
          else {
            if(opNumber==90) {
              posAbsolute = true;
              sendOk();
              c = RESET;
            } else if(opNumber==1 || opNumber==2) {
              c = G_MOVE;
            } else {
              sendOk();
              c=RESET;
            }
          }
          break;
  
        case FOUND_M:
          //Serial.print("FOUND_M ");
          //Serial.println(ch);
          if(isDigit(ch))
            opNumber = opNumber*10 + (ch-'0');
          else {
            if(opNumber==2) {
              sendOk();
              reset();
            } else {
              reset();
              done = true;
            }
          }
          break;
  
        case G_MOVE:
          //Serial.print("G_MOVE ");
          //Serial.println(ch);
          num = "";
          if(ch=='X') {
            c = FOUND_X;
          } else if(ch=='Y') {
            c = FOUND_Y;
          } else if(ch=='Z') {
            c = FOUND_Z;
          }
          break;
  
        case FOUND_X:
          //Serial.print("FOUND_X ");
          //Serial.println(ch);
          if(isDigit(ch) || ch=='-' || ch=='.') {
            num+=ch;
          } else if(ch==' ') {
            nextPosition.x = atof(num.c_str());
            c = G_MOVE;
          } else {
            reset();
            done = true;
          }
          break;
  
        case FOUND_Y:
          //Serial.print("FOUND_Y ");
          //Serial.println(ch);
          if(isDigit(ch) || ch=='-' || ch=='.') {
            num+=ch;
          } else if(ch==' ' || ch=='\n' || ch=='\r' || ch==';') {
            clearInput();
            nextPosition.y = atof(num.c_str());
            move(nextPosition);
            sendOk();
            c = RESET;
          } else {
            reset();
            done = true;
          }
          break;
  
        case FOUND_Z:
          //Serial.print("FOUND_Z ");
          //Serial.println(ch);
          if(isDigit(ch)) {
            clearInput();
            goTo(PENUP);
          } else if(ch=='-') {
            clearInput();
            goTo(PENDOWN);
          } else {
            reset();
            done = true;
          }
          sendOk();
          c = RESET;
          break;
      }
    }
  }
  //Serial.println("END");
  while(1)
    delay(10000);

}
