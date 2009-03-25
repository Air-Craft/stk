/***************************************************/
/*! \class Shakers
    \brief PhISEM and PhOLIES class.

    PhISEM (Physically Informed Stochastic Event
    Modeling) is an algorithmic approach for
    simulating collisions of multiple independent
    sound producing objects.  This class is a
    meta-model that can simulate a Maraca, Sekere,
    Cabasa, Bamboo Wind Chimes, Water Drops,
    Tambourine, Sleighbells, and a Guiro.

    PhOLIES (Physically-Oriented Library of
    Imitated Environmental Sounds) is a similar
    approach for the synthesis of environmental
    sounds.  This class implements simulations of
    breaking sticks, crunchy snow (or not), a
    wrench, sandpaper, and more.

    Control Change Numbers: 
      - Shake Energy = 2
      - System Decay = 4
      - Number Of Objects = 11
      - Resonance Frequency = 1
      - Shake Energy = 128
      - Instrument Selection = 1071
        - Maraca = 0
        - Cabasa = 1
        - Sekere = 2
        - Guiro = 3
        - Water Drops = 4
        - Bamboo Chimes = 5
        - Tambourine = 6
        - Sleigh Bells = 7
        - Sticks = 8
        - Crunch = 9
        - Wrench = 10
        - Sand Paper = 11
        - Coke Can = 12
        - Next Mug = 13
        - Penny + Mug = 14
        - Nickle + Mug = 15
        - Dime + Mug = 16
        - Quarter + Mug = 17
        - Franc + Mug = 18
        - Peso + Mug = 19
        - Big Rocks = 20
        - Little Rocks = 21
        - Tuned Bamboo Chimes = 22

    by Perry R. Cook, 1996 - 1999.
*/
/***************************************************/

#if !defined(__SHAKERS_H)
#define __SHAKERS_H

#include "Instrmnt.h"

#define MAX_FREQS 8
#define NUM_INSTR 24

class Shakers : public Instrmnt
{
 public:
  //! Class constructor.
  Shakers();

  //! Class destructor.
  ~Shakers();

  //! Start a note with the given instrument and amplitude.
  /*!
    Use the instrument numbers above, converted to frequency values
    as if MIDI note numbers, to select a particular instrument.
  */
  virtual void noteOn(MY_FLOAT instrument, MY_FLOAT amplitude);

  //! Stop a note with the given amplitude (speed of decay).
  virtual void noteOff(MY_FLOAT amplitude);

  //! Compute one output sample.
  MY_FLOAT tick();

  //! Perform the control change specified by \e number and \e value (0.0 - 128.0).
  virtual void controlChange(int number, MY_FLOAT value);

 protected:

  int setupName(char* instr);
  int setupNum(int inst);
  int setFreqAndReson(int which, MY_FLOAT freq, MY_FLOAT reson);
  void setDecays(MY_FLOAT sndDecay, MY_FLOAT sysDecay);
  void setFinalZs(MY_FLOAT z0, MY_FLOAT z1, MY_FLOAT z2);
  MY_FLOAT wuter_tick();
  MY_FLOAT tbamb_tick();
  MY_FLOAT ratchet_tick();

  int instType;
  int ratchetPos, lastRatchetPos;
  MY_FLOAT shakeEnergy;
  MY_FLOAT inputs[MAX_FREQS];
  MY_FLOAT outputs[MAX_FREQS][2];
  MY_FLOAT coeffs[MAX_FREQS][2];
  MY_FLOAT sndLevel;
  MY_FLOAT baseGain;
  MY_FLOAT gains[MAX_FREQS];
  int nFreqs;
  MY_FLOAT t_center_freqs[MAX_FREQS];
  MY_FLOAT center_freqs[MAX_FREQS];
  MY_FLOAT resons[MAX_FREQS];
  MY_FLOAT freq_rand[MAX_FREQS];
  int freqalloc[MAX_FREQS];
  MY_FLOAT soundDecay;
  MY_FLOAT systemDecay;
  MY_FLOAT nObjects;
  MY_FLOAT collLikely;
  MY_FLOAT totalEnergy;
  MY_FLOAT ratchet,ratchetDelta;
  MY_FLOAT finalZ[3];
  MY_FLOAT finalZCoeffs[3];
  MY_FLOAT defObjs[NUM_INSTR];
  MY_FLOAT defDecays[NUM_INSTR];
  MY_FLOAT decayScale[NUM_INSTR];

};

#endif
