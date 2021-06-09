// Fundamentos de Procesamiento Digital de Señales: Teoría e Implementación Práctica
// Jorge Iván Marín-Hurtado, Alexander López-Parrado, Luis Miguel Capacho-Valbuena
// ISBN: 978-958-48-2955-9
// (c) 2017
// ==================================================================================
// Filtro IIR en punto fijo a 16 bits mediante estructura en cascada de secciones de
// segundo orden
// ==================================================================================

#include <simdsp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coefs_iir_bp_sos_fp.h"

//Tipo de dato para las líneas de retardos, considera 16 de bits de guarda para la ganancia de las etapas
typedef int32_t delay_t;
//Tipo de dato para los coeficientes
typedef int16_t coeff_t;
//Tipo de dato para los acumuladores antes de correr la coma
typedef int64_t acc_t;


/*Implementa una sección de segundo orden usando estructura directa tipo I*/
delay_t sos_d1(const coeff_t *bk,const coeff_t *ak,delay_t *x,delay_t *y,delay_t xn, int8_t q){
    acc_t yn;

    /*Muestra actual*/
    x[0]=xn;

    /*Parte no recursiva y actualización*/
    yn=((acc_t)bk[0])*((acc_t)x[0])+((acc_t)bk[1])*((acc_t)x[1])+((acc_t)bk[2])*((acc_t)x[2]);
    x[2]=x[1];
    x[1]=x[0];


    /*Parte recursiva y actualización*/
    yn=(yn-(((acc_t)ak[1])*((acc_t)y[0]))-(((acc_t)ak[2])*((acc_t)y[1])))>>q;
    y[1]=y[0];
    y[0]=yn;

    /**Retorna salida*/
    return yn;


}

/*Implementa una sección de segundo orden usando estructura directa tipo II*/
delay_t sos_d2(const coeff_t *bk,const coeff_t *ak,delay_t *w,delay_t xn, uint8_t q){

    acc_t yn;

	/*Acumuladores*/
	acc_t w_n;

        /*Entrada a punto fijo*/
        w_n=(((acc_t)(xn)<<q)-(acc_t)ak[1]*(acc_t)w[1]-(acc_t)ak[2]*(acc_t)w[2])>>q;



        w[0]=w_n;

        /*Cálculo de la rama de salida*/

                yn =((acc_t)bk[0]*(acc_t)w[0]+(acc_t)bk[1]*(acc_t)w[1]+(acc_t)bk[2]*(acc_t)w[2])>>q;
                w[2] = w[1];
                w[1] = w[0];


    /**Retorna salida*/
    return yn;


}


/*Implementa una sección de segundo orden usando estructura directa tipo III*/
delay_t sos_d3(const coeff_t *bk,const coeff_t *ak,delay_t *v,delay_t xn, uint8_t q){
    acc_t yn;

    /*Calcula salida*/
    yn=(((acc_t)bk[0])*((acc_t)xn)+(((acc_t)v[0])<<q))>>q;

    /*Actualización de la memoria del filtro*/
    v[0]=(((acc_t)bk[1])*((acc_t)xn)-(((acc_t)ak[1])*((acc_t)yn))+(((acc_t)v[1])<<q))>>q;
    v[1]=(((acc_t)bk[2])*((acc_t)xn)-(((acc_t)ak[2])*((acc_t)yn)))>>q;

    /**Retorna salida*/
    return yn;


}








/*Implementa un filtro IIR usando secciones de segundo orden (Estructura tipo I)*/
delay_t sos_filterd1(const coeff_t *sos, const coeff_t gk, int8_t nsect, delay_t *w,delay_t xn,int8_t q){

int8_t k;
delay_t yn;

/*Se filtra a través de las secciones de segundo orden*/
for(k=0;k<nsect;k++){
    //k-ésima sección de segundo orden
    yn=sos_d1(sos+6*k,sos+3+6*k,w+5*k,w+3+5*k,xn, q);
     yn=((acc_t)yn*(acc_t)gk)>>q;

    xn=yn;
}

/*Retorna salida total*/
return yn;



}

/*Implementa un filtro IIR usando secciones de segundo orden (Estructura tipo II)*/
delay_t sos_filterd2(const coeff_t *sos,const coeff_t gk, int8_t nsect, delay_t *w,delay_t xn,uint8_t q){

int8_t k;
delay_t yn;

/*Se filtra a través de las secciones de segundo orden*/
for(k=0;k<nsect;k++){
    //k-ésima sección de segundo orden
    yn=sos_d2(sos+6*k,sos+3+6*k,w+3*k,xn, q);

    yn=((acc_t)yn*(acc_t)gk)>>q;

    xn=yn;

}



/*Retorna salida total*/
return yn;



}

/*Implementa un filtro IIR usando secciones de segundo orden (Estructura tipo III)*/
delay_t sos_filterd3(const coeff_t *sos,const coeff_t gk, int8_t nsect, delay_t *v,delay_t xn,uint8_t q){

int8_t k;
delay_t yn;

/*Se filtra a través de las secciones de segundo orden*/
for(k=0;k<nsect;k++){
    //k-ésima sección de segundo orden
    yn=sos_d3(sos+6*k,sos+3+6*k,v+2*k,xn, q);

    yn=((acc_t)yn*(acc_t)gk)>>q;

    xn=yn;

}



/*Retorna salida total*/
return yn;



}


/*Memoria para los NSECT filtros en estructura tipo I, 5 taps para cada uno*/
delay_t w1[NSECT*5];

/*Memoria para los NSECT filtros en estructura tipo II, 3 taps para cada uno*/
delay_t w2[NSECT*3];


/*Memoria para los NSECT filtros en estructura tipo III, 2 taps para cada uno*/
delay_t w3[NSECT*2];



void dsp_setup()
{
	// put your setup code here, to run once:
int k;
   setfs(2.4e6);

memset(w1,0,NSECT*5*sizeof(delay_t));
memset(w2,0,NSECT*3*sizeof(delay_t));
memset(w3,0,NSECT*2*sizeof(delay_t));


}

void dsp_loop()
{
delay_t y_n,x_n;

	/*Lectura del ADC*/
   x_n=(delay_t)readADC();

y_n=sos_filterd1(SOS,gk, NSECT,w1,x_n,Q);
//y_n=sos_filterd2(SOS,gk, NSECT,w2,x_n,Q);
//y_n=sos_filterd3(SOS,gk, NSECT,w3,x_n,Q);

   	/*Escritura al DAC*/
writeDAC((short)(y_n));



}
