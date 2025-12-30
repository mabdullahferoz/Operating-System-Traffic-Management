#include "utils.h"

// History Log
#define MAX_HISTORY 5
static Vehicle history_log[MAX_HISTORY];
static int history_count = 0;

void log_vehicle(Vehicle v) {
    // Shift right
    for (int i = MAX_HISTORY - 1; i > 0; i--) {
        history_log[i] = history_log[i-1];
    }
    history_log[0] = v;
    if (history_count < MAX_HISTORY) history_count++;
}

// Linked List: Add to tail
void add_vehicle(Node** head, Vehicle v) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->vehicle = v;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
        return;
    }

    Node* temp = *head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = new_node;
}

// Priority Remove: Search for Emergency first, else Head
Vehicle remove_vehicle(Node** head) {
    Vehicle v = { -1, -1, -1, 0, 0 };
    if (*head == NULL) return v;

    // 1. Scan for Emergency
    Node* temp = *head;
    Node* prev = NULL;
    Node* target = NULL;
    Node* target_prev = NULL;

    while (temp != NULL) {
        if (temp->vehicle.type != REGULAR_CAR) {
            target = temp;
            target_prev = prev;
            break; // Found one, take it immediately
        }
        prev = temp;
        temp = temp->next;
    }

    // 2. If no emergency, take head
    if (target == NULL) {
        target = *head;
        target_prev = NULL;
    }

    // 3. Remove Target
    v = target->vehicle;
    
    if (target_prev == NULL) {
        *head = target->next; // Head was removed
    } else {
        target_prev->next = target->next; // Middle removed
    }
    
    free(target);
    return v;
}

// Count nodes
int count_vehicles(Node* head) {
    int count = 0;
    while (head != NULL) {
        count++;
        head = head->next;
    }
    return count;
}

// UI Helpers
void clear_screen() {
    printf("\033[H\033[J");
}

void print_header() {
    // Header is printed inside draw function for better layout control/refresh
}

const char* get_vehicle_type_str(int type) {
    switch (type) {
        case AMBULANCE: return "AMB";
        case POLICE: return "POL";
        case FIRE_TRUCK: return "FIRE";
        default: return "CAR";
    }
}

