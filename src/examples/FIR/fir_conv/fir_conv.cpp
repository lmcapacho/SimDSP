// Fundamentos de Procesamiento Digital de Señales: Teoría e Implementación Práctica
// Jorge Iván Marín-Hurtado, Alexander López-Parrado, Luis Miguel Capacho-Valbuena
// ISBN: 978-958-48-2955-9
// (c) 2017
// =============================================================================================
// Implementacion de la suma de convolucion usando punteros
// =============================================================================================

#include <simdsp.h>
#include "coefs_filter.h"

void conv(int p, double *h, double *x, double *y);

double x[P+1];
double yn;

void dsp_setup()
{
	setfs(1e3);
}

double xn;

void dsp_loop()
{

	/*Lectura del ADC*/
	xn=(double)readADC();
	x[0]=xn;

	conv(P,h, x, &yn);

	/*Escritura en DAC*/
	writeDAC((short)(yn));

}

void conv(int p, double *h, double *x, double *y){

	int k;
	double *xk,*bk;

	xk = x+p-1;
	bk = h+p-1;

	*y=0.0;

	for(k=0;k<p;k++){
		*y +=(*(bk--))*(*(xk--));
		*(xk+1) = *xk;
	}
}
