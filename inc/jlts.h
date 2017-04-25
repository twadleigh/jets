#ifndef __JLTS_H__
#define __JLTS_H__

typedef struct jlts_worker_s jlts_worker_t;

#ifdef __cplusplus
extern "C" {
#endif

void jlts_init();
void jlts_finalize();

// executes on the global worker
void jlts_call(void *io);

// jlts_worker_t *jlts_new_worker(uv_async_t *async);
void jlts_finalize_worker(jlts_worker_t *wkr);
void jlts_call_worker(jlts_worker_t *wkr, void *io);

#ifdef __cplusplus
}
#endif
#endif
