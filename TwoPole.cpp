/*******************************************/
/*  Two Pole Filter Class,                 */
/*  by Perry R. Cook, 1995-96              */ 
/*  See books on filters to understand     */
/*  more about how this works.  Nothing    */
/*  out of the ordinary in this version.   */
/*******************************************/

#include "TwoPole.h"

TwoPole :: TwoPole() : Filter()
{
    outputs = (MY_FLOAT *) malloc(2 * MY_FLOAT_SIZE);
    poleCoeffs[0] = 0.0;
    poleCoeffs[1] = 0.0;
    gain = 1.0;
    this->clear();
}

TwoPole :: ~TwoPole()
{
//    free(inputs);
    free(outputs);
}

void TwoPole :: clear()
{
    outputs[0] = 0.0;
    outputs[1] = 0.0;
    lastOutput = 0.0;
}

void TwoPole :: setPoleCoeffs(MY_FLOAT *coeffs)
{
    poleCoeffs[0] = coeffs[0];
    poleCoeffs[1] = coeffs[1];
}

void TwoPole :: setFreqAndReson(MY_FLOAT freq, MY_FLOAT reson)
{
    poleCoeffs[1] = - (reson * reson);
    poleCoeffs[0] = 2.0 * reson * cos(TWO_PI * freq / SRATE);
}

void TwoPole :: setGain(MY_FLOAT aValue)
{
    gain = aValue;
}

MY_FLOAT TwoPole :: tick(MY_FLOAT sample) /*   Perform Filter Operation  */
{                                         /*  TwoPole is a two pole filter (duh!) */
    MY_FLOAT temp;                        /*  Look it up in your favorite DSP text */
    temp = sample * gain;
    temp += poleCoeffs[0] * outputs[0];
    temp += poleCoeffs[1] * outputs[1];
    outputs[1] = outputs[0];
    outputs[0] = temp;
    lastOutput = outputs[0];
    return lastOutput;
}

