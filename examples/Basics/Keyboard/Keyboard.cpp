// Fundamentos de Procesamiento Digital de Señales: Teoría e Implementación Práctica
// Jorge Iván Marín-Hurtado, Alexander López-Parrado, Luis Miguel Capacho-Valbuena
// ISBN: 978-958-48-2955-9
// (c) 2017
// ===================================================================================
// Manejo del teclado
// ===================================================================================

#include <simdsp.h>

void printKey()
{
	// Imprime la tecla presionada
	println(readKeyboard());
}

void dsp_setup()
{
	// Asigna la función a ejecutar cuando
	// se presiona una tecla
	attachInterrupt(printKey);
}

void dsp_loop()
{
	// No hace nada en la función principal
}
