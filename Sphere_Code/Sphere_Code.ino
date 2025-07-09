#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <EEPROM.h>
#include <Ticker.h>

//IMPORTANT: Code was originally compiled with FastLED ver 3.10.0, ESP8266 core 3.1.2, and IRremoteESP866 ver 2.8.6
//So use those versions if you run into any bugs

#define FASTLED_INTERNAL 1  //turns off the pragma messages from FastLED
#include <Pixel_Spork.h>

/*IMPORTANT: 
    While IR controls do work with ESP8266 and FastLED,
    the interrupts used to process the IR signal causes the LEDs to "glitch" 
    for a split second whenever an IR signal is received. 
    This applies to ANY IR signal, as the sensor catches them all, so you
    should avoid places where there are a lot of IR signals are being used.

    Also note that some settings are saved between boots using EEPROM.
    To limit the number of writes and extend the EEPROM life,
    there is a saving delay of a few seconds (see EEPROM_COM_TIME)
    after a button push to allow for more inputs. 
    If you shutdown the system too quickly after a button push, it will not be saved.
*/

//IR Button Mappings:
//Note that each IR remote's button hex values are unique,
//you will need to replace the hex codes below with those from your remote.
//run "IRrecvDemo" example from IRremoteESP8266 library to get hex values for your remote's buttons.
//You can map the functions to whatever buttons you wish.
//Also note that some settings are saved after shutdown using EEPROM.
#define BRI_INC_BUT 0xFF18E7       //up arrow, brightness increase, saves.
#define BRI_DEC_BUT 0xFF4AB5       //down arrow, brightness decrease, saves.
#define NEXT_EFFECT_BUT 0xFF5AA5   //right arrow, next effect, wraps back to start.
#define PREV_EFFECT_BUT 0xFF10EF   //left arrow, previous effect, wraps back to end.
#define EFFECT_LOCK_BUT 0xFF38C7   //"OK", toggles effect cycling, so the current effect is run forever, saves.
#define EFFECTS_OFF_BUT 0xFF6897   //"*", turns all effects on/off, also blocks all other button inputs while off.
#define EFFECT_RESET_BUT 0xFFB04F  //"#", restarts the current effect (with new inputs if random).

//EEPROM Addresses for settings
//we want to store the brightness, current effect index, and the effect rotation toggle
//so they can be recalled after the system has been shutdown.
//Note that EEPROM has limited lifetime writes ~10K, so we want to commit all the writes after a delay.
//This allows the data to change while the user is pressing buttons, and we'll only commit
//the final values once they are done, minimizing total writes.
#define BRIGHTNESS_ADDR 2     //brightness address
#define CUR_EFFECT_ADDR 0     //index of current effect address
#define EFFECT_LOCK_ADDR 1    //effect rotation bool address
#define EEPROM_COM_TIME 3000  //EEPROM write time delay (ms)

//How many leds in your strip?
#define NUM_LEDS 120

//LED data and IR receiver pins
#define DATA_PIN D6
#define IR_RECV_PIN D5

//Total number of effects
//See "Spare Effects" file for unused effects
#define NUM_EFFECTS 20

//IR receiver setup
IRrecv irrecv(IR_RECV_PIN);
decode_results IRsig;

//initialize ticker objects for EEPROM saving
Ticker EEPROMcommitter;  //timer for committing data to EEPROM

//FastLED LED color storage
CRGB leds[NUM_LEDS];

//========================================================================
//                          Segment Sets Setups
//========================================================================
//see segDefs file for segment set definitions
#include "segDefs.h"

//========================================================================
//                          Palette Classes Setups
//========================================================================
//Random palettes
//(initialized with red, green, and blue starting colors, they will be randomized during setup())
CRGB rand1Pallet_Arr[] = {CRGB::Red};
palettePS rand1Pallet1 = {rand1Pallet_Arr, SIZE(rand1Pallet_Arr)};

CRGB rand2Pallet_Arr[] = {CRGB::Red, CRGB::Blue};
palettePS rand2Pallet1 = {rand2Pallet_Arr, SIZE(rand2Pallet_Arr)};

CRGB rand2Pallet_Arr2[] = {CRGB::Red, CRGB::Blue};
palettePS rand2Pallet2 = {rand2Pallet_Arr2, SIZE(rand2Pallet_Arr2)};

CRGB rand3Pallet_Arr[] = {CRGB::Red, CRGB::Blue, CRGB::Green};
palettePS rand3Pallet1 = {rand3Pallet_Arr, SIZE(rand3Pallet_Arr)};

