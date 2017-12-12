
#define MOD32INC 1
#ifdef MOD32INC
#include <stdbool.h>
#include <stdint.h>
#include "stdlib.h"
#include <string.h>
//#include "inc/hw_types.h"
//#include "utils/uartstdio.h"

#include "global.h"
#include "mod32.h"

//#include "ff.h"

#define min(X,Y) ((X) < (Y) ? (X) : (Y))

// Sorted Amiga periods
static const uint16_t amigaPeriods[296] = {
 907, 900, 894, 887, 881, 875, 868, 862, //  -8 to -1
 856, 850, 844, 838, 832, 826, 820, 814, // C-1 to +7
 808, 802, 796, 791, 785, 779, 774, 768, // C#1 to +7
 762, 757, 752, 746, 741, 736, 730, 725, // D-1 to +7
 720, 715, 709, 704, 699, 694, 689, 684, // D#1 to +7
 678, 675, 670, 665, 660, 655, 651, 646, // E-1 to +7
 640, 636, 632, 628, 623, 619, 614, 610, // F-1 to +7
 604, 601, 597, 592, 588, 584, 580, 575, // F#1 to +7
 570, 567, 563, 559, 555, 551, 547, 543, // G-1 to +7
 538, 535, 532, 528, 524, 520, 516, 513, // G#1 to +7
 508, 505, 502, 498, 494, 491, 487, 484, // A-1 to +7
 480, 477, 474, 470, 467, 463, 460, 457, // A#1 to +7
 453, 450, 447, 444, 441, 437, 434, 431, // B-1 to +7
 428, 425, 422, 419, 416, 413, 410, 407, // C-2 to +7
 404, 401, 398, 395, 392, 390, 387, 384, // C#2 to +7
 381, 379, 376, 373, 370, 368, 365, 363, // D-2 to +7
 360, 357, 355, 352, 350, 347, 345, 342, // D#2 to +7
 339, 337, 335, 332, 330, 328, 325, 323, // E-2 to +7
 320, 318, 316, 314, 312, 309, 307, 305, // F-2 to +7
 302, 300, 298, 296, 294, 292, 290, 288, // F#2 to +7
 285, 284, 282, 280, 278, 276, 274, 272, // G-2 to +7
 269, 268, 266, 264, 262, 260, 258, 256, // G#2 to +7
 254, 253, 251, 249, 247, 245, 244, 242, // A-2 to +7
 240, 238, 237, 235, 233, 232, 230, 228, // A#2 to +7
 226, 225, 223, 222, 220, 219, 217, 216, // B-2 to +7
 214, 212, 211, 209, 208, 206, 205, 203, // C-3 to +7
 202, 200, 199, 198, 196, 195, 193, 192, // C#3 to +7
 190, 189, 188, 187, 185, 184, 183, 181, // D-3 to +7
 180, 179, 177, 176, 175, 174, 172, 171, // D#3 to +7
 170, 169, 167, 166, 165, 164, 163, 161, // E-3 to +7
 160, 159, 158, 157, 156, 155, 154, 152, // F-3 to +7
 151, 150, 149, 148, 147, 146, 145, 144, // F#3 to +7
 143, 142, 141, 140, 139, 138, 137, 136, // G-3 to +7
 135, 134, 133, 132, 131, 130, 129, 128, // G#3 to +7
 127, 126, 125, 125, 123, 123, 122, 121, // A-3 to +7
 120, 119, 118, 118, 117, 116, 115, 114, // A#3 to +7
 113, 113, 112, 111, 110, 109, 109, 108  // B-3 to +7
};

static const uint8_t sine[64] = {
   0,  24,  49,  74,  97, 120, 141, 161,
 180, 197, 212, 224, 235, 244, 250, 253,
 255, 253, 250, 244, 235, 224, 212, 197,
 180, 161, 141, 120,  97,  74,  49,  24
};

/*
const uint8_t invertLoopTable[16] = {
 0, 5, 6, 7, 8, 10, 11, 13, 16, 19, 22, 26, 32, 43, 64, 128
};
*/

static uint16_t word(uint8_t h, uint8_t l) {
 return h << 8 | l;
}

