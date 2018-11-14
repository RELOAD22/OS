#ifndef INCLUDE_MAIL_BOX_
#define INCLUDE_MAIL_BOX_

#define NUM_MAX_BUFFER 10
//#define MAX_BUFFER 10
typedef struct mailbox
{
    char name[40];
    int buffer[NUM_MAX_BUFFER];
    int count;
} mailbox_t;


void mbox_init();
mailbox_t *mbox_open(char *);
void mbox_close(mailbox_t *);
void mbox_send(mailbox_t *, void *, int);
void mbox_recv(mailbox_t *, void *, int);

#endif