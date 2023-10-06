/*   
 *    
 *
 * Compile: mpicc  -o ptread ptread.c -lpthread -lrt
 * Usage:    mpiexec -n 3 ./ptread
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <mpi.h>     

#define THREAD_NUM 9    // Tamanho do pool de threads
#define BUFFER_SIZE 6 // Númermo máximo de tarefas enfileiradas


typedef struct Clock { 
   int p[4];
} Clock;
//relogio para salvar as snapshot
typedef struct SnapshotState {
    Clock vectorClock
} SnapshotState;

int snapshotInProgress = 0;

Clock clock = {{0,0,0,0}};

Clock clock2 = {{-1,-1,-1,-1}};

pthread_t thread[THREAD_NUM]; 

void mainThread1(void args);  

void mainThread2(void args);

void mainThread0(void args);

void startRecepcaoThread(void args);

void startEnvioThread(void args);

Clock fila1[BUFFER_SIZE];
Clock fila2[BUFFER_SIZE];
int taskCount = {0,0,0};
pthread_mutex_t mutex;
pthread_mutex_t mutex2;
pthread_cond_t condFull;
pthread_cond_t condEmpty;
pthread_cond_t condFull2;
pthread_cond_t condEmpty2;

void Event(int pid, Clock clock){
   clock.p[pid]++;
  printf("%d %d %d event Process: %d\n",clock->p[0],clock->p[1],clock->p[2],pid);
}

void Send(int pid, Clock clock,int pid2){
   clock.p[pid]++;
   submitTask2(clock)
  //MPI_Send(clock->p, 4, MPI_INT, pid2, 0, MPI_COMM_WORLD); 
  printf("%d %d %d send Process: %d\n",clock->p[0],clock->p[1],clock->p[2],pid);
}

void recieve(int pid, Clock clock){
   clock.p[pid]++;
   int p[4];
   p = getTask1(pid).p
   
   //MPI_Recv(p, 4, MPI_INT, pid2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
   if(clock->p[0]<p[0])
      clock->p[0]=p[0]; 
    if(clock->p[1]<p[1])
         clock->p[1]=p[1]; 
    if(clock->p[2]<p[2])
         clock->p[2]=p[2]; 
printf("%d %d %d recieve Process: %d\n",clock->p[0],clock->p[1],clock->p[2],pid);

}
void criarThreads(int n){
   
   for (int i = 0; i < THREAD_NUM-3; i++){  
       if(i%2==0)
      if (pthread_create(&thread[i], NULL, &startRecepcaoThread, (void*) n) != 0) {
         perror("Failed to create the thread");
      }  
      else
       if (pthread_create(&thread[i], NULL, &startEnvioThread, (void*) n) != 0) {
         perror("Failed to create the thread");
      } 
   }
}

Clock getTask2(int pid){
   pthread_mutex_lock(&mutex2);
   
   while (taskCount[pid] == 0&& fila2[0][pid].p[4]==pid){
        printf("vazio\n");
      pthread_cond_wait(&condEmpty, &mutex);
   }
   
   Clock clock = fila2[0];
   int i;
   for (i = 0; i < taskCount[pid] - 1; i++){
      fila1[i] = fila1[i+1];
   }
   taskCount[pid]--;
   
   pthread_mutex_unlock(&mutex2);
   pthread_cond_signal(&condFull2);
   return clock;
}

Clock getTask1(int pid){
   pthread_mutex_lock(&mutex);
   
   while (taskCount[pid] == 0&& fila1[0][pid].p==pid){
        printf("vazio\n");
      pthread_cond_wait(&condEmpty, &mutex);
   }
   
   Clock clock = fila1[0];
   int i;
   for (i = 0; i < taskCount[pid] - 1; i++){
      fila1[i] = fila1[i+1];
   }
   taskCount[pid]--;
   
   pthread_mutex_unlock(&mutex);
   pthread_cond_signal(&condFull);
   return clock;
}

void submitTask1(Clock clock){
   pthread_mutex_lock(&mutex);

   while (taskCount == BUFFER_SIZE){
      printf("cheio\n");
      pthread_cond_wait(&condFull, &mutex);
   }

   fila1[taskCount] = clock;
   taskCount++;

   pthread_mutex_unlock(&mutex);
   pthread_cond_signal(&condEmpty);
}

void submitTask2(Clock clock){
   pthread_mutex_lock(&mutex2);

   while (taskCount == BUFFER_SIZE){
      printf("cheio\n");
      pthread_cond_wait(&condFull, &mutex2);
   }

   fila2[taskCount] = clock;
   taskCount++;

   pthread_mutex_unlock(&mutex2);
   pthread_cond_signal(&condEmpty2);
}

/*--------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
   pthread_mutex_init(&mutex, NULL);
   pthread_cond_init(&condEmpty, NULL);
   pthread_cond_init(&condFull, NULL);

   pthread_mutex_init(&mutex2, NULL);
   pthread_cond_init(&condEmpty2, NULL);
   pthread_cond_init(&condFull2, NULL);

   srand(time(NULL));
   long i;
   
   int my_rank;               
   MPI_Init(NULL, NULL); 
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 


   if (my_rank == 0) { 
      if (pthread_create(&thread[0], NULL, &mainThread0, (void*) 0) != 0) {
         perror("Failed to create the thread");
      }  
      if (pthread_create(&thread[1], NULL, &startRecepcaoThread, (void*) 0) != 0) {
         perror("Failed to create the thread");
      }  
       if (pthread_create(&thread[2], NULL, &startEnvioThread, (void*) 0) != 0) {
         perror("Failed to create the thread");
      } 
   } else if (my_rank == 1) {  
      if (pthread_create(&thread[3], NULL, &mainThread1, (void*) 1) != 0) {
         perror("Failed to create the thread");
      } 
      if (pthread_create(&thread[4], NULL, &startRecepcaoThread, (void*) 1) != 0) {
         perror("Failed to create the thread");
      }  
       if (pthread_create(&thread[5], NULL, &startEnvioThread, (void*) 1) != 0) {
         perror("Failed to create the thread");
      } 
   } else if (my_rank == 2) {  
      if (pthread_create(&thread[6], NULL, &mainThread2, (void*) 2) != 0) {
         perror("Failed to create the thread");
      }  
      if (pthread_create(&thread[7], NULL, &startRecepcaoThread, (void*) 2) != 0) {
         perror("Failed to create the thread");
      }  
       if (pthread_create(&thread[8], NULL, &startEnvioThread, (void*) 2) != 0) {
         perror("Failed to create the thread");
      } 
   }

   
   for (int i = 0; i < THREAD_NUM; i++){  
      if (pthread_join(thread[i], NULL) != 0) {
         perror("Failed to join the thread");
      }  
   }
   
   /* Finaliza MPI */
   MPI_Finalize(); 
   
   pthread_mutex_destroy(&mutex);
   pthread_cond_destroy(&condEmpty);
   pthread_cond_destroy(&condFull);
   pthread_mutex_init(&mutex2, NULL);
   pthread_cond_init(&condEmpty2, NULL);
   pthread_cond_init(&condFull2, NULL);
   return 0;
} 
 /* main */
