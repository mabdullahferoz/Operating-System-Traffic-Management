#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <pthread.h>
#include <time.h>
#include "utils.h"
#include "ipc_manager.h"
#include "traffic_logic.h"

// Globals
pid_t generator_pid;
volatile int keep_running = 1;
struct termios orig_termios;
mqd_t global_mq;

#define GREEN_DURATION_SEC 8
#define CROSSING_TIME_USEC 1500000 // 1.5s per car

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void handle_sigint(int sig) {
    keep_running = 0;
}

void vehicle_generator_process() {
    mqd_t mq = join_queue();
    if (mq == (mqd_t)-1) exit(1);
    
    srand(time(NULL));
    int v_id = 1000; 
    
    while (keep_running) {
        VehicleMessage msg;
        msg.id = v_id++;
        msg.lane = rand() % 4; 
        
        int r = rand() % 100;
        if (r < 5) msg.type = AMBULANCE;
        else if (r < 8) msg.type = POLICE;
        else if (r < 10) msg.type = FIRE_TRUCK;
        else msg.type = REGULAR_CAR;
        
        msg.timestamp = time(NULL);
        
        send_vehicle_msg(mq, &msg);
        
        // Random arrival: 3s to 6s
        usleep((rand() % 3000000) + 3000000);
    }
    mq_close(mq);
    exit(0);
}

void* user_input_thread(void* arg) {
    int v_id_user = 1; 
    char c;
    while (keep_running) {
        if (read(STDIN_FILENO, &c, 1) == 1) {
            VehicleMessage msg;
            msg.id = v_id_user++;
            msg.timestamp = time(NULL);
            int valid = 0;

            if (c >= '1' && c <= '4') {
                msg.lane = c - '1';
                msg.type = REGULAR_CAR;
                valid = 1;
            }
            else if (c >= 'a' && c <= 'd') {
                msg.lane = c - 'a';
                msg.type = AMBULANCE; 
                valid = 1;
            }
            if (valid) {
                send_vehicle_msg(global_mq, &msg);
            }
        }
    }
    return NULL;
}

void check_mq_updates() {
    VehicleMessage msg;
    while (receive_vehicle_msg(global_mq, &msg) == 0) {
        Vehicle v;
        v.id = msg.id;
        v.type = msg.type;
        v.lane = msg.lane;
        v.arrival_time = msg.timestamp;
        v.priority_score = 0;
        
        pthread_mutex_lock(&queue_mutex);
        add_vehicle(&lane_queues[v.lane], v);
        pthread_mutex_unlock(&queue_mutex);
    }
}

void render(int green_lane, Vehicle* crossing) {
    clear_screen();
    print_header();
    pthread_mutex_lock(&queue_mutex);
    draw_traffic_scene(lane_queues, green_lane, crossing);
    pthread_mutex_unlock(&queue_mutex);
}

int main() {
    signal(SIGINT, handle_sigint);
    
    global_mq = create_queue();
    init_traffic_system();
    enable_raw_mode(); 

    // Sensors
    pthread_t sensors[NUM_LANES];
    SensorArgs args[NUM_LANES];
    char* names[] = {"NORTH", "SOUTH", "EAST", "WEST"};
    for(int i=0; i<NUM_LANES; i++) {
        args[i].lane_id = i;
        strcpy(args[i].lane_name, names[i]);
        pthread_create(&sensors[i], NULL, sensor_thread, &args[i]);
    }

    // Input thread
    pthread_t input_th;
    pthread_create(&input_th, NULL, user_input_thread, NULL);

    // Generator
    generator_pid = fork();
    if (generator_pid == 0) vehicle_generator_process(); 
    
    int current_lane_idx = 0;
    
    // START MAIN LOOP
    while (keep_running) {
        check_mq_updates();
        
        int next_lane = select_next_lane(current_lane_idx);
        
        if (next_lane == -1) {
            render(-1, NULL);
            usleep(200000);
            continue;
        }

        current_lane_idx = next_lane;
        
        // Check mode
        pthread_mutex_lock(&queue_mutex);
        int is_emergency_round = has_emergency(current_lane_idx);
        pthread_mutex_unlock(&queue_mutex);
        
        // If Emergency Lane: Process 1 car then rotate (RR for fairness among multiple emergencies)
        // If Normal Lane: Process for GREEN_DURATION, but PREEMPT if emergency appears elsewhere.
        
        time_t green_start = time(NULL);
        int max_duration = is_emergency_round ? 0 : GREEN_DURATION_SEC; // 0 ensures at least 1 loop
        
        // GREEN LIGHT TIMER LOOP
        while ( (is_emergency_round || (time(NULL) - green_start < max_duration)) && keep_running) {
             check_mq_updates();

             // PREEMPTION CHECK (For Normal Lanes)
             if (!is_emergency_round) {
                 pthread_mutex_lock(&queue_mutex);
                 int emergency_exists = is_any_emergency_active();
                 pthread_mutex_unlock(&queue_mutex);
                 
                 if (emergency_exists) {
                     // Emergency appeared! Yield immediately.
                     // (Render yellow/red transition will happen after break)
                     break; 
                 }
             }

             // Check if vehicles exist in green lane
             Vehicle v_crossing = { -1 };
             pthread_mutex_lock(&queue_mutex);
             if (lane_queues[current_lane_idx] != NULL) {
                 v_crossing = remove_vehicle(&lane_queues[current_lane_idx]);
                 // Apply Aging to others
                 for(int i=0; i<NUM_LANES; i++) handle_aging(&lane_queues[i]);
             }
             pthread_mutex_unlock(&queue_mutex);

             if (v_crossing.id != -1) {
                 // Animate Crossing
                 render(current_lane_idx, &v_crossing);
                 usleep(CROSSING_TIME_USEC);
                 log_vehicle(v_crossing); // Add to history
             } else {
                 render(current_lane_idx, NULL);
                 usleep(500000);
                 break; // Empty
             }
             
             // If this was an emergency round, we processed 1 car (or emptied). 
             // Strictly break now to Rotate to next emergency (N->S->W...).
             if (is_emergency_round) break;
             
             render(current_lane_idx, NULL);
             usleep(200000); // brief pause between cars
        }
        
        // Yellow/Red Transition
        render(-1, NULL); 
        usleep(1000000); // 1s all red safety
    }
    
    disable_raw_mode(); 
    printf("\nShutting down...\n");
    kill(generator_pid, SIGTERM);
    wait(NULL);
    
    cleanup_traffic_system();
    cleanup_queue(global_mq);
    destroy_queue();
    
    return 0;
}
