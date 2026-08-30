#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/structs/io_bank0.h>
#include <pico/binary_info.h>
#include <pico/stdio.h>
#include <pico/stdio_usb.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <stdio.h>

/*************************************
 * Hardware configuraion definitions *
 ************************************/

#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) ||             \
    !defined(PICO_DEFAULT_I2C_SCL_PIN)
#error                                                                         \
    "Compilation aborted: my_i2c_example requires default i2c pins to be defined"
#endif

#define MPU_ADDR 0x68          // acquired from bus scan example
#define WHO_AM_I_REGISTER 0x75 // acquired from register map doc

/****************************
 * Component Initialization *
 ***************************/

void init_mpu_i2c(uint sda, uint scl, uint baudrate_kHz) {
  i2c_init(i2c_default, baudrate_kHz * 1000);
  gpio_set_function(scl, GPIO_FUNC_I2C);
  gpio_set_function(sda, GPIO_FUNC_I2C);
  gpio_pull_up(scl);
  gpio_pull_up(sda);
}

/*********************
 * Component Control *
 ********************/

/**************************
 * Other useful functions *
 *************************/
// I2C reserves some addresses for special purposes. We exclude these from the
// scan. These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr) {
  return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

/*****************
 *  MAIN PROGRAM *
 ****************/
int main() {
  // Turn on the default LED as way to show the firmware is running.

  // Initialize Components
  stdio_init_all();

  // Initialize State variables and run precomputations

  // Wait for usb connection before printing to output.
  while (!stdio_usb_connected()) {
    sleep_ms(10);
  }
  printf("USB Connected, starting bus scan.\n");

  // This example will use I2C0 on the default SDA and SCL pins (GP4, GP5 on a
  // Pico) and use a baud rate of 400kHz.
  init_mpu_i2c(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, 400);

  printf("\nI2C Bus Scan\n");
  printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

  for (int addr = 0; addr < (1 << 7); ++addr) {
    if (addr % 16 == 0) {
      printf("%02x ", addr);
    }

    // Perform a 1-byte dummy read from the probe address. If a slave
    // acknowledges this address, the function returns the number of bytes
    // transferred. If the address byte is ignored, the function returns
    // -1.

    // Skip over any reserved addresses.
    int ret;
    uint8_t rxdata;
    if (reserved_addr(addr)) {
      ret = PICO_ERROR_GENERIC;
    } else {
      ret = i2c_read_blocking(i2c_default, addr, &rxdata, 1, false);
    }

    printf(ret < 0 ? "." : "@");
    printf(addr % 16 == 15 ? "\n" : "  ");
  }
  printf("Done.\n");
  stdio_flush();
  sleep_ms(50);
  return 0;
}
