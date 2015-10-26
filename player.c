/*
 * player.c
 *
 *  Created on: 10 2012
 *      Author: R.K.
 */
//#include "strcasestr.c"
#define PLAYERSRC
#include "global.h"
#undef PLAYERSRC
#include <string.h>
//#include "strcasestr.c"
#include "utils/uartstdio.h"



#include "mod32.h"
#include "s3m32.h"

//static uint16_t modFileNumber = 0;

//Define file format handlers
FileHandler g_fhHandlers[]={{".MOD",mod_player,mod_mixer,loadMod,mod_getSamplesPerTick},
							{".S3M",s3m_player,s3m_mixer,loadS3m,s3m_getSamplesPerTick}};

//FileHandler g_fhHandlers[]={{".MOD",mod_player,mod_mixer,loadMod,mod_getSamplesPerTick}};

FileHandler *g_pCurrentHandler;

void player() {
	if(g_pCurrentHandler!=NULL)
		(*(g_pCurrentHandler->player))();
}

void mixer(){
	if(g_pCurrentHandler!=NULL)
		(*(g_pCurrentHandler->mixer))();
}

uint16_t getSamplesPerTick(){
	if(g_pCurrentHandler!=NULL)
		return (*(g_pCurrentHandler->getSamplesPerTick))();
	return 0;
}

FileHandler *getHandler(const char *fileName){
	int i;
	if(fileName==NULL)
		return NULL;
	if(fileName[0]=='.' && fileName[1]==NULL)
		return NULL;

	if(fileName[0]=='.' && fileName[1]=='.' && fileName[2]==NULL)
			return NULL;

	for(i=0;i<sizeof(g_fhHandlers);i++){
		if(strcasestr(fileName,g_fhHandlers[i].szFileExt)!=NULL){
			return &(g_fhHandlers[i]);
		}
	}
	return NULL;
}

void loadNextFile() {
	UARTprintf("\033[2J");
	UARTprintf("\033[0;0H");
	nextprevfile = 0;
	uint8_t *dotPointer;
	g_pCurrentHandler = NULL;

	do {
		f_readdir(&dir, &fileInfo);
		UARTprintf("Filename: %s\n",fileInfo.fname);
		if(strcmp(fileInfo.fname ,"GAME18~1.MOD")==0){
			//f_unlink(fileInfo.fname);
			f_readdir(&dir, &fileInfo);
		}
		if (fileInfo.fname[0] == 0)
			break;
		dotPointer = strrchr(fileInfo.fname, '.');
		if (dotPointer != NULL) {
			g_pCurrentHandler = getHandler(dotPointer);
			if (g_pCurrentHandler != NULL)
				break;
		}

	} while (dotPointer == NULL || g_pCurrentHandler == NULL);

	if (fileInfo.fname[0] != 0 && g_pCurrentHandler!=NULL) {
		f_open(&file, fileInfo.fname, FA_READ);

		UARTprintf("[%d]: Opened File: %s with handler: [%s]\n",modFileNumber, fileInfo.fname,g_pCurrentHandler->szFileExt);
		modFileNumber++;
		(*(g_pCurrentHandler->loader))();
		//UARTprintf("Song name: [%s] paterns: %d\n", uMod.S3m.name,
		//		uMod.S3m.songLength);
	}else{
		UARTprintf("Can't open any files\n");
	}
}

void loadPreviousFile() {
	 UARTprintf("\033[2J");
	   UARTprintf("\033[0;0H");
nextprevfile = 1;
 uint16_t i;
 uint8_t *dotPointer;

 if(modFileNumber > 1) {
  modFileNumber--;
  f_readdir(&dir, NULL);
  for(i = 0; i < modFileNumber; i++) {
   do {
    f_readdir(&dir, &fileInfo);
    dotPointer = strrchr(fileInfo.fname, '.');
    if(dotPointer!=NULL){
    	g_pCurrentHandler = getHandler(fileInfo.fname);

    }
   } while(dotPointer == NULL || g_pCurrentHandler == NULL);
  }
  f_open(&file, fileInfo.fname, FA_READ);
  UARTprintf("Opened File: %s ",fileInfo.fname);
  (*(g_pCurrentHandler->loader))();
  //UARTprintf("Song name: [%s]\n",uMod.S3m.name);
 }
}

void loadRandomFile(uint8_t skips) {
	 UARTprintf("\033[2J");
	   UARTprintf("\033[0;0H");
	nextprevfile = 0;
	uint8_t *dotPointer;
	uint8_t i;
		g_pCurrentHandler = NULL;

		do {
			for(i=0;i<skips;i++){
			f_readdir(&dir, &fileInfo);
			}
			if (fileInfo.fname[0] == 0)
				break;
			dotPointer = strrchr(fileInfo.fname, '.');
			if (dotPointer != NULL) {
				g_pCurrentHandler = getHandler(dotPointer);
				if (g_pCurrentHandler != NULL)
					break;

			}
		} while (dotPointer == NULL || g_pCurrentHandler == NULL);


		if (fileInfo.fname[0] != 0 && g_pCurrentHandler!=NULL) {
			f_open(&file, fileInfo.fname, FA_READ);

			UARTprintf("Opened File: %s with handler: [%s]\n", fileInfo.fname,g_pCurrentHandler->szFileExt);
			modFileNumber+= skips;
			(*(g_pCurrentHandler->loader))();
			//UARTprintf("Song name: [%s] paterns: %d\n", uMod.S3m.name,
			//		uMod.S3m.songLength);
		}else{
			UARTprintf("Can't open any files\n");
		}
}

