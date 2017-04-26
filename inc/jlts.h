#ifndef __JLTS_H__
#define __JLTS_H__

#ifdef __cplusplus
extern "C" {
#endif

void jlts_init(const char * expr);
void jlts_teardown();
void jlts_call(void *io);

#ifdef __cplusplus
}
#endif
#endif
