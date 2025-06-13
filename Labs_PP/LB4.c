#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>


typedef struct Stack
{
    int number;           // Значення вузла
    struct Stack* next;  // Вказівник на наступний вузол
} Stack;

// Вказівник на вершину стека
Stack* stack_top = NULL;

pthread_t th1,th2,th3,th4,th5,th6;

int buffer_clear_loops = 2;

FILE* thread_print;

int ai1 = 0, ai2 = 0;
unsigned au1 = 0, au2 = 0;
long al1 = 0, al2 = 0;
long unsigned alu1 = 0, alu2 = 0;

sem_t scr21,scr1;

pthread_mutex_t mcr1 = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t sig1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t sig2 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mcr21 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mcr22 = PTHREAD_MUTEX_INITIALIZER;
int sig1_flag = 0, sig2_flag = 0;


int use_usleep = 1; // Налаштування затримок
int usleep_1;
int usleep_2;
int usleep_3;


void init_usleep()
{
    if(use_usleep)
    {
        usleep_1 = 100;
        usleep_2 = 250;
        usleep_3 = 500;

    }
    else
    {
        usleep_1 = 0;
        usleep_2 = 0;
        usleep_3 = 0;

    }


}



/* Додавання нового елементу до стека */
void add_elem()
{
    Stack* p = (Stack*)malloc(sizeof(Stack));
    if (!p)
    {
        printf("Помилка виділення пам'яті!\n");
        return;
    }
    p->next = stack_top;
    p->number = (stack_top ? stack_top->number + 1 : 0);
    stack_top = p;
}

Stack* get_elem()
{
    if (stack_top == NULL)
        return NULL;

    Stack* p = stack_top;
    stack_top = stack_top->next;
    return p;
}


int is_stack_empty()
{
    return (stack_top == NULL) ? 1 : 0;  // Повертає 1, якщо стек порожній; інакше 0
}



void use_atom(int thread_number)
{
    fprintf(thread_print,"Thread%d use_atom START: \n\n\n", thread_number);
    fprintf(thread_print, "int: %d, %d\n", ai1, ai2);
    fprintf(thread_print, "unsigned: %u, %u\n", au1, au2);
    fprintf(thread_print, "long: %ld, %ld\n", al1, al2);
    fprintf(thread_print, "long unsigned: %lu, %lu\n", alu1, alu2);

}


