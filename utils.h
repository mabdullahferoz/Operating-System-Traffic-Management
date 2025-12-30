#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// Vehicle Types
#define REGULAR_CAR 0
#define AMBULANCE 1
#define POLICE 2
#define FIRE_TRUCK 3

// Lane Constants
#define NUM_LANES 4
#define NORTH 0
#define SOUTH 1
#define EAST 2
#define WEST 3

// ANSI Colors
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define BOLD "\033[1m"
#define BOLD_RED_BLINK "\033[1;5;31m"

typedef struct Vehicle {
    int id;
    int type; // 0=Regular, 1=Ambulance, 2=Police, 3=FireTruck
    int lane; // 0=N, 1=S, 2=E, 3=W
    time_t arrival_time;
    int priority_score; // Calculated based on type + wait time
} Vehicle;

typedef struct Node {
    Vehicle vehicle;
    struct Node* next;
} Node;

// Linked List Functions
void add_vehicle(Node** head, Vehicle v);
Vehicle remove_vehicle(Node** head); /* Removes head (FIFO/Priority depending on logic elsewhere, but here we just pop head for simplicity or could be specific) - actually usually we serve head. */
Vehicle remove_vehicle_by_id(Node** head, int id); // If needed
int count_vehicles(Node* head);
void print_lane_status(Node* head, char* lane_name, int is_green);

// UI Helpers
void clear_screen();
void print_header();
const char* get_vehicle_type_str(int type);
void draw_traffic_scene(Node* lanes[], int green_lane_idx, Vehicle* crossing_v);
void log_vehicle(Vehicle v);

#endif
