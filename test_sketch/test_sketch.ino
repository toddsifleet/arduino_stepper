#include <MultiStepper.h>

int steps_per_rev = 200 * 32;
byte motor_count = 3;

//3 Motors on PORTA and no limit switches
//MultiStepper stepper_controller(&PORTA, &DDRA, motor_counte, steps_per_rev);

//3 Motors on PORTA and 3 limit switches on PORTC
MultiStepper stepper_controller(&PORTA, &DDRA, motor_count, &PINC, &DDRC, steps_per_rev);

void setup() {
  stepper_controller.setPrinter(Serial);
  stepper_controller.setSpeed(6);
  Serial.begin(9600);
}

void osscilate(long speed) {
  stepper_controller.setSpeed(speed);
  while (true) {
    long start_time = millis();
    stepper_controller.move(
      steps_per_rev,
      steps_per_rev,
      steps_per_rev
    );
    Serial.println(millis() - start_time);
    start_time = millis();
    
    stepper_controller.move(
      -1 * steps_per_rev,
      -1 * steps_per_rev,
      -1 * steps_per_rev 
    );
    Serial.println(millis() - start_time);
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

long string_to_long(String input) {
    int len = input.length() + 1;
    char temp[len];
    input.toCharArray(temp, len);    
    return atol(temp);
}
void run_command(String input) {
  if (input.length() <= 0) {
    return; 
  }

  long long_values[3];
  int current = -1;
  for (int i = 0; i < 3; i++) {
    //get each segment of the string
    int n = input.indexOf(' ', ++current);
    String value = input.substring(current, n);
    current = n;

    long_values[i] = string_to_long(value);
  }
  
  stepper_controller.move(
    long_values[0],
    long_values[1],
    long_values[2]
  );
}

void run_serial_commands() {
  while (Serial.available() <= 0) {
    Serial.println("Starting Up...");
    delay(100);
  }
  Serial.read();
  Serial.println("ready");
  while (true) {
    delay(3);
    if (Serial.available() > 0) {
      String readString = "";
      while (Serial.available() > 0) {
        delay(3);
        char c = Serial.read();
        readString += c; 
      }
      run_command(readString);
     }
   } 
}

void rectangle(long width, long height){
  stepper_controller.move(width, height);
  stepper_controller.move(-width);
  stepper_controller.move(0, -height);
  stepper_controller.move(width);
  stepper_controller.move(0, height);
  stepper_controller.move(0, -height);
  stepper_controller.move(-width, height);
  while (true) {};
}

void loop() {
//  rectangle(60000L, 30000L);
//  osscilate(60);
  run_serial_commands();
}

