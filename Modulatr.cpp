/*******************************************/
/*  Modulator Class, Perry R. Cook, 1995-96*/ 
/*  This Object combines random and        */
/*  periodic modulations to give a nice    */
/*  natural human modulation function.     */  
/*******************************************/

#define POLE_POS  0.999
#define RND_SCALE 10.0

#include "Modulatr.h"

Modulatr :: Modulatr()
{
    vibwave = new RawWave("rawwaves/sinewave.raw");
    vibwave->normalize();
    vibwave->setFreq(6.0);
    vibwave->setLooping(1);
    vibAmt = 0.04;
    noise = new SubNoise(330);
    rndAmt = 0.005;
    onepole = new OnePole;
    onepole->setPole(POLE_POS);
    onepole->setGain(rndAmt * RND_SCALE);
}

Modulatr :: ~Modulatr()
{
    delete vibwave;
    delete noise;
    delete onepole;
}

void Modulatr :: reset()
{
    lastOutput = 0.0;
}

void Modulatr :: setVibFreq(double vibFreq)
{
    vibwave->setFreq(vibFreq);
}

void Modulatr :: setVibAmt(double vibAmount)
{
    vibAmt = vibAmount;
}

void Modulatr :: setRndAmt(double rndAmount)
{
    rndAmt = rndAmount;
    onepole->setGain(RND_SCALE * rndAmt);
}

double Modulatr ::  tick()
{
    lastOutput = vibAmt * vibwave->tick();       /*  Compute periodic and */
    lastOutput += onepole->tick(noise->tick());  /*   random modulations  */
    return lastOutput;                        
}

double Modulatr :: lastOut()
{
    return lastOutput;
}

/************   Test Main Program   *****************/
/*
void main()
{
    Modulatr testMod;
    FILE *fd;
    short data;
    long i;
    
    fd = fopen("test.raw","wb");
    
    for (i=0;i<20000;i++) {
	data = testMod.tick() * 32000.0;
	fwrite(&data,2,1,fd);
    }
    fclose(fd);
}
*/
