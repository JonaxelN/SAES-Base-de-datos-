#if !defined(__SB_H)
#define __SB_H
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include "dma.h"

#define WRITE_DATA_SB   	0x10
#define ON_SOUND_SB		0xD1
#define OFF_SOUND_SB		0xD3
#define HALT_DMA_8		0xD0
#define TIME_CONSTANT 		0x40
#define DMA_8_BIT_DAC_LO	0x14
#define DMA_8_BIT_DAC_HI	0x91
#define SET_LEN_DMA_8_BIT	0x48
#define MONO	0
#define STEREO	1
#define USE_FILTER	0
#define BYPASS_FILTER	1
#define MAX_BASE 5
#define lo(value) (unsigned char)((value) & 0x00FF)
#define hi(value) (unsigned char)((value) >> 8)


#ifdef __cplusplus
	#define __CPPARGS ...
#else
	#define __CPPARGS
#endif

typedef struct {
	char id_riff[4];long long_riff;char id_chuck[4];char fmt[4];long long_chuck;
	int tipo;int canales;long freq;long bytes;int align;int bits;
} TituloWave;

typedef struct {
	char id_dato[4];long long_dato;
} TituloDatosWave;

typedef struct{
	int	puerto;char irq;char dma;int ver;
} Config;
//**************************************************************************
//*               MANEJA LA TARJETA DE SONIDO                              *
//**************************************************************************
class SOUNDBLASTER{
	FILE *f;
	char far *raw;
	static Config isb;
	unsigned int len,l;
	static DMA_Buf dBuf;
	static volatile int DMA_complete;
	static int bases[MAX_BASE],baseAddr,ver,termino,abierto;
	static char SixteenBit,Stereo,MaxFrequency,Fifo,DMAChannel,IRQ,inicializado;
		static void far interrupt (*OldIRQ)(__CPPARGS);
		static void far interrupt Handler(__CPPARGS);
		void play( char far * , unsigned int );
		void WriteMixer( char , char  );
		void Write(unsigned char);
		unsigned char Read();
		int	EnvData();
		char Check();
		int Setup();
	public:
		SOUNDBLASTER();
		~SOUNDBLASTER();
		unsigned int rate( unsigned int );
		void mastervolume( char , char );
		void playwave( char * );
		void verifica();
		void reset();
		int version();
		int inicia();
		int libre();
};
//********************* VARIABLES ESTATICAS *******************************
int SOUNDBLASTER::bases[MAX_BASE]={0x220,0x230,0x240,0x250,0x260,};
int SOUNDBLASTER::baseAddr=0x220;
int SOUNDBLASTER::ver=0;
int SOUNDBLASTER::termino=1;
int SOUNDBLASTER::abierto=0;
volatile int SOUNDBLASTER::DMA_complete=1;
Config SOUNDBLASTER::isb;
DMA_Buf SOUNDBLASTER::dBuf;
char SOUNDBLASTER::inicializado=0;
char SOUNDBLASTER::SixteenBit=0;
char SOUNDBLASTER::Stereo=0;
char SOUNDBLASTER::MaxFrequency=0;
char SOUNDBLASTER::Fifo=0;
char SOUNDBLASTER::DMAChannel=1;
char SOUNDBLASTER::IRQ=5;
//*************************************************************************
void far interrupt (*SOUNDBLASTER::OldIRQ)(__CPPARGS);
void far interrupt SOUNDBLASTER::Handler(__CPPARGS){
	DMA_complete = 1;
	inportb(baseAddr+0xE);
	outportb(0x20,0x20);
}
//*************************************************************************
SOUNDBLASTER::SOUNDBLASTER(){
	if(inicia()==0){textcolor(128+14);
	cprintf("\nSound Blaster no inicializado...");delay(1000);}delay(1000);
	textcolor(7);clrscr();
}
//*************************************************************************
SOUNDBLASTER::~SOUNDBLASTER(){
	reset();
}
//*************************************************************************
int SOUNDBLASTER::inicia(){
	clrscr();
	if ( EnvData()==0 ){
		printf("No esta configurado la variable BLASTER !!!\n");
		printf("teclee por ejemplo:  SET BLASTER A220 I5 D1 T3\n");return(0);}
	/*else printf("\nSOUND BLASTER:\nPuerto:%X\nIRQ:%d\nDMA:%d\nT:%d\n",isb.puerto,isb.irq,isb.dma,isb.ver);*/
	if ( Setup()==0 ){
		printf("HardWare o Driver de Sound Blaster no disponible\n");return(0);}
	/*textcolor(128+14);cprintf("\nSound Blaster Ok...");return(1);*/
}
//*************************************************************************
unsigned char SOUNDBLASTER::Read(){
	unsigned int value;
	while (!(inportb((baseAddr+0xE)) & 0x80));
	value = inportb((baseAddr+0xA));
	return value;
}
//*************************************************************************
int	SOUNDBLASTER::EnvData(){
	char *t, *t1, *blaster;
	isb.puerto=0x220; isb.irq=5; isb.dma=1; isb.ver=0x310;
	t = getenv("BLASTER");
	if ( t == NULL ) return 0;
	blaster = strdup(t);
	t = strtok(blaster," \t");
	while ( t ){
		switch(t[0]){
			case 'A':case 'a':isb.puerto = (int)strtol(t+1,&t1,16);break;
			case 'I':case 'i':isb.irq = atoi(t+1);break;
			case 'D':case 'd':isb.dma = atoi(t+1);break;
			case 'T':case 't':isb.ver = atoi(t+1);break;
		}
	t = strtok(NULL," \t");
	}
	free(blaster);
	return 1;
}

