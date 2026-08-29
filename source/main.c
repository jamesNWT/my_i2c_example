#include <pico/stdio.h>
#include <pico/stdio_usb.h>
#include <pico/time.h>
#include <stdio.h>

int main() {
  stdio_init_all();
  while (!stdio_usb_connected()) {
    sleep_ms(10);
  }
  printf("Hello from my i2c example code!\n");
}
