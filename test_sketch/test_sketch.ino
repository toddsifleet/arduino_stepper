#include <MultiStepper.h>

int steps_per_rev = 200 * 32;
byte motor_mask = 0b00111111;
MultiStepper stepper_controller(&PORTA, &DDRA, motor_mask, steps_per_rev);


long last_step;
long step_count = 0;

void setup() {
  last_step = millis();
  stepper_controller.setPrinter(Serial);
  Serial.begin(9600);
}

byte direction = 0b101010;
void loop() {
    if (millis() - last_step >= 1) {
      step_count++;
      if (step_count % steps_per_rev == 0) {
         direction = ~direction;
      }
        
      stepper_controller.step(direction);
      last_step = millis();
    }
}