static void loadHeader() {
 uint32_t count;
 uint8_t i;
 char temp[4];
 numsteps = 0;
 f_read(&file, uMod.Mod.name, 20, &count);
 if(strcmp(uMod.Mod.name, "")==0){
     strcpy(uMod.Mod.name,fileInfo.fname);
     //strcpy(currentsongname, uMod.Mod.name);
 }
 for(i = 0; i < SAMPLES; i++) {
  f_read(&file, uMod.Mod.samples[i].name, 22, &count);
  f_read(&file, temp, 2, &count);
  uMod.Mod.samples[i].length = word(temp[0], temp[1]) * 2;
  f_read(&file, &uMod.Mod.samples[i].fineTune, 1, &count);
  if(uMod.Mod.samples[i].fineTune > 7) uMod.Mod.samples[i].fineTune -= 16;
  f_read(&file, &uMod.Mod.samples[i].volume, 1, &count);
  f_read(&file, temp, 2, &count);
  uMod.Mod.samples[i].loopBegin = word(temp[0], temp[1]) * 2;
  f_read(&file, temp, 2, &count);
  uMod.Mod.samples[i].loopLength = word(temp[0], temp[1]) * 2;
  if(uMod.Mod.samples[i].loopBegin + uMod.Mod.samples[i].loopLength > uMod.Mod.samples[i].length)
   uMod.Mod.samples[i].loopLength = uMod.Mod.samples[i].length - uMod.Mod.samples[i].loopBegin;
 }

 f_read(&file, &uMod.Mod.songLength, 1, &count);
 f_read(&file, temp, 1, &count); // Discard this byte

 uMod.Mod.numberOfPatterns = 0;
 for(i = 0; i < 128; i++) {
  f_read(&file, &uMod.Mod.order[i], 1, &count);
  if(uMod.Mod.order[i] > uMod.Mod.numberOfPatterns)
   uMod.Mod.numberOfPatterns = uMod.Mod.order[i];
 }
 uMod.Mod.numberOfPatterns++;

 // Offset 1080
 f_read(&file, temp, 4, &count);

 if(!strncmp(temp + 1, "CHN", 3))
  uMod.Mod.numberOfChannels = temp[0] - '0';
 else if(!strncmp(temp + 2, "CH", 2))
  uMod.Mod.numberOfChannels = (temp[0] - '0') * 10 + temp[1] - '0';
 else
  uMod.Mod.numberOfChannels = 4;
}

static void loadSamples() {
 uint8_t i;
 uint32_t fileOffset = 1084 + uMod.Mod.numberOfPatterns * ROWS * uMod.Mod.numberOfChannels * 4 - 1;

 for(i = 0; i < SAMPLES; i++) {

  if(uMod.Mod.samples[i].length) {
   Mixer.sampleBegin[i] = fileOffset;
   Mixer.sampleEnd[i] = fileOffset + uMod.Mod.samples[i].length;
   if(uMod.Mod.samples[i].loopLength > 2) {
    Mixer.sampleLoopBegin[i] = fileOffset + uMod.Mod.samples[i].loopBegin;
    Mixer.sampleLoopLength[i] = uMod.Mod.samples[i].loopLength;
    Mixer.sampleLoopEnd[i] = Mixer.sampleLoopBegin[i] + Mixer.sampleLoopLength[i];
   } else {
    Mixer.sampleLoopBegin[i] = 0;
    Mixer.sampleLoopLength[i] = 0;
    Mixer.sampleLoopEnd[i] = 0;
   }
   fileOffset += uMod.Mod.samples[i].length;
  }

 }
}

static void loadPattern(uint8_t pattern) {
 static uint8_t row;
 static uint8_t channel;
 static uint32_t count;
 static uint8_t i;
 static uint8_t temp[4];
 static uint16_t amigaPeriod;

 f_lseek(&file, 1084 + pattern * ROWS * uMod.Mod.numberOfChannels * 4);

 for(row = 0; row < ROWS; row++) {
  for(channel = 0; channel < uMod.Mod.numberOfChannels; channel++) {

   f_read(&file, temp, 4, &count);

   uPlayer.Mod_player.currentPattern.sampleNumber[row][channel] = (temp[0] & 0xF0) + (temp[2] >> 4);

   amigaPeriod = ((temp[0] & 0xF) << 8) + temp[1];
   uPlayer.Mod_player.currentPattern.note[row][channel] = NONOTE_MOD;
   for(i = 1; i < 37; i++)
    if(amigaPeriod > amigaPeriods[i * 8] - 3 &&
       amigaPeriod < amigaPeriods[i * 8] + 3)
     uPlayer.Mod_player.currentPattern.note[row][channel] = i * 8;

   uPlayer.Mod_player.currentPattern.effectNumber[row][channel] = temp[2] & 0xF;
   uPlayer.Mod_player.currentPattern.effectParameter[row][channel] = temp[3];

  }
 }
}

