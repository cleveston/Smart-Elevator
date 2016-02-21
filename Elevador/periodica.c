#include "periodica.h"

//Configura sinal do timer

void configuraTimer() {

    sigset_t alarm_sig;
    int i;

    sigemptyset(&alarm_sig);

    for (i = SIGRTMIN; i <= SIGRTMAX; i++)
        sigaddset(&alarm_sig, i);

    sigprocmask(SIG_BLOCK, &alarm_sig, NULL);

}

//Configura periódica

int configuraPeriodica(int unsigned period, struct periodic_info *info) {
    static int next_sig;
    int ret;
    unsigned int ns;
    unsigned int sec;
    struct sigevent sigev;
    timer_t timer_id;
    struct itimerspec itval;

    if (next_sig == 0)
        next_sig = SIGRTMIN;

    if (next_sig > SIGRTMAX)
        return -1;
    info->sig = next_sig;
    next_sig++;

    sigemptyset(&(info->alarm_sig));
    sigaddset(&(info->alarm_sig), info->sig);

    sigev.sigev_notify = SIGEV_SIGNAL;
    sigev.sigev_signo = info->sig;
    sigev.sigev_value.sival_ptr = (void *) &timer_id;
    ret = timer_create(CLOCK_MONOTONIC, &sigev, &timer_id);
    if (ret == -1)
        return ret;

    sec = period;
    ns = 0;
    itval.it_interval.tv_sec = sec;
    itval.it_interval.tv_nsec = ns;
    itval.it_value.tv_sec = sec;
    itval.it_value.tv_nsec = ns;
    ret = timer_settime(timer_id, 0, &itval, NULL);

    return ret;
}

//Função que aguarda x segundos

void aguardaSeg(int segundos, struct periodic_info *info) {

    int contador = segundos;

    while (contador > 0) {

        //Aguarda por novo periodo
        aguardaPeriodo(info);

        contador--;
    }

}

//Obtém a hora exata do sistema

void obtemHora(char* buffer) {

    volatile time_t rawtime;
    struct tm * timeinfo;

    //Obtem a hora exata e formata
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 25, "%I:%M:%S", timeinfo);

}

//Função aguarda o período

void aguardaPeriodo(struct periodic_info *info) {
    int sig;
    sigwait(&(info->alarm_sig), &sig);
}
