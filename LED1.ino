// TODO: Figure out why the hell it crashes. Hint: use GOFAST
// TODO: Heartbeat mode
// TODO: Get rid of my clear and use FastLED.clear()?
// TODO: Do a sort by cycle index so when the flashing is in order it's a sine wave or something, but they start out shuffled. Hash lookup.

#include <FastLED-3.1.3\FastLED.h>

// For debugging, uncomment this line
//#define GOFAST

// LED OBJECTS
#define NUM_LEDS 50
#define DATA_PIN 3
int colornumber = 1;

#ifdef GOFAST
#define DELAY_TIME 2
#else
#define DELAY_TIME 30
#endif

#define COLOR_SHIFT_SPEED 10
CRGB leds[NUM_LEDS];
CHSV HSVleds[NUM_LEDS];
//int lastLED;
uint8_t	globalBrightness = 8;

// UTILITY OBJECTS
char debugString[1024];
int dimCounter;

void setup()
{
	//Serial.println("Setting up");
	// Add the LEDs so FastLED knows about them. Duh.
	FastLED.addLeds<WS2811, DATA_PIN>(leds, NUM_LEDS);

	// The best way to really get a different random number seed every time, 
	// since Arduinos don't ship with radioactive particles...
	randomSeed(analogRead(0) && analogRead(1) && analogRead (2) && analogRead(3) && analogRead(4) && analogRead(5));

	//Serial.begin(9600);
	//Serial.println("IR Receiver Button Decode");
	//irrecv.enableIRIn(); // Start the receiver
	dim();
	dimCounter = 4;
}

int availableMemory() {
	int size = 1024; // Use 2048 with ATmega328
	byte *buf;

	while ((buf = (byte *)malloc(--size)) == NULL)
		;

	free(buf);

	return size;
}

void loop()
{
	//if (shutoffcounter-- == 0) {
	//	Serial.println("Done!");
	//	exit(0);
	//}

	random16_add_entropy(random());

	switch (random8(0, 5)) {
	case 0:
		// Hostile streetlights
		//Serial.println("Doing street");
		doCityStreet();
		break;
	case 1:
		// Random color flickering
		//Serial.println("Doing rando");
		doRando();
		break;
	case 2:
		// Fibonacci march
		//Serial.println("Doing Fib");
		doFibonacciMarch();
		break;
	case 3:
		// Boolean Sort
		//Serial.println("Doing Swap sort");
		doSwapSort();
		break;
	case 4:
		// Man of the hour!
		//Serial.println("Doing Pac Man");
		doPacMan();
		break;
	}
	if (!--dimCounter) {
		dimCounter = 4;
		dim();
	}
	//Serial.print("Available memory: ");
	//Serial.println(availableMemory());
}

void dim() {
	globalBrightness--;
	//Serial.print("Dimmed, brightness is ");
	//Serial.println(globalBrightness);
	if (globalBrightness <= 0) {
		//Serial.println("Done! Clearingout!");
		//Serial.println("...");
		Serial.println();
		FastLED.clear(true);
		FastLED.delay(1000);
		exit(0);
	}
	FastLED.setBrightness(globalBrightness);

}

/**********************************************
*
* CITY STREET
*
**********************************************/

