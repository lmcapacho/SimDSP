// Fundamentos de Procesamiento Digital de Señales: Teoría e Implementación Práctica
// Jorge Iván Marín-Hurtado, Alexander López-Parrado, Luis Miguel Capacho-Valbuena
// ISBN: 978-958-48-2955-9
// (c) 2017
// =============================================================================================
// Código fuente de la Sección 6.2 para el cálculo de la Transformada Rápida de Fourier (FFT).
//
//  Función para calcular la FFT para vectores con el formato
//
//  |R|I|R|I| ... |R|I|
//  con cada valor |R| o |I| de tipo float.
//
//  Los datos deben ser entrados con el ordenamiento bit-reversal
// =============================================================================================

void fbasicfft( float* in, float *out, int Nfft, int inv );

#define  fft(in, out, N) fbasicfft(in, out, N, 1)
#define ifft(in, out, N) fbasicfft(in, out, N, -1)
