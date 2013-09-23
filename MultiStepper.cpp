#include "Arduino.h"
#include "MultiStepper.h"


//these are used to determine what pins to set for each step
const uint8_t STEPS[] = {
  0b10,
  0b11,
  0b01,
  0b00
};

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
  this->last_step_time = 0;
  this->step_delay_correction = 0;

  for (uint8_t i = 0; i < 4; i++) {
    this->motor_step[i] = 0;
    this->motor_position[i] = 0;
  }
}

void MultiStepper::setStepsPerRevolution(int steps) {
  this->steps_per_revolution = steps;
}

void MultiStepper::setSpeed(int rpm) {
  this->step_delay = 60L * 1000L * 1000L / this->steps_per_revolution / (long)rpm;
}

// MOVEMENT FUNCTIONS
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
    port_mask |= STEPS[motor_step[motor]] << motor * 2;
  }
  advanceMotors(port_mask);
}

void MultiStepper::advanceMotors(volatile uint8_t port_mask) {
  if (step_delay + step_delay_correction > 0) {
    while (micros() - last_step_time <= step_delay + this->step_delay_correction) {};
  }
  *this->motor_port = port_mask & this->motor_mask;

  unsigned long current_step_time = micros();
  if (0 != this->last_step_time) {
    this->step_delay_correction += this->step_delay - (long)(current_step_time - this->last_step_time);
  }
  this->last_step_time = current_step_time;
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
  printArray("Current Pos", motor_position, 3);
  long vector[4] = {motor_1, motor_2, motor_3, motor_4};
  printArray("Vector", vector, 3);
  long total_steps[motor_count];

  long steps[motor_count];
  int direction[motor_count];
  long steps_remaining;
  long max_steps = 0;
  for (uint8_t motor = 0; motor < motor_count; motor++) {
    if (vector[motor] < 0) { direction[motor] = -1; }
    else if (vector[motor] > 0) { direction[motor] = 1; }
    else { vector[motor] = 0; }
    steps[motor] = abs(vector[motor]);
    if (steps[motor] > max_steps) {
      max_steps = steps[motor];
    }
    total_steps[motor] = steps[motor];
  }
  steps_remaining = max_steps;

  // An unknown amount of time has passed since the last step.  To not upset the 
  // timing, we reset this value, it is set again after the first step.
  this->last_step_time = 0;
  while (steps_remaining > 0) {
    int next_step[motor_count];
    for (uint8_t motor = 0; motor < motor_count; motor++) {
      if (0 == steps[motor]) {
        next_step[motor] = 0;
      }
      else if (0 != direction[motor]) {
        if (total_steps[motor] == max_steps) {
          next_step[motor] = direction[motor];
        }
        else {
          long error_moved = abs(total_steps[motor] - max_steps*(steps[motor] - 1) / (steps_remaining - 1));
          long error_not_moved = abs(total_steps[motor] - max_steps*steps[motor] / (steps_remaining - 1));
          if (error_moved <= error_not_moved) {
            next_step[motor] = direction[motor];
          }
          else {
            next_step[motor] = 0;
            continue;
          }
        }
        steps[motor]--;
      }
    }
    step(next_step);
    steps_remaining--;
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

//given n returns a mask where everybit less than (n-1) * 2 is set to one
uint8_t calculateMask(uint8_t n) {
  uint8_t mask = 0;
  for (n; n > 0; n--) {
    mask |= 0b11 << (2 * (n - 1));
  }
  return mask;
}

//for debugging currently
void MultiStepper::printArray(char *label, long array[], int length) {
  if (!this->printer) return;
  this->printer->print(label);
  this->printer->print(": ");
  for (uint8_t i = 0; i < length; i++) {
    if (0 < i) Serial.write(", ");
    this->printer->print(array[i]);
  }
  this->printer->write("\n");
}

void MultiStepper::printArray(char *label, int array[], int length) {
  long long_array[length];
  for (uint8_t i = 0; i < length; i++) {
    long_array[i]= (long)array[i];
  }
  printArray(label, long_array, length);
}

void MultiStepper::printArray(char *label, uint8_t array[], int length) {
  long long_array[length];
  for (uint8_t i = 0; i < length; i++) {
    long_array[i]= (long)array[i];
  }
  printArray(label, long_array, length);
}