CRGB rand3Pallet_Arr2[] = {CRGB::Red, CRGB::Blue, CRGB::Green};
palettePS rand3Pallet2 = {rand3Pallet_Arr2, SIZE(rand3Pallet_Arr2)};

CRGB rand4Pallet_Arr[] = {CRGB::Red, CRGB::Blue, CRGB::Green, CRGB::Purple};
palettePS rand4Pallet1 = {rand4Pallet_Arr, SIZE(rand4Pallet_Arr)};

CRGB rand4Pallet_Arr2[] = {CRGB::Red, CRGB::Blue, CRGB::Green, CRGB::Purple};
palettePS rand4Pallet2 = {rand4Pallet_Arr2, SIZE(rand4Pallet_Arr2)};

CRGB lavaPal_arr2[] = {CRGB::DarkRed, CRGB::Maroon, CRGB::Red, CRGB::DarkOrange};
palettePS lavaPal = {lavaPal_arr2, SIZE(lavaPal_arr2)};
//========================================================================
//                          Util Classes Setups
//========================================================================
//The classes below either manipulate properties of effects or segments
//For most cases you'll only need one in your code

EffectBasePS *effArray[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};  //Create an effect array for our effects
//Create the effect set using the effect array above, its length, and a run time (10000 ms)
EffectSetPS effectSet(effArray, SIZE(effArray), 3, 10000);

//Palette blender with 2 colors, 5000ms pause between blends
PaletteBlenderPS Rand2PB(rand2Pallet1, rand2Pallet2, true, true, false, 5000, 20, 50);
palettePS &randPB2Pal = Rand2PB.blendPalette;

//Palette blender with 3 colors, 5000ms pause between blends
PaletteBlenderPS Rand3PB(rand3Pallet1, rand3Pallet2, true, true, false, 5000, 20, 50);
palettePS &randPB3Pal = Rand3PB.blendPalette;

//Palette blender with 4 colors, 5000ms pause between blends
//PaletteBlenderPS Rand4PB(rand4Pallet1, rand4Pallet2, true, true, false, 5000, 20, 50);
//palettePS &randPB4Pal = Rand4PB.blendPalette;

PaletteNoisePS PalNoise3(3, 0, 70, false, 15, 60, 60);
palettePS &noise3Pal = PalNoise3.noisePalette;

//========================================================================
//                          Effect Names
//========================================================================

//LavaPS *Lav;
RainSL *rainSL;
//RainSL *rainSL2;
BreathEyeSL *BreathEye;
ParticlesSL *Parti;
ColorModeFillPS *ColorFill;
//ColorModeFillPS *ColorFill2;
ShiftingSeaSL *SeaShift;
DissolveSL *Disolv;
EdgeBurstSL *Edge;
FirefliesSL *FireFlies;
RollingWavesFastSL *RollFa;
ScannerSL *Scan;
PrideWPalSL2 *Pride2;
PacificaHueSL *PaciH;
StreamerSL *Strem;
Twinkle2SLSeg *Twinkl2;
NoiseSL *NoiseL;
NoiseGradSL *NoiseG;
//NoiseWavesSL *NoiseW;

//=====================================================================================
//                              General Vars
//=====================================================================================
//Variables we need to track the effect cycling
uint8_t effectNum = 0;       //Tracks what effect we're on. Set to saved EEPROM value in setup().
bool effectSetup = false;    //Flag for if an effect has been configured for updating.
bool effectsLocked = false;  //If true, then the current effect will run indefinitely. Set to saved EEPROM value in setup().
bool effectsOff = false;     //If true, then all LEDs will be turned off, and effects are paused.

//brightness vars
int8_t brightnessIndex = 1;  //fallback brightness index, actual value is pulled from saved EEPROM setup().
//brightness levels array, max possible brightness is 255
const uint8_t brightnessLevels[] = {20, 50, 80, 120, 180};
const uint8_t numBrightnessLevels = SIZE(brightnessLevels);

