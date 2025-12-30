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

static void format_queue(Node* head, char* buffer, int reverse) {
    Node* temp = head;
    char items[10][32];
    int total = 0;
    while(temp && total < 10) {
        const char* type_code = (temp->vehicle.type == REGULAR_CAR) ? "C" : 
                                (temp->vehicle.type == AMBULANCE) ? "A" : "P";
        const char* color = (temp->vehicle.type == REGULAR_CAR) ? "" : BOLD_RED_BLINK;
        const char* reset = (temp->vehicle.type == REGULAR_CAR) ? "" : RESET;
        sprintf(items[total], "[%s%s%d%s]", color, type_code, temp->vehicle.id % 100, reset);
        
        total++;
        temp = temp->next;
    }

    strcpy(buffer, "");
    if (reverse) {
        for(int i=0; i<total; i++) { strcat(buffer, items[i]); }
    } else {
         for(int i=total-1; i>=0; i--) { strcat(buffer, items[i]); }       
    }
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
    char n_q[256] = "", s_q[256] = "", e_q[256] = "", w_q[256] = "";
    
    format_queue(lanes[NORTH], n_q, 0); 
    format_queue(lanes[SOUTH], s_q, 1); 
    format_queue(lanes[EAST], e_q, 1);  
    format_queue(lanes[WEST], w_q, 0);  

    // Colors
    char* n_c = (green_lane_idx == NORTH) ? BOLD GREEN : RED;
    char* s_c = (green_lane_idx == SOUTH) ? BOLD GREEN : RED;
    char* e_c = (green_lane_idx == EAST) ? BOLD GREEN : RED;
    char* w_c = (green_lane_idx == WEST) ? BOLD GREEN : RED;

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
        
        if (emergency_found) strcpy(note, BOLD_RED_BLINK "EMERGENCY!" RESET);
        else strcpy(note, "Normal");
        
        printf(" | %s | %s | %-6d | %-16s | %-28s |\n", dirs[i], st, sz, note, v_list);
    }
    printf(" +-------+-------+--------+------------------+------------------------------+\n");

    // 2. RECENT PROCESS LOG
    printf("\n");
    printf(BOLD CYAN "  RECENTLY PROCESSED VEHICLES  \n" RESET);
    printf(" +------------+----------+-----------------+----------+\n");
    printf(" | VEHICLE ID | LANE     | TYPE            | STATUS   |\n");
    printf(" +------------+----------+-----------------+----------+\n");
    
    // Show current crossing first if exists
    if (crossing_v) {
        char type_str[10];
        strcpy(type_str, get_vehicle_type_str(crossing_v->type));
        char status[32] = BOLD GREEN "CROSSING..." RESET;
        printf(" | %-10d | %-8s | %-15s | %-8s |\n", crossing_v->id, dirs[crossing_v->lane], type_str, status);
    } else {
        printf(" | %-10s | %-8s | %-15s | %-8s |\n", "-", "-", "-", "IDLE");
    }
    
    // Show history
    for(int i=0; i<history_count; i++) {
        Vehicle v = history_log[i];
        char type_str[10];
        strcpy(type_str, get_vehicle_type_str(v.type));
        
        int is_emergency = (v.type != REGULAR_CAR);
        char* color = is_emergency ? BOLD_RED_BLINK : "";
        char* reset = is_emergency ? RESET : "";

        // Manual printing to preserve alignment despite ANSI codes
        // ID Column (Width 10)
        printf(" | %s%d%s", color, v.id, reset);
        int len = snprintf(NULL, 0, "%d", v.id);
        for(int k=0; k<10-len; k++) putchar(' ');

        // Lane Column (Width 8) - No color needed usually, but let's keep it simple
        printf(" | %-8s", dirs[v.lane]);

        // Type Column (Width 15)
        printf(" | %s%s%s", color, type_str, reset);
        len = strlen(type_str);
        for(int k=0; k<15-len; k++) putchar(' ');

        // Status
        printf(" | COMPLETED|\n");
    }
     printf(" +------------+----------+-----------------+----------+\n");


    // 3. ASCII INTERSECTION
    char center[64] = "       ";
    if (crossing_v) {
        sprintf(center, BOLD YELLOW "[%s%02d]" RESET, 
            (crossing_v->type == REGULAR_CAR) ? "C" : "E", crossing_v->id % 100);
    }
    
    printf("\n         (NORTH)\n");
    printf("            %s|   |%s\n", n_c, n_c);
    printf("      %6s%s|   |%s\n", (green_lane_idx==NORTH)?"v ":"", n_c, n_c);
    printf("            %s| | |%s\n", n_c, n_c);
    printf("            %s| | |%s\n", n_q, n_c);
    printf("  __________|_| |_|__________\n");
    printf(" /            %s            \\\n", center);
    printf("%s %s   - - - - - - - - -   %s %s\n", w_q, w_c, e_c, e_q);
    printf(" \\___________________________/\n");
    printf("            %s| | |%s\n", s_c, s_q);
    printf("            %s| | |%s\n", s_c, s_c);
    printf("      %6s%s|   |%s\n", (green_lane_idx==SOUTH)?"^ ":"", s_c, s_c);
    printf("         (SOUTH)\n");
    
    printf("\nControls: [1-4] Add Car | [a-d] Add Emergency\n");
}
