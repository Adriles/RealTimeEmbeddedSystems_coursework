// Doty and Unkeles Code for Exercise 1 Pt 4
// NOTE: This code is derived from examples provided by Siewert
// fib, FIB10, and FIB20 are original work
// threadwrapper is original, but some of its internals are from Siewert
// main is a heavily modified version of Siewert's example
// Sleep code in threadwrapper is from RT_Clock example code from Siewert
// All other code is not claimed as original work

#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <time.h>
#include <syslog.h>

#define NUM_THREADS (2)
#define NUM_CPUS (1)

#define MAX_SLEEP_CALLS (3)

#define NSEC_PER_SEC (1000000000)
#define NSEC_PER_MSEC (1000000)
#define NSEC_PER_MICROSEC (1000)
#define DELAY_TICKS (1)
#define ERROR (-1)
#define OK (0)

typedef struct
{
    int deadline_ms;
    void (*load_ptr)(void);
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];
pthread_attr_t rt_sched_attr[NUM_THREADS];
int rt_max_prio, rt_min_prio;
struct sched_param rt_param[NUM_THREADS];
struct sched_param main_param;
pthread_attr_t main_attr;
pid_t mainpid;

// Synthetic Load Code
void fib(int iterations)
{
    int i;
    int operand_0 = 0;
    int operand_1 = 1;
    int fib_number = 0;
    for(i = 0; i < iterations; i++)
    {
        fib_number = operand_0 + operand_1;
        operand_0 = operand_1;
        operand_1 = fib_number;
    }
}

void FIB10(void)
{
  fib(551300);
}

void FIB20(void)
{
  fib(1102000);
}

// Function to calculate delta between two timespecs
int delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t)
{
  int dt_sec=stop->tv_sec - start->tv_sec;
  int dt_nsec=stop->tv_nsec - start->tv_nsec;

  if(dt_sec >= 0)
  {
    if(dt_nsec >= 0)
    {
      delta_t->tv_sec=dt_sec;
      delta_t->tv_nsec=dt_nsec;
    }
    else
    {
      delta_t->tv_sec=dt_sec-1;
      delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
    }
  }
  else
  {
    if(dt_nsec >= 0)
    {
      delta_t->tv_sec=dt_sec;
      delta_t->tv_nsec=dt_nsec;
    }
    else
    {
      delta_t->tv_sec=dt_sec-1;
      delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
    }
  }

  return(1);
}


void *threadwrapper(void *thread_args)
{
  struct timespec start_time = {0, 0};
  struct timespec finish_time = {0, 0};
  struct timespec thread_dt = {0, 0};
  struct timespec sleep_time = {0, 0};
  struct timespec remaining_time = {0, 0};
  struct timespec deadline_time = {0, 0};
  deadline_time.tv_nsec = ((threadParams_t*)thread_args)->deadline_ms * NSEC_PER_MSEC;

  char* thread = NULL;
  if(((threadParams_t*)thread_args)->load_ptr == &FIB10)
  {
    thread = "FIB10";
  }
  else
  {
    thread = "FIB20";
  }  

  int ex_count = 0;

  // For real applications, this would just be while(1)
  // Since we don't want to run forever, we only do 25 executions
  for(ex_count = 0; ex_count<15; ex_count++)
  {
    // Start Timer
    clock_gettime(CLOCK_REALTIME, &start_time);

    // Execute the passed load function
    ((threadParams_t*)thread_args)->load_ptr();

    // End Timer, Calc dT
    clock_gettime(CLOCK_REALTIME, &finish_time);
    delta_t(&finish_time, &start_time, &thread_dt);

    // Log execution time
    syslog(LOG_NOTICE,
            "Thread %s start %03ld.%03ld, end %03ld.%03ld\n",
            thread,
            start_time.tv_sec,
            (start_time.tv_nsec / NSEC_PER_MSEC),
            finish_time.tv_sec,
            (finish_time.tv_nsec / NSEC_PER_MSEC));

    // Calc sleep_time
    delta_t(&deadline_time , &thread_dt, &sleep_time);

    // If sleep_time is nearly zero, we've basically missed a deadline
    if(sleep_time.tv_nsec < 2)
    {
      syslog(LOG_NOTICE, "Thread %s has missed a deadline, exiting\n", thread);
      printf("/nThread %s missed a deadline, exiting early/n", thread);
      exit(0);
    }

    // Do sleep
    int sleep_count = 0;

    do 
    {
        nanosleep(&sleep_time, &remaining_time);
            
        sleep_time.tv_sec = remaining_time.tv_sec;
        sleep_time.tv_nsec = remaining_time.tv_nsec;
        sleep_count++;
    } 
    while (((remaining_time.tv_sec > 0) || (remaining_time.tv_nsec > 0)) && (sleep_count < MAX_SLEEP_CALLS));

    // Now we return back up to the top to execute the load again
  }

  pthread_exit(0);
}


