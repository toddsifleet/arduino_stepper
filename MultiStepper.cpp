#include "Arduino.h"
#include "MultiStepper.h"


//these are used to determine what pins to set for each step
const uint8_t steps[] = {
  0b10,
  0b11,
  0b01,
  0b00
};

MultiStepper::MultiStepper(
  volatile uint8_t *motor_port,
  volatile uint8_t *motor_port_ddr,
  uint8_t motor_mask,
  int steps_per_revolution ) 
{

  //init motor state
  for (uint8_t i = 0; i < 4; i++) {
    this->motor_step[i] = 0;
    this->motor_position[i] = 0;
  }

  initMotor(motor_port, motor_port_ddr, motor_mask, steps_per_revolution);

  //by default we assume no limit switches
  this->has_limit = false;

  this->printer = NULL;
}

MultiStepper::MultiStepper(
  volatile uint8_t *motor_port,
  volatile uint8_t *motor_port_ddr,
  uint8_t motor_mask,
  volatile uint8_t *limit_port,
  volatile uint8_t *limit_port_ddr,
  uint8_t limit_mask,
  int steps_per_revolution ) 
{

  MultiStepper(motor_port, motor_port_ddr, motor_mask, steps_per_revolution);
  initLimit(limit_port, limit_port_ddr, limit_mask);
}

void MultiStepper::printArray(char *label, int array[], int length) {
  if (!this->printer) return;
  this->printer->print(label);
  for (uint8_t i = 0; i < length; i++) {
    if (0 < i) Serial.write(", ");
    this->printer->print(array[i]);
  }
  this->printer->write("\n");
}

void MultiStepper::initLimit(
  volatile uint8_t *port,
  volatile uint8_t *ddr,
  uint8_t mask) 
{
  this->has_limit = true;
  this->limit_port = port;
  *ddr |= mask;
  this->limit_mask = mask;
}

void MultiStepper::initMotor(
  volatile uint8_t *port,
  volatile uint8_t *ddr,
  uint8_t mask,
  int steps_per_revolution ) 
{
  this->motor_mask = mask;
  *ddr |= mask;
  this->steps_per_revolution = steps_per_revolution;
  this->motor_port = port;

  bool found_first = false;
  for (uint8_t i = 0; i < 4; i++) {
    if (this->motor_mask & (1 << (2 * i))) {
      if (found_first) {
        found_first = true;
        this->first_motor = i;
      }
      this->last_motor = i;
    }
  }
}

void MultiStepper::step(uint8_t direction) {
  volatile uint8_t mask = 0;
  for (uint8_t motor = this->first_motor; motor <= this->last_motor; motor++) {

    uint8_t bit_shift = 2 * motor;
    if (direction & 1 << bit_shift) {
      //move forward
      incrementMotorCounters(motor);
    }
    else if (direction & 0b10 << bit_shift) {
      //move backwards
      decrementMotorCounters(motor);
    }
    mask |= steps[this->motor_step[motor] % 4] << bit_shift;
  }

  *this->motor_port = mask & this->motor_mask;
}

void MultiStepper::decrementMotorCounters(int motor) {
  int *motor_step = &this->motor_step[motor];
  if (0 == *motor_step) {
    *motor_step = this->steps_per_revolution;
  }
  (*motor_step)--;
}

void MultiStepper::incrementMotorCounters(int motor) {
  int *motor_step = &this->motor_step[motor];
  (*motor_step)++;
  if (*motor_step == this->steps_per_revolution) {
    *motor_step = 0;
  }
}

void MultiStepper::setPrinter (Print & p) {
  this->printer = &p;
}
