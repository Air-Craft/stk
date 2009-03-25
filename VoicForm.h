/*******************************************/
/*  4 Formant Synthesis Instrument         */
/*  by Perry R. Cook, 1995-96              */ 
/*  This instrument contains an excitation */
/*  singing wavetable (looping wave with   */
/*  random and periodic vibrato, smoothing */
/*  on frequency, etc.), excitation noise, */
/*  and four sweepable complex resonances. */
/*					   */
/*  Measured Formant data (from me) is     */
/*  included, and enough data is there to  */
/*  support either parallel or cascade     */
/*  synthesis.  In the floating point case */
/*  cascade synthesis is the most natural  */
/*  so that's what you'll find here.       */
/*					   */
/*  For right now, there's a simple command*/
/*  line score interface consisting of 3   */
/*  letter symbols for the phonemes, =xx   */
/*  sets the pitch to x, + and - add and   */
/*  subtract a half step, and ... makes it */
/*  keep doing what it's doing for longer. */
/*******************************************/

#if !defined(__VoicForm_h)
#define __VoicForm_h

#include "Instrmnt.h"
#include "Envelope.h"
#include "Noise.h"
#include "SingWave.h"
#include "FormSwep.h"
#include "OnePole.h"
#include "OneZero.h"

class VoicForm : public Instrmnt
{
  protected:  
    SingWave *voiced;
    Noise    *noise;
    Envelope *noiseEnv;
    FormSwep  *filters[4];
    OnePole  *onepole;
    OneZero  *onezero;
  public:
    VoicForm();
    ~VoicForm();
    void clear();
    void setFreq(double frequency);
    void setFormantAll(int whichOne, double freq, double reson, double gain);
    int  setPhoneme(char* phoneme);
    void setVoiced(double vGain);
    void setUnVoiced(double nGain);
    void setVoicedUnVoiced(double vGain, double nGain);
    void setFiltSweepRate(int whichOne,double rate);
    void setPitchSweepRate(double rate);
    void speak();
    void quiet();
    virtual void noteOn(MY_FLOAT freq, MY_FLOAT amp);
    virtual void noteOff(MY_FLOAT amp);
    MY_FLOAT tick();
    virtual void controlChange(int number, MY_FLOAT value);
};

#endif
