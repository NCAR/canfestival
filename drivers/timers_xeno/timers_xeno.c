#include <stdlib.h>

#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <sys/timerfd.h>
#include <time.h>

#include <applicfg.h>
#include <timers.h>

static pthread_mutex_t CanFestival_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t timer_mutex = PTHREAD_MUTEX_INITIALIZER;
int tfd;
static int timer_created = 0;
struct timespec last_occured_alarm;
struct timespec next_alarm;
struct timespec time_ref;

static pthread_t timer_thread; 
int stop_timer = 0;

/* Subtract the struct timespec values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0. */
int
timespec_subtract (struct timespec *result, struct timespec *x, struct timespec *y)
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_nsec < y->tv_nsec) {
    int seconds = (y->tv_nsec - x->tv_nsec) / 1000000000L + 1;
    y->tv_nsec -= 1000000000L * seconds;
    y->tv_sec += seconds;
  }
  if (x->tv_nsec - y->tv_nsec > 1000000000L) {
    int seconds = (x->tv_nsec - y->tv_nsec) / 1000000000L;
    y->tv_nsec += 1000000000L * seconds;
    y->tv_sec -= seconds;
  }

  /* Compute the time remaining to wait.
     tv_nsec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_nsec = x->tv_nsec - y->tv_nsec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}

void timespec_add (struct timespec *result, struct timespec *x, struct timespec *y)
{
  result->tv_sec = x->tv_sec + y->tv_sec;
  result->tv_nsec = x->tv_nsec + y->tv_nsec;

  while (result->tv_nsec > 1000000000L) {
      result->tv_sec ++;
      result->tv_nsec -= 1000000000L;
  }
}

void TimerCleanup(void)
{
}

void EnterTimerMutex(void)
{
	if(pthread_mutex_lock(&timer_mutex)) {
		perror("pthread_mutex_lock(timer_mutex) failed\n");
	}
}

void LeaveTimerMutex(void)
{
	if(pthread_mutex_unlock(&timer_mutex)) {
		perror("pthread_mutex_unlock(timer_mutex) failed\n");
	}
}

void EnterMutex(void)
{
	if(pthread_mutex_lock(&CanFestival_mutex)) {
		perror("pthread_mutex_lock(CanFestival_mutex) failed\n");
	}
}

void LeaveMutex(void)
{
	if(pthread_mutex_unlock(&CanFestival_mutex)) {
		perror("pthread_mutex_unlock(CanFestival_mutex) failed\n");
	}
}

void TimerInit(void)
{
    /* Initialize absolute time references */
    if(clock_gettime(CLOCK_MONOTONIC, &time_ref)){
        perror("clock_gettime(time_ref)");
    }
    next_alarm = last_occured_alarm = time_ref;
}

void StopTimerLoop(TimerCallback_t exitfunction)
{

	stop_timer = 1;

    pthread_cancel(timer_thread);
    pthread_join(timer_thread, NULL);

	EnterMutex();
	exitfunction(NULL,0);
	LeaveMutex();
}

void* xenomai_timerLoop(void* unused)
{
    struct sched_param param = { .sched_priority = 80 };
    int ret;

    if (ret = pthread_setname_np(pthread_self(), "canfestival_timer")) {
        fprintf(stderr, "pthread_setname_np(): %s\n",
                strerror(-ret));
        goto exit_timerLoop;
    }
    
    if (ret = pthread_setschedparam(pthread_self(), SCHED_FIFO, &param)) {
        fprintf(stderr, "pthread_setschedparam(): %s\n",
                strerror(-ret));
        goto exit_timerLoop;
    }

    EnterTimerMutex();

    tfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if(tfd == -1) {
		perror("timer_create() failed\n");
        goto exit_timerLoop_timermutex;
    }
    timer_created = 1;

    while (!stop_timer)
    {
        uint64_t ticks;

        LeaveTimerMutex();

        EnterMutex();
        TimeDispatch();
        LeaveMutex();

        /*  wait next timer occurence */

        ret = read(tfd, &ticks, sizeof(ticks));
        if (ret < 0) {
            perror("timerfd read()\n");
            break;
        }

        EnterTimerMutex();

        last_occured_alarm = next_alarm;
    }

    close(tfd);

    timer_created = 0;

exit_timerLoop_timermutex:
    LeaveTimerMutex();

exit_timerLoop:
    return NULL;
}


void StartTimerLoop(TimerCallback_t init_callback)
{
    int ret;

	stop_timer = 0;	

	EnterMutex();
	// At first, TimeDispatch will call init_callback.
	SetAlarm(NULL, 0, init_callback, 0, 0);
	LeaveMutex();

    ret = pthread_create(&timer_thread, NULL, &xenomai_timerLoop, NULL);
    if (ret) {
        fprintf(stderr, "StartTimerLoop pthread_create(): %s\n",
                strerror(-ret));
    }
}

static void (*unixtimer_ReceiveLoop_task_proc)(CAN_PORT) = NULL;

void* xenomai_canReceiveLoop(void* port)
{
    int ret;
    struct sched_param param = { .sched_priority = 82 };
    pthread_setname_np(pthread_self(), "canReceiveLoop");
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

    unixtimer_ReceiveLoop_task_proc((CAN_PORT)port);

    return NULL;
}

void CreateReceiveTask(CAN_PORT port, TASK_HANDLE* Thread, void* ReceiveLoopPtr)
{
    unixtimer_ReceiveLoop_task_proc = ReceiveLoopPtr;

	if(pthread_create(Thread, NULL, xenomai_canReceiveLoop, (void*)port)) {
		perror("CreateReceiveTask pthread_create()");
	}
}

void WaitReceiveTaskEnd(TASK_HANDLE *Thread)
{
	if(pthread_cancel(*Thread)) {
		perror("pthread_cancel()");
	}
	if(pthread_join(*Thread, NULL)) {
		perror("pthread_join()");
	}
}

#define maxval(a,b) ((a>b)?a:b)
void setTimer(TIMEVAL value)
{
    struct itimerspec timerValues;

    EnterTimerMutex();

    if(timer_created){
        long tv_nsec = (maxval(value,1)%1000000000LL);
        time_t tv_sec = value/1000000000LL;
        timerValues.it_value.tv_sec = tv_sec;
        timerValues.it_value.tv_nsec = tv_nsec;
        timerValues.it_interval.tv_sec = 0;
        timerValues.it_interval.tv_nsec = 0;

        /* keep track of when should alarm occur*/
        timespec_add(&next_alarm, &time_ref, &timerValues.it_value);
        timerfd_settime (tfd, 0, &timerValues, NULL);
    }

    LeaveTimerMutex();
}


TIMEVAL getElapsedTime(void)
{
    struct itimerspec outTimerValues;
    struct timespec ts;
    struct timespec last_occured_alarm_copy;

    TIMEVAL res = 0;

    EnterTimerMutex();

    if(timer_created){
        if(clock_gettime(CLOCK_MONOTONIC, &time_ref)){
            perror("clock_gettime(ts)");

        }
        /* timespec_substract modifies second operand */
        last_occured_alarm_copy = last_occured_alarm;

        timespec_subtract(&ts, &time_ref, &last_occured_alarm_copy);

        /* TIMEVAL is nano seconds */
        res = (ts.tv_sec * 1000000000L) + ts.tv_nsec;
    }

    LeaveTimerMutex();

	return res;
}

