// Fundamentos de Procesamiento Digital de Señales: Teoría e Implementación Práctica
// Jorge Iván Marín-Hurtado, Alexander López-Parrado, Luis Miguel Capacho-Valbuena
// ISBN: 978-958-48-2955-9
// (c) 2017
// =============================================================================================
// Filtro FIR usando DMA y procesamiento orientado a bloques
// =============================================================================================

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <simdsp.h>
#include "coefs_filter.h"


#define N_DATOS 8192

//Buffers de captura
short buffer_1[N_DATOS+P-1];
short buffer_2[N_DATOS+P-1];

//Buffer de salida
short buffer_salida[N_DATOS];

//punteros a los buffer
short *datos_adquiriendo;
short *datos_procesando;

void block_conv(double * coefs, short *entrada,  short *salida, short *init,int p,int Nbloque){

	int k;
	int n;
	double y;

	//calcula convolución del bloque
	for(n=0;n<Nbloque;n++){
		y=0;
		for(k=0;k<p;k++){
			y+=*(coefs+k)*(double)(*(entrada+n-k+p-1));
		}

		*(salida+n)=(short)y;
	}

	//Se copian los N_h-1 datos de entrada para el próximo bloque
	for(n=0;n<p-1;n++)
		*(init+n)=*(entrada+n+Nbloque);
}

//ISR del teclado
void isr_key(){

	println("Presionó %d",readKeyboard());

}

//Rutina que simulará la ISR que se invoca una vez ha finalizado
//la transferencia por DMA

void dma_callbackfnc(){
	short *p_temporal;

	//Se intercambian los punteros
	p_temporal=datos_adquiriendo;
	datos_adquiriendo=datos_procesando;
	datos_procesando=p_temporal;



	//Produce un nuevo llamado a la rutina para capturar datos por DMA
	captureBlock(datos_adquiriendo+P-1, N_DATOS, dma_callbackfnc );


	//Se filtra
	block_conv(h, datos_procesando,buffer_salida,datos_adquiriendo, P,N_DATOS);

	//Envia el buffer calculado al DAC a través de DMA
	playBlock(buffer_salida, N_DATOS, NULL);

}


void dsp_setup(){



	attachInterrupt(isr_key);

	/*Se inician los punteros*/
	datos_adquiriendo=buffer_1;
	datos_procesando=buffer_2;

	//Contenido de los arreglos en cero
	memset(buffer_1,0,(N_DATOS+P-1)*sizeof(short));
	memset(buffer_2,0,(N_DATOS+P-1)*sizeof(short));
	memset(buffer_salida,0,N_DATOS*sizeof(short));

	//Habilita entrada de micrófono
	enableMic(N_DATOS);

	//Configura la frecuencia de muestreo del sistema
	setfs(44100);

	//Produce un llamado a la captura de datos por DMA. Esta función invoca
	//la función callbackfnc una vez la captura ha finalizado, generando así
	//múltiples capturas de bloques.
	captureBlock(datos_adquiriendo+P-1, N_DATOS, dma_callbackfnc );

}

void dsp_loop(){
	//El programa principal en este caso no hace nada, pues todo el
	//procesamiento se hace vía DMA
}
