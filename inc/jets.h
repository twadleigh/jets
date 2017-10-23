#ifndef __JETS_H__
#define __JETS_H__

#ifdef JETS_EXPORTS
#define JETS_DLLEXPORT __declspec(dllexport)
#else
#define JETS_DLLEXPORT __declspec(dllimport)
#endif

typedef void(*cb_t)(void *);

#ifdef __cplusplus
extern "C" {
#endif

/* Creates the thread on which the julia runtime will be initialized and through which
   calls to the runtime will be routed.

   Must be called (on any thread) before any thread makes a call to `jets_exec`.

   It is idempotent. That is, it is safe to call multiple times, though, after the first
   call, subsequent calls have no effect. For example, while it is probably best that it
   is called on the main application thread before other threads are created, it would
   also be safe for each thread that wishes to invoke `jets_exec` invokes this first.

   Returns zero if the thread was created successfully, and a value less than zero if not.
*/
JETS_DLLEXPORT int jets_init();

/* Signals the julia runtime thread to shutdown and waits for it to die.

   This is final. A subsequent call to `jets_init` will not bring the runtime back up.

   Care should be taken to invoke this only after all threads have ceased to invoke
   `jets_exec`, as subsequent calls will block indefinitely.
*/
JETS_DLLEXPORT void jets_teardown();

/* Executes the thunk on the julia thread, invoking it with the given argument, and
   blocking the calling thread until execution on the julia thread is complete.

   Invocations of `jets_exec` are serialized on the julia thread, so that each can be
   considered to be effectively atomic with respect to interaction with the runtime.
*/
JETS_DLLEXPORT void jets_exec(cb_t cb, void *arg);

#ifdef __cplusplus
}
#endif
#endif
