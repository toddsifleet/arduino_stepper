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

    int steps_per_revolution;

    bool has_limit;
    void step(uint8_t direction);

    void setNoLimits();

    void setPrinter (Print & p);
    
    void setHome();

  private:
    void initMotors(
      volatile uint8_t *port,
      volatile uint8_t *ddr,
      uint8_t mask,
      int steps_per_revolution
    );
    void initLimits(
      volatile uint8_t *port,
      volatile uint8_t *ddr,
      uint8_t mask
    );

    void stepMotor(int this_step);
    void incrementMotorCounters(int motor);
    void decrementMotorCounters(int motor);

    // for debugging currently
    void printArray(char *label, int array[], int length);
    Print *printer;

    //motor pin config data
    uint8_t motor_mask;
    uint8_t limit_mask;
    uint8_t first_motor;
    uint8_t last_motor;

    //motor ports
    volatile uint8_t *motor_port;
    volatile uint8_t *limit_port;

    //motor state data
    uint8_t motor_step[4];
    long motor_position[4];
};

#endif