void mod_atom(int thread_number)
{
    fprintf(thread_print,"Thread%d mod_atom START: \n\n\n", thread_number);
    fprintf(thread_print, "int sub fetch: %d\n", __atomic_sub_fetch(&ai1, thread_number, __ATOMIC_RELAXED));
    fprintf(thread_print, "int xor fetch: %d\n", __atomic_xor_fetch(&ai2, thread_number, __ATOMIC_RELAXED));
    fprintf(thread_print, "unsigned nand fetch: %u\n", __atomic_nand_fetch(&au1, thread_number, __ATOMIC_RELAXED));
    fprintf(thread_print, "unsigned fetch add: %u\n", __atomic_fetch_add(&au2, thread_number, __ATOMIC_RELAXED));
    fprintf(thread_print, "long fetch and: %ld\n", __atomic_fetch_and(&al1, thread_number, __ATOMIC_RELAXED));
    fprintf(thread_print, "long fetch or: %ld\n", __atomic_fetch_or(&al2, thread_number, __ATOMIC_RELAXED));
    fprintf(thread_print, "long unsigned compare exchange_n: %d\n", __atomic_compare_exchange_n(&alu1, &alu2, thread_number, 0, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    fprintf(thread_print, "long unsigned compare exchange_n: before %lu, %lu;\n", alu2, alu1);
    __atomic_exchange(&alu2, &alu1, &alu2, __ATOMIC_RELAXED);
    fprintf(thread_print, " after useg compare exchange %lu, %lu\n", alu2, alu1);
}


void* thread_1(void* arg)
{
    int num = *(int*)arg;
    struct Stack* curr_elem = NULL;
    int sem_value;


    while (1)
    {
        if (sem_trywait(&scr1) != 0)
        {
            fprintf(thread_print,"Consumer thread%d does some useful work while semaphore is locked\n", num);
            usleep(usleep_1);
        }
        else
        {
            sem_getvalue(&scr1, &sem_value);
            clock_t start = clock();
            while (pthread_mutex_trylock(&mcr1) != 0)    ///
            {

                fprintf(thread_print,"Consumer thread%d does some useful work while mutex is locked\n",num);
                if (clock() - start > 1)usleep(10);

            }

            curr_elem = get_elem();
            fprintf(thread_print,"Consumer thread%d: semaphore=%d; element %d TAKEN; \n", num, sem_value, curr_elem->number);
            usleep(usleep_1);
            free(curr_elem);


            if (stack_top == NULL)

                if(buffer_clear_loops == 0)
                {
                    switch (num)
                    {
                    case 1:
                    {
                        pthread_cancel(th2);

                        pthread_cancel(th3);

                        pthread_cancel(th4);

                        pthread_cancel(th5);

                        pthread_cancel(th6);

                        //pthread_join(th2,NULL);
                        //pthread_join(th4,NULL);
                        //pthread_join(th5,NULL);






                        break;
                    }
                    case 2:
                    {
                        pthread_cancel(th1);

                        pthread_cancel(th3);

                        pthread_cancel(th4);

                        pthread_cancel(th5);

                        pthread_cancel(th6);

                        //pthread_join(th1,NULL);
                        //pthread_join(th4,NULL);
                        //pthread_join(th5,NULL);



                        break;
                    }
                    case 5:
                    {
                        pthread_cancel(th1);

                        pthread_cancel(th2);

                        pthread_cancel(th3);

                        pthread_cancel(th4);

                        pthread_cancel(th6);

                        //pthread_join(th1,NULL);
                        //pthread_join(th2,NULL);
                        //pthread_join(th4,NULL);



                        break;
                    }

                    }

                    fprintf(thread_print,"Consumer thread%d stopped !!!\n", num);
                    pthread_mutex_unlock(&mcr1);   ///
                    pthread_exit(NULL);
                }
                else
                {
                    buffer_clear_loops--;
                    fprintf(thread_print,"\nConsumer thread%d: buffer_clear_loops=%d \n\n\n", num, buffer_clear_loops);

                }
            else {}
            fprintf(thread_print,"Consumer thread%d: LEAVE the buffer; \n", num);
            pthread_mutex_unlock(&mcr1);  ///
        }
        usleep(usleep_3);

    }
    return NULL;
}


void* thread_2(void* arg)
{

    int num = *(int*)arg;
    struct Stack* curr_elem = NULL;
    int sem_value;
    int scr21_value;

    while (1)
    {


        mod_atom(num);

        sem_getvalue(&scr21, &scr21_value);
        if(scr21_value == 0)
        {
            fprintf(thread_print,"Thread%d opened semaphore scr21\n", num);
            sem_post(&scr21);
        }
        else
        {
            fprintf(thread_print,"Thread%d semaphores cr21 is binary = %d\n", num, scr21_value);
        }



        if (sem_trywait(&scr1) != 0)
        {
            fprintf(thread_print,"Consumer thread%d does some useful work while semaphore is locked\n", num);
            usleep(usleep_1);
        }
        else
        {
            sem_getvalue(&scr1, &sem_value);
            clock_t start = clock();
            while (pthread_mutex_trylock(&mcr1) != 0)    ///
            {
                if (clock() - start > 1) usleep(10);
                fprintf(thread_print,"Consumer thread%d does some useful work while mutex is locked\n",num);
            }

            curr_elem = get_elem();
            fprintf(thread_print,"Consumer thread%d: semaphore=%d; element %d TAKEN; \n", num, sem_value, curr_elem->number);
            usleep(usleep_1);
            free(curr_elem);


            if (stack_top == NULL)

                if(buffer_clear_loops == 0)
                {
                    switch (num)
                    {
                    case 1:
                    {
                        pthread_cancel(th2);

                        pthread_cancel(th3);

                        pthread_cancel(th4);

                        pthread_cancel(th5);

                        pthread_cancel(th6);

                        //pthread_join(th2,NULL);
                        //pthread_join(th4,NULL);
                        //pthread_join(th5,NULL);





                        break;
                    }
                    case 2:
                    {
                        pthread_cancel(th1);

                        pthread_cancel(th3);

                        pthread_cancel(th4);

                        pthread_cancel(th5);

                        pthread_cancel(th6);

                        //pthread_join(th1,NULL);
                        //pthread_join(th4,NULL);
                        //pthread_join(th5,NULL);




                        break;
                    }
                    case 5:
                    {
                        pthread_cancel(th1);

                        pthread_cancel(th2);

                        pthread_cancel(th3);

                        pthread_cancel(th4);

                        pthread_cancel(th6);

                        //pthread_join(th1,NULL);
                        //pthread_join(th2,NULL);
                        //pthread_join(th4,NULL);



                        break;
                    }

                    }

                    fprintf(thread_print,"Consumer thread%d stopped !!!\n", num);
                    pthread_mutex_unlock(&mcr1);   ///
                    pthread_exit(NULL);
                }
                else
                {
                    buffer_clear_loops--;
                    fprintf(thread_print,"\nConsumer thread%d: buffer_clear_loops=%d \n\n\n", num, buffer_clear_loops);

                }
            else {}
            fprintf(thread_print,"Consumer thread%d: LEAVE the buffer; \n", num);
            pthread_mutex_unlock(&mcr1);  ///

        }
        usleep(usleep_2);


        use_atom(num);

        pthread_mutex_lock(&mcr21);
        fprintf(thread_print,"Thread%d entered from mcr21\n", num);
        sig1_flag = 1;
        pthread_cond_signal(&sig1);
        fprintf(thread_print,"Thread%d sent signal sig1\n", num);
        pthread_mutex_unlock(&mcr21);

    }


}

void* thread_3(void* arg)
{
    int num = *(int*)arg;
    int sem_value;


    while(1)
    {

        if(sem_trywait(&scr21) != 0)
        {
            fprintf(thread_print,"Thread%d does some useful work while semaphore is locked\n", num);
            usleep(usleep_1 - 50);
        }
        else
        {
            fprintf(thread_print,"Thread%d passed the semaphore and is moving on\n", num);
            use_atom(num);
        }



    }

}




// Потокова функція продюсера
void* thread_4(void* arg)
{
    int num = *(int*)arg;
    int sem_value;

    while (1)
    {
        sem_getvalue(&scr1, &sem_value);
        if (sem_value < 20)
        {
            while (pthread_mutex_trylock(&mcr1) != 0)
            {
                fprintf(thread_print,"Producer thread%d does some useful work while mutex is locked\n",num);

                usleep(usleep_1);
            }
            if (stack_top == NULL)
                if(buffer_clear_loops == 0)
                {
                    pthread_cancel(th1);

                    pthread_cancel(th3);

                    pthread_cancel(th2);

                    pthread_cancel(th5);

                    pthread_cancel(th6);

                    //pthread_join(th1,NULL);
                    //pthread_join(th2,NULL);
                    //pthread_join(th5,NULL);



                    fprintf(thread_print,"Producer thread%d stopped !!!\n",num);
                    pthread_exit(NULL);
                }
                else
                {

                }
            else {}

            add_elem();
            fprintf(thread_print,"Producer thread%d: semaphore=%d; element CREATED; \n", num, sem_value);
            sem_post(&scr1);
            fprintf(thread_print,"Producer thread%d: LEAVE the buffer; \n", num);
            pthread_mutex_unlock(&mcr1);

        }
        usleep(usleep_2);


    }
    return NULL;
}



void* thread_5(void* arg)
{

    int num = *(int*)arg;
    struct Stack* curr_elem = NULL;
    int sem_value;

    while(1)
    {

        pthread_mutex_lock(&mcr21);
        while(sig1_flag == 0)
        {
            fprintf(thread_print,"Thread%d waiting for a signal sig1: \n", num);
            pthread_cond_wait(&sig1,&mcr21);
        }
        fprintf(thread_print,"Thread%d take signal sig1: \n", num);
        sig1_flag = 0;
        pthread_mutex_unlock(&mcr21);


        if (sem_trywait(&scr1) != 0)
        {
            fprintf(thread_print,"Consumer thread%d does some useful work while semaphore is locked\n", num);
            usleep(usleep_2);
        }
        else
        {
            sem_getvalue(&scr1, &sem_value);
            clock_t start = clock();
            while (pthread_mutex_trylock(&mcr1) != 0)    ///
            {
                if (clock() - start > 1) usleep(10);
                fprintf(thread_print,"Consumer thread%d does some useful work while mutex is locked\n",num);
            }

            curr_elem = get_elem();
            fprintf(thread_print,"Consumer thread%d: semaphore=%d; element %d TAKEN; \n", num, sem_value, curr_elem->number);
            usleep(usleep_2);
            free(curr_elem);


            if (stack_top == NULL)

                if(buffer_clear_loops == 0)
                {
                    switch (num)
                    {
                    case 1:
                    {
                        pthread_cancel(th2);

                        pthread_cancel(th3);

                        pthread_cancel(th4);

                        pthread_cancel(th5);

                        pthread_cancel(th6);

                        //pthread_join(th2,NULL);
                        //pthread_join(th4,NULL);
                        //pthread_join(th5,NULL);







                        break;
                    }
                    case 2:
                    {
                        pthread_cancel(th1);

                        pthread_cancel(th3);

                        pthread_cancel(th4);

                        pthread_cancel(th5);

                        pthread_cancel(th6);

                        //pthread_join(th1,NULL);
                        //pthread_join(th4,NULL);
                        //pthread_join(th5,NULL);




                        break;
                    }
                    case 5:
                    {
                        pthread_cancel(th1);

                        pthread_cancel(th2);

                        pthread_cancel(th3);

                        pthread_cancel(th4);

                        pthread_cancel(th6);

                        //pthread_join(th1,NULL);
                        //pthread_join(th2,NULL);
                        //pthread_join(th4,NULL);


                        break;
                    }

                    }

                    fprintf(thread_print,"Consumer thread%d stopped !!!\n", num);
                    pthread_mutex_unlock(&mcr1);   ///
                    pthread_exit(NULL);
                }
                else
                {
                    buffer_clear_loops--;
                    fprintf(thread_print,"\nConsumer thread%d: buffer_clear_loops=%d \n\n\n", num, buffer_clear_loops);

                }
            else {}
            fprintf(thread_print,"Consumer thread%d: LEAVE the buffer; \n", num);
            pthread_mutex_unlock(&mcr1);  ///
        }
        usleep(usleep_3);

        mod_atom(num);

        pthread_mutex_lock(&mcr22);
        fprintf(thread_print,"Thread%d entered from mcr22\n", num);
        sig2_flag = 1;
        pthread_cond_signal(&sig2);
        fprintf(thread_print,"Thread%d sent signal sig2\n", num);
        pthread_mutex_unlock(&mcr22);


    }


}


void* thread_6(void* arg)
{
    int num = *(int*)arg;
    int sem_value;

    while(1)
    {
        pthread_mutex_lock(&mcr22);
        while(sig2_flag == 0)
        {
            fprintf(thread_print,"Thread%d waiting for a signal sig2: \n", num);
            pthread_cond_wait(&sig2,&mcr22);
        }
        fprintf(thread_print,"Thread%d take signal sig2: \n", num);
        sig2_flag = 0;
        pthread_mutex_unlock(&mcr22);

        use_atom(num);
    }
}




int main()
{

    if ((thread_print = fopen("log.txt", "w")) == NULL)
    {
        perror("Error occurred while opening file");
        return 1;
    }

    sem_init (&scr1, 0, 0);
    sem_init (&scr21, 0, 0);

    int sem_value;
    sem_getvalue(&scr1,&sem_value);
    fprintf(thread_print,"semaphore=%d\n",sem_value);

    init_usleep();

    for(int i=0; i<5; i++)
    {
        add_elem();
        sem_post(&scr1);
    }
    sem_getvalue(&scr1,&sem_value);
    fprintf(thread_print,"semaphore=%d\n",sem_value);




    int thread1_number = 1;
    int thread2_number = 2;
    int thread3_number = 3;
    int thread4_number = 4;
    int thread5_number = 5;
    int thread6_number = 6;


    pthread_create(&th1, NULL, &thread_1, (void*)&thread1_number);
    pthread_create(&th2, NULL, &thread_2, (void*)&thread2_number);
    pthread_create(&th3, NULL, &thread_3, (void*)&thread3_number);
    pthread_create(&th4, NULL, &thread_4, (void*)&thread4_number);
    pthread_create(&th5, NULL, &thread_5, (void*)&thread5_number);
    pthread_create(&th6, NULL, &thread_6, (void*)&thread6_number);

    pthread_join(th1,NULL);
    pthread_join(th2,NULL);
    pthread_join(th3,NULL);
    pthread_join(th4,NULL);
    pthread_join(th5,NULL);
    pthread_join(th6,NULL);

    fprintf(thread_print,"All threads stopped !!!\n");
    pthread_mutex_destroy (&mcr1);
    pthread_mutex_destroy (&mcr21);

    pthread_cond_destroy(&sig1);
    pthread_cond_destroy(&sig2);

    sem_destroy (&scr1);
    sem_destroy (&scr21);
    return 0;

}
