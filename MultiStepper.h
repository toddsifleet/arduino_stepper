#include <inttypes.h>
#include "Arduino.h"

#ifndef MultiStepper_h
#define MultiStepper_h`

uint8_t calculateMask(uint8_t n);

class MultiStepper {
  public:
    // CONSTRUCTORS
    MultiStepper(
      volatile uint8_t *motor_port,
      volatile uint8_t *motor_port_ddr,
      uint8_t motor_count,
      volatile uint8_t *limit_port,
      volatile uint8_t *limit_port_ddr,
      int steps_per_revolution
    );

    MultiStepper(
      volatile uint8_t *motor_port,
      volatile uint8_t *motor_port_ddr,
      uint8_t motor_count,
      int steps_per_revolution
    );

    // CONFIGURATION FUNCTIONS
    void setNoLimits();
    void setLimits(
      volatile uint8_t *port,
      volatile uint8_t *ddr
    );

    void setStepsPerRevolution(int steps);
    void setSpeed(int rpm);

    void setPrinter (Print & p);

    void setHome();

    // MOVEMENT FUNCTIONS
    void step(int direction[]);
    void goTo(
      long motor_1 = 0,
      long motor_2 = 0,
      long motor_3 = 0,
      long motor_4 = 0
    );
    void move(
      long motor_1 = 0,
      long motor_2 = 0,
      long motor_3 = 0,
      long motor_4 = 0
    );
    void goHome();

  private:
    void initMotors(
      volatile uint8_t *port,
      volatile uint8_t *ddr,
      uint8_t motor_count,
      int steps_per_revolution
    );

    void incrementMotorCounters(int motor);
    void decrementMotorCounters(int motor);

    void advanceMotors(volatile uint8_t port_mask);

    // for debugging currently
    void printArray(char *label, uint8_t array[], int length);
    void printArray(char *label, int array[], int length);
    void printArray(char *label, long array[], int length);
    Print *printer;

    //motor config data
    int steps_per_revolution;
    long step_delay;
    uint8_t motor_mask;
    uint8_t motor_count;
    volatile uint8_t *motor_port;

    //limit config data
    bool has_limit;
    volatile uint8_t *limit_port;

    //motor state data
    uint8_t motor_step[4];
    long motor_position[4];
    unsigned long last_step_time;
    long step_delay_correction;

};

#endif

