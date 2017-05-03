#define JETS_EXPORTS

#include "jets.h"

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
    bool done;
    Mutex mutex;
    Cond cond;
    cb_t cb;
    void *arg;

    WorkItem(cb_t _cb, void *_arg) : done(false), cb(_cb), arg(_arg) {}

    void process() {
        ScopeLock lock(mutex);
        cb(arg);
        done = true;
        cond.signal();
    }

    void wait() {
        while (!done) {
            cond.wait(mutex);
        }
    }
};

bool g_init = false;
bool g_ready = false;
bool g_done = false;
WorkItem *g_work_item = NULL;
Mutex g_mutex;
Cond g_cond;
uv_thread_t g_thread;

void jets_loop(void *) {
    ScopeLock lock(g_mutex);

    // init the julia runtime
    jl_init();

    // signal to the initializer that we are now ready for execution
    g_ready = true;
    g_cond.signal();

    // loop, waiting for work items to be submitted
    for(;;) {
        g_cond.wait(g_mutex);
        if (g_done) break;
        if (g_work_item) {
            g_work_item->process();
            g_work_item = NULL;
        }
    }

    // finalize
    jl_atexit_hook(0);
}

void jets_init() {
    ScopeLock lock(g_mutex);

    if (g_init) return;

    uv_thread_create(&g_thread, jets_loop, NULL);

    // wait for the processing thread to be ready
    while (!g_ready) g_cond.wait(g_mutex);

    g_init = true;
}

void jets_teardown() {
    { // signal the thread to end
        ScopeLock lock(g_mutex);
        g_done = true;
        g_cond.signal();
    }

    // wait for the thread to complete
    uv_thread_join(&g_thread);
}

// means by which other threads have work executed on the julia thread
void jets_eval(cb_t cb, void *arg) {
    WorkItem wi(cb, arg);
    ScopeLock l(wi.mutex);

    { // submit the work item
        ScopeLock lock(g_mutex);
        g_work_item = &wi;
        g_cond.signal();
    }

    // wait for the work to be completed
    wi.wait();
}
