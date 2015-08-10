#ifndef __DBG_LOGH__
#define __DBG_LOGH__

#include <stdint.h>

#ifdef __cplusplus 
extern "C"
{                  
#endif

//#define	NO_DEBUG_LOG

#define DBGLVL_ERROR    0
#define DBGLVL_WARN	    1
#define DBGLVL_SETUP    2
#define DBGLVL_STAT     3
#define DBGLVL_TRACE    4
#define DBGLVL_FNTRACE  5
#define DBGLVL_FRAME    6
#define DBGLVL_PACKET   7
#define DBGLVL_WAITLOOP 8

extern void dbg_printf (const char *format, ...);
extern int gDbgLevel;

#ifdef NO_DEBUG_LOG
#define DBG_LOG(DbgLevel, x)
#else
#define DBG_LOG(DbgLevel, x) do { if(DbgLevel <= gDbgLevel) { \
									fprintf(stderr,"%s:%s:%d:", __FILE__, __FUNCTION__, __LINE__); \
									dbg_printf x;                                                  \
									fprintf(stderr,"\n");                                          \
								}                                                                  \
							} while(0);
#endif

#ifdef NO_DEBUG_LOG
#define DBG_LOG_M(DbgLevel, x)
#else
#define DBG_LOG_M(DbgLevel, x) do { if(DbgLevel <= mDbgLevel) { \
									fprintf(stderr,"%s:%s:%d:", __FILE__, __FUNCTION__, __LINE__); \
									dbg_printf x;                                                  \
									fprintf(stderr,"\n");                                          \
								}                                                                  \
							} while(0);
#endif

#define TRACE_PROGRESS printf("At %s:%d \n", __FUNCTION__, __LINE__); fflush(stdout); 

#define ERROR(...)                                                             \
          if (1)                                                               \
          {                                                                    \
            fprintf(stderr, "ERROR: %s: %d: ", __FILE__, __LINE__);            \
            fprintf(stderr, __VA_ARGS__);                                      \
            fprintf(stderr, "\n");                                             \
            exit (1);                                                          \
          }

// DBG_LOG fails on OMX callbacks. Use this function instead
#ifdef NO_DEBUG_LOG
#define DBG_MSG(...)
#else
#define DBG_MSG(...)                                                           \
          if (gDbgLevel > DBGLVL_SETUP)                                        \
          {                                                                    \
            printf("%s:%s:%d: ", __FILE__, __FUNCTION__, __LINE__);   \
            printf(__VA_ARGS__);                                      \
            printf( "\n");                                            \
          }
#endif

#ifdef NO_DEBUG_LOG
#define DBG_PRINT(...)
#else
#define DBG_PRINT(...)                                                \
          if (1)                                                      \
          {                                                           \
            printf(__VA_ARGS__);                                      \
          }
#endif

#ifdef __cplusplus
}
#endif

#endif


