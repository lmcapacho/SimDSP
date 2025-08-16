// Fundamentos de Procesamiento Digital de Señales: Teoría e Implementación Práctica
// Jorge Iván Marín-Hurtado, Alexander López-Parrado, Luis Miguel Capacho-Valbuena
// ISBN: 978-958-48-2955-9
// (c) 2017
// ==================================================================================
// Filtro IIR en estructura directa tipo III y punto fijo
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

//Linea de retardos del filtro
delay_t v[N-1];

delay_t iir3_fp(coeff_t x_n,delay_t *v,const coeff_t *b,const coeff_t *a,int8_t n,int8_t q){

   uint8_t k;
   acc_t y_n;


   /*Cálculo de la salida*/
   y_n=(((acc_t)b_k[0])*((acc_t)x_n)+((acc_t)v[0]<<q))>>q;

   /*Cálculo de v[n]*/
   for(k=0; k<n-2; k++)
   v[k] =(((acc_t)b_k[k+1])*((acc_t)x_n)-((acc_t)a_k[k+1])*((acc_t)y_n)+((acc_t)v[k+1]<<q))>>q;

   /*Cálculo de v_{n-1}*/
   v[n-2]=(((acc_t)b_k[n-1])*((acc_t)x_n)-((acc_t)a_k[n-1])*((acc_t)y_n))>>q;

   return (delay_t)y_n;

}


void dsp_setup()
{

   int k;

   setfs(10000);
   for(k=0; k<(N-1); k++) v[k] = 0;

}

void dsp_loop()
{

   double x_n,y_n;


   /*Lectura del ADC*/
   x_n=(coeff_t)readADC();

   y_n=iir3_fp(x_n,v,b_k,a_k,N,Q);

   /*Escritura al DAC*/
   writeDAC((short)(y_n));

}