enum TLColor {
	TLGreen, TLYellow, TLRed
};
#define MAX_CARS 20
#define CAR_FRAME_TIME 125
#define NUM_OF_TRAFFIC_LIGHTS 6
#ifdef GOFAST
#define TOTAL_DURATION 2000
#else
#define TOTAL_DURATION 30000
#endif
#define RED_DURATION 6000
#define YELLOW_DURATION 2000
#define GREEN_DURATION 8000
#define CAR_MIN_SPAWN 500
#define CAR_MAX_SPAWN 1500
int lightTimer[NUM_OF_TRAFFIC_LIGHTS];
int lightColor[NUM_OF_TRAFFIC_LIGHTS];
// Note that none of these can be within 2 of the end
int lightIndex[NUM_OF_TRAFFIC_LIGHTS] = { 7, 13, 24, 32, 39, 44 };
int carIndex[MAX_CARS];
int numOfCars = 0;
bool thisCarShouldMove;
void doCityStreet() {
	// Draw the traffic lights
	FastLED.clear(true);
	int i;
	for (i = 0; i < NUM_OF_TRAFFIC_LIGHTS; i++) {
		lightTimer[i] = random(0, RED_DURATION + YELLOW_DURATION + GREEN_DURATION);
	}
	long duration = TOTAL_DURATION;
	int carDelay = random(CAR_MIN_SPAWN, CAR_MAX_SPAWN);
	unsigned long lastMillis = millis();
	int timeDelta;
	do {
		timeDelta = millis() - lastMillis;
		carDelay -= timeDelta;
		duration -= timeDelta;
		for (i = 0; i < NUM_OF_TRAFFIC_LIGHTS; i++) {
			if (lightTimer[i] < RED_DURATION) {
				lightColor[i] = TLRed;
			}
			else if (lightTimer[i] < RED_DURATION + YELLOW_DURATION) {
				lightColor[i] = TLYellow;
			}
			else {
				lightColor[i] = TLGreen;
			}
			lightTimer[i] -= timeDelta;
			if (lightTimer[i] < 0) {
				lightTimer[i] = RED_DURATION + YELLOW_DURATION + GREEN_DURATION;
			}
		}
		if (carDelay < 0) {
			if (numOfCars < MAX_CARS) {
				carIndex[numOfCars++] = 0;
			}
			carDelay = random(CAR_MIN_SPAWN, CAR_MAX_SPAWN);
		}
		FastLED.clear(true);
		moveCars();
		drawCars();
		drawLights();
		lastMillis = millis();
		FastLED.delay(CAR_FRAME_TIME);
	} while (duration > 0);
}

void moveCars() {
	for (int i = numOfCars - 1; i >= 0; i--) {
		thisCarShouldMove = true;
		// If the car is at the end, remove it. We assume FIFO, so shifting down is enough.
		if (carIndex[i] == NUM_LEDS - 1) {
			for (int j = 1; j < numOfCars; j++) {
				carIndex[j - 1] = carIndex[j];
			}
			numOfCars--;
			continue;
		}
		// If I'm at a red light, stop
		for (int j = 0; j < NUM_OF_TRAFFIC_LIGHTS; j++) {
			if (lightIndex[j] == carIndex[i] + 1 && lightColor[j] == TLRed) {
				thisCarShouldMove = false;
				break;
			}
		}
		// If there's a car in front of me, stop
		if (i != 0 && carIndex[i - 1] == carIndex[i] + 1) {
			thisCarShouldMove = false;
		}
		// move forward
		if (thisCarShouldMove) {
			carIndex[i] = carIndex[i] + 1;
		}
	}
}

void drawCars() {
	for (int i = numOfCars - 1; i >= 0; i--) {
		leds[carIndex[i]] = CRGB::White;
	}
}

void drawLights() {
	for (int i = 0; i < NUM_OF_TRAFFIC_LIGHTS; i++)
	{
		switch (lightColor[i])
		{
		case TLGreen:
			leds[lightIndex[i]] = CRGB::Green;
			break;
		case TLYellow:
			leds[lightIndex[i]] = CRGB::Yellow;
			break;
		case TLRed:
			leds[lightIndex[i]] = CRGB::Red;
			break;
		default:
			break;
		}
		//leds[lightIndex[i]] = (lightColor[i] == TLGreen) ? CRGB::Green : CRGB(0, 10, 0);
		//leds[lightIndex[i] + 1] = (lightColor[i] == TLYellow) ? CRGB::Yellow : CRGB(10, 10, 0);
		//leds[lightIndex[i] + 2] = (lightColor[i] == TLRed) ? CRGB::Red : CRGB(10, 0, 0);
	}
}

/**********************************************
*
* PAC MAN
*
**********************************************/

