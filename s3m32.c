#define S32MINC 1
#ifdef S32MINC
#include <stdbool.h>
#include <stdint.h>
#include "stdlib.h"
#include <string.h>
//#include "inc/hw_types.h"
//#include "driverlib/rom.h" //only needed for debug
//#include "driverlib/gpio.h"
//#include "utils/uartstdio.h"

#include "global.h"
#include "s3m32.h"

//#include "ff.h"

//#include "debughlpr.h"

#define min(X,Y) ((X) < (Y) ? (X) : (Y))

//#define CHECKMEMORY {if(file.size!=19367){ROM_IntMasterDisable(); UARTprintf("uhOh (%s) size: %d at:%d\n",__FILE__,file.size,__LINE__); while(1){}}}
//#define CHECKMEMORY CHECKMEMORY_E

// Amiga periods
static const uint16_t amigaPeriods[110] = {
 27392, 25856, 24384, 23040, 21696, 20480, 19328, 18240, 17216, 16256, 15360, 14496, // 0
 13696, 12928, 12192, 11520, 10848, 10240,  9664,  9120,  8608,  8128,  7680,  7248, // 1
  6848,  6464,  6096,  5760,  5424,  5120,  4832,  4560,  4304,  4064,  3840,  3624, // 2
  3424,  3232,  3048,  2880,  2712,  2560,  2416,  2280,  2152,  2032,  1920,  1812, // 3
  1712,  1616,  1524,  1440,  1356,  1280,  1208,  1140,  1076,  1016,   960,   906, // 4
   856,   808,   762,   720,   678,   640,   604,   570,   538,   508,   480,   453, // 5
   428,   404,   381,   360,   339,   320,   302,   285,   269,   254,   240,   226, // 6
   214,   202,   190,   180,   170,   160,   151,   143,   135,   127,   120,   113, // 7
   107,   101,    95,    90,    85,    80,    75,    71,    67,    63,    60,    56, // 8
     0,     0                                                                        // NONOTE, KEYOFF
};

static const uint16_t fineTuneToHz[16] = {
 8363, 8413, 8463, 8529, 8581, 8651, 8723, 8757,
 7895, 7941, 7985, 8046, 8107, 8169, 8232, 8280
};

static const uint8_t sine[64] = {
   0,  24,  49,  74,  97, 120, 141, 161,
 180, 197, 212, 224, 235, 244, 250, 253,
 255, 253, 250, 244, 235, 224, 212, 197,
 180, 161, 141, 120,  97,  74,  49,  24
};

static uint16_t wordlh(uint8_t l, uint8_t h) {
 return h << 8 | l;
}


