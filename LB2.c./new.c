#include <pthread.h>
#include <stdio.h>
#include <math.h>
#define PI 3.14159

double res_func_1;
double res_func_2;
double res_func_3;

pthread_barrier_t bar_1;
pthread_barrier_t bar_2;


struct Data{
double a;
double b;
int n;
};

void* func_1(void* proc){
double x;
double h;
struct Data* data1 = (struct Data*)proc;
for(int i = 0; i<=data1->n;i++){
h = (data1->b - data1->a)/data1->n;
x = data1->a - i*h;
res_func_1 = cos(x) * cos(x) + sin(x);
pthread_barrier_wait(&bar_1);
pthread_barrier_wait(&bar_2);
}
return NULL;
}

void* func_2(void* proc){
double x;
double h;
struct Data* data1 = (struct Data*)proc;
for(int i = 0; i<=data1->n;i++){
h = (data1->b - data1->a)/data1->n;
x = data1->a - i*h;
res_func_2 = cos(x) * cos(x) *(1 - sin(x));
pthread_barrier_wait(&bar_1);
pthread_barrier_wait(&bar_2);
}
return NULL;

}
void* func_3(void* proc){
double x;
double h;
struct Data* data1 = (struct Data*)proc;
for(int i = 0; i<=data1->n;i++){
h = (data1->b - data1->a)/data1->n;
x = data1->a - i*h;
res_func_3 = cos(x) *(1 - pow(sin(x),2));
pthread_barrier_wait(&bar_1);
pthread_barrier_wait(&bar_2);
}
return NULL;

}

int main(){
double a = -PI;
double b = PI;
int n = 10;
double x;
double h;
h = (b-a)/n;

pthread_t f1;
pthread_t f2;
pthread_t f3;

struct Data data;
data.a = a;
data.b = b;
data.n = n;


printf("     x     | cos^2 + sin | cos^2 ×(1-sin) | cos×(1-sin^2)  |\n");
printf("-----------|-------------|----------------|----------------|---\n");

pthread_barrier_init(&bar_1,NULL,4);
pthread_barrier_init(&bar_2,NULL,4);
pthread_create(&f1,NULL,&func_1,&data);
pthread_create(&f2,NULL,&func_2,&data);
pthread_create(&f3,NULL,&func_3,&data);


for(int i = 0; i<=n;i++){
x = a + i*h;
printf(" %8.5f  |",x);
pthread_barrier_wait(&bar_1);
printf(" %9.5f   |",res_func_1);
printf(" %11.5f    |",res_func_2);
printf(" %11.5f    |\n",res_func_3);
pthread_barrier_wait(&bar_2);

}
printf("-----------|-------------|----------------|----------------|---\n");

pthread_join(f1,NULL);
pthread_join(f2,NULL);
pthread_join(f3,NULL);

pthread_barrier_destroy(&bar_1);
pthread_barrier_destroy(&bar_2);


return 0;
}