#ifdef GOFAST
#define PACMAN_APPEAR_DELAY 4
#define PACMAN_MOVE_SPEED 2
#else
#define PACMAN_APPEAR_DELAY 500
#define PACMAN_MOVE_SPEED 100
#endif
void doPacMan() {
	int i, j, k;
	int pac, ghosts[4], frameFlipper;
	CRGB ghostColor1, ghostColor2, pacColorMouth;
	bool ghostIsEaten[4];
	FastLED.clear(true);
	for (j = 0; j < 255; j++) {
		for (i = 0; i < NUM_LEDS; i++) {
			if (!(i % 2)) {
				leds[i] = CRGB(j, j, j);
			}
		}
		FastLED.delay(PACMAN_MOVE_SPEED);
	}
	FastLED.delay(PACMAN_APPEAR_DELAY);

	// A New Pac

	// appear
	leds[0] = CRGB::Yellow;
	FastLED.delay(PACMAN_MOVE_SPEED);

	leds[1] = CRGB::Yellow;
	FastLED.delay(PACMAN_MOVE_SPEED);

	for (i = 1; i < NUM_LEDS - 1; i++) {
		leds[i] = CRGB::Yellow;
		// TODO  - the front one should wobble as if the mouth is open/closing
		leds[i + 1] = CRGB::Yellow;
		leds[i - 1] = CRGB::Black;
		FastLED.delay(PACMAN_MOVE_SPEED);
		leds[i + 1] = CRGB::Black;
		FastLED.delay(PACMAN_MOVE_SPEED);
	}

	leds[NUM_LEDS - 2] = CRGB::Black;
	FastLED.delay(PACMAN_MOVE_SPEED);

	leds[NUM_LEDS - 1] = CRGB::Black;
	FastLED.delay(PACMAN_MOVE_SPEED);

	// tense pause...
	FastLED.delay(1000);

	// The Ghosts Strike Back

	for (i = NUM_LEDS; i >= -19; i--) {
		// This might look a little weird, but is easier to copy/paste.
		// i walks from the end of the string back, j is the offset into the pacman-ghost scene
		// j starts at 0 (pacman's nose) and goes back to the end of the scene

		// First, erase the last frame
		for (j = 0; j < 20; j++) {
			if (i + j >= 0 && i + j < NUM_LEDS) {
				leds[i + j] = CRGB::Black;
			}
		}

		// Pac man - 2 yellows
		j = 0;
		if (i + j >= 0 && i + j < NUM_LEDS) {
			leds[i + j] = CRGB::Yellow;
		}
		j = 1;
		if (i + j >= 0 && i + j < NUM_LEDS) {
			leds[i + j] = CRGB::Yellow;
		}

		// #1 - Clyde, orange
		j = 5;
		if (i + j >= 0 && i + j < NUM_LEDS) {
			leds[i + j] = CRGB(96, 32, 0);
		}
		j = 6;
		if (i + j >= 0 && i + j < NUM_LEDS) {
			leds[i + j] = CRGB(96, 32, 0);
		}

		// #2 - Inky, blue
		j = 17;
		if (i + j >= 0 && i + j < NUM_LEDS) {
			leds[i + j] = CRGB::SkyBlue;
		}
		j = 18;
		if (i + j >= 0 && i + j < NUM_LEDS) {
			leds[i + j] = CRGB::SkyBlue;
		}

		// #3 - Blinky, red
		j = 13;
		if (i + j >= 0 && i + j < NUM_LEDS) {
			leds[i + j] = CRGB::Maroon;
		}
		j = 14;
		if (i + j >= 0 && i + j < NUM_LEDS) {
			leds[i + j] = CRGB::Maroon;
		}

		// #4 - Pinky, pink
		j = 9;
		if (i + j >= 0 && i + j < NUM_LEDS) {
			leds[i + j] = CRGB::Plaid;
		}
		j = 10;
		if (i + j >= 0 && i + j < NUM_LEDS) {
			leds[i + j] = CRGB::Plaid;
		}

		FastLED.delay(PACMAN_MOVE_SPEED);

		// Mouth closed
		j = 0;
		if (i + j >= 0 && i + j < NUM_LEDS) {
			leds[i + j] = CRGB::Black;
		}

		FastLED.delay(PACMAN_MOVE_SPEED);
	} // END Ghosts chase Pac

	  // Another tense pause...
	FastLED.delay(1000);
	// Return of the PacMan

	// TODO - rewrite everything using objects like this

	ghosts[0] = -1;
	ghosts[1] = -5;
	ghosts[2] = -9;
	ghosts[3] = -13;
	pac = -37;
	frameFlipper = 0;
	ghostIsEaten[0] = ghostIsEaten[1] = ghostIsEaten[2] = ghostIsEaten[3] = false;
	do {
		FastLED.clear(true);

		if (!(frameFlipper % 2)) {
			ghostColor1 = CRGB::DarkBlue;
			ghostColor2 = CRGB::White;
			pacColorMouth = CRGB::Yellow;
		}
		else {
			ghostColor1 = CRGB::White;
			ghostColor2 = CRGB::DarkBlue;
			pacColorMouth = CRGB::Black;
		}

		// Draw 
		// Ghosts
		for (i = 0; i < 4; i++) {
			if (ghostIsEaten[i]) {
				if (ghosts[i] >= 0 && ghosts[i] < NUM_LEDS) {
					leds[ghosts[i]] = CRGB::DarkGray;
				}
				if (ghosts[i] + 1 >= 0 && ghosts[i] + 1 < NUM_LEDS) {
					leds[ghosts[i] + 1] = CRGB::DarkGray;
				}
			}
			else {
				if (ghosts[i] >= 0 && ghosts[i] < NUM_LEDS) {
					leds[ghosts[i]] = ghostColor1;
				}
				if (ghosts[i] + 1 >= 0 && ghosts[i] + 1 < NUM_LEDS) {
					leds[ghosts[i] + 1] = ghostColor2;
				}
			}
		}
		// Pacman
		if (pac >= 0 && pac < NUM_LEDS) {
			leds[pac] = CRGB::Yellow;
		}
		if (pac + 1 >= 0 && pac + 1 < NUM_LEDS) {
			leds[pac + 1] = pacColorMouth;
		}

		FastLED.delay(PACMAN_MOVE_SPEED);

		// Update

		frameFlipper++;

		// Move pacman always
		if (!(frameFlipper % 2)) {
			pac++;
		}

		// Did pac-man catch a ghost?
		for (i = 0; i < 4; i++) {
			if (!ghostIsEaten[i]) {
				if (pac + 1 == ghosts[i]) {
					ghostIsEaten[i] = true;
				}
				else if (!(frameFlipper % 4)) {
					ghosts[i]++;
				}
			}
			else {
				ghosts[i]--;
			}

		}
	} while (pac < NUM_LEDS);
}

