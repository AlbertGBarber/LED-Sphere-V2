//========================================================================
//                          Segment Sets Setups
//========================================================================
//Segment sets re-arrange the LEDs virtually, allowing us to create complex 2D shapes.
//See Pixel Spork wiki for more info on segment sets.

//-----------------------------------------------------
//As connected:
    //Here the LEDs are arranged to match how they are physically connected in a single line.

    const PROGMEM segmentSecCont mainSec[] = { {0, NUM_LEDS} };
    SegmentPS mainSegment = { mainSec, SIZE(mainSec), true };
    SegmentPS *main_arr[] = { &mainSegment };
    SegmentSetPS mainSegments = {leds, NUM_LEDS, main_arr, SIZE(main_arr)};

//-----------------------------------------------------
//Continuous line spinning clockwise (from below), from lower rings to upper rings
//With the leds arranged as they are laid out, the upper rings are flipped verses the lower rings,
//so the LEDs move clockwise around the lower rings, but counter clockwise around the upper.
//This segment set reverses the uppers, so that they are also a clockwise line.
//Note that to do this, we need to split the upper rings into their first pixels (which align with the lower rings),
//and the rest of the ring, which we give a negative length to so that it counts backwards 
//(see the Segment Basics and Advanced segment sections of the Pixel Spork wiki)

//                                               -lower-  -----24 upper----   ------16 upper----  ------12 upper-----   ------8 upper------
    const PROGMEM segmentSecCont lineCWSec[] = { {0, 60}, {60, 1}, {83, -23}, {84, 1}, {99, -15}, {100, 1}, {111, -11}, {112, 1}, {119, -7} };
    SegmentPS lineCWSegment = { lineCWSec, SIZE(lineCWSec), true };
    SegmentPS *lineCW_arr[] = { &lineCWSegment };
    SegmentSetPS lineCWsegments = {leds, NUM_LEDS, lineCW_arr, SIZE(lineCW_arr)};

//-----------------------------------------------------
//Rings: 
    //The LEDs are arranged into rings around the sphere, from bottom to top
    //The rings are set so that they aligned in the same direction

    //lower set of rings, these are just the LED rings
    const PROGMEM segmentSecCont ringSec0[] = { {0, 8} };           //Ring 0, 8 LEDs
    SegmentPS ringSegment0 = {ringSec0, SIZE(ringSec0), true};

    const PROGMEM segmentSecCont ringSec1[] = { {8, 12} };          //Ring 1, 12 LEDs
    SegmentPS ringSegment1 = {ringSec1, SIZE(ringSec1), true};

    const PROGMEM segmentSecCont ringSec2[] = { {20, 16} };         //Ring 2, 16 LEDs
    SegmentPS ringSegment2 = {ringSec2, SIZE(ringSec2), true};

    const PROGMEM segmentSecCont ringSec3[] = { {36, 24} };         //Ring 3, 24 LEDs
    SegmentPS ringSegment3 = {ringSec3, SIZE(ringSec3), true};

    //Upper set of rings, because these rings are flipped verses the lower set,
    //we need to lay them out so that they mirror the lowers.
    //This requires reversing the rings direction, while shifting the first LED to the end of each ring
    //so that the upper and lower rings start points line up
    const PROGMEM segmentSecCont ringSec4[] = { {61, 23}, {60, 1} };   //Ring 4, 24 LEDs (reversed)
    SegmentPS ringSegment4 = {ringSec4, SIZE(ringSec4), false};

    const PROGMEM segmentSecCont ringSec5[] = { {85, 15}, {84, 1} };   //Ring 5, 16 LEDs (reversed)
    SegmentPS ringSegment5 = {ringSec5, SIZE(ringSec5), false};

    const PROGMEM segmentSecCont ringSec6[] = { {101, 11}, {100, 1} };  //Ring 6, 12 LEDs (reversed)
    SegmentPS ringSegment6 = {ringSec6, SIZE(ringSec6), false};

    const PROGMEM segmentSecCont ringSec7[] = { {113, 7}, {112, 1} };  //Ring 7, 8 LEDs (reversed)
    SegmentPS ringSegment7 = {ringSec7, SIZE(ringSec7), false};


    //the ring segments are grouped together into one segment set, so that effects can act across all of them
    SegmentPS *rings_arr[] = { &ringSegment0, &ringSegment1, &ringSegment2, &ringSegment3, 
                               &ringSegment4, &ringSegment5, &ringSegment6, &ringSegment7 };
    SegmentSetPS ringSegments = {leds, NUM_LEDS, rings_arr, SIZE(rings_arr)};

//-----------------------------------------------------
//Rings, but split into upper and lower halves
//This just splits the rings segment set above into two sets, one for the upper rings, and one for the lower,
//which allows you to run different effects on the upper and lower halves.
//The halves are mirrored about the sphere's center, so that the 24 pixel rings are the first segment of each set
//(note that to make the sets, we can just reuse the segment definitions from the Rings segment set above)

    //upper rings segment set
    SegmentPS *ringsUpper_arr[] = { &ringSegment4, &ringSegment5, &ringSegment6, &ringSegment7 };
    SegmentSetPS ringUpperSegments = {leds, NUM_LEDS, ringsUpper_arr, SIZE(ringsUpper_arr)};

    //lower rings segment set
    SegmentPS *ringsLower_arr[] = { &ringSegment3, &ringSegment2, &ringSegment1, &ringSegment0 };
    SegmentSetPS ringLowerSegments = {leds, NUM_LEDS, ringsLower_arr, SIZE(ringsLower_arr)};

//-----------------------------------------------------
//The rings, but arranged into a "single" segment, where each ring is treated as 
//a single LED, whose color is copied to the rest of the ring
//The default direction is from top to bottom
    const PROGMEM segmentSecCont ringSingleSec[] = { {0, 8, true},   {8, 12, true},  {20, 16, true},  {36, 24, true},    //<--lower rings
                                                     {60, 24, true}, {84, 16, true}, {100, 12, true}, {112, 8, true} }; //<-- upper rings

    SegmentPS ringSingleSeg = { ringSingleSec, SIZE(ringSingleSec), false }; //set default direction

    SegmentPS *ringsSingleSeg_arr[] = { &ringSingleSeg };
    SegmentSetPS ringSingleSet = {leds, NUM_LEDS, ringsSingleSeg_arr, SIZE(ringsSingleSeg_arr)};

//-----------------------------------------------------



