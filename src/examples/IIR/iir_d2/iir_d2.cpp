// Fundamentos de Procesamiento Digital de Señales: Teoría e Implementación Práctica
// Jorge Iván Marín-Hurtado, Alexander López-Parrado, Luis Miguel Capacho-Valbuena
// ISBN: 978-958-48-2955-9
// (c) 2017
// ==================================================================================
// Filtro IIR en estructura directa tipo II
// ==================================================================================

#include "coefs_iir_bp.h"

#include <simdsp.h>

double iir2(double x_n,double *w,double *b,double *a,int n){

   double y_n,w_n;
   int k;

   w_n=x_n;

   /*Cálculo de w[n]*/
   for(k=n-1; k>=1; k--)
   w_n -=a[k]*w[k];

   /*w_n actual*/
   w[0]=w_n;

   y_n=0;
   /*Cálculo de la rama de salida*/
   for(k=n-1; k>=0; k--) {
      y_n +=b[k]*w[k];
      w[k] = w[k-1];
   }


   return y_n;

}


double w[N];

void dsp_setup()
{
   // put your setup code here, to run once:
   int k;

   setfs(10000);
   for(k=0; k<N; k++) w[k] = 0;

}

void dsp_loop()
{
   float x_n,y_n;

   /*Lectura del ADC*/
   x_n=(float)readADC();

   y_n=iir2(x_n,w,(double *)b_k,(double *)a_k,N);

   /*Escritura al DAC*/
   writeDAC((short)(y_n));

}