void setup() {
    //Serial.begin(115200);

    //Start the IR receiver
    irrecv.enableIRIn();

    //Setup the LED strip using FastLED
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
    segDrawUtils::turnSegSetOff(lineCWsegments);          //clear the LEDs, just in case

    //EEPROM Memory Initialization
    EEPROM.begin(512);

    //read EEPROM values for current effect, brightness, and effect cycling
    //If effects are not locked, we start from the first effect, otherwise, we start from the last used effect
    brightnessIndex = EEPROM.read(BRIGHTNESS_ADDR);
    effectsLocked = EEPROM.read(EFFECT_LOCK_ADDR);
    effectSet.infinite = effectsLocked;
    if(effectsLocked) {
        effectNum = EEPROM.read(CUR_EFFECT_ADDR);
    } else {
        effectNum = 0;
    }

    //Set the strip brightness
    FastLED.setBrightness(brightnessLevels[brightnessIndex % numBrightnessLevels]); //using mod to prevent any EEPROM weirdness on first boot
    //set a random seed for the esp
    randomSeed(ESP.getCycleCount());
    random16_add_entropy(ESP.getCycleCount());

    //assign the effect utility classes to the effect controller
    effectSet.setEffect(&Rand2PB, 0);
    effectSet.setEffect(&Rand3PB, 1);
    //effectSet.setEffect(&Rand4PB, 2);
    effectSet.setEffect(&PalNoise3, 2);

    //Randomize the colors in the random color palettes
    paletteUtilsPS::randomize(rand2Pallet1);
    paletteUtilsPS::randomize(rand2Pallet2);

    paletteUtilsPS::randomize(rand3Pallet1);
    paletteUtilsPS::randomize(rand3Pallet2);

    //paletteUtilsPS::randomize(rand4Pallet1);
    //paletteUtilsPS::randomize(rand4Pallet2);

    //Call the reset function ot turn on 
    //the runOffset setting for all the segment sets
    //This allows the color mode rainbows to shift
    resetLoopSettings();
}

//=====================================================================================
//                              Main Loop Code
//=====================================================================================
//To remove an effect, simply set its effectSet.runTime = 0 and it will be skipped over

//To add a new effect, add the following code block to the large switch statement below,
//filling in the case number, effect constructor, and effect name.
//If using a new type of effect you must add it in the "effect names" section above.
//You'll also need to increment NUM_EFFECTS above.
/* 
case <<case number>>: {
    if(!def) {
        <<effect constructor>>
        effectSet.runTime = 20000; //runtime in ms
        effectSet.setEffect(<<effect name>>, 2);
    } else {
        //
    }
} break;
*/

