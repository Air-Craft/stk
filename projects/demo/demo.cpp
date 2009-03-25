// demo.cpp
//
// An example STK program that allows voice playback and control of
// most of the STK instruments.

#include "SKINI.msg"
#include "WvOut.h"
#include "Instrmnt.h"
#include "PRCRev.h"
#include "Voicer.h"
#include "Skini.h"

#if defined(__STK_REALTIME__)
  #include "RtAudio.h"
  #include "Mutex.h"
#endif

// Miscellaneous command-line parsing and instrument allocation
// functions are defined in utilites.cpp ... specific to this program.
#include "utilities.h"

#include <signal.h>
#include <iostream>
#include <algorithm>
#if !defined(__OS_WINDOWS__) // Windoze bogosity for VC++ 6.0
  using std::min;
#endif

bool done;
static void finish(int ignore){ done = true; }

// The TickData structure holds all the class instances and data that
// are shared by the various processing functions.
struct TickData {
  WvOut **wvout;
  Instrmnt **instrument;
  Voicer *voicer;
  Effect *reverb;
  Messager messager;
  Skini::Message message;
  StkFloat volume;
  StkFloat t60;
  unsigned int nWvOuts;
  int nVoices;
  int currentVoice;
  int channels;
  int counter;
  bool realtime;
  bool settling;
  bool haveMessage;

  // Default constructor.
  TickData()
    : wvout(0), instrument(0), voicer(0), reverb(0), volume(1.0), t60(1.0),
      nWvOuts(0), nVoices(1), currentVoice(0), channels(2), counter(0),
      realtime( false ), settling( false ), haveMessage( false ) {}
};

#define DELTA_CONTROL_TICKS 64 // default sample frames between control input checks

// The processMessage() function encapsulates the handling of control
// messages.  It can be easily relocated within a program structure
// depending on the desired scheduling scheme.
void processMessage( TickData* data )
{
  register StkFloat value1 = data->message.floatValues[0];
  register StkFloat value2 = data->message.floatValues[1];

  switch( data->message.type ) {

  case __SK_Exit_:
    if ( data->settling == false ) goto settle;
    done = true;
    return;

  case __SK_NoteOn_:
    if ( value2 == 0.0 ) // velocity is zero ... really a NoteOff
      data->voicer->noteOff( value1, 64.0 );
    else // a NoteOn
      data->voicer->noteOn( value1, value2 );
    break;

  case __SK_NoteOff_:
    data->voicer->noteOff( value1, value2 );
    break;

  case __SK_ControlChange_:
    if (value1 == 44.0)
      data->reverb->setEffectMix(value2  * ONE_OVER_128);
    else if (value1 == 7.0)
      data->volume = value2 * ONE_OVER_128;
    else if (value1 == 49.0)
      data->voicer->setFrequency( value2 );
    else
      data->voicer->controlChange( (int) value1, value2 );
    break;

  case __SK_AfterTouch_:
    data->voicer->controlChange( 128, value1 );
    break;

  case __SK_PitchChange_:
    data->voicer->setFrequency( value1 );
    break;

  case __SK_PitchBend_:
    data->voicer->pitchBend( value1 );
    break;

  case __SK_Volume_:
    data->volume = value1 * ONE_OVER_128;
    break;

  case __SK_ProgramChange_:
    if ( data->currentVoice == (int) value1 ) break;

    // Two-stage program change process.
    if ( data->settling == false ) goto settle;

    // Stage 2: delete and reallocate new voice(s)
    for ( int i=0; i<data->nVoices; i++ ) {
      data->voicer->removeInstrument( data->instrument[i] );
      delete data->instrument[i];
      data->currentVoice = voiceByNumber( (int)value1, &data->instrument[i] );
      if ( data->currentVoice < 0 )
        data->currentVoice = voiceByNumber( 0, &data->instrument[i] );
      data->voicer->addInstrument( data->instrument[i] );
      data->settling = false;
    }

  } // end of switch

  data->haveMessage = false;
  return;

 settle:
  // Exit and program change messages are preceeded with a short settling period.
  data->voicer->silence();
  data->counter = (int) (0.3 * data->t60 * Stk::sampleRate());
  data->settling = true;
}