static void loadHeader() {
 uint32_t count;
 uint8_t i;
 uint16_t songLength;
 uint16_t numberOfPatterns;
 uint16_t temp16;
 uint8_t masterVolume;
 uint8_t defaultPanning;
 uint8_t temp8;

 f_read(&file, uMod.S3m.name, 28, &count);
 //strcpy(currentsongname, uMod.S3m.name);
 f_lseek(&file, 0x20);

 f_read(&file, &songLength, 2, &count);
 f_read(&file, &uMod.S3m.numberOfInstruments, 2, &count);
 f_read(&file, &numberOfPatterns, 2, &count);


 uMod.S3m.fastVolumeSlides = false;
 f_read(&file, &temp16, 2, &count);                       // Flags
 if(temp16 & 64) uMod.S3m.fastVolumeSlides = true;             // Fast volume slides flag
 f_read(&file, &temp16, 2, &count);                       // Created with tracker / version
 if((temp16 & 0xFFF) == 300) uMod.S3m.fastVolumeSlides = true; // Fast volume slides for Scream Tracker 3.00

 f_lseek(&file, 0x30);

 f_read(&file, &uMod.S3m.globalVolume, 1, &count);
 f_read(&file, &uPlayer.S3m_player.speed, 1, &count);
 f_read(&file, &temp8, 1, &count);
 uPlayer.S3m_player.samplesPerTick = SAMPLERATE / (2 * temp8 / 5);    // Hz = 2 * BPM / 5
 f_read(&file, &masterVolume, 1, &count);                 // Check bit 8 later
 f_read(&file, &temp8, 1, &count);                        // Skip ultraclick removal
 f_read(&file, &defaultPanning, 1, &count);

 f_lseek(&file, 0x40);
 // Find the number of channels and remap the used channels linearly
 uMod.S3m.numberOfChannels = 0;
 memset(uMod.S3m.channelRemapping, 255, CHANNELS);
 for(i = 0; i < CHANNELS; i++) {
  f_read(&file, &temp8, 1, &count);
  if(temp8 < 16) {
   uMod.S3m.channelRemapping[i] = uMod.S3m.numberOfChannels;
   if(temp8 < 8)
    Mixer.channelPanning[uMod.S3m.numberOfChannels] = 0x3;
   else
    Mixer.channelPanning[uMod.S3m.numberOfChannels] = 0xC;
   uMod.S3m.numberOfChannels++;
  }
 }

 f_lseek(&file, 0x60);

 // Load order data
 f_read(&file, uMod.S3m.order, songLength, &count);
 // Calculate number of physical patterns
 uMod.S3m.songLength = 0;
 uMod.S3m.numberOfPatterns = 0;
 for(i = 0; i < songLength; i++) {
  uMod.S3m.order[uMod.S3m.songLength] = uMod.S3m.order[i];
  if(uMod.S3m.order[i] < 254) {
   uMod.S3m.songLength++;
   if(uMod.S3m.order[i] > uMod.S3m.numberOfPatterns)
    uMod.S3m.numberOfPatterns = uMod.S3m.order[i];
  }
 }
 // Load parapointers
 f_read(&file, uMod.S3m.instrumentParapointers, uMod.S3m.numberOfInstruments * 2, &count);
 f_read(&file, uMod.S3m.patternParapointers, numberOfPatterns * 2, &count);
 // If the panning flag is set then set default panning
 if(defaultPanning == 0xFC) {
  for(i = 0; i < CHANNELS; i++) {
   f_read(&file, &temp8, 1, &count);
   //R.K. added the uMod.S3m.channelRemapping[i]<CHANNELS because otherwise we would write to
   //an unknown memory
   if((temp8 & 0x20) && uMod.S3m.channelRemapping[i]<CHANNELS)
    Mixer.channelPanning[uMod.S3m.channelRemapping[i]] = temp8 & 0xF;
  }
 }
 // If stereo flag is not set then make song mono
 if(!(masterVolume & 128))
  for(i = 0; i < CHANNELS; i++)
   Mixer.channelPanning[i] = 0x8;

 // Avoid division by zero for unused instruments
 for(i = 0; i < INSTRUMENTS; i++)
  uMod.S3m.instruments[i].middleC = 8363;
 // Load instruments
 for(i = 0; i < uMod.S3m.numberOfInstruments; i++) {

  // Jump to instrument parapointer and skip filename
  f_lseek(&file, (uMod.S3m.instrumentParapointers[i] << 4) + 13);

  // Find parapointer to actual sample data (3 bytes)
  f_read(&file, &temp8, 1, &count);                                  // High byte
  f_read(&file, &temp16, 2, &count);                                 // Low word
  uMod.S3m.instruments[i].sampleParapointer = temp16;                     // (temp8 << 16) + temp16

  f_read(&file, &uMod.S3m.instruments[i].length, 2, &count);
  f_read(&file, &temp16, 2, &count);                                 // Skip two bytes
  f_read(&file, &uMod.S3m.instruments[i].loopBegin, 2, &count);
  f_read(&file, &temp16, 2, &count);                                 // Skip two bytes
  f_read(&file, &uMod.S3m.instruments[i].loopEnd, 2, &count);
  f_read(&file, &temp16, 2, &count);                                 // Skip two bytes
  f_read(&file, &uMod.S3m.instruments[i].volume, 1, &count);
  f_read(&file, &temp16, 2, &count);                                 // Skip two bytes
  f_read(&file, &temp8, 1, &count);                                  // Instrument flags
  uMod.S3m.instruments[i].loop = temp8 & 1;                               // Loop enable flag
  f_read(&file, &uMod.S3m.instruments[i].middleC, 2, &count);
  if(!uMod.S3m.instruments[i].middleC) uMod.S3m.instruments[i].middleC = 8363; // Avoid division by zero
  f_lseek(&file, count + 14);

  f_read(&file, uMod.S3m.instruments[i].name, 28, &count); // Followed by "SCRS"

  if(uMod.S3m.instruments[i].loopEnd > uMod.S3m.instruments[i].length)
   uMod.S3m.instruments[i].loopEnd = uMod.S3m.instruments[i].length;

 }
}

static void loadSamples() {
 uint8_t i;
 uint32_t fileOffset;

 for(i = 0; i < uMod.S3m.numberOfInstruments; i++) {

  fileOffset = (uMod.S3m.instruments[i].sampleParapointer << 4) - 1;

  if(uMod.S3m.instruments[i].length) {
   Mixer.sampleBegin[i] = fileOffset;
   Mixer.sampleEnd[i] = fileOffset + uMod.S3m.instruments[i].length;
   if(uMod.S3m.instruments[i].loop && uMod.S3m.instruments[i].loopEnd - uMod.S3m.instruments[i].loopBegin > 2) {
    Mixer.sampleLoopBegin[i] = fileOffset + uMod.S3m.instruments[i].loopBegin;
    Mixer.sampleLoopLength[i] = uMod.S3m.instruments[i].loopEnd - uMod.S3m.instruments[i].loopBegin;
    Mixer.sampleLoopEnd[i] = fileOffset + uMod.S3m.instruments[i].loopEnd;
   } else {
    Mixer.sampleLoopBegin[i] = 0;
    Mixer.sampleLoopLength[i] = 0;
    Mixer.sampleLoopEnd[i] = 0;
   }
  }

 }
}

static void loadPattern(uint8_t pattern) {
 static uint8_t row;
 static uint8_t channel;
 static uint32_t count;
 static uint8_t temp8;
 static uint16_t temp16;

 f_lseek(&file, (uMod.S3m.patternParapointers[pattern] << 4) + 2);

 for(row = 0; row < ROWS; row++)
  for(channel = 0; channel < CHANNELS; channel++)
   uPlayer.S3m_player.currentPattern.note[row][channel] = NONOTE;
 memset(uPlayer.S3m_player.currentPattern.instrumentNumber, 0, ROWS * CHANNELS);
 memset(uPlayer.S3m_player.currentPattern.volume, NOVOLUME, ROWS * CHANNELS);
 memset(uPlayer.S3m_player.currentPattern.effectNumber, 0, ROWS * CHANNELS);
 memset(uPlayer.S3m_player.currentPattern.effectParameter, 0, ROWS * CHANNELS);

 row = 0;
 while(row < ROWS) {
  f_read(&file, &temp8, 1, &count);
  if(temp8) {
      channel = uMod.S3m.channelRemapping[temp8 & 31];
      if(channel==0XFF) //no channel
      {
             //R.K.
      }
      else
      {
          if(temp8 & 32) {
              f_read(&file, &temp16, 1, &count);
              switch(temp16) {
              case 255:
                  uPlayer.S3m_player.currentPattern.note[row][channel] = NONOTE;
                  break;
              case 254:
                  uPlayer.S3m_player.currentPattern.note[row][channel] = KEYOFF;
                  break;
              default:
                  uPlayer.S3m_player.currentPattern.note[row][channel] = (temp16 >> 4) * 12 + (temp16 & 0xF);
              }
              f_read(&file, &uPlayer.S3m_player.currentPattern.instrumentNumber[row][channel], 1, &count);
          }

          if(temp8 & 64)
              f_read(&file, &uPlayer.S3m_player.currentPattern.volume[row][channel], 1, &count);

          if(temp8 & 128) {
              f_read(&file, &uPlayer.S3m_player.currentPattern.effectNumber[row][channel], 1, &count);
              f_read(&file, &uPlayer.S3m_player.currentPattern.effectParameter[row][channel], 1, &count);
          }
      }
  } else
   row++;
 }
}

