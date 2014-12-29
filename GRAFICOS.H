#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
void asigna_rgb(char *dir_lectura);
#define VIDEO 0x10
#define MASC 0xCF
//unsigned char obt_tecla(void)
//{
//   char tecla;
//   tecla=bioskey(0);
  /* int valor;         */
  /* valor=inport(0x60);*/
  /* outport(0x61,0x80);*/
  /* outport(0x61,0x7F);*/
  /* valor=valor/0xFF;  */
  /* tecla=valor;       */
//   return(tecla);

 /*  unsigned int *cabeza = (unsigned int *)0x0000041A;  */
 /*  unsigned int *cola = (unsigned int *)0x0000041C;    */
 /*  *cabeza=*cola;                                      */
 /*  return inportb(0x60);                               */
//}

void modgraf(int modo)
 /* Cambia de modo gr fico dando como argumento el modo en formato decimal */
{
   union REGS regs;
   unsigned char far *puntfar;
   unsigned char car;
   unsigned char vidbits;
   if (modo==7)
      vidbits=0x30;
   else vidbits=0x10;
   puntfar = (char far *) 0x410;
   car=*(puntfar);
   car=car&MASC;
   *(puntfar)=car|vidbits;
   regs.h.al=(char)modo;
   regs.h.ah=0;
   int86(VIDEO, &regs, &regs);
}

void pon_pcx (char fichero[])
/* Pone un fichero .PCX en pantalla dando como argumento el nombre del fi-*/
/* chero con su extension                                                 */
{
   FILE *f;
   char far *puntfar=(char far *)0xA0000000;
   char *dir_lectura;
   int cont, cabecera, col, filas;
   unsigned char tmp, byte;
   cont=cabecera=0;
   filas=200;
   col=320;
   if ((f=fopen(fichero, "rb"))==NULL)
      {
      modgraf (3);
      printf("No se puede abrir el fichero %s", fichero);
      exit(1);
      }
   for(cabecera=0; cabecera<128; cabecera++)
      tmp=fgetc(f);
   for(; filas!=0; filas--)
      {
      col=320;
      while(col!=0)
	 {
	 if (cont!=0)
	    {
	    *puntfar=byte;
	    puntfar++;
	    col--;
	    cont--;
	    }
	 else
	    {
	    byte=fgetc(f);
	    if(byte<=192)
	       {
	       *puntfar=byte;
	       puntfar++;
	       col--;
	       }
	    else
	       {
	       cont=byte&63;
	       byte=fgetc(f);
	       *puntfar=byte;
	       puntfar++;
	       col--;
	       cont--;
	       }
	 }
      }
   }
   fgetc(f);
   if((dir_lectura=(char *)malloc(768))==NULL)
      {
      modgraf(3);
      printf("NO HAY MEMORIA, PEDAZO DE CAPULLO.");
      exit(1);
      }
   cont=0;
   for(cont=0;cont<768;cont++)
      {
      *dir_lectura=fgetc(f);
      dir_lectura++;
      }
   dir_lectura-=768;
   asigna_rgb(dir_lectura);
   fclose(f);
   free(dir_lectura);
}

