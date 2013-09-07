#include <MultiStepper.h>

int steps_per_rev = 200 * 32;
byte motor_count = 3;

//3 Motors on PORTA and no limit switches
//MultiStepper stepper_controller(&PORTA, &DDRA, motor_counte, steps_per_rev);

//3 Motors on PORTA and 3 limit switches on PORTC
MultiStepper stepper_controller(&PORTA, &DDRA, motor_count, &PINC, &DDRC, steps_per_rev);

void setup() {
  stepper_controller.setPrinter(Serial);
  stepper_controller.setSpeed(10);
  Serial.begin(9600);
}

void osscilate() {
  while (true) {
    stepper_controller.move(
      steps_per_rev,
      steps_per_rev,
      steps_per_rev
    );

    stepper_controller.move(
      -1 * steps_per_rev,
      -1 * steps_per_rev,
      -1 * steps_per_rev
    );
  }
}

void two_steps_forward_one_step_backwards(){
  while (true) {
    stepper_controller.move(
      2 * steps_per_rev,
      2 * steps_per_rev,
      2 * steps_per_rev
    );

    stepper_controller.move(
      -1 * steps_per_rev,
      -1 * steps_per_rev,
      -1 * steps_per_rev
    );
  }

}

void box(){
  stepper_controller.move((long)steps_per_rev);
  stepper_controller.move(0, (long)steps_per_rev);
  stepper_controller.move(-1 * (long)steps_per_rev);
  stepper_controller.move(0, -1 * (long)steps_per_rev);
}

void loop() {
  box();
}