static void portamento(uint8_t channel) {
 if(uPlayer.S3m_player.lastAmigaPeriod[channel] < uPlayer.S3m_player.portamentoNote[channel]) {
  uPlayer.S3m_player.lastAmigaPeriod[channel] += uPlayer.S3m_player.portamentoSpeed[channel] << 2;
  if(uPlayer.S3m_player.lastAmigaPeriod[channel] > uPlayer.S3m_player.portamentoNote[channel])
   uPlayer.S3m_player.lastAmigaPeriod[channel] = uPlayer.S3m_player.portamentoNote[channel];
 }
 if(uPlayer.S3m_player.lastAmigaPeriod[channel] > uPlayer.S3m_player.portamentoNote[channel]) {
  uPlayer.S3m_player.lastAmigaPeriod[channel] -= uPlayer.S3m_player.portamentoSpeed[channel] << 2;
  if(uPlayer.S3m_player.lastAmigaPeriod[channel] < uPlayer.S3m_player.portamentoNote[channel])
   uPlayer.S3m_player.lastAmigaPeriod[channel] = uPlayer.S3m_player.portamentoNote[channel];
 }
 Mixer.channelFrequency[channel] = uPlayer.S3m_player.amiga / uPlayer.S3m_player.lastAmigaPeriod[channel];
}

static void vibrato(uint8_t channel, bool fine) {
 uint16_t delta;
 uint16_t temp;

 temp = uPlayer.S3m_player.vibratoPos[channel] & 31;

 switch(uPlayer.S3m_player.waveControl[channel] & 3) {
  case 0:
   delta = sine[temp];
   break;
  case 1:
   temp <<= 3;
   if(uPlayer.S3m_player.vibratoPos[channel] < 0)
    temp = 255 - temp;
   delta = temp;
   break;
  case 2:
   delta = 255;
   break;
  case 3:
   delta = rand() & 255;
   break;
 }

 delta *= uPlayer.S3m_player.vibratoDepth[channel];
 if(fine)
  delta >>= 7;
 else
  delta >>= 5;

 if(uPlayer.S3m_player.vibratoPos[channel] >= 0)
  Mixer.channelFrequency[channel] = uPlayer.S3m_player.amiga / (uPlayer.S3m_player.lastAmigaPeriod[channel] + delta);
 else
  Mixer.channelFrequency[channel] = uPlayer.S3m_player.amiga / (uPlayer.S3m_player.lastAmigaPeriod[channel] - delta);

 uPlayer.S3m_player.vibratoPos[channel] += uPlayer.S3m_player.vibratoSpeed[channel];
 if(uPlayer.S3m_player.vibratoPos[channel] > 31) uPlayer.S3m_player.vibratoPos[channel] -= 64;
}

static void tremolo(uint8_t channel) {
 uint16_t delta;
 uint16_t temp;

 temp = uPlayer.S3m_player.tremoloPos[channel] & 31;

 switch(uPlayer.S3m_player.waveControl[channel] & 3) {
  case 0:
   delta = sine[temp];
   break;
  case 1:
   temp <<= 3;
   if(uPlayer.S3m_player.tremoloPos[channel] < 0)
    temp = 255 - temp;
   delta = temp;
   break;
  case 2:
   delta = 255;
   break;
  case 3:
   delta = rand() & 255;
   break;
 }

 delta *= uPlayer.S3m_player.tremoloDepth[channel];
 delta >>= 6;

 if(uPlayer.S3m_player.tremoloPos[channel] >= 0) {
  if(uPlayer.S3m_player.volume[channel] + delta > 64) delta = 64 - uPlayer.S3m_player.volume[channel];
  Mixer.channelVolume[channel] = uPlayer.S3m_player.volume[channel] + delta;
 } else {
  if(uPlayer.S3m_player.volume[channel] - delta < 0) delta = uPlayer.S3m_player.volume[channel];
  Mixer.channelVolume[channel] = uPlayer.S3m_player.volume[channel] - delta;
 }

 uPlayer.S3m_player.tremoloPos[channel] += uPlayer.S3m_player.tremoloSpeed[channel];
 if(uPlayer.S3m_player.tremoloPos[channel] > 31) uPlayer.S3m_player.tremoloPos[channel] -= 64;
}

