#include "traffic_logic.h"
#include <unistd.h>
#include <stdio.h>

Node* lane_queues[NUM_LANES] = {NULL, NULL, NULL, NULL};
pthread_mutex_t intersection_mutex;
pthread_mutex_t queue_mutex;
int current_green_lane = -1;

void init_traffic_system() {
    pthread_mutex_init(&intersection_mutex, NULL);
    pthread_mutex_init(&queue_mutex, NULL);
}

void* sensor_thread(void* arg) {
    (void)arg; // Silence unused warning
    while (1) {
        pthread_mutex_lock(&queue_mutex);
        // Monitor implementation
        // int count = count_vehicles(lane_queues[args->lane_id]);
        
        // Simple logic: if count is extremely high w/o emergency, maybe flag it? 
        // For now, this thread just "monitors" - in a real HW system it would populate the list.
        // Here we just use it to maybe print "Sensor [North]: Density High" if needed.
        // But main loop does printing, so we keep this minimal.
        
        pthread_mutex_unlock(&queue_mutex);
        
        sleep(2); // Monitor every 2 seconds
    }
    return NULL;
}

// Aging Algorithm: Increment priority of waiting cars
#define AGING_THRESHOLD 10

void handle_aging(Node** head) {
    Node* temp = *head;
    while (temp != NULL) {
        if (temp->vehicle.type == REGULAR_CAR) {
             temp->vehicle.priority_score++;
        }
        temp = temp->next;
    }
}

// Helper: Check if specific lane has emergency
int has_emergency(int lane_id) {
    if (lane_id < 0 || lane_id >= NUM_LANES) return 0;
    Node* temp = lane_queues[lane_id];
    while(temp != NULL) {
        // Emergency if not regular car OR priority score is high
        if(temp->vehicle.type != REGULAR_CAR || temp->vehicle.priority_score >= AGING_THRESHOLD) return 1;
        temp = temp->next;
    }
    return 0;
}

// Global Emergency Check
int is_any_emergency_active() {
    for(int i=0; i<NUM_LANES; i++) {
        if (has_emergency(i)) return 1;
    }
    return 0;
}

int select_next_lane(int current_lane) {
    // 1. Priority Loop: Check for Emergency (Round Robin starting next to current)
    for (int i = 1; i <= NUM_LANES; i++) {
        int idx = (current_lane + i) % NUM_LANES;
        if (has_emergency(idx)) return idx;
    }

    // 2. Normal Loop: Round Robin for any car
    // Note: If ANY emergency exists anywhere, we should NOT return a normal lane here
    // unless that logic is handled in main. 
    // Actually, strict priority means if ANY emergency exists, we must only pick emergency.
    // The loop 1 above covers it. If loop 1 fails, it means NO emergency exists anywhere.
    
    for (int i = 1; i <= NUM_LANES; i++) {
        int idx = (current_lane + i) % NUM_LANES;
        if (lane_queues[idx] != NULL) return idx;
    }
    
    return -1; // All empty
}

void enter_intersection(int lane_id) {
    // Critical Section
    pthread_mutex_lock(&intersection_mutex);
    
    current_green_lane = lane_id;
    
    // Simulate traffic flow
    // Process 1 car from this lane
    pthread_mutex_lock(&queue_mutex);
    if (lane_queues[lane_id] != NULL) {
        remove_vehicle(&lane_queues[lane_id]);
        // Note: In real world, multiple cars go during Green.
        // Here we can process one or few.
        // Let's pretend we process 1 car per cycle for slower visual.
        
        // Apply Aging to others
        for(int i=0; i<NUM_LANES; i++) handle_aging(&lane_queues[i]);
    }
    pthread_mutex_unlock(&queue_mutex);

    // Sleep to simulate crossing time
    usleep(500000); // 0.5s
    
    current_green_lane = -1; // Yellow/Red
    
    pthread_mutex_unlock(&intersection_mutex);
}

void cleanup_traffic_system() {
    pthread_mutex_destroy(&intersection_mutex);
    pthread_mutex_destroy(&queue_mutex);
    // Free lists if needed
}