static void portamento(uint8_t channel) {
 if(uPlayer.Mod_player.lastAmigaPeriod[channel] < uPlayer.Mod_player.portamentoNote[channel]) {
     uPlayer.Mod_player.lastAmigaPeriod[channel] += uPlayer.Mod_player.portamentoSpeed[channel];
  if(uPlayer.Mod_player.lastAmigaPeriod[channel] > uPlayer.Mod_player.portamentoNote[channel])
      uPlayer.Mod_player.lastAmigaPeriod[channel] = uPlayer.Mod_player.portamentoNote[channel];
 }
 if(uPlayer.Mod_player.lastAmigaPeriod[channel] > uPlayer.Mod_player.portamentoNote[channel]) {
     uPlayer.Mod_player.lastAmigaPeriod[channel] -= uPlayer.Mod_player.portamentoSpeed[channel];
  if(uPlayer.Mod_player.lastAmigaPeriod[channel] < uPlayer.Mod_player.portamentoNote[channel])
      uPlayer.Mod_player.lastAmigaPeriod[channel] = uPlayer.Mod_player.portamentoNote[channel];
 }
 Mixer.channelFrequency[channel] = uPlayer.Mod_player.amiga / uPlayer.Mod_player.lastAmigaPeriod[channel];
}

static void vibrato(uint8_t channel) {
 uint16_t delta;
 uint16_t temp;

 temp = uPlayer.Mod_player.vibratoPos[channel] & 31;

 switch(uPlayer.Mod_player.waveControl[channel] & 3) {
  case 0:
   delta = sine[temp];
   break;
  case 1:
   temp <<= 3;
   if(uPlayer.Mod_player.vibratoPos[channel] < 0)
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

 delta *= uPlayer.Mod_player.vibratoDepth[channel];
 delta >>= 7;

 if(uPlayer.Mod_player.vibratoPos[channel] >= 0)
  Mixer.channelFrequency[channel] = uPlayer.Mod_player.amiga / (uPlayer.Mod_player.lastAmigaPeriod[channel] + delta);
 else
  Mixer.channelFrequency[channel] = uPlayer.Mod_player.amiga / (uPlayer.Mod_player.lastAmigaPeriod[channel] - delta);

 uPlayer.Mod_player.vibratoPos[channel] += uPlayer.Mod_player.vibratoSpeed[channel];
 if(uPlayer.Mod_player.vibratoPos[channel] > 31) uPlayer.Mod_player.vibratoPos[channel] -= 64;
}

static void tremolo(uint8_t channel) {
 uint16_t delta;
 uint16_t temp;

 temp = uPlayer.Mod_player.tremoloPos[channel] & 31;

 switch(uPlayer.Mod_player.waveControl[channel] & 3) {
  case 0:
   delta = sine[temp];
   break;
  case 1:
   temp <<= 3;
   if(uPlayer.Mod_player.tremoloPos[channel] < 0)
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

 delta *= uPlayer.Mod_player.tremoloDepth[channel];
 delta >>= 6;

 if(uPlayer.Mod_player.tremoloPos[channel] >= 0) {
  if(uPlayer.Mod_player.volume[channel] + delta > 64) delta = 64 - uPlayer.Mod_player.volume[channel];
  Mixer.channelVolume[channel] = uPlayer.Mod_player.volume[channel] + delta;
 } else {
  if(uPlayer.Mod_player.volume[channel] - delta < 0) delta = uPlayer.Mod_player.volume[channel];
  Mixer.channelVolume[channel] = uPlayer.Mod_player.volume[channel] - delta;
 }

 uPlayer.Mod_player.tremoloPos[channel] += uPlayer.Mod_player.tremoloSpeed[channel];
 if(uPlayer.Mod_player.tremoloPos[channel] > 31) uPlayer.Mod_player.tremoloPos[channel] -= 64;
}

static void processRow() {
 static bool jumpFlag;
 static bool breakFlag;
 static uint8_t channel;
 static uint8_t sampleNumber;
 static uint16_t note;
 static uint8_t effectNumber;
 static uint8_t effectParameter;
 static uint8_t effectParameterX;
 static uint8_t effectParameterY;
 static uint16_t sampleOffset;

 uPlayer.Mod_player.lastRow = uPlayer.Mod_player.row++;
 jumpFlag = false;
 breakFlag = false;
 for(channel = 0; channel < uMod.Mod.numberOfChannels; channel++) {

  sampleNumber = uPlayer.Mod_player.currentPattern.sampleNumber[uPlayer.Mod_player.lastRow][channel];
  note = uPlayer.Mod_player.currentPattern.note[uPlayer.Mod_player.lastRow][channel];
  effectNumber = uPlayer.Mod_player.currentPattern.effectNumber[uPlayer.Mod_player.lastRow][channel];
  effectParameter = uPlayer.Mod_player.currentPattern.effectParameter[uPlayer.Mod_player.lastRow][channel];
  effectParameterX = effectParameter >> 4;
  effectParameterY = effectParameter & 0xF;
  sampleOffset = 0;

  if(sampleNumber) {
   uPlayer.Mod_player.lastSampleNumber[channel] = sampleNumber - 1;
   if(!(effectParameter == 0xE && effectParameterX == NOTEDELAY))
    uPlayer.Mod_player.volume[channel] = uMod.Mod.samples[uPlayer.Mod_player.lastSampleNumber[channel]].volume;
  }

  if(note != NONOTE_MOD) {
   uPlayer.Mod_player.lastNote[channel] = note;
   uPlayer.Mod_player.amigaPeriod[channel] = amigaPeriods[note + uMod.Mod.samples[uPlayer.Mod_player.lastSampleNumber[channel]].fineTune];

   if(effectNumber != TONEPORTAMENTO && effectNumber != PORTAMENTOVOLUMESLIDE)
    uPlayer.Mod_player.lastAmigaPeriod[channel] = uPlayer.Mod_player.amigaPeriod[channel];

   if(!(uPlayer.Mod_player.waveControl[channel] & 0x80)) uPlayer.Mod_player.vibratoPos[channel] = 0;
   if(!(uPlayer.Mod_player.waveControl[channel] & 0x08)) uPlayer.Mod_player.tremoloPos[channel] = 0;
  }

  switch(effectNumber) {
   case TONEPORTAMENTO:
    if(effectParameter) uPlayer.Mod_player.portamentoSpeed[channel] = effectParameter;
    uPlayer.Mod_player.portamentoNote[channel] = uPlayer.Mod_player.amigaPeriod[channel];
    note = NONOTE_MOD;
    break;

   case VIBRATO:
    if(effectParameterX) uPlayer.Mod_player.vibratoSpeed[channel] = effectParameterX;
    if(effectParameterY) uPlayer.Mod_player.vibratoDepth[channel] = effectParameterY;
    break;

   case PORTAMENTOVOLUMESLIDE:
    uPlayer.Mod_player.portamentoNote[channel] = uPlayer.Mod_player.amigaPeriod[channel];
    note = NONOTE_MOD;
    break;

   case TREMOLO:
    if(effectParameterX) uPlayer.Mod_player.tremoloSpeed[channel] = effectParameterX;
    if(effectParameterY) uPlayer.Mod_player.tremoloDepth[channel] = effectParameterY;
    break;

   case SETCHANNELPANNING:
    Mixer.channelPanning[channel] = effectParameter >> 1;
    break;

   case SETSAMPLEOFFSET:
    sampleOffset = effectParameter << 8;
    if(sampleOffset > uMod.Mod.samples[uPlayer.Mod_player.lastSampleNumber[channel]].length)
     sampleOffset = uMod.Mod.samples[uPlayer.Mod_player.lastSampleNumber[channel]].length;
    break;

   case JUMPTOORDER:
    uPlayer.Mod_player.orderIndex = effectParameter;
    if(uPlayer.Mod_player.orderIndex >= uMod.Mod.songLength){
        //UARTprintf("Possible End 1?\n");
     uPlayer.Mod_player.orderIndex = 0;
    }
    uPlayer.Mod_player.row = 0;
    jumpFlag = true;
    break;

   case SETVOLUME:
    if(effectParameter > 64) uPlayer.Mod_player.volume[channel] = 64;
    else uPlayer.Mod_player.volume[channel] = effectParameter;
    break;

   case BREAKPATTERNTOROW:
    uPlayer.Mod_player.row = effectParameterX * 10 + effectParameterY;
    if(uPlayer.Mod_player.row >= ROWS)
     uPlayer.Mod_player.row = 0;
    if(!jumpFlag && !breakFlag) {
     uPlayer.Mod_player.orderIndex++;
     numsteps++;
        //UARTprintf("\033[2J");
        //UARTprintf("\033[0;0H");
        //UARTprintf("Current Song: [%s] - Number of Channels [%d]\n",uMod.Mod.name,uMod.Mod.numberOfChannels);
        //UARTprintf("Current Position: [%d] of [%d]\n",uPlayer.Mod_player.orderIndex, uMod.Mod.songLength);
     if(numsteps > uMod.Mod.songLength){
         numsteps = 0;
         //UARTprintf("Let's end early.\n");
         loadNextFile();
     }
     if(uPlayer.Mod_player.orderIndex >= uMod.Mod.songLength){
         //UARTprintf("Possible End 2?\n");
      uPlayer.Mod_player.orderIndex = 0;
     }
    }
    breakFlag = true;
    break;

   case 0xE:
    switch(effectParameterX) {
     case FINEPORTAMENTOUP:
      uPlayer.Mod_player.lastAmigaPeriod[channel] -= effectParameterY;
      break;

     case FINEPORTAMENTODOWN:
      uPlayer.Mod_player.lastAmigaPeriod[channel] += effectParameterY;
      break;

     case SETVIBRATOWAVEFORM:
      uPlayer.Mod_player.waveControl[channel] &= 0xF0;
      uPlayer.Mod_player.waveControl[channel] |= effectParameterY;
      break;

     case SETFINETUNE:
      uMod.Mod.samples[uPlayer.Mod_player.lastSampleNumber[channel]].fineTune = effectParameterY;
      if(uMod.Mod.samples[uPlayer.Mod_player.lastSampleNumber[channel]].fineTune > 7)
       uMod.Mod.samples[uPlayer.Mod_player.lastSampleNumber[channel]].fineTune -= 16;
      break;

     case PATTERNLOOP:
      if(effectParameterY) {
       if(uPlayer.Mod_player.patternLoopCount[channel])
        uPlayer.Mod_player.patternLoopCount[channel]--;
       else
        uPlayer.Mod_player.patternLoopCount[channel] = effectParameterY;
       if(uPlayer.Mod_player.patternLoopCount[channel])
        uPlayer.Mod_player.row = uPlayer.Mod_player.patternLoopRow[channel] - 1;
      } else
       uPlayer.Mod_player.patternLoopRow[channel] = uPlayer.Mod_player.row;
      break;

     case SETTREMOLOWAVEFORM:
      uPlayer.Mod_player.waveControl[channel] &= 0xF;
      uPlayer.Mod_player.waveControl[channel] |= effectParameterY << 4;
      break;

     case FINEVOLUMESLIDEUP:
      uPlayer.Mod_player.volume[channel] += effectParameterY;
      if(uPlayer.Mod_player.volume[channel] > 64) uPlayer.Mod_player.volume[channel] = 64;
      break;

     case FINEVOLUMESLIDEDOWN:
      uPlayer.Mod_player.volume[channel] -= effectParameterY;
      if(uPlayer.Mod_player.volume[channel] < 0) uPlayer.Mod_player.volume[channel] = 0;
      break;

     case NOTECUT:
      note = NONOTE_MOD;
      break;

     case PATTERNDELAY:
      uPlayer.Mod_player.patternDelay = effectParameterY;
      break;

     case INVERTLOOP:

      break;
    }
    break;

   case SETSPEED:
    if(effectParameter < 0x20)
     uPlayer.Mod_player.speed = effectParameter;
    else
     uPlayer.Mod_player.samplesPerTick = SAMPLERATE / (2 * effectParameter / 5);
    break;
  }

  if(note != NONOTE_MOD || uPlayer.Mod_player.lastAmigaPeriod[channel] &&
     effectNumber != VIBRATO && effectNumber != VIBRATOVOLUMESLIDE &&
     !(effectNumber == 0xE && effectParameterX == NOTEDELAY))
   Mixer.channelFrequency[channel] = uPlayer.Mod_player.amiga / uPlayer.Mod_player.lastAmigaPeriod[channel];

  if(note != NONOTE_MOD)
   Mixer.channelSampleOffset[channel] = sampleOffset << DIVIDER;

  if(sampleNumber)
   Mixer.channelSampleNumber[channel] = uPlayer.Mod_player.lastSampleNumber[channel];

  if(effectNumber != TREMOLO)
   Mixer.channelVolume[channel] = uPlayer.Mod_player.volume[channel];

 }
}

static void processTick() {
 static uint8_t channel;
 static uint8_t sampleNumber;
 static uint16_t note;
 static uint8_t effectNumber;
 static uint8_t effectParameter;
 static uint8_t effectParameterX;
 static uint8_t effectParameterY;
 static uint16_t tempNote;

 for(channel = 0; channel < uMod.Mod.numberOfChannels; channel++) {

  if(uPlayer.Mod_player.lastAmigaPeriod[channel]) {

   sampleNumber = uPlayer.Mod_player.currentPattern.sampleNumber[uPlayer.Mod_player.lastRow][channel];
   note = uPlayer.Mod_player.currentPattern.note[uPlayer.Mod_player.lastRow][channel];
   effectNumber = uPlayer.Mod_player.currentPattern.effectNumber[uPlayer.Mod_player.lastRow][channel];
   effectParameter = uPlayer.Mod_player.currentPattern.effectParameter[uPlayer.Mod_player.lastRow][channel];
   effectParameterX = effectParameter >> 4;
   effectParameterY = effectParameter & 0xF;

   switch(effectNumber) {
    case ARPEGGIO:
     if(effectParameter)
      switch(uPlayer.Mod_player.tick % 3) {
       case 0:
        Mixer.channelFrequency[channel] = uPlayer.Mod_player.amiga / uPlayer.Mod_player.lastAmigaPeriod[channel];
        break;
       case 1:
        tempNote = uPlayer.Mod_player.lastNote[channel] + effectParameterX * 8 + uMod.Mod.samples[uPlayer.Mod_player.lastSampleNumber[channel]].fineTune;
        if(tempNote < 296) Mixer.channelFrequency[channel] = uPlayer.Mod_player.amiga / amigaPeriods[tempNote];
        break;
       case 2:
        tempNote = uPlayer.Mod_player.lastNote[channel] + effectParameterY * 8 + uMod.Mod.samples[uPlayer.Mod_player.lastSampleNumber[channel]].fineTune;
        if(tempNote < 296) Mixer.channelFrequency[channel] = uPlayer.Mod_player.amiga / amigaPeriods[tempNote];
        break;
      }
     break;

    case PORTAMENTOUP:
     uPlayer.Mod_player.lastAmigaPeriod[channel] -= effectParameter;
     if(uPlayer.Mod_player.lastAmigaPeriod[channel] < 113) uPlayer.Mod_player.lastAmigaPeriod[channel] = 113;
     Mixer.channelFrequency[channel] = uPlayer.Mod_player.amiga / uPlayer.Mod_player.lastAmigaPeriod[channel];
     break;

    case PORTAMENTODOWN:
     uPlayer.Mod_player.lastAmigaPeriod[channel] += effectParameter;
     if(uPlayer.Mod_player.lastAmigaPeriod[channel] > 856) uPlayer.Mod_player.lastAmigaPeriod[channel] = 856;
     Mixer.channelFrequency[channel] = uPlayer.Mod_player.amiga / uPlayer.Mod_player.lastAmigaPeriod[channel];
     break;

    case TONEPORTAMENTO:
     portamento(channel);
     break;

    case VIBRATO:
     vibrato(channel);
     break;

    case PORTAMENTOVOLUMESLIDE:
     portamento(channel);
     uPlayer.Mod_player.volume[channel] += effectParameterX - effectParameterY;
     if(uPlayer.Mod_player.volume[channel] < 0) uPlayer.Mod_player.volume[channel] = 0;
     else if(uPlayer.Mod_player.volume[channel] > 64) uPlayer.Mod_player.volume[channel] = 64;
     Mixer.channelVolume[channel] = uPlayer.Mod_player.volume[channel];
     break;

    case VIBRATOVOLUMESLIDE:
     vibrato(channel);
     uPlayer.Mod_player.volume[channel] += effectParameterX - effectParameterY;
     if(uPlayer.Mod_player.volume[channel] < 0) uPlayer.Mod_player.volume[channel] = 0;
     else if(uPlayer.Mod_player.volume[channel] > 64) uPlayer.Mod_player.volume[channel] = 64;
     Mixer.channelVolume[channel] = uPlayer.Mod_player.volume[channel];
     break;

    case TREMOLO:
     tremolo(channel);
     break;

    case VOLUMESLIDE:
     uPlayer.Mod_player.volume[channel] += effectParameterX - effectParameterY;
     if(uPlayer.Mod_player.volume[channel] < 0) uPlayer.Mod_player.volume[channel] = 0;
     else if(uPlayer.Mod_player.volume[channel] > 64) uPlayer.Mod_player.volume[channel] = 64;
     Mixer.channelVolume[channel] = uPlayer.Mod_player.volume[channel];
     break;

    case 0xE:
     switch(effectParameterX) {
      case RETRIGGERNOTE:
       if(!effectParameterY) break;
       if(!(uPlayer.Mod_player.tick % effectParameterY)) {
        Mixer.channelSampleOffset[channel] = 0;
       }
       break;

      case NOTECUT:
       if(uPlayer.Mod_player.tick == effectParameterY)
        Mixer.channelVolume[channel] = uPlayer.Mod_player.volume[channel] = 0;
       break;

      case NOTEDELAY:
       if(uPlayer.Mod_player.tick == effectParameterY) {
        if(sampleNumber) uPlayer.Mod_player.volume[channel] = uMod.Mod.samples[uPlayer.Mod_player.lastSampleNumber[channel]].volume;
        if(note != NONOTE_MOD) Mixer.channelSampleOffset[channel] = 0;
        Mixer.channelFrequency[channel] = uPlayer.Mod_player.amiga / uPlayer.Mod_player.lastAmigaPeriod[channel];
        Mixer.channelVolume[channel] = uPlayer.Mod_player.volume[channel];
       }
       break;
     }
     break;
   }

  }

 }
}

void mod_player() {
 if(uPlayer.Mod_player.tick == uPlayer.Mod_player.speed) {
  uPlayer.Mod_player.tick = 0;

  if(uPlayer.Mod_player.row == ROWS) {
   uPlayer.Mod_player.orderIndex++;
   numsteps++;
   //UARTprintf("\033[2J");
   //UARTprintf("\033[0;0H");
   //UARTprintf("Current Song: [%s] - Number of Channels [%d]\n",uMod.Mod.name,uMod.Mod.numberOfChannels);
   //UARTprintf("Current Position: [%d] of [%d]\n",uPlayer.Mod_player.orderIndex, uMod.Mod.songLength);
   if(numsteps > uMod.Mod.songLength){
       numsteps = 0;
       //UARTprintf("Let's end early.\n");
       loadNextFile();
   }
   if(uPlayer.Mod_player.orderIndex == uMod.Mod.songLength){
       //UARTprintf("Possible End 3?\n");
       loadNextFile();
    uPlayer.Mod_player.orderIndex = 0;
   }
   uPlayer.Mod_player.row = 0;
  }

  if(uPlayer.Mod_player.patternDelay) {
   uPlayer.Mod_player.patternDelay--;
  } else {
   if(uPlayer.Mod_player.orderIndex != uPlayer.Mod_player.oldOrderIndex)
    loadPattern(uMod.Mod.order[uPlayer.Mod_player.orderIndex]);
   uPlayer.Mod_player.oldOrderIndex = uPlayer.Mod_player.orderIndex;
   processRow();
  }

 } else {
  processTick();
 }
 uPlayer.Mod_player.tick++;
}

void mod_mixer() {
 static int16_t sumL;
 static int16_t sumR;
 static uint8_t channel;
 static uint32_t samplePointer;
 static uint16_t fatBufferSize;
 static uint32_t count;
 static int8_t current;
 static int8_t next;
 static int16_t out;

 sumL = 0;
 sumR = 0;
 for(channel = 0; channel < uMod.Mod.numberOfChannels; channel++) {

  if(!Mixer.channelFrequency[channel] ||
     !uMod.Mod.samples[Mixer.channelSampleNumber[channel]].length) continue;

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

   //LEDLAT = ~(1 << (channel & 0x3)); //TODO: led turn on
   f_lseek(&file, samplePointer);
   f_read(&file, FatBuffer.channels[channel], fatBufferSize, &count);
   //LEDLAT = 0xF;

   FatBuffer.samplePointer[channel] = samplePointer;
   FatBuffer.channelSampleNumber[channel] = Mixer.channelSampleNumber[channel];
  }

  current = FatBuffer.channels[channel][(samplePointer - FatBuffer.samplePointer[channel]) & FATBUFFERSIZE - 1]; //instead of FatBufferSIZE I've put 0x400 so that I can use non 2 power sizes
  next = FatBuffer.channels[channel][(samplePointer + 1 - FatBuffer.samplePointer[channel]) & FATBUFFERSIZE - 1];//instead of FatBufferSIZE I've put 0x400

  out = current;

  // Integer linear interpolation
  out += (next - current) * (Mixer.channelSampleOffset[channel] & (1 << DIVIDER) - 1) >> DIVIDER;

  // Upscale to BITDEPTH
  out <<= BITDEPTH - 8;

  // Channel volume
  out = out * Mixer.channelVolume[channel] >> 6;

  // Channel panning
  sumL += out * min(128 - Mixer.channelPanning[channel], 64) >> 6;
  sumR += out * min(Mixer.channelPanning[channel], 64) >> 6;
 }

 // Downscale to BITDEPTH
 //sumL /= uMod.Mod.numberOfChannels;
 //sumR /= uMod.Mod.numberOfChannels;
 sumL>>=(BITDEPTH - 8);
 sumR>>=(BITDEPTH - 8);

 // Fill the sound buffer with unsigned values
 SoundBuffer.left[SoundBuffer.writePos] = sumL + (1 << BITDEPTH - 1);
 SoundBuffer.right[SoundBuffer.writePos] = sumR + (1 << BITDEPTH - 1);
 SoundBuffer.writePos++;
 SoundBuffer.writePos &= SOUNDBUFFERSIZE - 1;
}

void loadMod() {
 uint8_t channel;

 loadHeader();
 //We want to ignore files with too many channels
 if(uMod.Mod.numberOfChannels > CHANNELS){
     //UARTprintf("Too many Channels: [%d]\n",uMod.Mod.numberOfChannels);
     if(nextprevfile == 0){
         loadNextFile();
     }
     else{
         loadPreviousFile();
     }
     return;
 }
 loadSamples();
 //UARTprintf("Song name: [%s] channels - [%d]\n",uMod.Mod.name,uMod.Mod.numberOfChannels);
 //UARTprintf("Length: [%d]\n",uMod.Mod.songLength);
 uPlayer.Mod_player.amiga = AMIGA_MOD;
 uPlayer.Mod_player.samplesPerTick = SAMPLERATE / (2 * 125 / 5); // Hz = 2 * BPM / 5
 uPlayer.Mod_player.speed = 6;
 uPlayer.Mod_player.tick = uPlayer.Mod_player.speed;
 uPlayer.Mod_player.row = 0;

 uPlayer.Mod_player.orderIndex = 0;
 uPlayer.Mod_player.oldOrderIndex = 0xFF;
 uPlayer.Mod_player.patternDelay = 0;

 for(channel = 0; channel < uMod.Mod.numberOfChannels; channel++) {
  uPlayer.Mod_player.patternLoopCount[channel] = 0;
  uPlayer.Mod_player.patternLoopRow[channel] = 0;

  uPlayer.Mod_player.lastAmigaPeriod[channel] = 0;

  uPlayer.Mod_player.waveControl[channel] = 0;

  uPlayer.Mod_player.vibratoSpeed[channel] = 0;
  uPlayer.Mod_player.vibratoDepth[channel] = 0;
  uPlayer.Mod_player.vibratoPos[channel] = 0;

  uPlayer.Mod_player.tremoloSpeed[channel] = 0;
  uPlayer.Mod_player.tremoloDepth[channel] = 0;
  uPlayer.Mod_player.tremoloPos[channel] = 0;

  FatBuffer.samplePointer[channel] = 0;
  FatBuffer.channelSampleNumber[channel] = 0xFF;

  Mixer.channelSampleOffset[channel] = 0;
  Mixer.channelFrequency[channel] = 0;
  Mixer.channelVolume[channel] = 0;
  switch(channel % 4) {
   case 0:
   case 3:
    Mixer.channelPanning[channel] = STEREOSEPARATION;
    break;
   default:
    Mixer.channelPanning[channel] = 128 - STEREOSEPARATION;
  }
 }

 SoundBuffer.writePos = 0;
 SoundBuffer.readPos = 0;
}

uint16_t mod_getSamplesPerTick(){
    return uPlayer.Mod_player.samplesPerTick;
}
#endif