static void tremor(uint8_t channel) {
 uint8_t on = (uPlayer.S3m_player.tremorOnOff[channel] >> 4) + 1;
 uint8_t off = (uPlayer.S3m_player.tremorOnOff[channel] & 0xF) + 1;

 uPlayer.S3m_player.tremorCount[channel] %= on + off;
 if(uPlayer.S3m_player.tremorCount[channel] < on)
  Mixer.channelVolume[channel] = uPlayer.S3m_player.volume[channel];
 else
  Mixer.channelVolume[channel] = 0;
 uPlayer.S3m_player.tremorCount[channel]++;
}

static void volumeSlide(uint8_t channel) {
 if(!(uPlayer.S3m_player.lastVolumeSlide[channel] & 0xF))
  uPlayer.S3m_player.volume[channel] += uPlayer.S3m_player.lastVolumeSlide[channel] >> 4;
 if(!(uPlayer.S3m_player.lastVolumeSlide[channel] >> 4))
  uPlayer.S3m_player.volume[channel] -= uPlayer.S3m_player.lastVolumeSlide[channel] & 0xF;

 if(uPlayer.S3m_player.volume[channel] > 64) uPlayer.S3m_player.volume[channel] = 64;
 else if(uPlayer.S3m_player.volume[channel] < 0) uPlayer.S3m_player.volume[channel] = 0;
 Mixer.channelVolume[channel] = uPlayer.S3m_player.volume[channel];
}

