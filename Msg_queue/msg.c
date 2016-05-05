#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <sys/ipc.h>

//#define DEBUG_MOD
#ifndef DEBUG_MOD
#define DEBUG
#else
#define DEBUG if(0)
#endif

#define ASSERT( cond, message )     \
 if (!(cond)) {                     \
    perror (message);               \
    putchar ('\n');                 \
    return -1;                      \
 }

#define msg_ASSERT( cond, message ) \
 if (!(cond)) {                     \
    perror (message);               \
    putchar ('\n');                 \
    msgctl (msg_id, IPC_RMID, NULL);\
    return -1;                      \
 }

struct Msg
{
    long type;
};// __attribute__((packed));

const int MSG_SIZE = 0;
const int BUF_SIZE = 0;

struct Msg msg = {1};
int i = 1;

int main (int argc, char** argv)
{
    int pid = 0, child_id = 0;

    i = 0;
    msg.type = 0;

//=============== CHECING_MAIN_ARGS =========================================
    ASSERT (argc == 2,
            "ERROR: the program needs one arguments");

    printf("msg %p i %p\n", &msg, &i);

//=============== GETTING_NUMBER ============================================
    char* end_ptr = NULL;
	long nForks = strtol (argv[1], &end_ptr, 10);
    ASSERT (*end_ptr == 0 && errno != ERANGE,
            "ERROR: bad input. The program needs one natural number");

//=============== CREATING_MSG_QUEUE ========================================
    ASSERT (creat ("msg_base", 0644) != -1,
           "ERROR: creat (msg_base) failed");

    int msg_key = ftok ("msg_base", 1);
    ASSERT (msg_key != -1,
            "ERROR: ftok failed");
    DEBUG printf ("DEBUG: %d\n", msg_key);

	int msg_id = msgget (msg_key, IPC_CREAT | 0600);
	ASSERT (msg_id != -1,
            "ERROR: msgget failed\n");

//=============== STEP 1. FORKING ===========================================
    void* msg_ptr = (void*) &msg;

    for (i = 1; i <= nForks; i++) {
        DEBUG printf ("*********%d\n", i);
        pid = fork();
        msg_ASSERT (pid >= 0,
                    "ERROR: fork failed");

        if (pid == 0) {
            //DEBUG printf ("DEBUG: %d. %d\n", i, getpid());
            msg_ASSERT (msgrcv (msg_id, msg_ptr, MSG_SIZE, i, 0) != -1,
                        "ERROR: msgrcv failed");
            printf ("%d\n", i);
            msg.type = i + 1;
            msg_ASSERT (msgsnd (msg_id, msg_ptr, MSG_SIZE, IPC_NOWAIT) == 0,
                        "ERROR: msgsnd failed");
            return 0;
        }
    }

//=============== STEP 2. PRINTING ============================================
    msg.type = 1;
    msg_ASSERT (msgsnd (msg_id, msg_ptr, MSG_SIZE, IPC_NOWAIT) == 0,
               "ERROR: msgsnd failed\n");
   msg_ASSERT (msgrcv (msg_id, msg_ptr, MSG_SIZE, nForks + 1, 0) != -1,
                "ERROR: msgrcv failed");

//=============================================================================
    msgctl(msg_id, IPC_RMID, NULL);
    return 0;
}
