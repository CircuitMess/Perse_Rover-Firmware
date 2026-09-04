#ifndef BIT_LIBRARY_PINS_HPP
#define BIT_LIBRARY_PINS_HPP

/**
 * Pinout for NASA Curiosity rover main board v0.4.
 *
 * Differences against the Perseverance v0.9 board this firmware originally targeted:
 *  - no battery ADC (see Devices/Battery.h), no pairing button (pairing starts automatically)
 *  - a latching power supply: PIN_PWR_HOLD must be driven high to stay powered, PIN_BTN reads the power button
 *  - a single module (UMAX) port on the left side of the rover, on its own I2C bus
 *  - USB is behind a CH340C on UART0, so IO43/IO44 are the console and native USB is unused
 */

#define I2C_SDA 11
#define I2C_SCL 10

// Dedicated I2C bus of the module (UMAX) port.
#define I2C_UMAX_SDA 7
#define I2C_UMAX_SCL 8

// Power latch. Held high for as long as the rover should stay on, released to power off.
#define PIN_PWR_HOLD 45
// Power button sense. Reads high while the button is pressed.
#define PIN_BTN 46

#define MOTOR_LEFT_A 14
#define MOTOR_LEFT_B 15
#define MOTOR_RIGHT_A 12
#define MOTOR_RIGHT_B 13

#define SERVO_1_PWM 18
#define SERVO_2_PWM 17
#define SERVO_3_PWM 16

#define CAM_PIN_RESET   -1 //software reset will be performed
#define CAM_PIN_PWDN    35
#define CAM_PIN_XCLK    33
#define CAM_PIN_D7      42
#define CAM_PIN_D6      41
#define CAM_PIN_D5      40
#define CAM_PIN_D4      39
#define CAM_PIN_D3      37
#define CAM_PIN_D2      20
#define CAM_PIN_D1      21
#define CAM_PIN_D0      38
#define CAM_PIN_VSYNC   36
#define CAM_PIN_HREF    34
#define CAM_PIN_PCLK    47

#define I2S_BCLK 9
#define I2S_LRCLK 3
#define I2S_DOUT 1

// Module port GPIOs. IO_3/IO_4/IO_5 of the connector hang off the XL9555 (TCA_CTRL_3..5).
#define MODULE_CTRL_1 4 // connector IO_1
#define MODULE_CTRL_2 2 // connector IO_2
#define MODULE_CTRL_6 5 // connector IO_6, input only, ADC capable

//AW9523 pins:
#define EXP_LED_STATUS_YELLOW 0  // STANDBY_LED
#define EXP_LED_FRONT_R 3        // HEADLIGHT4
#define EXP_LED_CAM 4            // CAMERA TOWER LED
#define EXP_LED_FRONT_L 5        // HEADLIGHT2
#define EXP_LED_DECO_2 6
#define EXP_LED_DECO_3 7
#define EXP_LED_ARM 8
#define EXP_LED_MOTOR_R 9        // MOTOR_LED1
#define EXP_LED_STATUS_RED 10    // ERROR_LED
#define EXP_LED_STATUS_GREEN 11  // GoodToGo_LED
#define EXP_LED_MOTOR_L 12       // MOTOR_LED2
#define EXP_LED_DECO_1 13
#define EXP_SPKR_EN 14
#define EXP_LED_REAR 15          // STATUS_LEDS

//XL9555 (TCA9555) pins:
#define TCA_CHARGE 0     // TP4056 CHRG, active low
#define TCA_STANDBY 1    // TP4056 STDBY, active low
#define TCA_CALIB_EN 2   // enables the 2.5V TL431 reference on the battery divider
#define TCA_BATT_READ 3  // battery divider tap - digital input only, see Devices/Battery.h
#define TCA_USB_DETECT 4 // high while USB is plugged in
#define TCA_CTRL_3 5     // module connector IO_3
#define TCA_CTRL_4 6     // module connector IO_4
#define TCA_CTRL_5 7     // module connector IO_5
#define TCA_ADDR_1 14
#define TCA_ADDR_2 13
#define TCA_ADDR_3 12
#define TCA_ADDR_4 11
#define TCA_ADDR_5 10
#define TCA_ADDR_6 9
#define TCA_DET_1 15
#define TCA_DET_2 8


#endif //BIT_LIBRARY_PINS_HPP