/*-------------------------------------------------------------------*/
void startRecepcaoThread(void args) {
   for(i=0;i<3;i++){
   MPI_Recv(p, 4, MPI_INT,  MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
   clock.p=p;
   if(clock.p =={-1,-1,-1,-1} ){
        //salvar a snapshot
        snapshotState0.vectorClock = clock;
   }
   submitTask1(clock);
      
   }
   return NULL;
} 

void startEnvioThread(void args) {
   int process=args;
   for(i=0;i<3;i++){
   Clock clock = getTask2(process);
   MPI_Send(clock.p, 4, MPI_INT, clock.p[4], 0, MPI_COMM_WORLD);
   }
   return NULL;
} 

void mainThread0(void args) {
   updateClock();
   Event(0, clock);
   send(0, clock,1);
   if (snapshotInProgress)==1 {
        // Capturar o estado local em snapshotState
        snapshotState0.vectorClock = clock;
        // enviar marcador para todos os processos 
        // faltar alterar o paramentro para enviar como um broadcast
        MPI_Send(clock2.p, 4, MPI_INT, clock2.p[4], 0, MPI_COMM_WORLD);
        // talvez essa variavel n seja precise quando implemenatr os mutex
        snapshotInProgress = 0;

    }
   send(0, clock,2);
   send((0, clock,1));
   Event(0, clock);
   return NULL;
} 
void mainThread1(void args) {
   recieve(1,clock);
   send((1, clock,0));
   return NULL;
}

void mainThread2(void args) {
   recieve(2,clock);
   send(2, clock,0);
   Event(2, clock);
   return NULL;
}