static void processRow() {
 static uint8_t jumpFlag;
 static uint8_t breakFlag;
 static uint8_t channel;
 static uint16_t note;
 static uint8_t instrumentNumber;
 static uint8_t volume;
 static uint8_t effectNumber;
 static uint8_t effectParameter;
 static uint8_t effectParameterX;
 static uint8_t effectParameterY;
 static uint16_t sampleOffset;
 static uint8_t retriggerSample;

 uPlayer.S3m_player.lastRow = uPlayer.S3m_player.row++;

 jumpFlag = false;
 breakFlag = false;
 for(channel = 0; channel < uMod.S3m.numberOfChannels; channel++) {

  note = uPlayer.S3m_player.currentPattern.note[uPlayer.S3m_player.lastRow][channel];
  instrumentNumber = uPlayer.S3m_player.currentPattern.instrumentNumber[uPlayer.S3m_player.lastRow][channel];
  volume = uPlayer.S3m_player.currentPattern.volume[uPlayer.S3m_player.lastRow][channel];
  effectNumber = uPlayer.S3m_player.currentPattern.effectNumber[uPlayer.S3m_player.lastRow][channel];
  effectParameter = uPlayer.S3m_player.currentPattern.effectParameter[uPlayer.S3m_player.lastRow][channel];
  effectParameterX = effectParameter >> 4;
  effectParameterY = effectParameter & 0xF;
  sampleOffset = 0;

  if(instrumentNumber) {
   uPlayer.S3m_player.lastInstrumentNumber[channel] = instrumentNumber - 1;
   if(!(effectParameter == 0x13 && effectParameterX == NOTEDELAY))
   {
       uPlayer.S3m_player.volume[channel] = uMod.S3m.instruments[uPlayer.S3m_player.lastInstrumentNumber[channel]].volume;
   }
  }

  if(note < NONOTE) {
   uPlayer.S3m_player.lastNote[channel] = note;
   uPlayer.S3m_player.amigaPeriod[channel] = amigaPeriods[note] * 8363 /
                                 uMod.S3m.instruments[uPlayer.S3m_player.lastInstrumentNumber[channel]].middleC;

   if(effectNumber != TONEPORTAMENTO && effectNumber != PORTAMENTOVOLUMESLIDE)
    uPlayer.S3m_player.lastAmigaPeriod[channel] = uPlayer.S3m_player.amigaPeriod[channel];

   if(!(uPlayer.S3m_player.waveControl[channel] & 0x80)) uPlayer.S3m_player.vibratoPos[channel] = 0;
   if(!(uPlayer.S3m_player.waveControl[channel] & 0x08)) uPlayer.S3m_player.tremoloPos[channel] = 0;
   uPlayer.S3m_player.tremorCount[channel] = 0;

   retriggerSample = true;
  } else
   retriggerSample = false;

  if(volume <= 0x40) uPlayer.S3m_player.volume[channel] = volume;
  if(note == KEYOFF) uPlayer.S3m_player.volume[channel] = 0;

  switch(effectNumber) {
   case SETSPEED:
    uPlayer.S3m_player.speed = effectParameter;
    break;

   case JUMPTOORDER:
     //  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,GPIO_PIN_1);
    uPlayer.S3m_player.orderIndex = effectParameter;
    if(uPlayer.S3m_player.orderIndex >= uMod.S3m.songLength){
       // UARTprintf("Possible End 1?\n");
     uPlayer.S3m_player.orderIndex = 0;

    }
    uPlayer.S3m_player.row = 0;
    jumpFlag = true;
    break;

   case BREAKPATTERNTOROW:
       //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2);
    uPlayer.S3m_player.row = effectParameterX * 10 + effectParameterY;
    if(uPlayer.S3m_player.row >= ROWS)
     uPlayer.S3m_player.row = 0;
    if(!jumpFlag && !breakFlag) {
     uPlayer.S3m_player.orderIndex++;
     //UARTprintf("\033[2J");
     //   UARTprintf("\033[0;0H");
     //   UARTprintf("Current Song: [%s] - Number of Channels [%d]\n",uMod.S3m.name,uMod.S3m.numberOfChannels);
     //   UARTprintf("Current Position: [%d] of [%d]\n",uPlayer.S3m_player.orderIndex, uMod.S3m.songLength);
     if(uPlayer.S3m_player.orderIndex >= uMod.S3m.songLength){
      //   UARTprintf("Possible End 2?\n");
      uPlayer.S3m_player.orderIndex = 0;
     }
    }
    breakFlag = true;
    break;

   case VOLUMESLIDE:
    if(effectParameter) uPlayer.S3m_player.lastVolumeSlide[channel] = effectParameter;
    if((uPlayer.S3m_player.lastVolumeSlide[channel] & 0xF) == 0xF)
     uPlayer.S3m_player.volume[channel] += uPlayer.S3m_player.lastVolumeSlide[channel] >> 4;
    else if(uPlayer.S3m_player.lastVolumeSlide[channel] >> 4 == 0xF)
     uPlayer.S3m_player.volume[channel] -= uPlayer.S3m_player.lastVolumeSlide[channel] & 0xF;
    if(uMod.S3m.fastVolumeSlides) {
     if(!(uPlayer.S3m_player.lastVolumeSlide[channel] & 0xF))
      uPlayer.S3m_player.volume[channel] += uPlayer.S3m_player.lastVolumeSlide[channel] >> 4;
     if(!(uPlayer.S3m_player.lastVolumeSlide[channel] >> 4))
      uPlayer.S3m_player.volume[channel] -= uPlayer.S3m_player.lastVolumeSlide[channel] & 0xF;
    }
    if(uPlayer.S3m_player.volume[channel] > 64) uPlayer.S3m_player.volume[channel] = 64;
    else if(uPlayer.S3m_player.volume[channel] < 0) uPlayer.S3m_player.volume[channel] = 0;
    break;

   case PORTAMENTODOWN:
    if(effectParameter) uPlayer.S3m_player.lastPortamento[channel] = effectParameter;
    if(uPlayer.S3m_player.lastPortamento[channel] >> 4 == 0xF)
     uPlayer.S3m_player.lastAmigaPeriod[channel] += (uPlayer.S3m_player.lastPortamento[channel] & 0xF) << 2;
    if(uPlayer.S3m_player.lastPortamento[channel] >> 4 == 0xE)
     uPlayer.S3m_player.lastAmigaPeriod[channel] += uPlayer.S3m_player.lastPortamento[channel] & 0xF;
    break;

   case PORTAMENTOUP:
    if(effectParameter) uPlayer.S3m_player.lastPortamento[channel] = effectParameter;
    if(uPlayer.S3m_player.lastPortamento[channel] >> 4 == 0xF)
     uPlayer.S3m_player.lastAmigaPeriod[channel] -= (uPlayer.S3m_player.lastPortamento[channel] & 0xF) << 2;
    if(uPlayer.S3m_player.lastPortamento[channel] >> 4 == 0xE)
     uPlayer.S3m_player.lastAmigaPeriod[channel] -= uPlayer.S3m_player.lastPortamento[channel] & 0xF;
    break;

   case TONEPORTAMENTO:
    if(effectParameter) uPlayer.S3m_player.portamentoSpeed[channel] = effectParameter;
    uPlayer.S3m_player.portamentoNote[channel] = uPlayer.S3m_player.amigaPeriod[channel];
    retriggerSample = false;
    break;

   case VIBRATO:
    if(effectParameterX) uPlayer.S3m_player.vibratoSpeed[channel] = effectParameterX;
    if(effectParameterY) uPlayer.S3m_player.vibratoDepth[channel] = effectParameterY;
    break;

   case TREMOR:
    if(effectParameter) uPlayer.S3m_player.tremorOnOff[channel] = effectParameter;
    tremor(channel);
    break;

   case VIBRATOVOLUMESLIDE:
    if(effectParameter) uPlayer.S3m_player.lastVolumeSlide[channel] = effectParameter;
    break;

   case PORTAMENTOVOLUMESLIDE:
    if(effectParameter) uPlayer.S3m_player.lastVolumeSlide[channel] = effectParameter;
    uPlayer.S3m_player.portamentoNote[channel] = uPlayer.S3m_player.amigaPeriod[channel];
    retriggerSample = false;
    break;

   case SETSAMPLEOFFSET:
    sampleOffset = effectParameter << 8;
    if(sampleOffset > uMod.S3m.instruments[uPlayer.S3m_player.lastInstrumentNumber[channel]].length)
     sampleOffset = uMod.S3m.instruments[uPlayer.S3m_player.lastInstrumentNumber[channel]].length;
    break;

   case RETRIGGERNOTEVOLUMESLIDE:
    if(effectParameter) {
     uPlayer.S3m_player.retriggerVolumeSlide[channel] = effectParameterX;
     uPlayer.S3m_player.retriggerSpeed[channel] = effectParameterY;
    }
    break;

   case TREMOLO:
    if(effectParameterX) uPlayer.S3m_player.tremoloSpeed[channel] = effectParameterX;
    if(effectParameterY) uPlayer.S3m_player.tremoloDepth[channel] = effectParameterY;
    break;

   case 0x13:
    switch(effectParameterX) {
     case SETFINETUNE:
      uMod.S3m.instruments[uPlayer.S3m_player.lastInstrumentNumber[channel]].middleC = fineTuneToHz[effectParameterY];
      break;

     case SETVIBRATOWAVEFORM:
      uPlayer.S3m_player.waveControl[channel] &= 0xF0;
      uPlayer.S3m_player.waveControl[channel] |= effectParameterY;
      break;

     case SETTREMOLOWAVEFORM:
      uPlayer.S3m_player.waveControl[channel] &= 0xF;
      uPlayer.S3m_player.waveControl[channel] |= effectParameterY << 4;
      break;

     case SETCHANNELPANNING:
      Mixer.channelPanning[channel] = effectParameterY;
      break;

     case STEREOCONTROL:
      if(effectParameterY > 7)
       effectParameterY -= 8;
      else
       effectParameterY += 8;
      Mixer.channelPanning[channel] = effectParameterY;
      break;

     case PATTERNLOOP:
      if(effectParameterY) {
       if(uPlayer.S3m_player.patternLoopCount[channel])
        uPlayer.S3m_player.patternLoopCount[channel]--;
       else
        uPlayer.S3m_player.patternLoopCount[channel] = effectParameterY;
       if(uPlayer.S3m_player.patternLoopCount[channel])
        uPlayer.S3m_player.row = uPlayer.S3m_player.patternLoopRow[channel] - 1;
      } else
       uPlayer.S3m_player.patternLoopRow[channel] = uPlayer.S3m_player.row;
      break;

     case NOTEDELAY:
      retriggerSample = false;
      break;

     case PATTERNDELAY:
      uPlayer.S3m_player.patternDelay = effectParameterY;
      break;
    }
    break;

   case SETTEMPO:
    uPlayer.S3m_player.samplesPerTick = SAMPLERATE / ((effectParameter<<1) / 5);
    break;

   case FINEVIBRATO:
    if(effectParameterX) uPlayer.S3m_player.vibratoSpeed[channel] = effectParameterX;
    if(effectParameterY) uPlayer.S3m_player.vibratoDepth[channel] = effectParameterY;
    break;

   case SETGLOBALVOLUME:
    break;
  }

  if(retriggerSample)
   Mixer.channelSampleOffset[channel] = sampleOffset << DIVIDER;

  if(retriggerSample || uPlayer.S3m_player.lastAmigaPeriod[channel] &&
     effectNumber != VIBRATO && effectNumber != VIBRATOVOLUMESLIDE &&
     !(effectNumber == 0x13 && effectParameterX == NOTEDELAY) &&
     effectNumber != FINEVIBRATO)
   Mixer.channelFrequency[channel] = uPlayer.S3m_player.amiga / uPlayer.S3m_player.lastAmigaPeriod[channel];

  if(instrumentNumber)
   Mixer.channelSampleNumber[channel] = uPlayer.S3m_player.lastInstrumentNumber[channel];

  if(effectNumber != TREMOR && effectNumber != TREMOLO)
   Mixer.channelVolume[channel] = uPlayer.S3m_player.volume[channel];
 }
}

