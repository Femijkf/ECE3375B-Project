#include <stdio.h>

/* Hardware Base Addresses */
#define LED_BASE        0xFF200000
#define SW_BASE         0xFF200040
#define KEY_BASE        0xFF200050

/* Global State Variables */
int occupancy_count = 0;
int current_light_state = 0; // 0 = OFF, 1 = ON

void update_hardware() {
    volatile int *led_ptr = (int *)LED_BASE;
    *led_ptr = current_light_state; // Sets LEDR0 based on the toggled state
}

int main(void) {
    volatile int *sw_ptr  = (int *)SW_BASE;
    volatile int *key_ptr = (int *)KEY_BASE;
    
    int last_sw_state = 0;
    int last_key_state = 0;

    printf("System Active. Initial Light State: OFF\n");

    while (1) {
        int current_sw = *sw_ptr;
        int current_key = *key_ptr;

        /* 1. OCCUPANCY LOGIC (Simulated with SW0 and SW1) */
        // Entry Trigger (Sensor 2 -> Sensor 1)
        if ((current_sw & 0x1) && !(last_sw_state & 0x1)) {
            occupancy_count++;
            printf("Entry Detected. Occupancy: %d\n", occupancy_count);
            
            // AUTO-ON FEATURE: If lights were off, turn them on because someone entered 
            if (current_light_state == 0) {
                current_light_state = 1;
                printf("Auto-On Triggered by Entry.\n");
            }
        }
        
        // Exit Trigger (Sensor 1 -> Sensor 2)
        if ((current_sw & 0x2) && !(last_sw_state & 0x2)) {
            if (occupancy_count > 0) occupancy_count--;
            printf("Exit Detected. Occupancy: %d\n", occupancy_count);
            
            // AUTO-OFF FEATURE: If room becomes empty, turn off the lights
            if (occupancy_count == 0) {
                current_light_state = 0;
                printf("Auto-Off: Room is now empty.\n");
            }
        }
        last_sw_state = current_sw;

        /* 2. MANUAL OVERRIDE (Simulated Clap with KEY0) */
        // This remains a "Master Toggle" that works anytime
        if ((current_key & 0x1) && !(last_key_state & 0x1)) {
            current_light_state = !current_light_state; 
            printf("Clap Detected! Light manually toggled to: %s\n", 
                   current_light_state ? "ON" : "OFF");
        }
        last_key_state = current_key;

        update_hardware();
    }
    return 0;
}