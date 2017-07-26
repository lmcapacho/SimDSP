// SimDSP main file

#include "simdsp.h"

#define  BUFF_LEN  44100
short bufferin [BUFF_LEN];
short bufferout[BUFF_LEN];


//Rutina que implementa un filtro FIR
float xk[5];
float hk[] = {0.2, 0.2, 0.2, 0.2, 0.2};

float filtro(float x)
{
    int k;
    float y = 0;
    xk[0] = x;
    for (k=0; k<5; k++) {
        y += hk[k] * xk[k];

    }
    for (k=4; k>0; k--) {
        xk[k] = xk[k-1];
    }
    return y;
}


//Rutina que simular� la ISR que se invoca una vez ha finalizado
//la transferencia por DMA
void callbackDMA()
{
    int i;

    //Calcula la salida del filtro por cada muestra de entrada.
    //No es la forma m�s eficiente computacionalmente pero si la m�s simple para Dummies
    for (i=0; i<BUFF_LEN; i++)
    {
        bufferout[i] = filtro( bufferin[i] );
    }

    //Env�a el buffer calculado al DAC a trav�s de DMA
    playBlock( bufferout, BUFF_LEN, NULL );

    //Produce un nuevo llamado a la rutina para capturar datos por DMA
    captureBlock( bufferin, BUFF_LEN, callbackDMA );
}


void dsp_setup()
{
	setfs(44100);
	enableMic(BUFF_LEN);
	captureBlock( bufferin, BUFF_LEN, callbackDMA );
}

void dsp_loop()
{

}
