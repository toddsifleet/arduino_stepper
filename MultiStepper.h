#include <inttypes.h>
#include "Arduino.h"

#ifndef MultiStepper_h
#define MultiStepper_h`

class MultiStepper {
  public:
    MultiStepper(
      volatile uint8_t *motor_port,
      volatile uint8_t *motor_port_ddr,
      uint8_t motor_mask,
      volatile uint8_t *limit_port,
      volatile uint8_t *limit_port_ddr,
      uint8_t limit_mask,
      int steps_per_revolution
    );

    MultiStepper(
      volatile uint8_t *motor_port,
      volatile uint8_t *motor_port_ddr,
      uint8_t motor_mask,
      int steps_per_revolution
    );

    volatile uint8_t *motor_port;
    volatile uint8_t *limit_port;

    int steps_per_revolution;
    int current_step[4];

    bool has_limit;
    void step(uint8_t direction);

    void setPrinter (Print & p);

  private:
    void stepMotor(int this_step);
    void initMotor(
      volatile uint8_t *port,
      volatile uint8_t *ddr,
      uint8_t mask,
      int steps_per_revolution
    );
    void initLimit(
      volatile uint8_t *port,
      volatile uint8_t *ddr,
      uint8_t mask
    );
    void incrementStep(int *current_step);
    void decrementStep(int *current_step);
    void printArray(char *label, int array[], int length);

    // for debugging currently
    Print *printer;
    uint8_t motor_mask;
    uint8_t limit_mask;
    uint8_t first_motor;
    uint8_t last_motor;
};

#endif

