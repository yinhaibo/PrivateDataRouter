//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <math.h>
#include "Unit1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;

/*******************************************************************
* 求[a,b]上的均匀分布
* 输入: a--双精度实型变量，给出区间的下限
*       b--双精度实型变量，给出区间的上限
*    seed--长整型指针变量，*seed为随机数的种子
********************************************************************/
double Uniform(double a,double b,long int*seed)
{
double t;
*seed=2045*(*seed)+1;
*seed=*seed-(*seed/1048576)*1048576;
t=(*seed)/1048576.0;
t=a+(b-a)*t;
return(t);
}
/*******************************************************************
* 求参数为lambda的泊松分布
* 输入: lambda--双精度实型变量，平均抽样间隔
*       nSeed--长整型变量，nSeed为随机数的种子
********************************************************************/
double Poisson(double lambda, long nSeed)
{
double u = Uniform(0.0, 1.0, &nSeed);
double fRet =  log(u) * (-1) *lambda;
if(fRet < 1.0)
{
return Poisson(lambda, ++nSeed);
}
return fRet;
}

#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define EPS  1.2e-7
#define RNMX (1.0-EPS)
/**
 * "Minimal" random number generator of Park and Miller with Bay-Durham shuffle and added
 * safeguards. Returns a uniform random deviate between 0.0 and 1.0(exclusive of the endpoint
 * values). Call with idum as negative iteger to initialize; thereafter, do not alter idum between
 * successive deviates in a sequence. RNMX should approximate the largest floating value that is
 * less that 1.
 * Refer: Numerical Recipes in C
 */

#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

float ran1(long *idum)
{
	int j;
	long k;
	static long iy=0;
	static long iv[NTAB];
	float temp;

	if (*idum <= 0 || !iy) {
		if (-(*idum) < 1) *idum=1;
		else *idum = -(*idum);
		for (j=NTAB+7;j>=0;j--) {
			k=(*idum)/IQ;
			*idum=IA*(*idum-k*IQ)-IR*k;
			if (*idum < 0) *idum += IM;
			if (j < NTAB) iv[j] = *idum;
		}
		iy=iv[0];
	}
	k=(*idum)/IQ;
	*idum=IA*(*idum-k*IQ)-IR*k;
	if (*idum < 0) *idum += IM;
	j=iy/NDIV;
	iy=iv[j];
	iv[j] = *idum;
	if ((temp=AM*iy) > RNMX) return RNMX;
	else return temp;
}
#undef IA
#undef IM
#undef AM
#undef IQ
#undef IR
#undef NTAB
#undef NDIV
#undef EPS
#undef RNMX


float gammln(float xx)
{
	double x,y,tmp,ser;
	static double cof[6]={76.18009172947146,-86.50532032941677,
		24.01409824083091,-1.231739572450155,
		0.1208650973866179e-2,-0.5395239384953e-5};
	int j;

	y=x=xx;
	tmp=x+5.5;
	tmp -= (x+0.5)*log(tmp);
	ser=1.000000000190015;
	for (j=0;j<=5;j++) ser += cof[j]/++y;
	return -tmp+log(2.5066282746310005*ser/x);
}


#define PI 3.141592654
/**
 * Returns as a floating-point number an interger value that is a random
 * deviate drawn from a Possion distribution of mean xm, using ran1(idum) as 
 * a source of uniform random deviates.
 */
float poidev(float xm, long *idum)
{
	float gammln(float xx);
	float ran1(long *idum);
	static float sq,alxm,g,oldm=(-1.0);
	float em,t,y;

	if (xm < 12.0) {
		if (xm != oldm) {
			oldm=xm;
			g=exp(-xm);
		}
		em = -1;
		t=1.0;
		do {
			++em;
			t *= ran1(idum);
		} while (t > g);
	} else {
		if (xm != oldm) {
			oldm=xm;
			sq=sqrt(2.0*xm);
			alxm=log(xm);
			g=xm*alxm-gammln(xm+1.0);
		}
		do {
			do {
				y=tan(PI*ran1(idum));
				em=sq*y+xm;
			} while (em < 0.0);
			em=floor(em);
			t=0.9*(1.0+y*y)*exp(em*alxm-gammln(em+1.0)-g);
		} while (ran1(idum) > t);
	}
	return em;
}

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
#define N 100
#define NPTS 10000
#define ISCAL 200
void __fastcall TForm1::Button1Click(TObject *Sender)
{
    int i,j;
    int dist[N+1]={0};
    long int seed =0- StrToInt(Edit1->Text);
    int value;
    Memo1->Lines->Clear();
    for (i=1;i<=NPTS;i++) {
        j=(int) (0.5+N*(ran1(&seed)));
        if ((j >= 0) && (j <= N)) ++dist[j];
     }
     Series1->Clear();
    for (i = 1; i <= N; i++){
        Memo1->Lines->Add(IntToStr(dist[i]));
        Series1->AddXY(i, dist[i],"",0xFF0000);
    }


    long idum = 0- StrToInt(Edit1->Text);
     memset(dist, 0, sizeof(dist));
     Memo2->Lines->Clear();
     float xm = N/2;
     for (i=1;i<=NPTS;i++) {
        j=(int) (0.5+poidev(xm,&idum));
        if ((j >= 0) && (j <= N)) ++dist[j];
     }
     Series2->Clear();
    for (i = 1; i <= N; i++){
//        value = (int)(poidev(100, &seed));
        Memo2->Lines->Add(IntToStr(dist[i]));
        Series2->AddXY(i, dist[i],"",255);
    }


}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormCreate(TObject *Sender)
{
    Button1Click(Sender);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button2Click(TObject *Sender)
{
    int val;
    int dist[N+1]={0};
    FSeed = 0- StrToInt(Edit1->Text);
    Memo1->Lines->Clear();
    Series1->Clear();
    memset(dist, 0, sizeof(dist));
    for (int i = 1; i <= NPTS; i++){
        val = getRandRange(1, 1, 100);
        if ((val >= 0) && (val <= N)) ++dist[val];
    }
    for (int i = 1; i <= N; i++){
        Memo1->Lines->Add(IntToStr(dist[i]));
        Series1->AddXY(i, dist[i],"",255);
    }

    FSeed = 0- StrToInt(Edit1->Text);
    Memo2->Lines->Clear();
    Series2->Clear();
    memset(dist, 0, sizeof(dist));
    for (int i = 1; i <= NPTS; i++){
        val = getRandRange(2, 1, 100);
        if ((val >= 0) && (val <= N)) ++dist[val];
    }
    for (int i = 1; i <= N; i++){
        Memo2->Lines->Add(IntToStr(dist[i]));
        Series2->AddXY(i, dist[i],"",255);
    }
}
//---------------------------------------------------------------------------
int __fastcall TForm1::getRandRange(int mode, int from , int to)
{
    if (mode == 1){
        return from + 0.5 + ran1(&FSeed) * (to - from);
    }else if(mode == 2){
        return from + 0.5 + poidev((to - from)/2, &FSeed);
    }else{
        return from;
    }
}
