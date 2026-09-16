#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/structs/io_bank0.h>
#include <pico/binary_info.h>
#include <pico/error.h>
#include <pico/stdio.h>
#include <pico/stdio_usb.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/*************************************
 * Hardware configuraion definitions *
 ************************************/

#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) ||             \
    !defined(PICO_DEFAULT_I2C_SCL_PIN)
#error                                                                         \
    "Compilation aborted: my_i2c_example requires default i2c pins to be defined"
#endif

#define MPU_ADDR 0x68    // acquired from bus scan example
#define I2C_PORT i2c0    // the i2c port we're using
#define MPU_6050_ID 0x68 // What's in the WHO_AM_I register of the MPU-6050
#define MPU_6500_ID 0x70 // ... and the MPU-6500
const uint8_t WHO_AM_I_REGISTER = 0x75; // acquired from register map doc

/****************************
 * Component Initialization *
 ***************************/

// TODO: Move function to a seperate file
int i2c_write_blocking_wrapper(const uint8_t *src, size_t len, bool nostop) {

  int ret = i2c_write_blocking(I2C_PORT, MPU_ADDR, src, len, nostop);
  if (ret < 0 || (size_t)ret != len) {
    printf("Something went wrong trying to write to the MPU:\n");
    if (ret == PICO_ERROR_GENERIC) {
      printf("\twrite operation returned PICO_ERROR_GENERIC\n");
    } else {
      printf("\texpected write operation to return %u, got %d\n", len, ret);
    }
  }
  return ret;
}

// TODO: Move function to a seperate file
int i2c_read_blocking_wrapper(uint8_t *dst, size_t len, bool nostop) {

  int ret = i2c_read_blocking(I2C_PORT, MPU_ADDR, dst, len, nostop);
  if (ret < 0 || (size_t)ret != len) {
    printf("Something went wrong trying to read from the MPU:\n");
    if (ret == PICO_ERROR_GENERIC) {
      printf("\tread operation returned PICO_ERROR_GENERIC\n");
    } else {
      printf("\texpected read operation to return %u, got %d\n", len, ret);
    }
  }
  return ret;
}

void init_mpu_i2c_connection(uint sda, uint scl, uint baudrate_kHz) {
  i2c_init(I2C_PORT, baudrate_kHz * 1000);
  gpio_set_function(scl, GPIO_FUNC_I2C);
  gpio_set_function(sda, GPIO_FUNC_I2C);
  gpio_pull_up(scl);
  gpio_pull_up(sda);
}

int init_mpu(void) {
  sleep_ms(500); // give the chip time to do its internal boot-up stuff
  // check connection by reading WHO_AM_I
  uint8_t chip_id[1];
  chip_id[0] = 0;

  // the write_blocking function must be called before a read because we need
  // to tell the chip which register to read from. this sets the chip's
  // register pointer to WHO_AM_I, which will tell the chip what to send in the
  // subsequent read.
  i2c_write_blocking_wrapper(&WHO_AM_I_REGISTER, 1, true);
  // read the register
  i2c_read_blocking_wrapper(chip_id, 1, true);
  if (chip_id[0] == MPU_6050_ID) {
    printf("MPU-6050 detected!\n");
    return 0;
  }
  if (chip_id[0] == MPU_6500_ID) {
    printf("MPU-6500 detected!\n");
    return 0;
  }
  printf("Error: got unexpected value of %d from WHO_AM_I mpu register read "
         "attempt",
         chip_id[0]);
  return 1;
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
  init_mpu_i2c_connection(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN,
                          400);
  int ret = init_mpu();
  if (ret != 0) {
    printf("Successfully read WHO_AM_I\n");
  }

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
