#ifndef __MOD32_H__
#define __MOD32_H__

// Effects
#define ARPEGGIO              0x0
#define PORTAMENTOUP          0x1
#define PORTAMENTODOWN        0x2
#define TONEPORTAMENTO        0x3
#define VIBRATO               0x4
#define PORTAMENTOVOLUMESLIDE 0x5
#define VIBRATOVOLUMESLIDE    0x6
#define TREMOLO               0x7
#define SETCHANNELPANNING     0x8
#define SETSAMPLEOFFSET       0x9
#define VOLUMESLIDE           0xA
#define JUMPTOORDER           0xB
#define SETVOLUME             0xC
#define BREAKPATTERNTOROW     0xD
#define SETSPEED              0xF

// 0xE subset
#define SETFILTER             0x0
#define FINEPORTAMENTOUP      0x1
#define FINEPORTAMENTODOWN    0x2
#define GLISSANDOCONTROL      0x3
#define SETVIBRATOWAVEFORM    0x4
#define SETFINETUNE           0x5
#define PATTERNLOOP           0x6
#define SETTREMOLOWAVEFORM    0x7
#define RETRIGGERNOTE         0x9
#define FINEVOLUMESLIDEUP     0xA
#define FINEVOLUMESLIDEDOWN   0xB
#define NOTECUT               0xC
#define NOTEDELAY             0xD
#define PATTERNDELAY          0xE
#define INVERTLOOP            0xF

//Prototypes
void mod_player();
void mod_mixer();
void loadMod();
uint16_t mod_getSamplesPerTick();

#endif
