#include <MultiStepper.h>

int steps_per_rev = 200 * 32;
byte motor_count = 3;

//3 Motors on PORTA and no limit switches
//MultiStepper stepper_controller(&PORTA, &DDRA, motor_counte, steps_per_rev);

//3 Motors on PORTA and 3 limit switches on PORTC
MultiStepper stepper_controller(&PORTA, &DDRA, motor_count, &PINC, &DDRC, steps_per_rev);

void setup() {
  stepper_controller.setPrinter(Serial);
  stepper_controller.setSpeed(100);
  Serial.begin(9600);
}

void osscilate() {
  long direction[] = {1, 1, 1};
  long step_count = 0;
  while (true) {
    step_count++;
    if (step_count % steps_per_rev == 0) {
      for (int i = 0; i < 3; i++) {
        direction[i] *= -1;
      }
    }
    stepper_controller.move(direction);
  }
}

void loop() {
  osscilate();
}

