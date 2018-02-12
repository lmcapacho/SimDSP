// Fundamentos de Procesamiento Digital de Señales: Teoría e Implementación Práctica
// Jorge Iván Marín-Hurtado, Alexander López-Parrado, Luis Miguel Capacho-Valbuena
// ISBN: 978-958-48-2955-9
// (c) 2017
// ==================================================================================
// Filtro IIR en estructura directa tipo I
// ==================================================================================

#include <simdsp.h>
#include <string.h>
#include "coefs_iir_bp.h"

double x[M];
double y[N];

//Función que implementa la estructura directa tipo I para filtros IIR
double iir1(double xn,double *x,double *y,double *b,double *a,int p){

  int i;
  double yn;

  yn=0.0;


  x[0]=xn;

  //Rama de entrada
  for(i=(p-1);i>=0;i--)
  {
    yn+=b[i]*x[i];
    x[i]=x[i-1];
  }

  //Rama de salida
  for(i=(p-1);i>=1;i--)
  {
    yn-=a[i]*y[i];
    y[i]=y[i-1];
  }

  y[1]=yn;

  return yn;

}

void dsp_setup()
{

  setfs(10000);
  memset(x, 0, M*sizeof(double));
  memset(y, 0,  N*sizeof(double));

}

void dsp_loop()
{
  double xn,yn;
  xn=(double)readADC();
  yn=iir1(xn,x,y,(double *)b_k,(double *)a_k,N);
  writeDAC((short)yn);
}
