#include <stdio.h>

/* Hardware Base Addresses */
#define LED_BASE      0xFF200000
#define SW_BASE       0xFF200040
#define KEY_BASE      0xFF200050
#define HEX3_0_BASE   0xFF200020 // HEX0, HEX1, HEX2, HEX3
#define HEX5_4_BASE   0xFF200030 // HEX4, HEX5

/* Global State Variables */
int occupancy_count = 0;
int current_light_state = 0; 

/* 7-Segment bit patterns for digits 0-9 (Active High for CPULator/DE10) */
unsigned char hex_digits[] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

void update_hardware() {
    volatile int *led_ptr = (int *)LED_BASE;
    volatile int *hex3_0_ptr = (int *)HEX3_0_BASE;
    volatile int *hex5_4_ptr = (int *)HEX5_4_BASE;

    // Update LEDR0
    *led_ptr = current_light_state; 

    // Handle Wrap-around
    if (occupancy_count > 999999) {
        occupancy_count = 0;
    }

    // Extract digits for 6-digit display
    int temp_count = occupancy_count;
    int d0 = temp_count % 10;
    int d1 = (temp_count / 10) % 10;
    int d2 = (temp_count / 100) % 10;
    int d3 = (temp_count / 1000) % 10;
    int d4 = (temp_count / 10000) % 10;
    int d5 = (temp_count / 100000) % 10;

    // Pack HEX0-HEX3 (Each digit is 8 bits)
    *hex3_0_ptr = (hex_digits[d3] << 24) | (hex_digits[d2] << 16) | 
                  (hex_digits[d1] << 8)  | (hex_digits[d0]);

    // Pack HEX4-HEX5
    *hex5_4_ptr = (hex_digits[d5] << 8) | (hex_digits[d4]);
}

int main(void) {
    volatile int *sw_ptr  = (int *)SW_BASE;
    volatile int *key_ptr = (int *)KEY_BASE;
    
    int last_sw_state = 0;
    int last_key_state = 0;
    int sensor_triggered = -1; // -1: None, 0: Sensor1 (SW1), 1: Sensor2 (SW0)

    printf("System Active. Occupancy: 0, Light: OFF\n");

    while (1) {
        int current_sw = *sw_ptr;
        int current_key = *key_ptr;

        /* 1. SEQUENTIAL OCCUPANCY LOGIC*/
        // Check for Sensor 1 (SW1) rising edge
        if ((current_sw & 0x2) && !(last_sw_state & 0x2)) {
            if (sensor_triggered == 1) { // Sequence: SW0 then SW1 = ENTRY
                occupancy_count++;
                if (current_light_state == 0) current_light_state = 1;
                printf("Entry Detected. Total: %d\n", occupancy_count);
                sensor_triggered = -1;
            } else {
                sensor_triggered = 0; 
            }
        }
        
        // Check for Sensor 2 (SW0) rising edge
        if ((current_sw & 0x1) && !(last_sw_state & 0x1)) {
            if (sensor_triggered == 0) { // Sequence: SW1 then SW0 = EXIT
                if (occupancy_count > 0) occupancy_count--;
                if (occupancy_count == 0) current_light_state = 0;
                printf("Exit Detected. Total: %d\n", occupancy_count);
                sensor_triggered = -1;
            } else {
                sensor_triggered = 1;
            }
        }

        // Reset sequence if all switches are turned off
        if (current_sw == 0) sensor_triggered = -1;

        last_sw_state = current_sw;

        /* 2. MANUAL OVERRIDE (Simulated Clap) */
        if ((current_key & 0x1) && !(last_key_state & 0x1) && occupancy_count > 0) {
            current_light_state = !current_light_state; 
            printf("Manual Toggle: Light %s\n", current_light_state ? "ON" : "OFF");
        }
        last_key_state = current_key;

        update_hardware();
    }
    return 0;
}