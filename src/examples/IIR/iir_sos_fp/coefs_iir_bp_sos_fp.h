/*Second Order Sections (SOS) automatically generated header file*/
/*Alexander López Parrado, PhD*/

/*The number of sections*/
#define NSECT 6

/*Number of bits in fractional part of coeffcients*/
/*Fixed point format with 16 bits ([3].[13])*/
#define Q 13

/*Gain on each stage*/
const int16_t gk = 1885;

/*Filter Coefficients b00,b01,b02,a00,a01,a02,b10,b11,b12,b10,b11,b12,...*/
const int16_t SOS[NSECT*3*2] = {
8192, //1.000000
16416, //2.003874
8224, //1.003879
8192, //1.000000
-2526, //-0.308392
4722, //0.576461
8192, //1.000000
16384, //1.999999
8192, //1.000004
8192, //1.000000
-4394, //-0.536414
4833, //0.589957
8192, //1.000000
16352, //1.996127
8160, //0.996132
8192, //1.000000
-969, //-0.118321
5501, //0.671487
8192, //1.000000
-16418, //-2.004127
8226, //1.004133
8192, //1.000000
-6197, //-0.756445
5735, //0.700025
8192, //1.000000
-16384, //-1.999995
8192, //1.000001
8192, //1.000000
-39, //-0.004797
7106, //0.867452
8192, //1.000000
-16350, //-1.995878
8158, //0.995883
8192, //1.000000
-7687, //-0.938387
7242, //0.884042
};
