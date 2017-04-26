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

void jlts_loop(void *);

struct Jlts {
    bool ready;
    bool done;
    WorkItem *work_item;
    Mutex mutex;
    Cond cond;
    uv_thread_t thread;

    Jlts() : ready(false), done(false), work_item(NULL) {
        ScopeLock lock(mutex);
        uv_thread_create(&thread, jlts_loop, NULL);

        // wait for the processing thread to be ready
        while (!ready) cond.wait(mutex);
    }

    ~Jlts() {
        { // signal the thread to end
            ScopeLock lock(mutex);
            done = true;
            cond.signal();
        }

        // wait for the thread to complete
        uv_thread_join(&thread);
    }

    void call(void *io) {
        Mutex m;
        ScopeLock m_lock(m);

        Cond c;
        WorkItem wi;
        wi.mutex = &m;
        wi.cond = &c._cond;
        wi.io = io;
        wi.done = NULL;

        { // submit the work item
            ScopeLock mutex_lock(mutex);
            work_item = &wi;
            cond.signal();
        }

        // wait for the work to be completed
        while(wi.done == NULL) {
            c.wait(m);
        }
    }

    void loop() {
        ScopeLock lock(mutex);

        // init the julia runtime
        jl_init();

        // get the julia function callback for submitting work items
        jl_eval_string("println(pwd())");
        uv_thread_cb jl_submit = (uv_thread_cb)jl_unbox_voidpointer(jl_eval_string("include(\"C:/tw/jlts/jl/boot.jl\")"));

        // signal to constructor that we are now ready for execution
        ready = true;
        cond.signal();

        // loop, waiting for work items to be submitted
        for(;;) {
            cond.wait(mutex);
            if (done) break;
            if (work_item) {
                jl_submit(work_item);
                work_item = NULL;
            }
        }

        // finalize
        jl_atexit_hook(0);
    }

} g_jlts;

void jlts_loop(void *) {
    g_jlts.loop();
}

void jlts_call(void *io) {
    g_jlts.call(io);
}