//Each loop, we update the current effect,
//and check if it's time to cycle to the next one (once its "runtime" has passed)
//We also check the IR receiver for any button inputs.
//Note that some button inputs may pause the effect cycling, see "handleIR()" for more.
//Note that when effect cycling is off, the system will resume from the last played effect on re-boot.
void loop() {

    //Use a switch statement to switch between what effect is being run
    switch(effectNum) {
        case 0:
        default: {
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                ColorFill = new ColorModeFillPS(ringSegments, 3, 30);  //lines
                ringSegments.offsetRateOrig = *ColorFill->rate;
                effectSet.setEffect(ColorFill, 3);
                effectSet.runTime = 10000;  //set the effect runtime (ms)
            } else {
                //We could add more code here that would run while the Streamer is running
            }
        } break;
        case 1: {
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                rainSL = new RainSL(ringSegments, randPB2Pal, 0, true, 8, 1, 1, 0, 2, 100, false);
                effectSet.setEffect(rainSL, 3);
                effectSet.runTime = 15000;  //set the effect runtime (ms)
            } else {
                //We could add more code here that would run while the effect is running
            }
        } break;
        case 2: {
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                Parti = new ParticlesSL(lineCWsegments, randPB3Pal, 0, 10, 2, 50, 40, 1, 0, 1, 2, 6, 2, randPB3Pal.length, true);
                Parti->colorMode = 4;
                effectSet.setEffect(Parti, 3);
                effectSet.runTime = 15000;  //set the effect runtime (ms)
            } else {
                //We could add more code here that would run while the effect is running
            }
        } break;
        case 3: {
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                paletteUtilsPS::setColor(rand1Pallet1, CRGB::White, 0);
                Parti = new ParticlesSL(lineCWsegments, rand1Pallet1, 0, 12, 2, 50, 30, 1, 0, 0, 2, 6, 2, 0, false);
                Parti->bgColorMode = 2;
                effectSet.setEffect(Parti, 3);
                effectSet.runTime = 15000;  //set the effect runtime (ms)
            } else {
                //We could add more code here that would run while the effect is running
            }
        } break;
        case 4: {
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                NoiseG = new NoiseGradSL(lineCWsegments, randPB3Pal, 0, 12, 0, 10, 10, 30, 5000, 80);
                NoiseG->doBrightness = false;
                effectSet.setEffect(NoiseG, 3);
                effectSet.runTime = 15000;  //set the effect runtime (ms)
            } else {
                //We could add more code here that would run while the effect is running
            }
        } break;
        case 5: {
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                Edge = new EdgeBurstSL(ringSegments, 0, true, true, 20, 30);
                effectSet.setEffect(Edge, 3);
                effectSet.runTime = 15000;  //set the effect runtime (ms)
            } else {
                //We could add more code here that would run while the effect is running
            }
        } break;
        case 6: {
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                BreathEye = new BreathEyeSL(ringSegments, 0, 0, 255, 13, true, true, 10, 50);
                BreathEye->dimPow = 0;
                effectSet.setEffect(BreathEye, 3);
                effectSet.runTime = 15000;  //set the effect runtime (ms)
            } else {
                //We could add more code here that would run while the effect is running
            }
        } break;
        case 7: {
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                FireFlies = new FirefliesSL(lineCWsegments, randPB3Pal, 40, 50, 2000, 3000, 3, 6, 30);
                //FireFlies->colorMode = 1;  //6
                FireFlies->flicker = false;
                //FireFlies->rateOrig = 60;  //fix for weird rate slowdown bug
                effectSet.setEffect(FireFlies, 3);
                effectSet.runTime = 15000;  //set the effect runtime (ms)
            } else {
                //We could add more code here that would run while the effect is running
            }
        } break;
        case 8: {
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                Disolv = new DissolveSL(lineCWsegments, randPB3Pal, 3, 0, 80, 40);  //fully random colors
                effectSet.setEffect(Disolv, 3);
                effectSet.runTime = 10000;  //set the effect runtime (ms)
            } else {
                //We could add more code here that would run while the effect is running
            }
        } break;
        case 9: {
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                Twinkl2 = new Twinkle2SLSeg(ringUpperSegments, CRGB(222, 156, 42), 0, 12, 500, 5, 5, 5, 5, 2, 70);
                PaciH = new PacificaHueSL(ringLowerSegments, 40);
                effectSet.setEffect(PaciH, 3);
                effectSet.setEffect(Twinkl2, 4);
                effectSet.runTime = 15000;  //set the effect runtime (ms)
            } else {
                //We could add more code here that would run while the effect is running
            }
        } break;
        case 10: {
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                SeaShift = new ShiftingSeaSL(ringSegments, noise3Pal, 20, 0, 1, 1, 60);
                SeaShift->randomShift = true;
                effectSet.setEffect(SeaShift, 3); 
                effectSet.runTime = 20000; //set the effect runtime (ms)
            } else {
                //We could add more code here that would run while the effect is running
            }
        } break;
        case 11: {
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                Scan = new ScannerSL(ringSingleSet, randPB3Pal, 0, 1, 0, 5, 1, true, true, true, true, true, false, 100);
                Scan->randMode = 2;
                effectSet.setEffect(Scan, 3);
                effectSet.runTime = 10000;  //set the effect runtime (ms)
            } else {
                //We could add more code here that would run while the effect is running
            }
        } break;
        case 12: {
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                RollFa = new RollingWavesFastSL(ringSegments, 3, 0, 12, 1, 0, 80);
                RollFa->randMode = 1;
                effectSet.setEffect(RollFa, 3);
                effectSet.runTime = 15000;  //set the effect runtime (ms)
            } else {
                //We could add more code here that would run while the effect is running
            }
        } break;
        case 13: {
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                Parti = new ParticlesSL(ringSegments, randPB3Pal, 0, 5, 2, 60, 30, 1, 0, 0, 2, 6, 2, randPB3Pal.length, true);
                Parti->colorMode = 2;
                effectSet.setEffect(Parti, 3);
                effectSet.runTime = 15000;  //set the effect runtime (ms)
            } else {
                //We could add more code here that would run while the effect is running
            }
        } break;
        case 14: {
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                NoiseL = new NoiseSL(ringSegments, randPB3Pal, 20, 30, 60, 15, 0, 60);
                effectSet.setEffect(NoiseL, 3);
                effectSet.runTime = 20000;  //set the effect runtime (ms)
            } else {
                //We could add more code here that would run while the effect is running
            }
        } break;
        case 15: {
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                ColorFill = new ColorModeFillPS(ringSegments, 2, 40);  //rings
                ringSegments.offsetRateOrig = *ColorFill->rate;
                effectSet.setEffect(ColorFill, 3);
                effectSet.runTime = 10000;  //set the effect runtime (ms)
            } else {
                //We could add more code here that would run while the effect is running
            }
        } break;
        case 16: {
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                Strem = new StreamerSL(lineCWsegments, randPB2Pal, 0, 1, 2, 20, 20);
                effectSet.setEffect(Strem, 3);
                effectSet.runTime = 15000;  //set the effect runtime (ms)
            } else {
                //We could add more code here that would run while the effect is running
            }
        } break;
        case 17: {
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                Twinkl2 = new Twinkle2SLSeg(lineCWsegments, randPB2Pal, 0, 30, 500, 3, 3, 4, 4, 2, 70);
                effectSet.setEffect(Twinkl2, 3);
                effectSet.runTime = 15000;  //set the effect runtime (ms)
            } else {
                //We could add more code here that would run while the effect is running
            }
        } break;
        case 18: {
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                Edge = new EdgeBurstSL(ringSingleSet, 3, true, true, 22, 20);
                effectSet.setEffect(Edge, 3);
                effectSet.runTime = 15000;  //set the effect runtime (ms)
            } else {
                //We could add more code here that would run while the effect is running
            }
        } break;
        case 19: {
            //If we are just starting the effect, we need to add it to the effect set, and set a run time
            if(!effectSetup) {
                Pride2 = new PrideWPalSL2(ringSegments, true, true, 80);
                effectSet.setEffect(Pride2, 3);
                effectSet.runTime = 60000;  //set the effect runtime (ms)
            } else {
                //We could add more code here that would run while the effect is running
            }
        } break;
    }

    //In the switch statement above, we always setup an effect if needed (see the "default" case is shared with case 0).
    //So we can move the effectSetup flag handling code out of each switch case statement.
    //The code below is run once an effect is setup, so that we flag it not to setup again.
    if(!effectSetup) {
        effectSetup = true;
    }

    //With each cycle, we either want to keep updating the current effect,
    //or swap to the next one when the time is right
    if(effectSet.done) {
        //Once the current effect's time is up, we reset and cycle to the next effect
        switchEffect(true);  //advance to the next effect
    } else {
        yield();
        //Update the current effect (if effects are turned on)
        if(!effectsOff) {
            effectSet.update();
            //if(irrecv.isIdle()) {effectSet.update();}
        }
    }

    //Handle any IR button inputs
    if(irrecv.decode(&IRsig)) {
        handleIR();
        irrecv.resume();  //prep for the next value
    }
}

