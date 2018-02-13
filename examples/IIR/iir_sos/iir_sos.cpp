// Fundamentos de Procesamiento Digital de Señales: Teoría e Implementación Práctica
// Jorge Iván Marín-Hurtado, Alexander López-Parrado, Luis Miguel Capacho-Valbuena
// ISBN: 978-958-48-2955-9
// (c) 2017
// ==================================================================================
// Filtro IIR en estructura en cascada de secciones de segundo orden (SOS)
// ==================================================================================

#include "coefs_iir_bp_sos.h"
#include <simdsp.h>
#include <string.h>
#include <stdint.h>


/*Implementa una sección de segundo orden usando estructura directa tipo I*/
double sos_d1(const double *bk,const double *ak,double *x,double *y,double xn){
    double yn;

    /*Muestra actual*/
    x[0]=xn;

    /*Parte no recursiva y actualización*/
    yn=((double)bk[0])*((double)x[0])+((double)bk[1])*((double)x[1])+((double)bk[2])*((double)x[2]);
    x[2]=x[1];
    x[1]=x[0];


    /*Parte recursiva y actualización*/
    yn=(yn-(((double)ak[1])*((double)y[0]))-(((double)ak[2])*((double)y[1])));
    y[1]=y[0];
    y[0]=yn;

    /**Retorna salida*/
    return yn;


}

/*Implementa una sección de segundo orden usando estructura directa tipo II*/
double sos_d2(const double *bk,const double *ak,double *w,double xn){

    double yn;

	/*Acumuladores*/
	double w_n;

        /*Entrada a punto fijo*/
        w_n=(((double)(xn))-(double)ak[1]*(double)w[1]-(double)ak[2]*(double)w[2]);



        w[0]=w_n;

        /*Cálculo de la rama de salida*/

                yn =((double)bk[0]*(double)w[0]+(double)bk[1]*(double)w[1]+(double)bk[2]*(double)w[2]);
                w[2] = w[1];
                w[1] = w[0];


    /**Retorna salida*/
    return yn;


}


/*Implementa una sección de segundo orden usando estructura directa tipo III*/
double sos_d3(const double *bk,const double *ak,double *v,double xn){
    double yn;

    /*Calcula salida*/
    yn=(((double)bk[0])*((double)xn)+(((double)v[0])));

    /*Actualización de la memoria del filtro*/
    v[0]=(((double)bk[1])*((double)xn)-(((double)ak[1])*((double)yn))+(((double)v[1])));
    v[1]=(((double)bk[2])*((double)xn)-(((double)ak[2])*((double)yn)));

    /**Retorna salida*/
    return yn;


}








/*Implementa un filtro IIR usando secciones de segundo orden (Estructura tipo I)*/
double sos_filterd1(const double *sos, const double gk, int8_t nsect, double *w,double xn){

int8_t k;
double yn=0;

/*Se filtra a través de las secciones de segundo orden*/
for(k=0;k<nsect;k++){
    //k-ésima sección de segundo orden
    yn=sos_d1(sos+6*k,sos+3+6*k,w+5*k,w+3+5*k,xn);
     yn=((double)yn*(double)gk);

    xn=yn;
}

/*Retorna salida total*/
return yn;



}

/*Implementa un filtro IIR usando secciones de segundo orden (Estructura tipo II)*/
double sos_filterd2(const double *sos,const double gk, int8_t nsect, double *w,double xn){

int8_t k;
double yn=0;

/*Se filtra a través de las secciones de segundo orden*/
for(k=0;k<nsect;k++){
    //k-ésima sección de segundo orden
    yn=sos_d2(sos+6*k,sos+3+6*k,w+3*k,xn);

    yn=((double)yn*(double)gk);

    xn=yn;

}



/*Retorna salida total*/
return yn;



}

/*Implementa un filtro IIR usando secciones de segundo orden (Estructura tipo III)*/
double sos_filterd3(const double *sos,const double gk, int8_t nsect, double *v,double xn){

int8_t k;
double yn=0;

/*Se filtra a través de las secciones de segundo orden*/
for(k=0;k<nsect;k++){
    //k-ésima sección de segundo orden
    yn=sos_d3(sos+6*k,sos+3+6*k,v+2*k,xn);

    yn=((double)yn*(double)gk);

    xn=yn;

}



/*Retorna salida total*/
return yn;



}


/*Memoria para los NSECT filtros en estructura tipo I, 5 taps para cada uno*/
double w1[NSECT*5];

/*Memoria para los NSECT filtros en estructura tipo II, 3 taps para cada uno*/
double w2[NSECT*3];


/*Memoria para los NSECT filtros en estructura tipo III, 2 taps para cada uno*/
double w3[NSECT*2];






void dsp_setup()
{
	// put your setup code here, to run once:

   setfs(2.4e6);
   memset(w1,0,NSECT*5*sizeof(double));
   memset(w2,0,NSECT*3*sizeof(double));
   memset(w3,0,NSECT*2*sizeof(double));

}

void dsp_loop()
{
	// put your main code here, to run repeatedly:






   float x_n,y_n;





		/*Lectura del ADC*/
        x_n=(float)readADC();

//y_n=sos_filterd3(SOS, gk, NSECT, w3,x_n);
//y_n=sos_filterd2(SOS, gk, NSECT, w2,x_n);
y_n=sos_filterd1(SOS, gk, NSECT, w1,x_n);


   	/*Escritura al DAC*/
writeDAC((short)(y_n));



}
