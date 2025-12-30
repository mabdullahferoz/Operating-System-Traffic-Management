#include "ipc_manager.h"
#include <stdio.h>
#include <errno.h>

mqd_t create_queue() {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MQ_MSGS;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    mqd_t mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR | O_NONBLOCK, 0644, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open (create) failed");
        // Try to unlink and recreate if it exists and is stuck
        mq_unlink(QUEUE_NAME);
        mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR | O_NONBLOCK, 0644, &attr);
        if (mq == (mqd_t)-1) {
             perror("mq_open (re-create) failed");
        }
    }
    return mq;
}

mqd_t join_queue() {
    mqd_t mq = mq_open(QUEUE_NAME, O_WRONLY); // Vehicles only write
    if (mq == (mqd_t)-1) {
        perror("mq_open (join) failed");
    }
    return mq;
}

int send_vehicle_msg(mqd_t mq, VehicleMessage* msg) {
    if (mq_send(mq, (const char*)msg, sizeof(VehicleMessage), 0) == -1) {
        perror("mq_send failed");
        return -1;
    }
    return 0;
}

int receive_vehicle_msg(mqd_t mq, VehicleMessage* msg) {
    ssize_t bytes_read = mq_receive(mq, (char*)msg, MAX_MSG_SIZE, NULL);
    if (bytes_read >= 0) {
        return 0; // Success
    }
    if (errno == EAGAIN) {
        return 1; // Empty
    }
    perror("mq_receive failed");
    return -1; // Error
}

void cleanup_queue(mqd_t mq) {
    mq_close(mq);
}

void destroy_queue() {
    mq_unlink(QUEUE_NAME);
}
