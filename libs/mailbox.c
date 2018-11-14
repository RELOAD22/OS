#include "string.h"
#include "mailbox.h"
#include "lock.h"
#include "cond.h"

#define MAX_NUM_BOX 32

static mailbox_t mboxs[MAX_NUM_BOX];
mutex_lock_t mutex_sun;
condition_t full_sun;
condition_t empty_sun;
mutex_lock_t mutex_liu;
condition_t full_liu;
condition_t empty_liu;

void mbox_init()
{
    int i;
    mutex_lock_init(&mutex_sun);
    mutex_lock_init(&mutex_liu);
    condition_init(&full_sun);
    condition_init(&empty_sun);
    condition_init(&full_liu);
    condition_init(&empty_liu);

    for(i = 0; i < 2; ++i){
        (mboxs[i].name)[0] = 0; 
        mboxs[i].count = 0;
    }    
}

mailbox_t *mbox_open(char *name)
{
    char *s = "SunQuan-Publish-PID";
    char *l = "LiuBei-Publish-PID";
    if(strcmp(name, s) == 0){   
        return (mailbox_t *)(mboxs);
    } else{
        return (mailbox_t *)(mboxs+1);        
    }
}

void mbox_close(mailbox_t *mailbox)
{
}

void mbox_send(mailbox_t *mailbox, void *msg, int msg_length)
{
    if(mailbox == mboxs){
        mutex_lock_acquire(&mutex_sun);
        while(mailbox->count == NUM_MAX_BUFFER){
            condition_wait(&mutex_sun, &full_sun);
        }
        add_msg_array(mailbox, msg, msg_length);
        mailbox->count ++;
        condition_signal(&empty_sun);       
        mutex_lock_release(&mutex_sun);
    }else{
        mutex_lock_acquire(&mutex_liu);
        while(mailbox->count == NUM_MAX_BUFFER){
            condition_wait(&mutex_liu, &full_liu);
        }
        add_msg_array(mailbox, msg, msg_length);
        mailbox->count ++;
        condition_signal(&empty_liu);      
        mutex_lock_release(&mutex_liu);
    }
}

void mbox_recv(mailbox_t *mailbox, void *msg, int msg_length)
{
    if(mailbox == mboxs){
        mutex_lock_acquire(&mutex_sun);
        while(mailbox->count == 0)
            condition_wait(&mutex_sun, &empty_sun);
        remove_msg_array(mailbox, msg, msg_length);
        mailbox->count --;
        condition_signal(&full_sun);    
        mutex_lock_release(&mutex_sun);
    }else {
        mutex_lock_acquire(&mutex_liu);
        while(mailbox->count == 0)
            condition_wait(&mutex_liu, &empty_liu);
        remove_msg_array(mailbox, msg, msg_length);
        mailbox->count --;
        condition_signal(&full_liu);    
        mutex_lock_release(&mutex_liu);        
    }
}

add_msg_array(mailbox_t *mailbox, void *msg, int msg_length){
    int temp = *((int *)msg);
    mailbox->buffer[mailbox->count] = temp;
}

remove_msg_array(mailbox_t *mailbox, void *msg, int msg_length){
    *((int *)msg) =  mailbox->buffer[0];
    int i;
    for(i = 0; i < mailbox->count; ++i){
        mailbox->buffer[i] = mailbox->buffer[i + 1];
    }
}
