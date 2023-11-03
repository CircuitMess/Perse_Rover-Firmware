#ifndef BIT_LIBRARY_PINS_HPP
#define BIT_LIBRARY_PINS_HPP

#define PIN_BATT 6

#define I2C_SDA 11
#define I2C_SCL 10

#define MOTOR_LEFT_A 13
#define MOTOR_LEFT_B 12
#define MOTOR_RIGHT_A 15
#define MOTOR_RIGHT_B 14

#define SERVO_1_PWM 16
#define SERVO_2_PWM 17
#define SERVO_3_PWM 18

#define CAM_PIN_RESET   -1 //software reset will be performed
#define CAM_PIN_PWDN    36
#define CAM_PIN_XCLK    33
#define CAM_PIN_D7      37
#define CAM_PIN_D6      38
#define CAM_PIN_D5      39
#define CAM_PIN_D4      40
#define CAM_PIN_D3      42
#define CAM_PIN_D2      44
#define CAM_PIN_D1      43
#define CAM_PIN_D0      41
#define CAM_PIN_VSYNC   35
#define CAM_PIN_HREF    34
#define CAM_PIN_PCLK    47

#define I2S_BCLK 1
#define I2S_LRCLK 3
#define I2S_DOUT 0

#define BATTERY_ADC 6

#define A_CTRL_1 9
#define A_CTRL_2 8
#define B_CTRL_1 5
#define B_CTRL_2 4

//AW9523 pins:
#define EXP_LED_CAM 0
#define EXP_LED_REAR 5
#define EXP_BTN_PAIR 6
#define EXP_SPKR_EN 7
#define EXP_LED_MOTOR_L 8
#define EXP_LED_ARM 9
#define EXP_LED_FRONT_L 10
#define EXP_LED_FRONT_R 11
#define EXP_LED_MOTOR_R 12
#define EXP_LED_STATUS_YELLOW 13
#define EXP_LED_STATUS_GREEN 14
#define EXP_LED_STATUS_RED 15
#define EXP_STANDBY_LED 13
#define EXP_GOOD_TO_GO_LED 14
#define EXP_ERROR_LED 15

//TCA9555 pins:
#define TCA_A_ADDR_1 6
#define TCA_A_ADDR_2 5
#define TCA_A_ADDR_3 4
#define TCA_A_ADDR_4 3
#define TCA_A_ADDR_5 2
#define TCA_A_ADDR_6 1
#define TCA_A_DET_1 7
#define TCA_A_DET_2 0
#define TCA_B_ADDR_1 14
#define TCA_B_ADDR_2 13
#define TCA_B_ADDR_3 12
#define TCA_B_ADDR_4 11
#define TCA_B_ADDR_5 10
#define TCA_B_ADDR_6 9
#define TCA_B_DET_1 15
#define TCA_B_DET_2 8


#endif //BIT_LIBRARY_PINS_HPP