static void processTick() {
 static uint8_t channel;
 static uint16_t note;
 static uint8_t instrumentNumber;
 static uint8_t volume;
 static uint8_t effectNumber;
 static uint8_t effectParameter;
 static uint8_t effectParameterX;
 static uint8_t effectParameterY;

 for(channel = 0; channel < uMod.S3m.numberOfChannels; channel++) {

  if(uPlayer.S3m_player.lastAmigaPeriod[channel]) {

   note = uPlayer.S3m_player.currentPattern.note[uPlayer.S3m_player.lastRow][channel];
   instrumentNumber = uPlayer.S3m_player.currentPattern.instrumentNumber[uPlayer.S3m_player.lastRow][channel];
   volume = uPlayer.S3m_player.currentPattern.volume[uPlayer.S3m_player.lastRow][channel];
   effectNumber = uPlayer.S3m_player.currentPattern.effectNumber[uPlayer.S3m_player.lastRow][channel];
   effectParameter = uPlayer.S3m_player.currentPattern.effectParameter[uPlayer.S3m_player.lastRow][channel];
   effectParameterX = effectParameter >> 4;
   effectParameterY = effectParameter & 0xF;

   switch(effectNumber) {
    case VOLUMESLIDE:
     volumeSlide(channel);
     break;

    case PORTAMENTODOWN:
     if(uPlayer.S3m_player.lastPortamento[channel] < 0xE0)
      uPlayer.S3m_player.lastAmigaPeriod[channel] += uPlayer.S3m_player.lastPortamento[channel] << 2;
     Mixer.channelFrequency[channel] = uPlayer.S3m_player.amiga / uPlayer.S3m_player.lastAmigaPeriod[channel];
     break;

    case PORTAMENTOUP:
     if(uPlayer.S3m_player.lastPortamento[channel] < 0xE0)
      uPlayer.S3m_player.lastAmigaPeriod[channel] -= uPlayer.S3m_player.lastPortamento[channel] << 2;
     Mixer.channelFrequency[channel] = uPlayer.S3m_player.amiga / uPlayer.S3m_player.lastAmigaPeriod[channel];
     break;

    case TONEPORTAMENTO:
     portamento(channel);
     break;

    case VIBRATO:
     vibrato(channel, false); // Fine = false
     break;

    case TREMOR:
     tremor(channel);
     break;

    case ARPEGGIO:
     if(effectParameter)
      switch(uPlayer.S3m_player.tick % 3) {
       case 0:
        Mixer.channelFrequency[channel] = uPlayer.S3m_player.amiga / uPlayer.S3m_player.lastAmigaPeriod[channel];
        break;
       case 1:
        Mixer.channelFrequency[channel] = uPlayer.S3m_player.amiga / amigaPeriods[uPlayer.S3m_player.lastNote[channel] + effectParameterX];
        break;
       case 2:
        Mixer.channelFrequency[channel] = uPlayer.S3m_player.amiga / amigaPeriods[uPlayer.S3m_player.lastNote[channel] + effectParameterY];
        break;
      }
     break;

    case VIBRATOVOLUMESLIDE:
     vibrato(channel, false); // Fine = false
     volumeSlide(channel);
     break;

    case PORTAMENTOVOLUMESLIDE:
     portamento(channel);
     volumeSlide(channel);
     break;

    case RETRIGGERNOTEVOLUMESLIDE:
     if(!uPlayer.S3m_player.retriggerSpeed[channel]) break;
     if(!(uPlayer.S3m_player.tick % uPlayer.S3m_player.retriggerSpeed[channel])) {
      if(uPlayer.S3m_player.retriggerVolumeSlide[channel]) {
       switch(uPlayer.S3m_player.retriggerVolumeSlide[channel]) {
        case 1:
         uPlayer.S3m_player.volume[channel]--;
         break;
        case 2:
         uPlayer.S3m_player.volume[channel] -= 2;
         break;
        case 3:
         uPlayer.S3m_player.volume[channel] -= 4;
         break;
        case 4:
         uPlayer.S3m_player.volume[channel] -= 8;
         break;
        case 5:
         uPlayer.S3m_player.volume[channel] -= 16;
         break;
        case 6:
         uPlayer.S3m_player.volume[channel] *= 2 / 3;
         break;
        case 7:
         uPlayer.S3m_player.volume[channel] >>= 1;
         break;
        case 9:
         uPlayer.S3m_player.volume[channel]++;
         break;
        case 0xA:
         uPlayer.S3m_player.volume[channel] += 2;
         break;
        case 0xB:
         uPlayer.S3m_player.volume[channel] += 4;
         break;
        case 0xC:
         uPlayer.S3m_player.volume[channel] += 8;
         break;
        case 0xD:
         uPlayer.S3m_player.volume[channel] += 16;
         break;
        case 0xE:
         uPlayer.S3m_player.volume[channel] *= 3 / 2;
         break;
        case 0xF:
         uPlayer.S3m_player.volume[channel] <<= 1;
         break;
       }
       if(uPlayer.S3m_player.volume[channel] > 64) uPlayer.S3m_player.volume[channel] = 64;
       else if(uPlayer.S3m_player.volume[channel] < 0) uPlayer.S3m_player.volume[channel] = 0;
       Mixer.channelVolume[channel] = uPlayer.S3m_player.volume[channel];
      }
      Mixer.channelSampleOffset[channel] = 0;
     }
     break;

    case TREMOLO:
     tremolo(channel);
     break;

    case 0x13:
     switch(effectParameterX) {
      case NOTECUT:
       if(uPlayer.S3m_player.tick == effectParameterY)
        Mixer.channelVolume[channel] = uPlayer.S3m_player.volume[channel] = 0;
       break;

      case NOTEDELAY:
       if(uPlayer.S3m_player.tick == effectParameterY) {
        if(instrumentNumber) uPlayer.S3m_player.volume[channel] = uMod.S3m.instruments[uPlayer.S3m_player.lastInstrumentNumber[channel]].volume;
        if(volume <= 0x40) uPlayer.S3m_player.volume[channel] = volume;
        if(note < NONOTE) Mixer.channelSampleOffset[channel] = 0;
        Mixer.channelFrequency[channel] = uPlayer.S3m_player.amiga / uPlayer.S3m_player.lastAmigaPeriod[channel];
        Mixer.channelVolume[channel] = uPlayer.S3m_player.volume[channel];
       }
       break;
     }
     break;

    case FINEVIBRATO:
     vibrato(channel, true); // Fine = true
     break;
   }

  }

 }
}

