#include "jlts.h"

#include <julia/julia.h>
#include <julia/uv.h>

struct jlts_worker_s {
    uv_async_t *async;
    uv_mutex_t mutex;
};

typedef struct {
    void *io;
    uv_cond_t *cond;
    void *done;
} jlts_worker_data_t;

jlts_worker_t *jlts_new_worker(uv_async_t *async) {
    jlts_worker_t *wkr = (jlts_worker_t *)malloc(sizeof(jlts_worker_t));
    uv_mutex_init(&wkr->mutex);
    wkr->async = async;
    return wkr;
}

void jlts_finalize_worker(jlts_worker_t *wkr) {
    wkr->async = NULL;
    uv_mutex_destroy(&wkr->mutex);
    free(wkr);
}

void jlts_call_worker(jlts_worker_t *wkr, void *io) {

    // create and initialize the condition variable on which execution will wait
    uv_cond_t cond;
    uv_cond_init(&cond);

    // set up the worker data struct
    jlts_worker_data_t data;
    data.io = io;
    data.cond = &cond;
    data.done = NULL;

    uv_mutex_lock(&wkr->mutex); // lock the worker for exclusive use
    wkr->async->data = &data; // set the worker data
    uv_async_send(wkr->async); // ask the worker to do work
    while(data.done == NULL) {
        uv_cond_wait(&cond, &wkr->mutex);
    }
    uv_mutex_unlock(&wkr->mutex); // free the worker

    // destroy the condition variable
    uv_cond_destroy(&cond);
}

int g_ref_count = 0;
jlts_worker_t *g_worker;

void jlts_init() {
    if (++g_ref_count > 1) return;

    jl_init_with_image("C:/tw/Julia-0.6.0-pre.beta/bin", "../lib/julia/sys.dll");
    g_worker = jlts_new_worker((uv_async_t *)jl_unbox_voidpointer(jl_eval_string("include(\"C:/tw/jlts/jl/boot.jl\")")));
}

void jlts_finalize() {
    if (g_ref_count <= 0) {
        g_ref_count = 0;
        return;
    }
    if (--g_ref_count > 0) return;

    jl_atexit_hook(0);
    jlts_finalize_worker(g_worker);
}

void jlts_call(void *io) {
    jlts_call_worker(g_worker, io);
}
