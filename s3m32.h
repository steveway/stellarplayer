#ifndef __S3M32_H__
#define __S3M32_H__

#include <stdint.h>
//TODO: Change Macros so that they are not redefined (add prefix?)
// Effects
#define SETSPEED                 0x1  // Axx
#define JUMPTOORDER              0x2  // Bxx
#define BREAKPATTERNTOROW        0x3  // Cxx
#define VOLUMESLIDE              0x4  // Dxx
#define PORTAMENTODOWN           0x5  // Exx
#define PORTAMENTOUP             0x6  // Fxx
#define TONEPORTAMENTO           0x7  // Gxx
#define VIBRATO                  0x8  // Hxy
#define TREMOR                   0x9  // Ixy
#define ARPEGGIO                 0xA  // Jxy
#define VIBRATOVOLUMESLIDE       0xB  // Kxy
#define PORTAMENTOVOLUMESLIDE    0xC  // Lxy
#define SETSAMPLEOFFSET          0xF  // Oxy
#define RETRIGGERNOTEVOLUMESLIDE 0x11 // Qxy
#define TREMOLO                  0x12 // Rxy
#define SETTEMPO                 0x14 // Txx
#define FINEVIBRATO              0x15 // Uxy
#define SETGLOBALVOLUME          0x16 // Vxx

// 0x13 subset
#define SETFILTER                0x0
#define SETGLISSANDOCONTROL      0x1
#define SETFINETUNE              0x2
#define SETVIBRATOWAVEFORM       0x3
#define SETTREMOLOWAVEFORM       0x4
#define SETCHANNELPANNING        0x8
#define STEREOCONTROL            0xA
#define PATTERNLOOP              0xB
#define NOTECUT                  0xC
#define NOTEDELAY                0xD
#define PATTERNDELAY             0xE
#define FUNKREPEAT               0xF

//Prototypes
void s3m_player(void);
void s3m_mixer(void);
void loadS3m(void);
uint16_t s3m_getSamplesPerTick(void);
extern void loadNextFile(void);
extern void loadPreviousFile(void);

#endif