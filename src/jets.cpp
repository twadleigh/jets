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

    // called on the client thread
    WorkItem(cb_t _cb, void *_arg) : done(false), cb(_cb), arg(_arg) {}

    // called on the julia runtime thread
    void process() {
        ScopeLock lock(mutex);
        try {
            cb(arg);
        } catch(...) {} // swallow any exceptions
        done = true;
        cond.signal();
    }

    // called on the client thread
    void wait() {
        ScopeLock lock(mutex);
        while (!done) { // protect against spurious wake-up
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

    // init the support library and julia runtime
    libsupport_init();
    jl_init();

    // signal to the initializer that we are now ready for execution
    g_ready = true;
    g_cond.signal();

    // loop, waiting for work items to be submitted
    for(;;) {
        g_cond.wait(g_mutex);
        if (g_done) break;
        if (g_work_item) { // protect against spurious wake-up
            g_work_item->process();
            g_work_item = NULL;
        }
    }

    // finalize
    jl_atexit_hook(0);
}

int jets_init() {
    ScopeLock lock(g_mutex);

    if (g_init) return 0;

	int result = uv_thread_create(&g_thread, jets_loop, NULL);
	if (0 > result) {
		jl_printf(jl_stderr_stream(), "Thread creation failed with %u.\n", result);
		return result;
	}

    // wait for the processing thread to be ready
    while (!g_ready) g_cond.wait(g_mutex);

    g_init = true;

	return result;
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
void jets_exec(cb_t cb, void *arg) {
    WorkItem wi(cb, arg);

    // submit the work item
    for(;;) {
        ScopeLock lock(g_mutex);
        assert(g_init && g_ready && !g_done);
        if (g_work_item == NULL) { // protect against spurious lock acquistion
            g_work_item = &wi;
            g_cond.signal();
            break;
        }
    }

    // wait for the work to be completed
    wi.wait();
}
