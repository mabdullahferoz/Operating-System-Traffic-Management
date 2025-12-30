#ifndef TRAFFIC_LOGIC_H
#define TRAFFIC_LOGIC_H

#include <pthread.h>
#include "utils.h"

// Shared Resources
extern Node* lane_queues[NUM_LANES];
extern pthread_mutex_t intersection_mutex;
extern pthread_mutex_t queue_mutex; // Protects linked lists
extern int current_green_lane; // -1 if all red

// Thread Structure for Sensors
typedef struct {
    int lane_id;
    char lane_name[10];
} SensorArgs;

// Functions
void init_traffic_system();
void* sensor_thread(void* arg);
int select_next_lane(int current_lane);
void enter_intersection(int lane_id);
void handle_aging(Node** head);
void cleanup_traffic_system();
int has_emergency(int lane_id);
int is_any_emergency_active();

#endif
