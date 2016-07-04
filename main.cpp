#include <iostream>
#include <pthread.h>
#include <unistd.h>

#define THREADS_COUNT 4

using namespace std;

union mutex_type {
    pthread_mutex_t     mutex;
    pthread_spinlock_t  spinlock;
    pthread_rwlock_t    rwlock;
};

struct thread_arg {
    int                 num;
    union mutex_type    mt;
};

static void* thread_start(void *arg)
{
    struct thread_arg *tharg = (struct thread_arg *)arg;

    cout << "in new thread N = " << tharg->num << endl;

    switch (tharg->num) {
    case 0:
        pthread_mutex_lock((pthread_mutex_t *)&tharg->mt);
        sleep(tharg->num + 1);
        pthread_mutex_unlock((pthread_mutex_t *)&tharg->mt);
        break;

    case 1:
        pthread_spin_lock((pthread_spinlock_t *)&tharg->mt);
        sleep(tharg->num + 1);
        pthread_spin_unlock((pthread_spinlock_t *)&tharg->mt);
        break;

    case 2:
        pthread_rwlock_rdlock((pthread_rwlock_t *)&tharg->mt);
        sleep(tharg->num + 1);
        pthread_rwlock_unlock((pthread_rwlock_t *)&tharg->mt);
        break;

    default:
        pthread_rwlock_wrlock((pthread_rwlock_t *)&tharg->mt);
        sleep(tharg->num + 1);
        pthread_rwlock_unlock((pthread_rwlock_t *)&tharg->mt);
        break;
    }

    cout << "close thread N = " << tharg->num << endl;

//    pthread_exit((void *)((struct thread_arg *)arg)->num);
    pthread_exit((void *)tharg->num);

}

int main()
{
    pthread_t thread[THREADS_COUNT];
    struct thread_arg arg[THREADS_COUNT];
    int res;
    int *ret;

    for (int i = 0; i < THREADS_COUNT; i++) {
        switch (i) {
        case 0:
            pthread_mutex_init((pthread_mutex_t *)&arg[i].mt, NULL);
            pthread_mutex_lock((pthread_mutex_t *)&arg[i].mt);
            break;

        case 1:
            pthread_spin_init((pthread_spinlock_t *)&arg[i].mt, PTHREAD_PROCESS_PRIVATE);
            pthread_spin_lock((pthread_spinlock_t *)&arg[i].mt);
            break;

        case 2:
            pthread_rwlock_init((pthread_rwlock_t *)&arg[i].mt, NULL);
            pthread_rwlock_rdlock((pthread_rwlock_t *)&arg[i].mt);
            break;

        default:
            pthread_rwlock_init((pthread_rwlock_t *)&arg[i].mt, NULL);
            pthread_rwlock_wrlock((pthread_rwlock_t *)&arg[i].mt);
            break;
        }

        arg[i].num = i;

        res = pthread_create(&thread[i], NULL, &thread_start, &(arg[i]));
        if (res != 0) {
            cout << "pthread_create() is fail for thread " << i << ", res = " << res << endl;
            return 1;
        }
    }

    sleep(20);

    for (int i = 0; i < THREADS_COUNT; i++) {
        switch (i) {
        case 0:
            pthread_mutex_unlock((pthread_mutex_t *)&arg[i].mt);
            break;

        case 1:
            pthread_spin_unlock((pthread_spinlock_t *)&arg[i].mt);
            break;

        default:
            pthread_rwlock_unlock((pthread_rwlock_t *)&arg[i].mt);
            break;
        }
    }

    for (int i = 0; i < THREADS_COUNT; i++) {
        res = pthread_join(thread[i], (void **)&ret);
        if (res != 0) {
            cout << "pthread_join() is fail for thread " << i << ", res = " << res << endl;
            return 1;
        }
    }

    for (int i = 0; i < THREADS_COUNT; i++) {
        switch (i) {
        case 0:
            pthread_mutex_destroy((pthread_mutex_t *)&arg[i].mt);
            break;

        case 1:
            pthread_spin_destroy((pthread_spinlock_t *)&arg[i].mt);
            break;

        default:
            pthread_rwlock_destroy((pthread_rwlock_t *)&arg[i].mt);
            break;
        }
    }

    return 0;
}
