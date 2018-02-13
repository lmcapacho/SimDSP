// Fundamentos de Procesamiento Digital de Señales: Teoría e Implementación Práctica
// Jorge Iván Marín-Hurtado, Alexander López-Parrado, Luis Miguel Capacho-Valbuena
// ISBN: 978-958-48-2955-9
// (c) 2017
// =============================================================================================
// Técnica denoising implementada por medio de la convolución rápida que asume
// el método de solapamiento y suma, traslape de bloques de entrada del 50% y enventanamiento.
// Para probar este programa se sugiere usar ruido aditivo (AWGN) de 5dB
//
// Para activar la captura por micrófono debe activar el micrófono descomentanto la línea 159.
// =============================================================================================


#include <string.h>
#include <simdsp.h>

#include "fft_f.h"
#include "window.h"

#define Nfft 1024
#define Nx Nfft/2
#define Nstep Nfft/4
#define umbral 3e10

//Buffers de captura
short buffer_1[Nx];
short buffer_2[Nx];

//Buffer de salida
short BufferSalida[Nstep];

//Punteros al doble buffer de DMA
short *BufferCaptura;
short *BufferProcesa;

//Buffers para hacer el procesamiento interno de la convolución rápida
float X_n[Nfft*2];
float X_k[Nfft*2];
float Y_n[Nfft*2];
float Y_k[Nfft*2];
float Yadd[Nfft];

//Función para hacer el bitreversal1024 para un bloque de Nfft = 256
void bitreversal1024(float *x_n, float *x_k) {
	int cbr;
	for (int c = 0; c<Nfft; c++) {
		cbr = 	((c & 1) << 9) |
		((c & 2) << 7) |
		((c & 4) << 5) |
		((c & 8) << 3) |
		((c & 0x10) << 1) |
		((c & 0x20) >> 1) |
		((c & 0x40) >> 3) |
		((c & 0x80) >> 5) |
		((c & 0x100) >> 7) |
		((c & 0x200) >> 9);
		x_k[cbr*2] 	= x_n[2*c]; 	  //Parte real
		x_k[cbr*2+1]= x_n[2*c+1];   //Parte imaginaria
	}
}

//Rutina que simulará la ISR que se invoca una vez ha finalizado
//la transferencia por DMA

void dma_callbackfnc(){
	int n,k;
	short *BufferT;

	//Intercambia los buffer de adquisición y de cálculo
	BufferT=BufferCaptura;
	BufferCaptura=BufferProcesa;
	BufferProcesa=BufferT;

	//Inicia una nueva captura por DMA para garantizar la ejecución en
	//tiempo real. Nótese que la dirección en la cual se inicia la captura es Nh-1,
	//con el fin de reservar las primeras posiciones para los elementos del bloque anterior
	captureBlock(BufferCaptura+Nstep, Nstep, dma_callbackfnc );

	//Completa el bloque de entrada con ceros, fija la parte imaginaria a 0.0, y enventana
	for(n=0;n<Nx;n++) {
		X_n[2*n]   = BufferProcesa[n] * w[n];
		X_n[2*n+1] = 0.0;
	}
	for(;n<Nfft;n++) {
		X_n[2*n]   = 0.0;
		X_n[2*n+1] = 0.0;
	}

	//Calcula la FFT
	bitreversal1024(X_n,X_k);
	fft(X_k,X_k,Nfft);

	float maxmag = 0.0;

	//Aplica el denoising
	for(k=0;k<Nfft;k++) {
		float mag = X_k[2*k] * X_k[2*k] + X_k[2*k+1] * X_k[2*k+1];
		if(mag>maxmag) maxmag = mag;
		if (mag > umbral) {
			Y_k[2*k]   = X_k[2*k]; //Parte real
			Y_k[2*k+1] = X_k[2*k+1]; //Parte imaginaria
		} else {
			Y_k[2*k]   = 0.0; //Parte real
			Y_k[2*k+1] = 0.0; //Parte imaginaria
		}
	}

	println("Max_mag %g", maxmag);

	//Calcula la IFFT
	bitreversal1024(Y_k,Y_n);
	ifft(Y_n,Y_n,Nfft);

	//Aplica la adición de bloques de salida según el método overlap-add (descarta la parte imaginaria de y, pues es cero.
	for(n=0;n<Nfft-Nstep;n++) {
		Yadd[n] += Y_n[2*n];
	}
	for(;n<Nfft;n++) {
		Yadd[n] = Y_n[2*n];
	}

	//Escribe al buffer de salida
	for(n=0;n<Nstep;n++) {
		BufferSalida[n] = Yadd[n];
	}

	//Prepara el buffer de adición para el siguiente traslape
	for(n=0;n<Nfft-Nstep;n++) {
		Yadd[n] = Yadd[n+Nstep];
	}

	//Prepara el buffer de entrada
	for(n=0;n<Nstep;n++) {
		BufferCaptura[n] = BufferProcesa[n+Nstep];
	}

	//Envia el buffer calculado al DAC a través de DMA
	playBlock(BufferSalida, Nstep, NULL);
}


void dsp_setup(){
	int n;

	/*Se inician los punteros*/
	BufferCaptura=buffer_1;
	BufferProcesa=buffer_2;

	//Contenido de los arreglos en cero
	memset(buffer_1,0,Nx*sizeof(short));
	memset(buffer_2,0,Nx*sizeof(short));
	memset(BufferSalida,0,Nstep*sizeof(short));

	for(n=0; n<Nfft; n++) {
		Yadd[n] = 0.0;
	}

	//Habilita entrada de micrófono
	//	enableMic(Nx);

	//Configura la frecuencia de muestreo del sistema
	setfs(22050);

	//Produce un llamado a la captura de datos por DMA. Esta función invoca
	//la función callbackfnc una vez la captura ha finalizado, generando así
	//múltiples capturas de bloques.
	captureBlock(BufferCaptura, Nstep, dma_callbackfnc );
}


void dsp_loop(){
	//El programa principal en este caso no hace nada, pues todo el
	//procesamiento se hace vía DMA
}
