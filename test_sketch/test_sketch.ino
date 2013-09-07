#include <MultiStepper.h>

int steps_per_rev = 200 * 32;
byte motor_count = 3;

//3 Motors on PORTA and no limit switches
//MultiStepper stepper_controller(&PORTA, &DDRA, motor_counte, steps_per_rev);

//3 Motors on PORTA and 3 limit switches on PORTC
MultiStepper stepper_controller(&PORTA, &DDRA, motor_count, &PINC, &DDRC, steps_per_rev);

long step_count = 0;
long start_of_rotation = 0;
void setup() {
  stepper_controller.setPrinter(Serial);
  stepper_controller.setSpeed(1);
  Serial.begin(9600);
}

long direction[] = {1, 1, 1};
void loop() {
      step_count++;
      if (step_count % steps_per_rev == 0) {
        Serial.print(millis() - start_of_rotation);
        Serial.write("\n");
         for (int i = 0; i < 3; i++) {
           direction[i] *= -1;
         }
         start_of_rotation = millis();
      }
        
      stepper_controller.move(direction);
}