//callback routine for committing EEPROM data
//EEPROM has limited writes, so we want to commit all the writes after a delay
//this allows the data to change while the user is pressing buttons, and we'll only commit
//the final values once they are done
void IRAM_ATTR commitEEPROM() {
    EEPROM.commit();
}

/* 
The function below handles the IR button inputs.
Each input is bound to a specific IR signal. 
You can re-bind the signal values using the IR button mappings block at the top of the code.
Some buttons save their states in EEPROM for re-applying after booting.
There are 7 button functions:
    1: Brightness increase/decrease buttons. These increment/decrement the brightness levels through the values 
       in the brightness levels array. The brightness index is always saved so it can be re-applied on next boot.

    2: Next/previous effect buttons. Cycle forwards or backwards through the effect list. 
       Only saves if the effect cycling is off (the sphere is locked to the current effect)

    3: Effect cycle lock button. Locks or unlocks the effect cycling. When locked, the current effect is played indefinitely. 
       Both the locked/unlocked state and the current effect number are saved. 
       When locked, the current effect will resume upon re-boot. 
       Likewise, when locked, the next/previous effect buttons will also save the current effect.
    
    4: Effect reset button. Restarts the current effect (with new random inputs if applicable).

    5: Effects off Button. Turns the LEDs on/off and pauses/resumes the effect updating. This is basically a software off switch. 
       When resumed, the effect's cycle runtime is reset, so it will run for a full run time. 
       When the effects are off, all other button inputs are ignored. 
*/
void handleIR() {

    if(!effectsOff || IRsig.value == EFFECTS_OFF_BUT) {  //when the effects are turned off we want to ignore any button inputs besides turing the effects on

        switch(IRsig.value) {

            case BRI_INC_BUT: {  //increase brightness, doesn't wrap, so it caps at max index
                if(brightnessIndex + 1 < numBrightnessLevels) {
                    brightnessIndex++;
                }
                FastLED.setBrightness(brightnessLevels[brightnessIndex]);

                //save the new brightness to EEPROM to retain after shutdown
                EEPROM.write(BRIGHTNESS_ADDR, brightnessIndex);
            } break;

            case BRI_DEC_BUT: {  //decrease brightness, doesn't wrap, so it caps at 0
                if(brightnessIndex - 1 >= 0) {
                    brightnessIndex--;
                }
                FastLED.setBrightness(brightnessLevels[brightnessIndex]);

                //save the new brightness to EEPROM to retain after shutdown
                EEPROM.write(BRIGHTNESS_ADDR, brightnessIndex);
            } break;

            case NEXT_EFFECT_BUT: {  //cycle to the next effect
                switchEffect(true);
                //If we aren't cycling effects, save the effect number so it can resume on restart
                if(effectsLocked) {
                    //save the new effect number to EEPROM to retain after shutdown
                    EEPROM.write(CUR_EFFECT_ADDR, effectNum);
                }
            } break;

            case PREV_EFFECT_BUT: {  //cycle to the previous effect
                switchEffect(false);
                //If we aren't cycling effects, save the effect number so it can resume on restart
                if(effectsLocked) {
                    //save the new effect number to EEPROM to retain after shutdown
                    EEPROM.write(CUR_EFFECT_ADDR, effectNum);
                }
            } break;

            case EFFECT_LOCK_BUT: {  //lock/unlock the effect cycle
                effectsLocked = !effectsLocked;
                effectSet.infinite = !effectSet.infinite;  //set the effect set's infinite flag so the current effect runs forever
                //save the locked state to EEPROM and the current effect,
                //so it can be resumed after shutdown
                EEPROM.write(EFFECT_LOCK_ADDR, effectsLocked);
                EEPROM.write(CUR_EFFECT_ADDR, effectNum);
            } break;

            case EFFECT_RESET_BUT: {  //restart the current effect
                //the easiest way to reset the effect is to advance the effect cycle (also reseting and clearing the effect)
                //and then set the effect number back to the current
                uint16_t effectNumTemp = effectNum;
                switchEffect(true);
                effectNum = effectNumTemp;
            } break;

            case EFFECTS_OFF_BUT: {  //toggles effect's on/off
                effectsOff = !effectsOff;
                if(!effectsOff) {  //when effects are turned back on, we restart the current effect's timer, to avoid any skipping/flashing
                    effectSet.reset();
                } else {
                    segDrawUtils::turnSegSetOff(mainSegments);
                    FastLED.show();
                }
            } break;

            default: {  //reject any signals that don't match the listed buttons
                return;
            } break;
        }

        //stop any other commit timers and start a new one
        //so that we commit any changes to EEPROM at once
        //(accounting for multiple button presses)
        EEPROMcommitter.detach();
        EEPROMcommitter.once_ms(EEPROM_COM_TIME, commitEEPROM);
    }
}