void asigna_rgb(char *dir_lectura)
/* Remapea toda la paleta de colores dando como argumento un puntero que  */
/* dirija la zona de memoria en la que se encuentra la paleta.            */
{
	struct   SREGS  seg;
	union     REGS  ent, sal;
	unsigned  int   n;
	long      int   dir;
		  char *dir_col;

	dir_col=dir_lectura;                      /* divide entre 4 */
	for(n=256*3; n>0; n--)
	   {                  /* los colores    */
	   *dir_col=*dir_col >> 2;
	   dir_col++;
	   }

	dir   =(long)dir_lectura;                 /* obtiene el segmento     */
	seg.es=(int)(dir >> 16);                  /* donde estan los colores */

        ent.h.al =  18;
        ent.h.ah =  16;
        ent.x.bx =   0;
	ent.x.cx = 256;
	ent.x.dx = (int)dir_lectura;              /* offset de los colores     */
	int86x(0x10, &ent, &sal, &seg);    /* funci¢n para asignar los colores */
}
void *lee_dgd(char fichero[])
/* Abre un fichero .DGD y lo guarda en memoria, devolviendo un puntero    */
/* a la direccion de memoria donde se ha almacenado.                      */
/***************­No poner imagenes muy grandes!****************************/
{
   char *dibujo;
   char ancho, alto;
   FILE *f;
   int contx, conty;
   if((f=fopen(fichero,"rb"))==NULL)
      {
      modgraf (3);
      printf("No se puede abrir el fichero %s", fichero);
      exit(1);
      }
   ancho=fgetc(f);
   alto=fgetc(f);
   if((dibujo=(char *)malloc((ancho*alto)+2))==NULL)
      {
      modgraf(3);
      printf("No hay memoria, capullo.");
      exit(1);
      }
   *dibujo=ancho;
   dibujo++;
   *dibujo=alto;
   dibujo++;
   for(contx=0;contx<alto;contx++)
      for (conty=0;conty<ancho;conty++)
	 {
	 *dibujo=fgetc(f);
	 dibujo++;
	 }
   dibujo-=(ancho*alto+2);
   return(dibujo);
}
void movimiento(void)
/* Una chorrada que mueve la pantalla a la ¨derecha? muy despacio. CACA   */
{
   char *linea_tmp, *linea_borrada;
   int cont, columnas;
   char far *puntfar=(char far *)0xA0000000;
   if((linea_tmp=(char *)malloc(200))==NULL)
      {
      modgraf(3);
      printf("No hay memoria, capullo.");
      exit(1);
      }
   if((linea_borrada=(char *)malloc(200))==NULL)
      {
      modgraf(3);
      printf("No hay memoria, capullo.");
      exit(1);
      }
   for(cont=0;cont<200;cont++)
      {
      *linea_borrada=*(puntfar+(320*cont));
      linea_borrada++;
      }
   linea_borrada-=200;
   for(columnas=1;columnas<320; columnas++)
      {
      for(cont=0;cont<200;cont++)
	 {
	 *linea_tmp=*(puntfar+columnas+(320*cont));
	 linea_tmp++;
	 }
      linea_tmp-=200;
      for(cont=0;cont<200;cont++)
	 {
	 *(puntfar+(columnas-1)+(320*cont))=*linea_tmp;
	 linea_tmp++;
	 }
      linea_tmp-=200;
      }
   for(cont=0;cont<200;cont++)
      {
      *(puntfar+319+(320*cont))=*linea_borrada;
      linea_borrada++;
      }
   linea_borrada-=200;
   free(linea_tmp);
   free(linea_borrada);
}
void pon_imagen(int columna,int fila,char ancho, char alto, char *imagen)
/* Esta funcion pone una imagan dada. El primer argumento es el n§ de col */
/* umna donde se situa el dibujo, el segundo, el n§ de fila, el tecero,   */
/* el ancho, el cuarto, el alto y el £ltimo es la direccion del dibujo.   */
{
   char far *puntfar=(char far *)0xA0000000;
   int contx, conty;
   puntfar+=(columna+(fila*320));
   for(contx=0; contx<alto; contx++)
      {
      for(conty=0; conty<ancho; conty++)
	 {
	 if(*imagen==0);
	 else
	    *puntfar=*imagen;
	 puntfar++;
	 imagen++;
	 }
      puntfar+=(320-ancho);
      }
}
void *corta(int columna,int fila,char ancho, char alto)
{
   char far *puntfar=(char far *)0xA0000000;
   int contx, conty;
   char *trozo;
   if((trozo=(char *)malloc(ancho*alto))==NULL)
      {
      modgraf (3);
      printf("No hay memoria, ­CAPULLO!");
      exit(1);
      }
   puntfar+=(columna+(fila*320));
   for(contx=0; contx<alto; contx++)
      {
      for(conty=0; conty<ancho; conty++)
	 {
	 *trozo=*puntfar;
	 trozo++;
	 puntfar++;
	 }
      puntfar+=(320-ancho);
      }
   trozo-=(ancho*alto);
   return(trozo);
}

void pon_corte(int columna,int fila,char ancho, char alto, char *imagen)
{
   char far *puntfar=(char far *)0xA0000000;
   int contx, conty;
   puntfar+=(columna+(fila*320));
   for(contx=0; contx<alto; contx++)
      {
      for(conty=0; conty<ancho; conty++)
	 {
	 *puntfar=*imagen;
	 puntfar++;
	 imagen++;
	 }
      puntfar+=(320-ancho);
      }
   imagen-=(ancho*alto);
   free(imagen);
}

