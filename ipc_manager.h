#ifndef IPC_MANAGER_H
#define IPC_MANAGER_H

#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "utils.h"

#define QUEUE_NAME "/traffic_mq"
#define MAX_MSG_SIZE sizeof(VehicleMessage)
#define MAX_MQ_MSGS 10

typedef struct {
    int id;
    int lane; // 0=N, 1=S, 2=E, 3=W
    int type; // Vehicle Type
    time_t timestamp;
} VehicleMessage;

// Function Prototypes
mqd_t create_queue();
mqd_t join_queue();
int send_vehicle_msg(mqd_t mq, VehicleMessage* msg);
int receive_vehicle_msg(mqd_t mq, VehicleMessage* msg);
void cleanup_queue(mqd_t mq);
void destroy_queue();

#endif