//*************************************************************************
char SOUNDBLASTER::Check(){
	int i, j;
	for ( j=0; j<MAX_BASE; j++ ){
		baseAddr=bases[j];
		outportb(baseAddr+6,1);delay(10);
		outportb(baseAddr+6,0);delay(10);
		for(i=0; i<30000; i++ )
			if ( Read()==0xAA ) return 1;
	}
	return 0;
}
//*************************************************************************
void SOUNDBLASTER::Write(unsigned char value){
	while (inportb(baseAddr+0xC) & 0x80);
	outportb(baseAddr+0xC, value);
}
//*************************************************************************
int SOUNDBLASTER::version(){
	char ch, ch1;
	Write(0xE1); ch=Read(); ch1=Read(); ver=ch<<8|ch1;
	if ( ver>=0x200 ) Fifo=1;
	if ( ver>=0x201 ) MaxFrequency=1;
	if ( ver>=0x300 ) Stereo=1;
	if ( ver>=0x301 ) SixteenBit=1;
	return ver;
}
//*************************************************************************
void SOUNDBLASTER::WriteMixer( char index, char val ){
	outportb(baseAddr+4,index);
	outportb(baseAddr+5,val);
}
//*************************************************************************
void SOUNDBLASTER::mastervolume( char left, char right ){
	if(inicializado==1){
	char val;val=right&0xf;val|=(left<<4)&0xf0;WriteMixer(0x22,val);}
}
//*************************************************************************
int SOUNDBLASTER::Setup(){
	unsigned long physical;unsigned char im, tm;
	if ( inicializado==1 ) return 0;
	switch ( isb.puerto ){
		case 0x210: case 0x220: case 0x230: case 0x240: case 0x250: case 0x260:break;
		default: isb.puerto=0;
	}
	baseAddr=isb.puerto;
	if ( isb.dma<0 || isb.dma>3 ) DMAChannel=1;
		else DMAChannel=isb.dma;
	if ( isb.irq!=2 && isb.irq!=3 && isb.irq!=5 && isb.irq!=7 )	IRQ=7;
		else IRQ=isb.irq;
	if ( Check()!=1 ) return 0;
	inicializado=1; version(); dBuf.len=1024;// TAMA¥O DE BUFFER
	if ( dmaMalloc(&dBuf)==0 ) return 0;
	disable(); // deshabilita para colocar nueva interrupcion
	OldIRQ = getvect(0x08 + IRQ);
	setvect(0x08 + IRQ,Handler);
	DMA_complete = 1;
	im = inportb(0x21);
	tm = ~(1 << IRQ);
	outportb(0x21,im & tm);
	enable(); // habilita nueva interrupcion
	Write(ON_SOUND_SB);
	return ver;
}
//*************************************************************************
void SOUNDBLASTER::reset(){
	unsigned char i;
	if(inicializado==1){
		inportb(baseAddr+0xE);
		outportb(0x20,0x20);
		dmaFree(&dBuf);
		Write(OFF_SOUND_SB);
		Write(HALT_DMA_8);
		disable(); // deshabilita para ubicar nuevamente la interrupcion original
		i = inportb(0x21);
		outportb(0x21, i | (1 << IRQ));
		setvect(0x08 + IRQ,OldIRQ);
		enable(); // habilita
		inicializado=0;
	}
}
//*************************************************************************
unsigned int SOUNDBLASTER::rate( unsigned int vrate ){
	unsigned char tc;
	if ( MaxFrequency==0 || vrate<22528 ){
		if ( vrate<5000 ) vrate=5000;
		if ( vrate>22528 ) vrate=22528;
		tc = (unsigned char)(256 - (1000000/vrate));
		vrate = (unsigned int)(1000000/(256-tc));
	}
	else{
		if ( vrate<5000 ) vrate=5000;
		if ( vrate>45056 ) vrate=45056;
		tc = (unsigned char)(hi((unsigned int)(65536-(256000000L/vrate))));
	}
	Write(TIME_CONSTANT);
	Write(tc);
	return vrate;
}
//*************************************************************************
void SOUNDBLASTER::play( char far * raw, unsigned int len ){
	int MaskPort, ClrPort, ModePort, ModeDMA, CountPort, PagePort, BaseAddrPort;
	int pagepuertos[4]={ 0x87, 0x83, 0x81, 0x82 };
	if ( len==0 ) return;
	if ( len>dBuf.len ) return;
	_fmemcpy(dBuf.dma,raw,len);
	DMA_complete=0;
	MaskPort=0x0A; ClrPort=0xC; ModePort=0xB;
	ModeDMA=0x48+DMAChannel;
	CountPort=1+DMAChannel*2;
	BaseAddrPort=DMAChannel*2;
	PagePort=pagepuertos[DMAChannel];
	outportb(MaskPort, 4 + DMAChannel);
	outportb(ClrPort, DMAChannel );
	outportb(ModePort, ModeDMA );
	outportb(BaseAddrPort,dBuf.lo_off);
	outportb(BaseAddrPort,dBuf.hi_off);
	outportb(PagePort,dBuf.page);
	outportb(CountPort,lo(len-1));
	outportb(CountPort,hi(len-1));
	outportb(MaskPort, DMAChannel );
	if ( MaxFrequency==0 ){
		Write(DMA_8_BIT_DAC_LO);
		Write(lo(len-1));
		Write(hi(len-1));
	}
	else{
		Write(SET_LEN_DMA_8_BIT);
		Write(lo(len-1));
		Write(hi(len-1));
		Write(DMA_8_BIT_DAC_HI);
	}
}
//*************************************************************************
int SOUNDBLASTER::libre(){
	return(DMA_complete);
}
//*************************************************************************
void SOUNDBLASTER::verifica(){
	if(termino==0){
		if(libre()!=0){
			l=fread((char *)raw,1,len,f);
			if(l!=0) play(raw,l);else termino=1;
		}
	}
	else{if(abierto==1){abierto=0;fclose(f);delete[] raw;}}
}
//*************************************************************************
void SOUNDBLASTER::playwave( char * name ){
	if(inicializado==1){
		int bandera=0;
		TituloWave tw;
		TituloDatosWave tdw;
		if(abierto==1){abierto=0;fclose(f);delete[] raw;}
		if ( (f = fopen(name,"rb")) == NULL ){
			printf("no existe el archivo - %s\n",name);return;}
		abierto=1;
		if ( (raw = new char far[dBuf.len])==NULL ){
			printf("no se pudo reservar memoria para - %s\n",name);return;}
		fread(&tw,sizeof(TituloWave),1,f);
		fseek( f, tw.long_chuck-16, SEEK_CUR);//con 4 en vez de 16 con SoundRec
		fread( &tdw, sizeof(TituloDatosWave),1, f);
		/*printf("LEN RIFF\t\t - %ld\n", tw.long_riff );
		printf("LEN Chuck\t\t - %ld\n", tw.long_chuck );
		printf("Tipo\t\t\t - %d\n", tw.tipo );
		printf("Canales\t\t\t - %d\n", tw.canales );
		printf("Sample por Segundo\t - %u\n", tw.freq );
		printf("Bytes por Segundo\t - %ld\n", tw.bytes );
		printf("Bits\t\t\t - %d\n", tw.bits );
		printf("Aligned\t\t\t - %d\n", tw.align );
		printf("LEN Data\t\t - %lu\n", tdw.long_dato );*/
		if ( strncmp(tw.id_riff,"RIFF",4)!=0 ){
			printf("Problemas para encontrar RIFF\n"); bandera=1;}
		if ( strncmp(tw.id_chuck,"WAVE",4)!=0 ){
			printf("Problemas para encontrar WAVE\n"); bandera=1;}
		if ( strncmp(tw.fmt,"fmt ",4)!=0 ){
			printf("Problemas para encontrar FMT\n"); bandera=1;}
		if ( strncmp(tdw.id_dato,"data",4)!=0 ){
			printf("Problemas para encontrar DATA\n"); bandera=1;}
		if(bandera==0){
			while(libre()==0);
			tw.freq=rate(tw.freq*tw.canales);
			//printf("tocando con rate - %u\n",tw.freq);
			len=dBuf.len;termino=0;
		}else printf("Problemas con el archivo o su estructura...\n");
	}
}

SOUNDBLASTER sb;
#endif