/*void *adapta (char *grafico,int ancho,int alto,int salida_ancho,
				       int salida_alto)
   {
   char *salida;
   float coefx, coefy;
   float x, y;
   if ((salida=malloc(salida_ancho*salida_alto))==NULL)
      {
      modgraf (3);
      printf("­Que te falla la memoria en los gr ficos, chaval!");
      exit(1);
      }
   coefx=ancho/salida_ancho;
   coefy=alto/salida_alto;
   x=y=0;
   for (y=0; y<salida_alto; y+=coefy)
      for(x=0; x<salida_ancho; x+=coefx)
	 {
	 *salida=*(grafico+x+(y*ancho));
	 salida++;
	 }
   salida-=salida_ancho*salida_alto;
   return(salida);
}
int raton_ini(void)
{
   union REGS regs;
   regs.x.ax=0;
   int86(0x33, &regs, &regs);
   if (regs.x.ax == 0)
      return (NULL);
   else
      return (1);
}

void activa_raton (void)
{
   union REGS regs;
   regs.x.ax=0x1;
   int86(0x33, &regs, &regs);
}

void desactiva_raton (void)
{
   union REGS regs;
   regs.x.ax=0x2;
   int86(0x33, &regs, &regs);
}

int click (void)
{
   union REGS regs;
   regs.x.ax=0x3;
   int86(0x33, &regs, &regs);
   if ((regs.x.bx==0x1) || (regs.x.ax==0x2))
      return (1);
   else
      return (0);
}

void posicionar_raton (int x, int y)
/* Esta funcion pone el cursor del raton en la posicion de la pantalla pe-*/
/* dida.                                                                  */
/*{
   union REGS regs;
   regs.x.ax=0x4;
   x=regs.x.cx;
   y=regs.x.dx;
   int86(0x33, &regs, &regs);
}

void posicion_raton (int *x, int *y)
/* Esta funcion devuelve las coordenadas del rat¢n en la pantalla. Es    */
/* necesario pasarle las direcciones de las variables de las coordenadas.*/
/*{
   union REGS regs;
   regs.x.ax=0x3;
   int86(0x33, &regs, &regs);
   *x=regs.x.cx;
   *y=regs.x.cx;
}

void cambia_raton (int *dibujo, int x, int y)
/* Esta funcion cambia el dibujo de cursor de raton. Hay que pasarle un  */
/* puntero con la direccion de memoria donde hay 64 bytes con el dibujo  */
/* y la mascara, y la posicion del "puntero del cursor". (PCMANIA)       */
/*{
   union REGS regs;
   struct SREGS seg;
   long int dir;
   regs.x.ax=0x9;
   regs.x.bx=x;
   regs.x.cx=y;
   regs.x.dx=(int)dibujo;
   dir=(long)dibujo;
   seg.es=(dir >> 16);
   int86x( 0x33, &regs, &regs, &seg);
}
  */
#define GRIS 0
#define COLOR 1                             /******************************/
static unsigned char paleta_antigua[256][3];/* Este es un array permanente*/
static unsigned char activado;              /******************************/
void gris (char modo)
/* Esta funcion cambia la paleta de colores a escala de grises y vice  */
/* versa. El modo puede ser 0 para pasar a grises o 1 para devolver    */
/* la paleta normal. (PC ACTUAL N§ 56)                                 */
{
   switch (modo)
      {
      case COLOR:;
	 if(!activado)
	    break;
	 _ES=FP_SEG(paleta_antigua);
	 _DX=FP_OFF(paleta_antigua);
	 _AX=0x1012;
	 _BX=0;
	 _CX=256;
	 geninterrupt(0x10);
	 activado=~activado;
	 break;
      case GRIS:;
	 if(activado)
	    break;
	 _ES=FP_SEG(paleta_antigua);
	 _DX=FP_OFF(paleta_antigua);
	 _AX=0x1017;
	 _BX=0;
	 _CX=256;
	 geninterrupt(0x10);
	 _AX=0x101B;
	 _BX=0;
	 _CX=256;
	 geninterrupt(0x10);
	 activado=~activado;
	 break;
      }
}

void remapea_color(char color, char rojo, char verde, char azul)
/* Esta funcion remapea un color. Para hacer esto hay que poner el n§   */
/*  del color en el puerto 3C8h y los tres colores en el 3C9h despues.  */
/*    ( PC ACTUAL N§ 56 )  */
{
   outportb(0x3C8, color);
   outportb(0x3C9, rojo);
   outportb(0x3C9, verde);
   outportb(0x3C9, azul);
}

void saca_color(char color, char *rojo, char *verde, char *azul)
/* Esta funcion consigue los tonos (RGB) de un color.  */
/* Funciona casi igual que la anterior.                */
{
   outportb(0x3C7, color);
   *rojo=inportb(0x3C9);
   *verde=inportb(0x3C9);
   *azul=inportb(0x3C9);
}