void s3m_player() {
 if(uPlayer.S3m_player.tick == uPlayer.S3m_player.speed) {
  uPlayer.S3m_player.tick = 0;

  if(uPlayer.S3m_player.row == ROWS) {
   uPlayer.S3m_player.orderIndex++;
   //UARTprintf("\033[2J");
   //        UARTprintf("\033[0;0H");
   //        UARTprintf("Current Song: [%s] - Number of Channels [%d]\n",uMod.S3m.name,uMod.S3m.numberOfChannels);
    //       UARTprintf("Current Position: [%d] of [%d]\n",uPlayer.S3m_player.orderIndex, uMod.S3m.songLength);
   //ROM_IntMasterDisable();
   //UARTprintf("Mod orderIndex: %d\n",uPlayer.S3m_player.orderIndex);
   //ROM_IntMasterEnable();

   if(uPlayer.S3m_player.orderIndex == uMod.S3m.songLength){
       //UARTprintf("Possible End 3?\n");
       //loadNextFile();
    uPlayer.S3m_player.orderIndex = 0;
   }
   uPlayer.S3m_player.row = 0;
  }

  if(uPlayer.S3m_player.patternDelay) {
   uPlayer.S3m_player.patternDelay--;
  } else {
   if(uPlayer.S3m_player.orderIndex != uPlayer.S3m_player.oldOrderIndex)
   {
    loadPattern(uMod.S3m.order[uPlayer.S3m_player.orderIndex]);
   }
   uPlayer.S3m_player.oldOrderIndex = uPlayer.S3m_player.orderIndex;
   processRow();

  }

 } else {
  processTick();
 }
 uPlayer.S3m_player.tick++;
}

