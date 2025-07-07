//--------------------------------------------------
//     Effects Depot For Unused Effects
//---------------------------------------------------

/*
If you want to use these effects, you may also have to uncomment 
the effect pointer in the "Effects Names" section of the main code.

Lav = new LavaPS(lineCWsegments, lavaPal, 20, 40, 10); 
Lav = new LavaPS(lineCWsegments, randPB3Pal, 20, 40, 20); 
effectSet.setEffect(Lav, 3);

Parti = new ParticlesSL(lineCWsegments, randPB3Pal, 0, 6, 2, 50, 40, 1, 0, 1, 2, 6, 2, randPB3Pal.length, true);
Parti->colorMode = 8;
effectSet.setEffect(Parti, 3);

rainSL = new RainSL(ringUpperSegments, randPB3Pal, 0, true, 8, 1, 1, 0, 2, 120, false);
rainSL2 = new RainSL(ringLowerSegments, randPB3Pal, 0, true, 8, 1, 1, 0, 2, 120, false);
effectSet.setEffect(rainSL, 3);
effectSet.setEffect(rainSL2, 4);

ColorFill = new ColorModeFillPS(ringUpperSegments, 5, 60); //out of sync solid rainbows
ColorFill2 = new ColorModeFillPS(ringLowerSegments, 4, 60);
ringUpperSegments.runOffset = false;
ringLowerSegments.runOffset = false;
effectSet.setEffect(ColorFill, 3);
effectSet.setEffect(ColorFill2, 4);

ColorFill = new ColorModeFillPS(lineCWsegments, 6, 30); //gradient one line
lineCWsegments.offsetRateOrig = *ColorFill->rate;

ColorFill = new ColorModeFillPS(ringSegments, 8, 20); //gradient lines
ringSegments.offsetRateOrig = *ColorFill->rate;

ColorFill = new ColorModeFillPS(ringUpperSegments, 7, 30); //gradient rings double
ColorFill2 = new ColorModeFillPS(ringLowerSegments, 7, 30);
ringUpperSegments.offsetRateOrig = *ColorFill->rate;
ringLowerSegments.offsetRateOrig = *ColorFill2->rate;
effectSet.setEffect(ColorFill, 3);
effectSet.setEffect(ColorFill2, 4);

Edge = new EdgeBurstSL(ringSingleSet, 3, true, true, 20, 60);
effectSet.setEffect(Edge, 3);

Strem = new StreamerSL(ringSegments, randPB2Pal, 0, 3, 3, 25, 20); //meh
Strem->colorMode = 3; //?
effectSet.setEffect(Strem, 3);

NoiseW = new NoiseWavesSL(lineCWsegments, randPB2Pal, 0, 30, 8, 3, 60); //meh
effectSet.setEffect(NoiseW, 3);

NoiseG = new NoiseGradSL(ringSegments, randPB3Pal, 0, 3, 5, 10, 10, 30, 2000, 80); //meh
effectSet.setEffect(NoiseG, 3);
*/