void fundido (void)
/* Remapea los colores de la pantalla haciendo un fundido.                */
{
   unsigned char rojo, verde, azul;
   char cont;
   int color;
   for (color=0; color<256; color++)
      {
      saca_color(color,&paleta_antigua[color][1],&paleta_antigua[color][2],
		   &paleta_antigua[color][3]);

      }
   for (cont=0; cont<64; cont++)
      {
      for (color=0; color<256; color++)
	 {
	 saca_color(color, &rojo, &verde, &azul);
	 if ( rojo != 0 )
	    rojo--;
	 if ( verde != 0 )
	    verde--;
	 if ( azul != 0 )
	    azul--;
	 remapea_color( color, rojo, verde, azul );
	 }
      delay (10);
      }
}

void restaura_paleta (void)
{
   int color;
   for (color=0; color<256; color++)
      {
      remapea_color(color,paleta_antigua[color][1],paleta_antigua[color][2],
		   paleta_antigua[color][3]);

      }
}

void efecto1 (void)
{
   int color, cont;
   char rojo, verde, azul;
   for (color=0; color<256; color++)
      {
      saca_color(color, &paleta_antigua[color][0], &paleta_antigua[color][1],
		      &paleta_antigua[color][2]);
      remapea_color( color, 63, 63, 63);
      }
   for (cont=0; cont<64; cont++)
      for (color=0; color<256; color++)
	 {
	 saca_color(color, &rojo, &verde, &azul);
	 if ( rojo != paleta_antigua[color][0] )
	    rojo--;
	 if ( verde != paleta_antigua[color][1] )
	    verde--;
	 if ( azul != paleta_antigua[color][2] )
	    azul--;
	 remapea_color(color, rojo, verde, azul);
	 delay (0.25);
	 }
}

void pagina_activa(char numero)
{
   _AH=5;
   _AL=numero;
   geninterrupt(0x10);
}
unsigned long dir_xms;

struct
   {
   unsigned long tamano;
   unsigned int shandle;
   unsigned long soffset;
   unsigned int dhandle;
   unsigned long doffset;
   }
   mover_xms;

unsigned long test_xms (void)
{
   _AX=0x4300;
   geninterrupt(0x2F);
   if ( _AL != 0x80 )
      return(NULL);
   else
      {
      asm push es
      asm mov ax,4310h
      asm int 2fh
      asm mov word ptr [dir_xms], bx
      asm mov word ptr [dir_xms+2], es
      asm pop es
      return(dir_xms);
      }
}

int reserva_xms (int cantidad)
{
   asm mov ah,09h
   asm mov dx,cantidad
   asm call dword ptr dir_xms
   if( _AX == 0)
      {
      if ( _BL == 0xA2)
	 {
	 modgraf (3);
	 printf("No quedan handles XMS libres.");
	 exit(1);
	 }
      else return(-1);
      }
   else
      return(_DX);
}

void base_a_xms (long cantidad, char *ptr, int handle, long offset_xms)
{
   mover_xms.tamano=cantidad;
   mover_xms.shandle=0;
   mover_xms.soffset=(unsigned long)ptr;
   mover_xms.dhandle=handle;
   mover_xms.doffset=offset_xms;
   asm push ds
   asm push si
   asm mov ax, seg mover_xms
   asm mov ds, ax
   asm mov ax, offset mover_xms
   asm mov si, ax
   _AH=0x0B;
   asm call dword ptr dir_xms
   asm pop si
   asm pop ds
   if( _AX == 0)
      {
      modgraf (3);
      printf("Fallo en memoria XMS.");
      exit(1);
      }
}

void xms_a_base (long cantidad, char *ptr, int handle, long offset_xms)
{
   mover_xms.tamano=cantidad;
   mover_xms.shandle=handle;
   mover_xms.soffset=offset_xms;
   mover_xms.dhandle=0;
   mover_xms.doffset=(unsigned long)ptr;
   asm push ds
   asm push si
   asm mov ax, seg mover_xms
   asm mov ds, ax
   asm mov ax, offset mover_xms
   asm mov si, ax
   asm mov ah, 0bh
   asm call dword ptr dir_xms
   asm pop si
   asm pop ds
   if ( _AX == 0)
      {
      modgraf(3);
      printf("Fallo en memoria XMS");
      exit(1);
      }
}

void libera_xms(int handle)
{
   _DX=handle;
   _AH=0xA;
   asm call dword ptr dir_xms
   if ( _AX == 0 )
      {
      modgraf (3);
      printf("Error al liberar memoria XMS.");
      exit(1);
      }
}