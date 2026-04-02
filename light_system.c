#include <stdio.h>

/* Hardware Base Addresses */
#define LED_BASE      0xFF200000
#define SW_BASE       0xFF200040
#define KEY_BASE      0xFF200050
#define HEX3_0_BASE   0xFF200020 // 7-Segment Display HEX3 to HEX0

/* Global State Variables */
int occupancy_count = 0;
int current_light_state = 0; 

/* 7-Segment bit patterns for digits 0-9 (Active Low) */
unsigned char hex_digits[] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

void update_hardware() {
    volatile int *led_ptr = (int *)LED_BASE;
    volatile int *hex_ptr = (int *)HEX3_0_BASE;

    // Update LEDR0
    *led_ptr = current_light_state; 

    // Update HEX0 Display with occupancy count (caps at 9 for single digit)
    int display_val = (occupancy_count > 9) ? 9 : occupancy_count;
    *hex_ptr = hex_digits[display_val];
}

int main(void) {
    volatile int *sw_ptr  = (int *)SW_BASE;
    volatile int *key_ptr = (int *)KEY_BASE;
    
    int last_sw_state = 0;
    int last_key_state = 0;
    int sensor_triggered = -1; // -1: None, 0: Sensor1 (SW0), 1: Sensor2 (SW1)

    printf("System Active. Occupancy: 0, Light: OFF\n");

    while (1) {
        int current_sw = *sw_ptr;
        int current_key = *key_ptr;

        /* 1. SEQUENTIAL OCCUPANCY LOGIC (SW0 and SW1) */
        // Check for Sensor 1 (SW0) rising edge
        if ((current_sw & 0x2) && !(last_sw_state & 0x2)) {
            if (sensor_triggered == 1) { // Sensor 2 was already hit: ENTRY
                occupancy_count++;
                if (current_light_state == 0) current_light_state = 1;
                printf("Entry Detected. Total: %d\n", occupancy_count);
                sensor_triggered = -1; // Reset
            } else {
                sensor_triggered = 0; 
            }
        }
        
        // Check for Sensor 2 (SW1) rising edge
        if ((current_sw & 0x1) && !(last_sw_state & 0x1)) {
            if (sensor_triggered == 0) { // Sensor 1 was already hit: EXIT
                if (occupancy_count > 0) occupancy_count--;
                if (occupancy_count == 0) current_light_state = 0;
                printf("Exit Detected. Total: %d\n", occupancy_count);
                sensor_triggered = -1; // Reset
            } else {
                sensor_triggered = 1;
            }
        }

        // Reset sequence if all switches are turned off
        if (current_sw == 0) sensor_triggered = -1;

        last_sw_state = current_sw;

        /* 2. MANUAL OVERRIDE (KEY0) */
        if ((current_key & 0x1) && !(last_key_state & 0x1) && occupancy_count > 0) {
            current_light_state = !current_light_state; 
            printf("Manual Toggle: Light %s\n", current_light_state ? "ON" : "OFF");
        }
        last_key_state = current_key;

        update_hardware();
    }
    return 0;
}