// Fundamentos de Procesamiento Digital de Señales: Teoría e Implementación Práctica
// Jorge Iván Marín-Hurtado, Alexander López-Parrado, Luis Miguel Capacho-Valbuena
// ISBN: 978-958-48-2955-9
// (c) 2017
// ==================================================================================
// Filtro IIR en estructura directa tipo II en punto fijo
// ==================================================================================

#include "coefs_iir_bp_fp.h"
#include <stdint.h>

#include <simdsp.h>

//Tipo de dato para los coeficientes
typedef int16_t coeff_t;
//Tipo de dato para las líneas de retardos, considera 16 de bits de guarda para la ganancia de las etapas
typedef int32_t delay_t;
//Tipo de dato para los acumuladores antes de correr la coma
typedef int64_t acc_t;

delay_t iir2_fp(coeff_t x_n,delay_t *w,const coeff_t *b,const coeff_t *a,int8_t n,int8_t q){

   acc_t y_n;
   acc_t w_n;
   int8_t k;

   w_n=(acc_t)(x_n<<q);

   /*Cálculo de w[n]*/
   for(k=n-1; k>=1; k--)
   w_n -=((acc_t)a[k]*(acc_t)w[k]);


   /*w_n actual*/
   w[0]=w_n>>q;

   y_n=0;
   /*Cálculo de la rama de salida*/
   for(k=n-1; k>=0; k--) {
      y_n +=(acc_t)b[k]*(acc_t)w[k];
      w[k] = w[k-1];
   }


   return (delay_t)(y_n>>q);

}



delay_t w[N];


void dsp_setup()
{
   // put your setup code here, to run once:
   int k;

   setfs(10000);
   for(k=0; k<N; k++) w[k] = 0;

}

void dsp_loop()
{
   coeff_t x_n;
   delay_t y_n;

   /*Lectura del ADC*/
   x_n=(coeff_t)readADC();

   y_n=iir2_fp(x_n,w,b_k,a_k,N,Q);

   /*Escritura al DAC*/
   writeDAC((short)(y_n));



}
