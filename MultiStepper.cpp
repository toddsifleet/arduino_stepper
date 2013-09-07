#include "Arduino.h"
#include "MultiStepper.h"


//these are used to determine what pins to set for each step
const uint8_t STEPS[] = {
  0b10,
  0b11,
  0b01,
  0b00
};

long HOME[] = {0, 0, 0, 0};

//CONSTRUCTOR FUNCTIONS
MultiStepper::MultiStepper(
  volatile uint8_t *motor_port,
  volatile uint8_t *motor_port_ddr,
  uint8_t motor_count,
  int steps_per_revolution )
{

  initMotors(motor_port, motor_port_ddr, motor_count, steps_per_revolution);
  setNoLimits();
}

MultiStepper::MultiStepper(
  volatile uint8_t *motor_port,
  volatile uint8_t *motor_port_ddr,
  uint8_t motor_count,
  volatile uint8_t *limit_port,
  volatile uint8_t *limit_port_ddr,
  int steps_per_revolution )
{

  initMotors(motor_port, motor_port_ddr, motor_count, steps_per_revolution);
  setLimits(limit_port, limit_port_ddr);
}

// CONFIGURATION FUNCTIONS
void MultiStepper::setPrinter (Print & p) {
  this->printer = &p;
}

void MultiStepper::setNoLimits(){
  this->limit_port = NULL;
  this->has_limit = false;
}

void MultiStepper::setLimits(
  volatile uint8_t *port,
  volatile uint8_t *ddr)
{
  this->has_limit = true;
  this->limit_port = port;
  *ddr = 0;
}

void MultiStepper::initMotors(
  volatile uint8_t *port,
  volatile uint8_t *ddr,
  uint8_t motor_count,
  int steps_per_revolution)
{
  this->motor_count = motor_count;
  this->motor_mask = calculateMask(motor_count);
  *ddr |= this->motor_mask;

  setStepsPerRevolution(steps_per_revolution);
  this->motor_port = port;
  this->last_step_time = micros();

  for (uint8_t i = 0; i < 4; i++) {
    this->motor_step[i] = 0;
    this->motor_position[i] = 0;
  }
}

void MultiStepper::setStepsPerRevolution(int steps) {
  this->steps_per_revolution = steps;
}

void MultiStepper::setSpeed(long rpm) {
  this->step_delay = 60L * 1000L * 1000L / this->steps_per_revolution / rpm;
}

//MOVEMENT FUNCTIONS
void MultiStepper::step(int direction[]) {
  volatile uint8_t port_mask = 0;
  for (uint8_t motor = 0; motor < motor_count; motor++) {
    uint8_t bit_mask = 1 << 2 * motor;
    if (1 == direction[motor] && !(has_limit && *limit_port & bit_mask)) {
      //move forward
      incrementMotorCounters(motor);
    }
    else if (-1 == direction[motor] && !(has_limit && *limit_port & bit_mask << 1)) {
      //move backwards
      decrementMotorCounters(motor);
    }
    port_mask |= STEPS[motor_step[motor] % 4] << motor * 2;
  }

  *this->motor_port = port_mask & this->motor_mask;
  this->last_step_time = micros();
}

void MultiStepper::goTo(long motor_1, long motor_2, long motor_3, long motor_4) {
  move(
    motor_1 - motor_position[0],
    motor_2 - motor_position[1],
    motor_3 - motor_position[2],
    motor_4 - motor_position[3]
  );
}

void MultiStepper::goHome() {
  goTo();
}

void MultiStepper::move(long motor_1, long motor_2, long motor_3, long motor_4) {
  long vector[4] = {motor_1, motor_2, motor_3, motor_4};
  long steps[motor_count];
  int direction[motor_count];
  long steps_remaining = 0;
  for (uint8_t motor = 0; motor < motor_count; motor++) {
    if (vector[motor] < 0) { direction[motor] = -1; }
    else if (vector[motor] > 0) { direction[motor] = 1; }
    else { vector[motor] = 0; }
    steps[motor] = abs(vector[motor]);
    if (steps[motor] > steps_remaining) {
      steps_remaining = steps[motor];
    }
  }
  while (steps_remaining > 0) {
    if (micros() - last_step_time >= step_delay) {
      for (uint8_t i = 0; i < motor_count; i++) {
        if (0 == steps[i]) {
          direction[i] = 0;
        }
        else if (0 != direction[i]) {
          steps[i]--;
        }
      }
      step(direction);
      steps_remaining--;
    }
  }
}

//UPDATE STATE FUNCTIONS
void MultiStepper::setHome() {
  for (uint8_t i = 0; i < 4; i++) {
    this->motor_position[i] = 0;
  }
}

void MultiStepper::decrementMotorCounters(int motor) {
  this->motor_position[motor]--;
  if (! this->motor_step[motor]--) {
    this->motor_step[motor] = 3;
  }
}

void MultiStepper::incrementMotorCounters(int motor) {
  this->motor_position[motor]++;
  if (4 == ++this->motor_step[motor]){
    this->motor_step[motor] = 0;
  }
}


//UTILITY FUNCTIONS

//given a returns a mask where everybit less than (n-1) * 2 is set to one
uint8_t MultiStepper::calculateMask(uint8_t n) {
  uint8_t mask = 0;
  for (n; n > 0; n--) {
    mask |= 0b11 << (2 * (n - 1));
  }
  return mask;
}

//for debugging currently
void MultiStepper::printArray(char *label, int array[], int length) {
  if (!this->printer) return;
  this->printer->print(label);
  this->printer->print(": ");
  for (uint8_t i = 0; i < length; i++) {
    if (0 < i) Serial.write(", ");
    this->printer->print(array[i]);
  }
  this->printer->write("\n");
}
