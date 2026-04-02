#include <stdio.h>

/* Hardware Base Addresses */
#define LED_BASE 0xFF200000
#define SW_BASE 0xFF200040
#define KEY_BASE 0xFF200050

/* Global State Variables */
int occupancy_count = 0;
int current_light_state = 0; // 0 = OFF, 1 = ON
int first_sensor = 0;

void update_hardware()
{
    volatile int *led_ptr = (int *)LED_BASE;
    *led_ptr = current_light_state; // Sets LEDR0 based on the toggled state
}

int main(void)
{
    volatile int *sw_ptr = (int *)SW_BASE;
    volatile int *key_ptr = (int *)KEY_BASE;

    int last_sw_state = 0;
    int last_key_state = 0;

    printf("System Active. Initial Light State: OFF\n");

    while (1)
    {
        int current_sw = *sw_ptr;
        int current_key = *key_ptr;

        /* 1. OCCUPANCY LOGIC (Simulated with SW0 and SW1) */

        if ((current_sw & 0x1) && !(last_sw_state & 0x1))
        {
            if (first_sensor == 0)
            {
                first_sensor = 1; // SW0 fired first, wait to see if SW1 follows
                printf("Sensor 0 flagged. Waiting for Sensor 1...\n");
            }
            else if (first_sensor == 2)
            {
                // SW1 had fired first, SW0 followed = EXIT
                if (occupancy_count > 0)
                    occupancy_count--;
                printf("Exit Detected. Occupancy: %d\n", occupancy_count);
                if (occupancy_count == 0)
                {
                    current_light_state = 0;
                    printf("Auto-Off: Room is now empty.\n");
                }
                first_sensor = 0; // reset
            }
        }

        if ((current_sw & 0x2) && !(last_sw_state & 0x2))
        {
            if (first_sensor == 0)
            {
                first_sensor = 2; // SW1 fired first, wait to see if SW0 follows
                printf("Sensor 1 flagged. Waiting for Sensor 0...\n");
            }
            else if (first_sensor == 1)
            {
                // SW0 had fired first, SW1 followed = ENTRY
                occupancy_count++;
                printf("Entry Detected. Occupancy: %d\n", occupancy_count);
                if (current_light_state == 0)
                {
                    current_light_state = 1;
                    printf("Auto-On Triggered by Entry.\n");
                }
                first_sensor = 0; // reset
            }
        }
        last_sw_state = current_sw;

        /* 2. MANUAL OVERRIDE (Simulated Clap with KEY0) */
        // This remains a "Master Toggle" that works anytime
        if ((current_key & 0x1) && !(last_key_state & 0x1))
        {
            current_light_state = !current_light_state;
            printf("Clap Detected! Light manually toggled to: %s\n",
                   current_light_state ? "ON" : "OFF");
        }
        last_key_state = current_key;

        update_hardware();
    }
    return 0;
}