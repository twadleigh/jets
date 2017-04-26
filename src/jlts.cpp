#include "jlts.h"

#include <julia.h>
#include <uv.h>

struct Mutex : public uv_mutex_t {
    Mutex() { uv_mutex_init(this); }
    ~Mutex() { uv_mutex_destroy(this); }
};

struct Cond {
    uv_cond_t _cond;
    Cond() { uv_cond_init(&_cond); }
    ~Cond() { uv_cond_destroy(&_cond); }

    void signal() { uv_cond_signal(&_cond); }
    void wait(Mutex &m) { uv_cond_wait(&_cond, &m); }
};

struct ScopeLock {
    Mutex &_mutex;
    ScopeLock(Mutex &m) : _mutex(m) { uv_mutex_lock(&_mutex); }
    ~ScopeLock() { uv_mutex_unlock(&_mutex); }
};

struct WorkItem {
    uv_mutex_t *mutex;
    uv_cond_t *cond;
    void *io;
    void *done;
};

bool g_init = false;
bool g_ready = false;
bool g_done = false;
WorkItem *g_work_item = NULL;
Mutex g_mutex;
Cond g_cond;
uv_thread_t g_thread;

void jlts_loop(void *expr) {
    ScopeLock lock(g_mutex);

    // init the julia runtime
    jl_init();

    // get the julia function callback for submitting work items
    uv_thread_cb jl_submit = (uv_thread_cb)jl_unbox_voidpointer(jl_eval_string((const char *)expr));

    // signal to the initializer that we are now ready for execution
    g_ready = true;
    g_cond.signal();

    // loop, waiting for work items to be submitted
    for(;;) {
        g_cond.wait(g_mutex);
        if (g_done) break;
        if (g_work_item) {
            jl_submit(g_work_item);
            g_work_item = NULL;
        }
    }

    // finalize
    jl_atexit_hook(0);
}

void jlts_init(const char * expr) {
    ScopeLock lock(g_mutex);

    if (g_init) return;

    uv_thread_create(&g_thread, jlts_loop, (void *)expr);

    // wait for the processing thread to be ready
    while (!g_ready) g_cond.wait(g_mutex);

    g_init = true;
}

void jlts_teardown() {
    { // signal the thread to end
        ScopeLock lock(g_mutex);
        g_done = true;
        g_cond.signal();
    }

    // wait for the thread to complete
    uv_thread_join(&g_thread);
}

void jlts_call(void *io) {
    Mutex m;
    ScopeLock l(m);

    Cond c;
    WorkItem wi;
    wi.mutex = &m;
    wi.cond = &c._cond;
    wi.io = io;
    wi.done = NULL;

    { // submit the work item
        ScopeLock lock(g_mutex);
        g_work_item = &wi;
        g_cond.signal();
    }

    // wait for the work to be completed
    while(wi.done == NULL) {
        c.wait(m);
    }
}