void print_scheduler(void)
{
   int schedType;

   schedType = sched_getscheduler(getpid());

   switch(schedType)
   {
     case SCHED_FIFO:
           printf("Pthread Policy is SCHED_FIFO\n");
           break;
     case SCHED_OTHER:
           printf("Pthread Policy is SCHED_OTHER\n");
       break;
     case SCHED_RR:
           printf("Pthread Policy is SCHED_OTHER\n");
           break;
     default:
       printf("Pthread Policy is UNKNOWN\n");
   }

}

int main (int argc, char *argv[])
{
   openlog("Sched_Test", LOG_PID, LOG_SYSLOG);
   syslog(LOG_NOTICE, "Program started");

   int rc;
   int i, scope;
   cpu_set_t cpuset;

   CPU_ZERO(&cpuset);
   for(i=0; i < NUM_CPUS; i++)
       CPU_SET(i, &cpuset);

   mainpid=getpid();

   rt_max_prio = sched_get_priority_max(SCHED_FIFO);
   rt_min_prio = sched_get_priority_min(SCHED_FIFO);

   print_scheduler();
   rc=sched_getparam(mainpid, &main_param);
   main_param.sched_priority=rt_max_prio;
   rc=sched_setscheduler(getpid(), SCHED_FIFO, &main_param);
   if(rc < 0) perror("main_param");
   print_scheduler();

   pthread_attr_getscope(&main_attr, &scope);

   if(scope == PTHREAD_SCOPE_SYSTEM)
     printf("PTHREAD SCOPE SYSTEM\n");
   else if (scope == PTHREAD_SCOPE_PROCESS)
     printf("PTHREAD SCOPE PROCESS\n");
   else
     printf("PTHREAD SCOPE UNKNOWN\n");

   printf("rt_max_prio=%d\n", rt_max_prio);
   printf("rt_min_prio=%d\n", rt_min_prio);

//=======================================================================

  // Set up thread 0
  rc=pthread_attr_init(&rt_sched_attr[0]);
  rc=pthread_attr_setinheritsched(&rt_sched_attr[0], PTHREAD_EXPLICIT_SCHED);
  rc=pthread_attr_setschedpolicy(&rt_sched_attr[0], SCHED_FIFO);
  rc=pthread_attr_setaffinity_np(&rt_sched_attr[0], sizeof(cpu_set_t), &cpuset);

  rt_param[0].sched_priority=rt_max_prio-1;
  pthread_attr_setschedparam(&rt_sched_attr[0], &rt_param[0]);

  threadParams[0].deadline_ms=20;
  threadParams[0].load_ptr = &FIB10;

  pthread_create(&threads[0],   // pointer to thread descriptor
                &rt_sched_attr[0],     // use default attributes
                threadwrapper, // thread function entry point
                (void *)&(threadParams[0]) // parameters to pass in
                );

  // Set up thread 2
  rc=pthread_attr_init(&rt_sched_attr[1]);
  rc=pthread_attr_setinheritsched(&rt_sched_attr[1], PTHREAD_EXPLICIT_SCHED);
  rc=pthread_attr_setschedpolicy(&rt_sched_attr[1], SCHED_FIFO);
  rc=pthread_attr_setaffinity_np(&rt_sched_attr[1], sizeof(cpu_set_t), &cpuset);

  rt_param[1].sched_priority=rt_max_prio-2;
  pthread_attr_setschedparam(&rt_sched_attr[1], &rt_param[1]);

  threadParams[1].deadline_ms=50;
  threadParams[1].load_ptr = &FIB20;

  pthread_create(&threads[1],   // pointer to thread descriptor
                &rt_sched_attr[1],     // use default attributes
                threadwrapper, // thread function entry point
                (void *)&(threadParams[1]) // parameters to pass in
                );

  // Call to end both threads (puts main thread as suspended till other threads terminate)
  for(i = 0; i<NUM_THREADS; i++)
  {
    pthread_join(threads[i], NULL);
  }
  

//=======================================================================


   syslog(LOG_NOTICE, "Program Ended");

   printf("\nTEST COMPLETE\n");
   closelog();
   exit(OK);
}
