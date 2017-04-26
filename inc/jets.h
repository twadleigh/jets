#ifndef __JETS_H__
#define __JETS_H__

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

   Must be called (on any thread) before any thread makes a call to `jets_call`.

   It is idempotent. That is, it is safe to call multiple times, though, after the first
   call, subsequent calls have no effect. For example, while it is probably best that it
   is called on the main application thread before other threads are created, it would
   also be safe for each thread that wishes to invoke `jets_call` invokes this first.

   The argument is a string representing a Julia expression that evaluates to C-callable
   function pointer which can be invoked to process submitted `WorkItem`s.
*/
JETS_DLLEXPORT void jets_init(const char * expr);

/* Signals the julia runtime thread to shutdown and waits for it to die.

   This is final. A subsequent call to `jets_init` will not bring the runtime back up.
   
   Care should be taken to invoke this only after all threads have ceased to invoke
   `jets_call`, as subsequent calls will block indefinitely.
*/
JETS_DLLEXPORT void jets_teardown();

/* Creates a `WorkItem` wrapping the given pointer, submits it to the Julia thread, and
   waits to be notified of completion.

   It is currently up to the user to implement on the Julia side the appropriate handling
   for a given input. The handling is built in to the interpretation of the expression
   with which `jets_init` is invoked. It is intended in the future to make the handling
   dynamically extensible through the `jets` interface.

   Invoking `jets_call` before initialization or after teardown will cause a thread to
   block indefinitely (so take care to avoid it).
*/
JETS_DLLEXPORT void jets_call(void *io);

#ifdef __cplusplus
}
#endif
#endif
