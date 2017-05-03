#ifndef __JETS_H__
#define __JETS_H__

#include <julia.h>

#ifdef JETS_EXPORTS
#define JETS_DLLEXPORT __declspec(dllexport)
#else
#define JETS_DLLEXPORT __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Creates the thread on which the julia runtime will be initialized and through which
   calls to the runtime will be routed.

   Must be called (on any thread) before any thread makes a call to `jets_invoke`.

   It is idempotent. That is, it is safe to call multiple times, though, after the first
   call, subsequent calls have no effect. For example, while it is probably best that it
   is called on the main application thread before other threads are created, it would
   also be safe for each thread that wishes to invoke `jets_invoke` invokes this first.
*/
JETS_DLLEXPORT void jets_init();

/* Signals the julia runtime thread to shutdown and waits for it to die.

   This is final. A subsequent call to `jets_init` will not bring the runtime back up.

   Care should be taken to invoke this only after all threads have ceased to invoke
   `jets_invoke`, as subsequent calls will block indefinitely.
*/
JETS_DLLEXPORT void jets_teardown();

JETS_DLLEXPORT jl_value_t *jets_eval_string(const char * expr);

#ifdef __cplusplus
}
#endif
#endif
