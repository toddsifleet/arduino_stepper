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

  initMotor(motor_port, motor_port_ddr, motor_mask, steps_per_revolution);

  //by default we assume no limit switches
  this->has_limit = false;
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

  initMotor(motor_port, motor_port_ddr, motor_mask, steps_per_revolution);
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
  *ddr = 0;
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


  //find first and last motor
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

  //init motor state
  for (uint8_t i = 0; i < 4; i++) {
    this->motor_step[i] = 0;
    this->motor_position[i] = 0;
  }
}

void MultiStepper::step(uint8_t direction) {
  volatile uint8_t port_mask = 0;
  for (uint8_t motor = this->first_motor; motor <= this->last_motor; motor++) {
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

void MultiStepper::setPrinter (Print & p) {
  this->printer = &p;
}
