// This SoftwareServo library example sketch was initially delivered without any comments.
// Below my own comments for SoftRcPulseOut library: by RC Navy (http://p.loussouarn.free.fr)
// Controlling the position of 2 servos using the Arduino built-in hardware UART (Arduino Serial object).
// This sketch do NOT work with an ATtinyX4 and ATtinyX5 since they do not have a built-in harware UART (no Arduino Serial object).
// orange = PWM
// brown = gnd
// Red = vcc

#include <Servo.h> 

Servo rightFHip;
Servo rightFKnee;
Servo rightRHip;

Servo leftFHip;
Servo leftFKnee;
Servo leftRHip;

#define QUADRANT_FRAMES 20

#define RIGHT_F_HIP_PIN 22
#define RIGHT_F_KNEE_PIN 23
#define RIGHT_R_HIP_PIN 24

#define LEFT_F_HIP_PIN 32
#define LEFT_F_KNEE_PIN 33
#define LEFT_R_HIP_PIN 34

#define rightFHipCenter 100
#define rightFKneeCenter 140
#define rightRHipCenter 60

#define leftFHipCenter 100
#define leftFKneeCenter 130
#define leftRHipCenter 100


struct TurtleModel {
  int rightFrontHip;
  int rightFrontKnee;
  int rightHindHip;
  int leftFrontHip;
  int leftFrontKnee;
  int leftHindHip;
};

struct AnimationFrame {
  TurtleModel pose;
  int duration;
};


struct Movement {
  TurtleModel startPose;
  TurtleModel endPose;
  unsigned long startTime;
  unsigned long endTime;
};

struct AnimationData {
  AnimationFrame *animation=NULL;
  int numAnimationFrames=0;
  int currFrame;
  Movement currMovement;
  int repetitions;
  int performedRepetitions;
  float animationSpeed;
};

AnimationData currAnimation;
TurtleModel currPose;

void sendPose(struct TurtleModel &pose) {
  rightFHip.write(rightFHipCenter + pose.rightFrontHip);
  rightFKnee.write(rightFKneeCenter + pose.rightFrontKnee);
  rightRHip.write(rightRHipCenter + pose.rightHindHip);

  leftFHip.write(leftFHipCenter - pose.leftFrontHip);
  leftFKnee.write(leftFKneeCenter - pose.leftFrontKnee);
  leftRHip.write(leftRHipCenter - pose.leftHindHip);
}

void updateMovement(struct Movement &movement) {
  unsigned long currTime=millis();  
  if (currTime >= movement.endTime) {
    currPose = movement.endPose;
  } else if (currTime <= movement.startTime) {
    currPose = movement.startPose;
  } else {
    float t = ((float)(currTime-movement.startTime)) / (movement.endTime-movement.startTime);
//    Serial.print(t);
//    Serial.print("\n");

    currPose.rightFrontHip = movement.endPose.rightFrontHip*t + movement.startPose.rightFrontHip*(1-t);
    currPose.rightFrontKnee = movement.endPose.rightFrontKnee*t + movement.startPose.rightFrontKnee*(1-t);
    currPose.rightHindHip = movement.endPose.rightHindHip*t + movement.startPose.rightHindHip*(1-t);
    currPose.leftFrontHip = movement.endPose.leftFrontHip*t + movement.startPose.leftFrontHip*(1-t);
    currPose.leftFrontKnee = movement.endPose.leftFrontKnee*t + movement.startPose.leftFrontKnee*(1-t);
    currPose.leftHindHip = movement.endPose.leftHindHip*t + movement.startPose.leftHindHip*(1-t);
  }
  sendPose(currPose);
}


void updateAnimation() {
  if (currAnimation.numAnimationFrames >0) {
    updateMovement(currAnimation.currMovement);
  }
  unsigned long currTime=millis();  
  if (currTime > currAnimation.currMovement.endTime) {
    if (currAnimation.repetitions == 0 || currAnimation.performedRepetitions < currAnimation.repetitions) {
      int nextFrame = currAnimation.currFrame + 1;
      if (nextFrame >= currAnimation.numAnimationFrames) {
        currAnimation.performedRepetitions++;
        if (currAnimation.repetitions >0 && currAnimation.performedRepetitions == currAnimation.repetitions) {
          return;
        }
        nextFrame = 0;
      }
      currAnimation.currMovement.startPose = currAnimation.animation[currAnimation.currFrame].pose;
      currAnimation.currMovement.endPose = currAnimation.animation[nextFrame].pose;
      currAnimation.currMovement.startTime = currAnimation.currMovement.endTime;
      Serial.print(nextFrame);
      Serial.print(": ");
      Serial.print(currAnimation.currMovement.endTime);
      Serial.print(": ");

      
      currAnimation.currMovement.endTime += currAnimation.animation[nextFrame].duration*currAnimation.animationSpeed;
      currAnimation.currFrame = nextFrame;
      Serial.print(currAnimation.animation[nextFrame].duration);
      
      Serial.print(": ");
      Serial.print(currTime);
      Serial.print(": ");
      Serial.print(currAnimation.currMovement.endTime);
      Serial.print("\n");
    }

  }
}

bool doneAnimation() {
  bool res=false;
  if (currAnimation.numAnimationFrames == 0) {
    res = true;
  } else if (currAnimation.repetitions > 0 && currAnimation.performedRepetitions >= currAnimation.repetitions) {
    res = true;
  }
  return res;
}
void startAnimation(struct AnimationFrame *animation, int numAnimationFrames, float animationSpeed, int repetitions) {
  currAnimation.animation=animation;
  currAnimation.numAnimationFrames=numAnimationFrames;
  currAnimation.currFrame=0;
  currAnimation.repetitions = repetitions;
  currAnimation.animationSpeed = animationSpeed;
  
  currAnimation.currMovement.startPose = currPose;
  currAnimation.currMovement.endPose = currAnimation.animation[0].pose;
  currAnimation.currMovement.startTime = millis();
  currAnimation.currMovement.endTime = currAnimation.currMovement.startTime + currAnimation.animation[0].duration;
}

