#include "Arduino.h"
#include "MultiStepper.h"


//these are used to determine what pins to set for each step
const uint8_t steps[] = {
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

  for (uint8_t i = 0; i < 4; i++) {
    this->motor_step[i] = 0;
    this->motor_position[i] = 0;
  }
}

void MultiStepper::setStepsPerRevolution(int steps) {
  this->steps_per_revolution = steps;
}

//MOVEMENT FUNCTIONS
void MultiStepper::step(uint8_t direction) {
  volatile uint8_t port_mask = 0;
  for (uint8_t motor = 0; motor < this->motor_count; motor++) {
    uint8_t bit_mask = 1 << 2 * motor;
    if (direction & bit_mask) {
      if (!this->has_limit || !(*this->limit_port & bit_mask)) {
        //move forward
        incrementMotorCounters(motor);
      }
    }
    else if (direction & bit_mask << 1) {
      if (!this->has_limit || !(*this->limit_port & bit_mask << 1)) {
        //move backwards
        decrementMotorCounters(motor);
      }
    }
    port_mask |= steps[this->motor_step[motor] % 4] << motor * 2;
  }

  *this->motor_port = port_mask & this->motor_mask;
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
  for (uint8_t i = 0; i < length; i++) {
    if (0 < i) Serial.write(", ");
    this->printer->print(array[i]);
  }
  this->printer->write("\n");
}
