/* mfcc.c */
#ifdef M5StackCore2
#include <M5Core2.h>
#include <drive/i2s.h>
#include "SD.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#endif

#include "mfcc.h"

/* boundary index of mel-filterbank channels */
/* for even channels */
const IDX pIdxEven[11]={0,5,10,17,24,34,45,58,74,93,115};
/* for odd channels */
const IDX pIdxOdd[11]={3,8,13,20,29,39,51,65,83,104,128}; 
/* frequency weights of mel-filtarbank channels */
/* for even channels */
const DTYPE pEven[129]={0.000000,0.470339,0.940678,0.624614,0.195048,0.214189,0.606516,0.998844,0.642738,0.284421,0.067491,0.394746,0.722002,0.955012,0.656126,0.357239,0.058353,0.219682,0.492658,0.765635,0.964736,0.715423,0.466110,0.216798,0.029696,0.257397,0.485097,0.712797,0.940497,0.846384,0.638422,0.430461,0.222500,0.014539,0.176655,0.366588,0.556522,0.746455,0.936388,0.884629,0.711160,0.537691,0.364223,0.190754,0.017286,0.142643,0.301074,0.459505,0.617935,0.776366,0.934797,0.914854,0.770157,0.625461,0.480764,0.336067,0.191371,0.046674,0.089525,0.221679,0.353832,0.485985,0.618138,0.750291,0.882444,0.986668,0.865971,0.745274,0.624577,0.503880,0.383182,0.262485,0.141788,0.021091,0.090971,0.201205,0.311439,0.421673,0.531907,0.642141,0.752375,0.862610,0.972844,0.924124,0.823446,0.722768,0.622090,0.521412,0.420734,0.320056,0.219378,0.118700,0.018021,0.075491,0.167442,0.259393,0.351343,0.443294,0.535244,0.627195,0.719146,0.811096,0.903047,0.994997,0.920590,0.836610,0.752631,0.668652,0.584673,0.500693,0.416714,0.332735,0.248755,0.164776,0.080797,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000,0.000000};
/* for odd channels */
const DTYPE pOdd[129]={0.000000,0.000000,0.000000,0.375386,0.804952,0.785811,0.393484,0.001156,0.357262,0.715579,0.932509,0.605254,0.277998,0.044988,0.343874,0.642761,0.941647,0.780318,0.507342,0.234365,0.035264,0.284577,0.533890,0.783202,0.970304,0.742603,0.514903,0.287203,0.059503,0.153616,0.361578,0.569539,0.777500,0.985461,0.823345,0.633412,0.443478,0.253545,0.063612,0.115371,0.288840,0.462308,0.635777,0.809246,0.982714,0.857357,0.698926,0.540495,0.382065,0.223634,0.065203,0.085146,0.229843,0.374539,0.519236,0.663933,0.808629,0.953326,0.910475,0.778321,0.646168,0.514015,0.381862,0.249709,0.117556,0.013332,0.134029,0.254726,0.375423,0.496120,0.616818,0.737515,0.858212,0.978909,0.909029,0.798795,0.688561,0.578327,0.468093,0.357859,0.247625,0.137390,0.027156,0.075876,0.176554,0.277232,0.377910,0.478588,0.579266,0.679944,0.780622,0.881300,0.981979,0.924509,0.832558,0.740607,0.648657,0.556706,0.464756,0.372805,0.280854,0.188904,0.096953,0.005003,0.079410,0.163390,0.247369,0.331348,0.415327,0.499307,0.583286,0.667265,0.751245,0.835224,0.919203,0.997093,0.920394,0.843694,0.766995,0.690295,0.613596,0.536896,0.460197,0.383497,0.306798,0.230098,0.153399,0.076699,0.000000};
/* */
const DTYPE pZ[100]={0.000000,0.078459,0.156434,0.233445,0.309017,0.382683,0.453990,0.522499,0.587785,0.649448,0.707107,0.760406,0.809017,0.852640,0.891007,0.923880,0.951057,0.972370,0.987688,0.996917,1.000000,0.996917,0.987688,0.972370,0.951057,0.923880,0.891007,0.852640,0.809017,0.760406,0.707107,0.649448,0.587785,0.522499,0.453990,0.382683,0.309017,0.233445,0.156434,0.078459,0.000000,-0.078459,-0.156434,-0.233445,-0.309017,-0.382683,-0.453990,-0.522499,-0.587785,-0.649448,-0.707107,-0.760406,-0.809017,-0.852640,-0.891007,-0.923880,-0.951057,-0.972370,-0.987688,-0.996917,-1.000000,-0.996917,-0.987688,-0.972370,-0.951057,-0.923880,-0.891007,-0.852640,-0.809017,-0.760406,-0.707107,-0.649448,-0.587785,-0.522499,-0.453990,-0.382683,-0.309017,-0.233445,-0.156434,-0.078459,-0.000000,0.078459,0.156434,0.233445,0.309017,0.382683,0.453990,0.522499,0.587785,0.649448,0.707107,0.760406,0.809017,0.852640,0.891007,0.923880,0.951057,0.972370,0.987688,0.996917};


/* pIn[ 20] */
/* pOut[20] */
void DCT_IIE_20(DTYPE *pIn,DTYPE *pOut)
{
  const IDX nDCTLength=20;
  IDX       k, n;  
  for (k=0;k<nDCTLength;k++) {
    *pOut=0;
    for (n=0;n<nDCTLength;n++) {
      *pOut+=2*pIn[n]*(pZ[(2*k*n+nDCTLength) % (4*nDCTLength)]*pZ[k+nDCTLength]-
		       pZ[ 2*k*n             % (4*nDCTLength)]*pZ[k]           );
    }
    *pOut++;
  }
}

/* calculate output of mel-filterbanks */
/* [in]  pEnergy[129] */
/* [out] pFBank[20]   */
void fbank(DTYPE *pEnergy,DTYPE *pFBank)
{
  DTYPE dTemp;
  
  // calculate filter-output
  for (IDX j=0;j<NCHANNELS/2;j++) {
    // Triangular filters
    dTemp=0;
    for (IDX k=pIdxEven[j];k<=pIdxEven[j+1];k++) {
      dTemp+=pEnergy[k]*pEven[k];
    }
    pFBank[2*j]=10*log10(2.0*dTemp);
    // Triangular filters    
    dTemp=0;
    for (IDX k=pIdxOdd[j];k<=pIdxOdd[j+1];k++) {
      dTemp+=pEnergy[k]*pOdd[k];
    }
    pFBank[2*j+1]=10*log10(2.0*dTemp);
  }
}
