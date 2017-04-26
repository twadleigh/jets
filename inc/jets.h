#ifndef __JETS_H__
#define __JETS_H__

#ifdef __cplusplus
extern "C" {
#endif

void jets_init(const char * expr);
void jets_teardown();
void jets_call(void *io);

#ifdef __cplusplus
}
#endif
#endif