void s3m_mixer() {
 static int16_t sumL;
 static int16_t sumR;
 static uint8_t channel;
 static uint32_t samplePointer;
 static uint16_t fatBufferSize;
 static uint32_t count;
 static uint8_t current;
 static uint8_t next;
 static int16_t out;

 sumL = 0;
 sumR = 0;
 for(channel = 0; channel < uMod.S3m.numberOfChannels; channel++) {

  if(!Mixer.channelFrequency[channel] ||
     !uMod.S3m.instruments[Mixer.channelSampleNumber[channel]].length) continue;

  Mixer.channelSampleOffset[channel] += Mixer.channelFrequency[channel];

  if(!Mixer.channelVolume[channel]) continue;

  samplePointer = Mixer.sampleBegin[Mixer.channelSampleNumber[channel]] +
                  (Mixer.channelSampleOffset[channel] >> DIVIDER);

  if(Mixer.sampleLoopLength[Mixer.channelSampleNumber[channel]]) {

   if(samplePointer >= Mixer.sampleLoopEnd[Mixer.channelSampleNumber[channel]]) {
    Mixer.channelSampleOffset[channel] -= Mixer.sampleLoopLength[Mixer.channelSampleNumber[channel]] << DIVIDER;
    samplePointer -= Mixer.sampleLoopLength[Mixer.channelSampleNumber[channel]];
   }

  } else {

   if(samplePointer >= Mixer.sampleEnd[Mixer.channelSampleNumber[channel]]) {
    Mixer.channelFrequency[channel] = 0;
    samplePointer = Mixer.sampleEnd[Mixer.channelSampleNumber[channel]];
   }

  }

  if(samplePointer < FatBuffer.samplePointer[channel] ||
     samplePointer >= FatBuffer.samplePointer[channel] + FATBUFFERSIZE - 1 ||
     Mixer.channelSampleNumber[channel] != FatBuffer.channelSampleNumber[channel]) {

   fatBufferSize = Mixer.sampleEnd[Mixer.channelSampleNumber[channel]] - samplePointer + 1;
   if(fatBufferSize > FATBUFFERSIZE) fatBufferSize = FATBUFFERSIZE;

   //LEDLAT = ~(1 << (channel & 0x3)); //TODO: more leds
   f_lseek(&file, samplePointer);
   f_read(&file, FatBuffer.channels[channel], fatBufferSize, &count);
   //LEDLAT = 0xF;

   FatBuffer.samplePointer[channel] = samplePointer;
   FatBuffer.channelSampleNumber[channel] = Mixer.channelSampleNumber[channel];
  }

  current = FatBuffer.channels[channel][(samplePointer - FatBuffer.samplePointer[channel]) & FATBUFFERSIZE - 1]; //instead of FatBufferSIZE I've put 0x400 so that I can use non 2 power sizes
  next = FatBuffer.channels[channel][(samplePointer + 1 - FatBuffer.samplePointer[channel]) & FATBUFFERSIZE - 1];

  // Unsigned to signed
  out = current - 128;

  // Integer linear interpolation
  out += (next - current) * (Mixer.channelSampleOffset[channel] & (1 << DIVIDER) - 1) >> DIVIDER;

  // Upscale to BITDEPTH
  out <<= BITDEPTH - 8;

  // Channel volume
  out = out * Mixer.channelVolume[channel] >> 6;

  // Channel panning
  sumL += out * min(0xF - Mixer.channelPanning[channel], 8) >> 3;
  sumR += out * min(Mixer.channelPanning[channel], 8) >> 3;
 }

 // Downscale to BITDEPTH
 //sumL /= uMod.S3m.numberOfChannels;
 //sumR /= uMod.S3m.numberOfChannels;
 sumL>>=(BITDEPTH - 8);
 sumR>>=(BITDEPTH - 8);

 // Fill the sound buffer with unsigned values
 SoundBuffer.left[SoundBuffer.writePos] = sumL + (1 << BITDEPTH - 1);
 SoundBuffer.right[SoundBuffer.writePos] = sumR + (1 << BITDEPTH - 1);
 SoundBuffer.writePos++;
 SoundBuffer.writePos &= SOUNDBUFFERSIZE - 1;
}

void loadS3m() {
 uint8_t channel;

 loadHeader();
 loadSamples();

// UARTprintf("Song name: [%s] channels - [%d]\n",uMod.S3m.name,uMod.S3m.numberOfChannels);

 uPlayer.S3m_player.amiga = AMIGA;
 uPlayer.S3m_player.tick = uPlayer.S3m_player.speed;
 uPlayer.S3m_player.row = 0;

 uPlayer.S3m_player.orderIndex = 0;
 uPlayer.S3m_player.oldOrderIndex = 0xFF;
 uPlayer.S3m_player.patternDelay = 0;

 for(channel = 0; channel < uMod.S3m.numberOfChannels; channel++) {
  uPlayer.S3m_player.patternLoopCount[channel] = 0;
  uPlayer.S3m_player.patternLoopRow[channel] = 0;

  uPlayer.S3m_player.lastAmigaPeriod[channel] = 0;

  uPlayer.S3m_player.waveControl[channel] = 0;

  uPlayer.S3m_player.vibratoSpeed[channel] = 0;
  uPlayer.S3m_player.vibratoDepth[channel] = 0;
  uPlayer.S3m_player.vibratoPos[channel] = 0;

  uPlayer.S3m_player.tremoloSpeed[channel] = 0;
  uPlayer.S3m_player.tremoloDepth[channel] = 0;
  uPlayer.S3m_player.tremoloPos[channel] = 0;

  uPlayer.S3m_player.tremorOnOff[channel] = 0;
  uPlayer.S3m_player.tremorCount[channel] = 0;

  FatBuffer.samplePointer[channel] = 0;
  FatBuffer.channelSampleNumber[channel] = 0xFF;

  Mixer.channelSampleOffset[channel] = 0;
  Mixer.channelFrequency[channel] = 0;
  Mixer.channelVolume[channel] = 0;
 }

 SoundBuffer.writePos = 0;
 SoundBuffer.readPos = 0;
}

uint16_t s3m_getSamplesPerTick()
{
    return uPlayer.S3m_player.samplesPerTick;
}
#endif