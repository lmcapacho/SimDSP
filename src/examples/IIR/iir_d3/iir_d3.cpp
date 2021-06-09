// Fundamentos de Procesamiento Digital de Señales: Teoría e Implementación Práctica
// Jorge Iván Marín-Hurtado, Alexander López-Parrado, Luis Miguel Capacho-Valbuena
// ISBN: 978-958-48-2955-9
// (c) 2017
// ==================================================================================
// Filtro IIR en estructura directa tipo III
// ==================================================================================

#include "coefs_iir_bp.h"
#include <simdsp.h>


double iir3(double x_n,double *v,double *b,double *a,int n){

   int k;

   double y_n;

   /*Cálculo de la salida una vez se lee el ADC*/
   y_n=b[0]*x_n+v[0];


   /*Cálculo de v[n]*/
   for(k=0; k<n-2; k++)
   v[k] =b[k+1]*x_n-a[k+1]*y_n+v[k+1];

   /*Cálculo de v_{n-1}*/
   v[n-2]=b[n-1]*x_n-a[n-1]*y_n;

   return y_n;
}

double v[N-1];


void dsp_setup()
{
   // put your setup code here, to run once:
   int k;

   double y_n;

   setfs(10000);
   for(k=0; k<(N-1); k++) v[k] = 0.0;

}

void dsp_loop()
{

   double x_n,y_n;


   /*Lectura del ADC*/
   x_n=(double)readADC();

   y_n=iir3(x_n,v,(double *)b_k,(double *)a_k,N);

   /*Escritura al DAC*/
   writeDAC((short)(y_n));

}
