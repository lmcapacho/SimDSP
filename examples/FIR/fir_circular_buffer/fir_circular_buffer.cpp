// Fundamentos de Procesamiento Digital de Señales: Teoría e Implementación Práctica
// Jorge Iván Marín-Hurtado, Alexander López-Parrado, Luis Miguel Capacho-Valbuena
// ISBN: 978-958-48-2955-9
// (c) 2017
// =============================================================================================
// Filtro FIR usando buffer circular
// =============================================================================================

//Archivo de cabecera con coeficientes de filtro FIR
#include "coefs_filter.h"

//Operación módulo con signo
#define MOD(a,b) ((((a)%(b))+(b))%(b))

#include <simdsp.h>

//Regresor/Memoria del filtro FIR
double x[P];

//Convolución usando buffer circular
void cbuff_conv(int p, double *h, double *x,double xn, double *y)
{

  //Índice que indica el inicio del buffer circular, variable estática
  static int xins=0;
  int i;

  //Entrada actual
  x[xins]=xn;
  *y=0.0;

  //Convolución usando direccionamiento circular
  for(i=0;i<p;i++)
  *y+=h[i]*x[MOD((xins+i),p)];

  //Actualización del índice del inicio del buffer ciruclar
  xins=MOD((xins-1),p);

}

void dsp_setup()
{

  setfs(1e3);
}

void dsp_loop()
{
  double xn;
  double yn;
  /*Lectura del ADC*/
  xn=(double)readADC();
  cbuff_conv(P,h, x,xn, &yn);
  /*Escritura en DAC*/
  writeDAC((short)(yn));
}
