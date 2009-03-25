 /******************************************/  
/*  Karplus-Strong plucked string model   */
/*  by Perry Cook, 1995-96                */
/*					  */
/*  There exist at least two patents,     */
/*  assigned to Stanford, bearing the     */
/*  names of Karplus and/or Strong.       */
/******************************************/

#include "Plucked.h"

Plucked :: Plucked(MY_FLOAT lowestFreq)
{
    length = (long) (SRATE / lowestFreq + 1);
    loopGain = 0.999;
    delayLine = new DLineA(length);
    loopFilt = new OneZero;
    pickFilt = new OnePole;
    noise = new Noise;
    this->clear();
}

Plucked :: ~Plucked()
{
    delete delayLine;
    delete loopFilt;
    delete pickFilt;
    delete noise;
}

void Plucked :: clear()
{
    delayLine->clear();
    loopFilt->clear();
    pickFilt->clear();
}

void Plucked :: setFreq(MY_FLOAT frequency)
{
    MY_FLOAT delay;
    delay = ((MY_FLOAT) SRATE / frequency) - 0.5;   /* length - delays */
    delayLine->setDelay(delay);
    loopGain = 0.995 + (frequency * 0.000005);
    if (loopGain>1.0) loopGain = 0.99999;
}

void Plucked :: pluck(MY_FLOAT amplitude)
{
    long i;
    pickFilt->setPole(0.999 - (amplitude*0.15));
    pickFilt->setGain(amplitude * 0.5);
    for (i=0;i<length;i++)                          
	delayLine->tick(delayLine->lastOut() * 0.6  /* fill delay with noise    */
		  + pickFilt->tick(noise->tick())); /* additively with current  */
						    /* contents                 */
}

void Plucked :: noteOn(MY_FLOAT freq, MY_FLOAT amp)
{
    this->setFreq(freq);
    this->pluck(amp);
#if defined(_debug_)        
    printf("Plucked : NoteOn: Freq=%lf Amp=%lf\n",freq,amp);
#endif    
}

void Plucked :: noteOff(MY_FLOAT amp)
{
    loopGain = 1.0 - amp;
#if defined(_debug_)        
    printf("Plucked : NoteOff: Amp=%lf\n",amp);
#endif    
}

MY_FLOAT Plucked :: tick()
{
    lastOutput = delayLine->tick(       /* check this out,           */
	       loopFilt->tick(          /* here's the whole inner    */
		delayLine->lastOut()    /* loop of the instrument!!  */
			* loopGain)); 
    lastOutput *= 3.0;
    return lastOutput;
}