/**********************************************
*
* SWAP SORT
*
**********************************************/

#ifdef GOFAST
#define SWAPSORT_DELAY 2
#else
#define SWAPSORT_DELAY 250
#endif 

void doSwapSort() {
	int i;
	bool foundSwap = true;
	int hue = random8(0, 255);
	CHSV tempHSV;
	// First give everything a random color
	for (i = 0; i < NUM_LEDS; i++) {
		tempHSV = CHSV(hue, 150, random(50, 255));
		leds[i] = tempHSV;
		HSVleds[i] = rgb2hsv_approximate(leds[i]);
		//sprintf(debugString, "HSV %d set to %d, %d, %d", i, HSVleds[i].h,
		//	HSVleds[i].s, HSVleds[i].v);
		//Serial.println(debugString);
	}


	// Now go through the list until we find nothing to swap
	while (foundSwap) {
		foundSwap = false;
		for (i = 0; i < NUM_LEDS - 1; i++) {
			if (HSVleds[i].v > HSVleds[i + 1].v) {
				foundSwap = true;
				//sprintf(debugString,
				//	"Swapping %d which is hue %d with hue %d",
				//	i, HSVleds[i].h, HSVleds[i + 1].h);
				//Serial.println(debugString);
				HSVswapTwo(i, i + 1);
				FastLED.delay(SWAPSORT_DELAY);
			}
		}
	}
}

/**********************************************
 *
 * FIBONACCI MARCH
 *
 **********************************************/
