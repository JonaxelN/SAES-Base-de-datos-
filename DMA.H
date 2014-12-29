#if !defined(__DMA_H)
#define __DMA_H

typedef struct
{
	char far * buf;
	char far * dma;
	unsigned int len;
	unsigned long fisico;
	unsigned char lo_off;
	unsigned char hi_off;
	unsigned int page;
} DMA_Buf;

//************************************************************************
int	dmaMalloc( DMA_Buf * dmb ){
	char * p;
	unsigned int s, l;
	dmb->buf=new char far [dmb->len];
	if ( dmb->buf==0 ){printf("No se pudo reservar memoria para DMA\n");return 0;}
	s=FP_SEG(dmb->buf);
	s&=0xfff;
	l=(dmb->len)>>4;
	if ( s < ( 0x1000-l) )
		dmb->dma=dmb->buf;
	else{
		if ( (0x1000-s)<(s+l-0x1000) ){
			dmb->dma=(char far *)MK_FP(FP_SEG(dmb->buf)+(0x1000-s),0);
			dmb->len-=((0x1000-s)<<4)-FP_OFF(dmb->buf);
		}
		else{
			dmb->dma=dmb->buf;
			dmb->len=((0x1000-s)<<4)-FP_OFF(dmb->buf);
		}
	}
	dmb->fisico=((unsigned long)FP_OFF(dmb->dma))+(((unsigned long)FP_SEG(dmb->dma))<<4);
	s=dmb->fisico % 0x10000L;
	dmb->lo_off=(unsigned char)(s & 0x00FF);
	dmb->hi_off=(unsigned char)(s >> 8);
	dmb->page=(unsigned char)((dmb->fisico>>16)&0x00FF);
	return 1;
}
//************************************************************************
void dmaFree( DMA_Buf * dmb ){
	if ( dmb->buf==0 ) return;
	delete[](dmb->buf);
	dmb->buf=dmb->dma=0;
	dmb->len=0;
}
#endif