// The tick() function handles sample computation and scheduling of
// control updates.  If doing realtime audio output, it will be called
// automatically when the system needs a new buffer of audio samples.
int tick(char *buffer, int bufferSize, void *dataPointer)
{
  TickData *data = (TickData *) dataPointer;
  register StkFloat sample, *samples = (StkFloat *) buffer;
  int counter, nTicks = bufferSize;

  while ( nTicks > 0 && !done ) {

    if ( !data->haveMessage ) {
      data->messager.popMessage( data->message );
      if ( data->message.type > 0 ) {
        data->counter = (long) (data->message.time * Stk::sampleRate());
        data->haveMessage = true;
      }
      else
        data->counter = DELTA_CONTROL_TICKS;
    }

    counter = min( nTicks, data->counter );
    data->counter -= counter;
    for ( int i=0; i<counter; i++ ) {
      sample = data->volume * data->reverb->tick( data->voicer->tick() );
      for ( unsigned int j=0; j<data->nWvOuts; j++ ) data->wvout[j]->tick(sample);
      if ( data->realtime )
        for ( int k=0; k<data->channels; k++ ) *samples++ = sample;
      nTicks--;
    }
    if ( nTicks == 0 ) break;

    // Process control messages.
    if ( data->haveMessage ) processMessage( data );
  }

  return 0;
}

int main( int argc, char *argv[] )
{
  TickData data;
  int i;

#if defined(__STK_REALTIME__)
  RtAudio *dac = 0;
#endif

  // If you want to change the default sample rate (set in Stk.h), do
  // it before instantiating any objects!  If the sample rate is
  // specified in the command line, it will override this setting.
  Stk::setSampleRate( 44100.0 );

  // By default, warning messages are not printed.  If we want to see
  // them, we need to specify that here.
  Stk::showWarnings( true );

  // Check the command-line arguments for errors and to determine
  // the number of WvOut objects to be instantiated (in utilities.cpp).
  data.nWvOuts = checkArgs(argc, argv);
  data.wvout = (WvOut **) calloc(data.nWvOuts, sizeof(WvOut *));

  // Instantiate the instrument(s) type from the command-line argument
  // (in utilities.cpp).
  data.nVoices = countVoices(argc, argv);
  data.instrument = (Instrmnt **) calloc(data.nVoices, sizeof(Instrmnt *));
  data.currentVoice = voiceByName(argv[1], &data.instrument[0]);
  if ( data.currentVoice < 0 ) {
    free( data.wvout );
    free( data.instrument );
    usage(argv[0]);
  }
  // If there was no error allocating the first voice, we should be fine for more.
  for ( i=1; i<data.nVoices; i++ )
    voiceByName(argv[1], &data.instrument[i]);

  data.voicer = (Voicer *) new Voicer( data.nVoices );
  for ( i=0; i<data.nVoices; i++ )
    data.voicer->addInstrument( data.instrument[i] );

  // Parse the command-line flags, instantiate WvOut objects, and
  // instantiate the input message controller (in utilities.cpp).
  try {
    data.realtime = parseArgs(argc, argv, data.wvout, data.messager);
  }
  catch (StkError &) {
    goto cleanup;
  }

  // If realtime output, allocate the dac here.
#if defined(__STK_REALTIME__)
  if ( data.realtime ) {
    RtAudioFormat format = ( sizeof(StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
    int bufferSize = RT_BUFFER_SIZE;
    try {
      dac = new RtAudio(0, data.channels, 0, 0, format, (int)Stk::sampleRate(), &bufferSize, 4);
    }
    catch (RtError& error) {
      error.printMessage();
      goto cleanup;
    }
  }
#endif

  // Set the reverb parameters.
  data.reverb = new PRCRev( data.t60 );
  data.reverb->setEffectMix(0.2);

  // Install an interrupt handler function.
	(void) signal(SIGINT, finish);

  // If realtime output, set our callback function and start the dac.
#if defined(__STK_REALTIME__)
  if ( data.realtime ) {
    try {
      dac->setStreamCallback(&tick, (void *)&data);
      dac->startStream();
    }
    catch (RtError &error) {
      error.printMessage();
      goto cleanup;
    }
  }
#endif

  // Setup finished.
  while ( !done ) {
#if defined(__STK_REALTIME__)
    if ( data.realtime )
      // Periodically check "done" status.
      Stk::sleep( 200 );
    else
#endif
      // Call the "tick" function to process data.
      tick( NULL, 256, (void *)&data );
  }

  // Shut down the callback and output stream.
#if defined(__STK_REALTIME__)
  if ( data.realtime ) {
    try {
      dac->cancelStreamCallback();
      dac->closeStream();
    }
    catch (RtError& error) {
      error.printMessage();
    }
  }
#endif

 cleanup:

  for ( i=0; i<(int)data.nWvOuts; i++ ) delete data.wvout[i];
  free( data.wvout );

#if defined(__STK_REALTIME__)
  delete dac;
#endif
  delete data.reverb;
  delete data.voicer;

  for ( i=0; i<data.nVoices; i++ ) delete data.instrument[i];
  free( data.instrument );

	std::cout << "\nStk demo finished ... goodbye.\n\n";
  return 0;
}

