// ------------------------------------------------------------------
// 
// ------------------------------------------------------------------
// 
// 
// ------------------------------------------------------------------


#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>

#include "iotTimer.h"

#define CLOCKID    CLOCK_REALTIME
#define SIG        SIGRTMIN

static timer_t timerid = 0;
static int     cbPar;
static timerCb cbFunction;



static void timerHandler(int sig, siginfo_t *si, void *uc)
{

    //printf("************* Caught signal %d - %d *****************\n", sig, cbPar );
    
    cbFunction( cbPar );

    //signal(sig, SIG_IGN);
    
    //timer_delete( timerid );
    //timerid = 0;
}

int timerStop( void ) {
    if ( timerid ) {
        //printf( "================================ Stopping timer 0x%x\n", (int)timerid );
        if (timer_delete(timerid) == -1) {
            perror("timer_delete");
            return 0;
        }
        timerid = 0;
    }
    return 1;
}

/**
 * \brief Start a timer that fires after <msec> milli-seconds to call the <cb> callback function with
 * parameter <par>
 * \param msec Number of milli-seconds after which timer fires
 * \param interv Number of milli-seconds after which timer fires
 * \param cb Call-back function
 * \param par Paramater to be supplied to the call-back function
 */

int timerStart( int msec,int interval, timerCb cb, int par ) {
  
    cbFunction = cb;
    cbPar      = par;
    
    struct sigaction sa;
    sigset_t mask;
    struct sigevent sev;
    struct itimerspec its;
    
    if ( timerid ) {
        printf( "Timer: already busy\n" );
        return 0;
    }
    
    //printf("Establishing handler for signal %d\n", SIG);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timerHandler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIG, &sa, NULL) == -1) {
        perror("sigaction");
        return 0;
    }
    
    //printf("Blocking signal %d\n", SIG);
    sigemptyset(&mask);
    sigaddset(&mask, SIG);
    if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
        perror("sigprocmask");
        return 0;
    }

    /* Create the timer */
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIG;
    sev.sigev_value.sival_ptr = &timerid;
    if (timer_create(CLOCKID, &sev, &timerid) == -1) {
        perror("timer_create");
        return 0;
    }

    //printf("timer ID is 0x%lx\n", (long) timerid);

    /* Start the timer */
    its.it_value.tv_sec = msec / 1000;
    its.it_value.tv_nsec = ( msec % 1000 ) * 1000;
    its.it_interval.tv_sec = interval / 1000;
    its.it_interval.tv_nsec = ( interval % 1000 ) * 1000;

    if (timer_settime(timerid, 0, &its, NULL) == -1) {
        perror("timer_settime");
        return 0;
    }

    //printf("Unblocking signal %d\n", SIG);
    if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1) {
        perror("sigprocmask");
        return 0;
    }
    
    return 1;
}