//Switches to the next (direct = true) or previous (direct = false) effect,
//including all the resetting and prep for the next effect.
void switchEffect(bool direct) {
    //cycle the active effect number, with wrapping
    if(direct) {
        effectNum = (effectNum + 1) % NUM_EFFECTS;
    } else {
        //When cycling backward, we add NUM_EFFECTS so that the mod is always positive
        //this does not change the mod result
        effectNum = (NUM_EFFECTS + effectNum - 1) % NUM_EFFECTS;
    }

    //reset the state of the effect set and clean up for the next effect
    effectSetup = false;
    effectSet.destructEffsAftLim();  //de-allocate the temporary effects. This step is CRITICAL!
    effectSet.reset();
    segDrawUtils::turnSegSetOff(mainSegments);
    resetLoopSettings();
}

//for reseting any general settings changed for effect setup,
//such as segment directions, color mode settings, etc.
void resetLoopSettings() {
    ringSegments.offsetRateOrig = 30;
    lineCWsegments.offsetRateOrig = 30;
    ringUpperSegments.offsetRateOrig = 30;
    ringLowerSegments.offsetRateOrig = 30;

    ringSegments.runOffset = true;
    //mainSegments.runOffset = true;
    lineCWsegments.runOffset = true;
    ringUpperSegments.runOffset = true;
    ringLowerSegments.runOffset = true;

    ringSegments.gradPalette = &randPB3Pal;
    //mainSegments.gradPalette = &randPB3Pal;
    lineCWsegments.gradPalette = &randPB3Pal;
    ringUpperSegments.gradPalette = &randPB3Pal;
    ringLowerSegments.gradPalette = &randPB3Pal;
}