void setupTurtle() {
    pinMode(RIGHT_F_HIP_PIN,OUTPUT);
  rightFHip.attach(RIGHT_F_HIP_PIN);

  pinMode(RIGHT_F_KNEE_PIN,OUTPUT);
  rightFKnee.attach(RIGHT_F_KNEE_PIN);

  pinMode(RIGHT_R_HIP_PIN,OUTPUT);
  rightRHip.attach(RIGHT_R_HIP_PIN);

  pinMode(LEFT_F_HIP_PIN,OUTPUT);
  leftFHip.attach(LEFT_F_HIP_PIN);

  pinMode(LEFT_F_KNEE_PIN,OUTPUT);
  leftFKnee.attach(LEFT_F_KNEE_PIN);

  pinMode(LEFT_R_HIP_PIN,OUTPUT);
  leftRHip.attach(LEFT_R_HIP_PIN);
}


/*
 * Start of application specific stuff
 */
int fKneeRotation = 60;
int fHipRotation = 40;
int rHipRotation = 20;

TurtleModel walkingPose0 = {
  .rightFrontHip = 0,
  .rightFrontKnee = fKneeRotation,
  .rightHindHip = -rHipRotation,
  .leftFrontHip = 0,
  .leftFrontKnee = -fKneeRotation,
  .leftHindHip = rHipRotation,
};

TurtleModel walkingPose1 = {
  .rightFrontHip = fHipRotation,
  .rightFrontKnee = 0,
  .rightHindHip = 0,
  .leftFrontHip = -fHipRotation,
  .leftFrontKnee = 0,
  .leftHindHip = 0,
};

TurtleModel walkingPose2 = {
  .rightFrontHip = walkingPose0.leftFrontHip,
  .rightFrontKnee = walkingPose0.leftFrontKnee,
  .rightHindHip = walkingPose0.leftHindHip,
  .leftFrontHip = walkingPose0.rightFrontHip,
  .leftFrontKnee = walkingPose0.rightFrontKnee,
  .leftHindHip = walkingPose0.rightHindHip,
};

TurtleModel walkingPose3 = {
  .rightFrontHip = walkingPose1.leftFrontHip,
  .rightFrontKnee = walkingPose1.leftFrontKnee,
  .rightHindHip = walkingPose1.leftHindHip,
  .leftFrontHip = walkingPose1.rightFrontHip,
  .leftFrontKnee = walkingPose1.rightFrontKnee,
  .leftHindHip = walkingPose1.rightHindHip,
};

TurtleModel straight = {
  .rightFrontHip = 0,
  .rightFrontKnee = 0,
  .rightHindHip = 0,
  .leftFrontHip = 0,
  .leftFrontKnee = 0,
  .leftHindHip = 0,
};

TurtleModel hoyre = {
  .rightFrontHip = 0,
  .rightFrontKnee = 0,
  .rightHindHip = -30,
  .leftFrontHip = 0,
  .leftFrontKnee = 0,
  .leftHindHip = 0,
};


TurtleModel venstre = {
  .rightFrontHip = 0,
  .rightFrontKnee = 0,
  .rightHindHip = 0,
  .leftFrontHip = 0,
  .leftFrontKnee = 0,
  .leftHindHip = -30,
};

AnimationFrame play[] = { 
  {
    .pose=straight, 
    .duration = 1000
  },
  {
    .pose=hoyre, 
    .duration = 1000
  },
  {
    .pose=straight, 
    .duration = 1000
  },
  {
    .pose=venstre, 
    .duration = 1000
  }
};



AnimationFrame walking[] = { 
  {
    .pose=walkingPose0, 
    .duration = 100
  },
  {
    .pose=walkingPose1, 
    .duration = 100
  },
  {
    .pose=walkingPose2, 
    .duration = 100
  },
  {
    .pose=walkingPose3, 
    .duration = 100
  }
};

void startSingleStep() {
  Serial.print("startSingleStep()\n");
  startAnimation(walking, 4, 50.0, 1);
}

void startWalking() {
  Serial.print("startWalking()\n");
  startAnimation(walking, 4, 10.0, 0);
}

void startPlay() {
  Serial.print("startPlay()\n");
  startAnimation(play, 4, 2.0, 1);
}


void stopMovement() {
  Serial.print("startSingleStep()\n");
  startAnimation(play, 1, 2.0, 1);
}


unsigned long lastTime;
enum LEG_MOVEMENT {STAND, LIE_DOWN, RAISE, WALK, REST, DANCE, SINGLESTEP, PLAY};
LEG_MOVEMENT legMovement = STAND;


void setup()
{
  Serial.begin(9600);
  Serial.print("Ready");
  setupTurtle();
  startPlay();
//  startWalking();
  legMovement = PLAY;
  lastTime=millis();
}


void loop()
{
  if (legMovement == PLAY && doneAnimation()){
    legMovement = STAND;
    stopMovement();
  }
  unsigned long currTime = millis();
  if (false && currTime-lastTime>10000) {
    lastTime = currTime;
    if (legMovement == STAND) {
      startSingleStep();
      legMovement = SINGLESTEP;
    } else if (legMovement == SINGLESTEP) {
      startWalking();
      legMovement = WALK;
    } else if (legMovement == WALK) {
      stopMovement();
      legMovement = STAND;
    }
  }
  updateAnimation();
}
