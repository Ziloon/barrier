#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>

#define NTHR    (8)
#define NUMNUM  (8000000L)
#define TNUM    (NUMNUM/NTHR)

long nums[NUMNUM];
long snums[NUMNUM];

pthread_barrier_t b;

int complong(const void *arg1, const void *arg2)
{
    long l1 = *(long *)arg1;
    long l2 = *(long *)arg2;

    if (l1 < l2) {
        return -1;
    } else if (l1 > l2) {
        return 1;
    } else {
        return 0;
    }
}

void * thr_fn(void *arg)
{
    long idx = (long)arg;
    qsort(&nums[idx], TNUM, sizeof(long), complong);
    pthread_barrier_wait(&b);

    return ((void *)0);
}

void merge()
{
    long idx[NTHR];
    long i;
    long minidx;
    long sidx;
    long num;

    for (i = 0; i < NTHR; ++i) {
        idx[i] = i * TNUM;
    }

    for (sidx = 0; sidx < NUMNUM; ++sidx) {
        num = LONG_MAX;
        for (int i = 0; i < NTHR; ++i) {
            if ((idx[i] < (i + 1) * TNUM) && (nums[idx[i]] < num)) {
                num = nums[idx[i]];
                minidx = i;
            }
        }

        snums[sidx] = nums[idx[minidx]];
        idx[minidx]++;
    }
}

int save(long nums[], unsigned long len, char *name)
{
    FILE *pf = NULL;
    if ((pf = fopen(name, "w")) != NULL) {
        for (int i = 0; i < len; ++i) {
            fprintf(pf, "%ld\n", nums[i]);
        }

        fclose(pf);
        printf("save %s succ.\n", name);
    } else {
        printf("save %s fail.\n", name);
    }

    return 0;
}

int main() {
#define USEC(x) ((x).tv_sec * 1e6 + (x).tv_usec)

    unsigned long i;
    struct timeval start, end;
    long long startusec, endusec;
    double elapsed;
    int err;
    pthread_t tid;

    srandom(1);
    for (i = 0; i < NUMNUM; ++i) {
        nums[i] = random();
    }
    save(nums, sizeof(nums), "org_data.txt");

    gettimeofday(&start, NULL);
    pthread_barrier_init(&b, NULL, NTHR + 1);
    for (i = 0; i < NTHR; ++i) {
        err = pthread_create(&tid, NULL, thr_fn, (void *)(i + TNUM));
        if (0 != err) {
            perror("can't create thread.\n");
            exit(1);
        }
    }

    pthread_barrier_wait(&b);
    merge();
    gettimeofday(&end, NULL);

    startusec = USEC(start);
    endusec = USEC(end);
    elapsed = (double)(endusec - startusec) / 1e6;
    printf("sort took %.4f seconds.\n", elapsed);
    save(snums, sizeof(snums), "result.txt");
    return 0;
}