void draw_traffic_scene(Node* lanes[], int green_lane_idx, Vehicle* crossing_v) {
    // 1. DATA TABLE
    printf(BOLD CYAN "==============================================================================\n");
    printf("                       SMART CITY TRAFFIC CONTROL SYSTEM       \n");
    printf("==============================================================================\n" RESET);

    printf(BOLD CYAN "  SMART TRAFFIC DASHBOARD  \n" RESET);
    printf(" +-------+-------+--------+------------------+------------------------------+\n");
    printf(" | LANE  | STATE | Q-SIZE | STATUS           | VEHICLES (First 5)           |\n");
    printf(" +-------+-------+--------+------------------+------------------------------+\n");
    
    char* dirs[] = {"NORTH", "SOUTH", "EAST ", "WEST "};
    for(int i=0; i<4; i++) {
        char* st = (green_lane_idx == i) ? BOLD GREEN "GO   " RESET : RED "STOP " RESET;
        int sz = count_vehicles(lanes[i]);
        char note[64] = "";
        char v_list[128] = "";
        
        // Scan for Emergency + Build ID String
        Node* t = lanes[i];
        int count = 0;
        int emergency_found = 0;
        
        while(t) {
            if(t->vehicle.type != REGULAR_CAR) emergency_found = 1;
            
            if (count < 5) {
                char tmp[20];
                const char* type_prefix = (t->vehicle.type == REGULAR_CAR) ? "C" : 
                                          (t->vehicle.type == AMBULANCE) ? "A" : "P";
                sprintf(tmp, "%s%d ", type_prefix, t->vehicle.id % 100);
                strcat(v_list, tmp);
            }
            count++;
            t = t->next;
        }
        if (count > 5) strcat(v_list, "...");
        
        if (emergency_found) strcpy(note, BOLD_RED_BLINK "EMERGENCY!      " RESET);
        else strcpy(note, "Normal");
        
        printf(" | %s | %s | %-6d | %-16s | %-28s |\n", dirs[i], st, sz, note, v_list);
    }
    printf(" +-------+-------+--------+------------------+------------------------------+\n");

    // 2. RECENT PROCESS LOG
printf("\n");
printf(BOLD CYAN "   RECENTLY PROCESSED VEHICLES  \n" RESET);
printf(" +------------+----------+-----------------+----------------+\n");
printf(" | VEHICLE ID | LANE     | TYPE            | STATUS         |\n");
printf(" +------------+----------+-----------------+----------------+\n");

// Show current crossing first if exists
if (crossing_v) {
    char type_str[10];
    strcpy(type_str, get_vehicle_type_str(crossing_v->type));
    
    // Manual printing for Crossing to handle ANSI code length
    printf(" | %-10d | %-8s | %-15s | ", crossing_v->id, dirs[crossing_v->lane], type_str);
    printf(BOLD GREEN "CROSSING..." RESET);
    printf("    |\n"); // Fixed padding after "CROSSING..." to hit 16 chars
} else {
    printf(" | %-10s | %-8s | %-15s | %-14s |\n", "-", "-", "-", "IDLE");
}

// Show history
for(int i = 0; i < history_count; i++) {
    Vehicle v = history_log[i];
    char type_str[10];
    strcpy(type_str, get_vehicle_type_str(v.type));
    
    int is_emergency = (v.type != REGULAR_CAR);
    char* color = is_emergency ? BOLD_RED_BLINK : "";
    char* res = is_emergency ? RESET : "";

    // ID Column (Width 10)
    printf(" | %s%d%s", color, v.id, res);
    int len = snprintf(NULL, 0, "%d", v.id);
    for(int k = 0; k < 10 - len; k++) putchar(' ');

    // Lane Column (Width 8)
    printf(" | %-8s", dirs[v.lane]);

    // Type Column (Width 15)
    printf(" | %s%s%s", color, type_str, res);
    len = strlen(type_str);
    for(int k = 0; k < 15 - len; k++) putchar(' ');

    // Status Column (Width 14) - Kept simple for alignment
    printf(" | COMPLETED      |\n");
}
// Fixed the bottom border length to match the top (60 chars total)
printf(" +------------+----------+-----------------+----------------+\n");


    // 3. TRAFFIC VISUALIZATION (Maintained in Terminal)
    char center[64] = "       ";
    if (crossing_v) {
        if (crossing_v->type == REGULAR_CAR) {
            sprintf(center, BOLD YELLOW "[ C%02d ]" RESET, crossing_v->id % 100);
        } else {
            sprintf(center, BOLD_BLUE_BLINK "* %s%02d *" RESET, 
                 (crossing_v->type == AMBULANCE) ? "A" : "P", crossing_v->id % 100);
        }
    }
    
    // Lane Colors
    char* c_n = (green_lane_idx == NORTH) ? BOLD GREEN : RED; 
    char* c_s = (green_lane_idx == SOUTH) ? BOLD GREEN : RED;
    char* c_e = (green_lane_idx == EAST) ? BOLD GREEN : RED;
    char* c_w = (green_lane_idx == WEST) ? BOLD GREEN : RED;
    char* r   = RESET;

printf("\n");
    // North Lane
    printf("                    %s║   ║%s\n", c_n, r);
    printf("                    %s║   ║%s\n", c_n, r);
    printf("                    %s║ | ║%s\n", c_n, r);
    printf("                    %s║ ↓ ║%s\n", c_n, r);
    printf("                    %s║   ║%s\n", c_n, r);
    
    // Top Row (West Border | North Entry | East Border)
    // Made wider (20 chars per side) for a better "Smart City" dashboard feel
    printf("    %s════════════════%s%s╝   ╚%s%s════════════════%s\n", c_w, r, c_n, r, c_e, r);
    
    // Middle Row (West Flow | Center | East Flow)
    printf("    %s   - - - - > %s  %s  %s < - - - -   %s\n", c_w, r, center, c_e, r);
    
    // Bottom Row (West Border | South Entry | East Border)
    printf("    %s════════════════%s%s╗   ╔%s%s════════════════%s\n", c_w, r, c_s, r, c_e, r);
    
    // South Lane
    printf("                    %s║   ║%s\n", c_s, r);
    printf("                    %s║ ↑ ║%s\n", c_s, r);
    printf("                    %s║ | ║%s\n", c_s, r);
    printf("                    %s║   ║%s\n", c_s, r);
    printf("                    %s║   ║%s\n", c_s, r);
    printf("\n");
    
    printf("\nControls: [1-4] Add Car | [a-d] Add Emergency\n");
}