#ifdef GOFAST
#define FIB_DELAY 2
#else 
#define FIB_DELAY 333
#endif
#define FIB_NUM_OF_BLANKS_BETWEEN 2
void doFibonacciMarch() {
	FastLED.clear(true);
	int numbers[2], fibnum, i;
	uint8_t index;
	numbers[0] = 1;
	numbers[1] = 1;
	fibnum = 2;
	leds[0] = ColorFromPalette(RainbowColors_p, 11);
	FastLED.delay(FIB_DELAY);
	for (i = 0; i < FIB_NUM_OF_BLANKS_BETWEEN; i++) {
		shiftUp();
		FastLED.delay(FIB_DELAY);
	}
	shiftUp();
	leds[0] = ColorFromPalette(RainbowColors_p, 15);
	FastLED.delay(FIB_DELAY);
	for (i = 0; i < FIB_NUM_OF_BLANKS_BETWEEN; i++) {
		shiftUp();
		FastLED.delay(FIB_DELAY);
	}
	do {
		for (i = 0; i < fibnum; i++) {
			inttolitend((fibnum * 29), &index);
			//sprintf(debugString, "n0, n1, fib, index: %d, %d, %d, %d", numbers[0], numbers[1], fibnum, index);
			//Serial.println(debugString);
			shiftUp();
			leds[0] = ColorFromPalette(RainbowColors_p, index);
			FastLED.delay(FIB_DELAY);
		}
		for (i = 0; i < FIB_NUM_OF_BLANKS_BETWEEN + (fibnum / 3); i++) {
			shiftUp();
			FastLED.delay(FIB_DELAY);
		}
		numbers[0] = numbers[1];
		numbers[1] = fibnum;
		fibnum = numbers[0] + numbers[1];
	} while (fibnum < NUM_LEDS);
	for (i = 0; i < NUM_LEDS; i++) {
		shiftUp();
		FastLED.delay(FIB_DELAY);
	}
}

/**********************************************
*
* RANDOM
*
**********************************************/
#ifdef GOFAST
#define RANDOM_MIN_ITERATIONS 1
#define RANDOM_MAX_ITERATIONS 2
#else
#define RANDOM_MIN_ITERATIONS 8
#define RANDOM_MAX_ITERATIONS 12
#endif

void doRando() {
	int i, j, k;
	int max, iterations;
	iterations = random8(RANDOM_MIN_ITERATIONS, RANDOM_MAX_ITERATIONS);
	for (i = 0; i < iterations; i++) {
		// Make up a target color
		blendToColor(random8(), random8(), random8());
	}
	blendToColor(0, 0, 0);
}

  /**********************************************
  *
  * UTILITY FUNCTIONS
  *
  **********************************************/
void shiftDown() {
	for (int i = 0; i < NUM_LEDS - 1; i++)
	{
		leds[i] = leds[i + 1];
	}
	leds[NUM_LEDS - 1] = CRGB::Black;
}

void shiftUp() {
	for (int i = NUM_LEDS - 1; i > 0; i--) {
		leds[i] = leds[i - 1];
	}
	leds[0] = CRGB::Black;
}

void swapTwo(int index1, int index2) {
	CRGB temp = leds[index1];
	leds[index1] = leds[index2];
	leds[index2] = temp;
}

void HSVswapTwo(int index1, int index2) {
	CHSV temp = HSVleds[index1];
	HSVleds[index1] = HSVleds[index2];
	HSVleds[index2] = temp;

	CRGB temp2 = leds[index1];
	leds[index1] = leds[index2];
	leds[index2] = temp2;
}

void inttolitend(uint32_t x, uint8_t *lit_int) {
	lit_int[0] = (uint8_t)(x >> 0);
	lit_int[1] = (uint8_t)(x >> 8);
	lit_int[2] = (uint8_t)(x >> 16);
	lit_int[3] = (uint8_t)(x >> 24);
}

void blendToColor(int targetR, int targetG, int targetB) {
	int deltaR, deltaG, deltaB, currentR, currentG, currentB;
	bool notThereYet;
	// Figure out the deltas for each color to get there
	deltaR = (leds[0].r > targetR) ? -1 : 1;
	deltaG = (leds[0].g > targetG) ? -1 : 1;
	deltaB = (leds[0].b > targetB) ? -1 : 1;
	do {
		notThereYet = false;
		// Grab the current leds
		currentR = leds[0].r;
		currentG = leds[0].g;
		currentB = leds[0].b;
		if (currentR != targetR) {
			currentR += deltaR;
			notThereYet = true;
		}
		if (currentG != targetG) {
			currentG += deltaG;
			notThereYet = true;
		}
		if (currentB != targetB) {
			currentB += deltaB;
			notThereYet = true;
		}
		fill_solid(leds, NUM_LEDS, CRGB(currentR, currentG, currentB));
		FastLED.delay(DELAY_TIME);
	} while (notThereYet);

}