#include <Servo.h>

int motor = 5;				// Winchmotor is connected to D5 (3V via Relais)
int limitSW = 4;        	// Limitswitch for winchmotor is connected to D4
int upLED = A0;         	// Up indicator light is connected to A0
int downLED = A2;       	// Down indicator light is connected to A2
int stateBtn = A3;			// Up button is connected to A3
int servoPin = 6;			// Servo PWM control is connected to D3
int rcRxPin = 11;			// PWM from RC Rx is connected to D11
int couple = 10;			// Servo position when attached to spool
int decouple = 35;			// Servo position when unattached to spool

enum states {			
	UP, 
	DOWN, 
	DROP,
	HOIST 
};

states prior_state, state;	


Servo myservo;			

void setup()
{
	pinMode(motor, OUTPUT);                    
	pinMode(upLED, OUTPUT);           
	pinMode(downLED, OUTPUT);                
	pinMode(stateBtn, INPUT);
	pinMode(rcRxPin, INPUT);     
	pinMode(limitSW, INPUT_PULLUP);

	/* Make sure outputs are in their base states */
	digitalWrite(motor, LOW);           
	digitalWrite(upLED, LOW);           
	digitalWrite(downLED, LOW);         

	/* Attach servo, reposition to base, give time to adjust and detach to save power */
	myservo.attach(servoPin);
	myservo.write(couple);		
	delay(700);				
	myservo.detach();				

	prior_state = NONE;

	/* Determine what state we're in based on limitSW */
	if (digitalRead(limitSW) == HIGH) {
		state = UP;
	} else {
		state = DOWN;
	}
}

void up() 
{
	/* Light up green LED when anchor is up */
	if (state != prior_state) {
		prior_state = state;
		digitalWrite(upLED, HIGH);
	}

	/* Change state to DROP if button is pressed, or RC pulsewidth goes above 1800 Milliseconds */
	if (digitalRead(stateBtn) == HIGH || pulseIn(rcRxPin, HIGH) > 1800) {
		state = DROP;
	}
  
	/* When changing state to DROP, flash red LED twice to indicate we're about to drop anchor. (Comment out when unwanted) */
	if (state != prior_state) {                
		for(int i=0; i<2; i++){
			digitalWrite(downLED, HIGH);
			delay(500);
			digitalWrite(downLED, LOW);
			delay(500);
		}
    		digitalWrite(upLED, LOW);
	}
}

void down()
{
	/* Light red LED when anchor is fully down */
	if (state != prior_state) {
		prior_state = state;
		digitalWrite(downLED, HIGH);
	}

  	/* Change state to HOIST if button is pressed or when RC pulsewidth goes above 1800 milliseconds */
	if (digitalRead(stateBtn) == HIGH || pulseIn(rcRxPin, HIGH) > 1800) {
		state = HOIST;
	}

	/* When changing state to HOIST, flash red LED twice to indicate we're about to hoist the anchor. (Comment out when unwanted) */
	if (state != prior_state) {
		digitalWrite(upLED, HIGH);
		for(int i=0; i<2; i++){
			digitalWrite(downLED, LOW);
			delay(500);
			digitalWrite(downLED, HIGH);
			delay(500);
		}
		digitalWrite(downLED, LOW);
		//digitalWrite(upLED, LOW);

		  
	}
}

void drop()
{
	/* Move servo to decouple from spool, Flash RED led fast to indicate anchor is dropping
	 * Move servo back to coupled position, detach servo to save power and change state to DOWN */
	myservo.attach(servoPin);
	myservo.write(decouple);

	for (int i=0; i<20; i++) {
		digitalWrite(downLED, HIGH);
		delay(100);
		digitalWrite(downLED, LOW);
		delay(100);
	}

	myservo.write(couple);
	delay(500);
	myservo.detach();
	state = DOWN;
}

void hoist()
{
	/* Run motor until limit switch is high.
	 * While running motor, flash green LED fast to indicate we're raising the anchor.
	 * When limit is reached, turn off motor and switch state to UP */

	digitalWrite(motor, HIGH);

	while (digitalRead(limitSW) == LOW) {
		digitalWrite(upLED, HIGH);
		delay(100);
		digitalWrite(upLED, LOW);
		delay(100);
	}

	digitalWrite(motor, LOW);
	state = UP;
}



void loop() 
{
	switch (state) {
		case UP:
		up();
		break;
		case DOWN:
		down();
		break;
		case DROP:
		drop();
		break;
		case HOIST:
		hoist();
		break;
	}
}
