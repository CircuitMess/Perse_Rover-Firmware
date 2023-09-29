#ifndef BIT_LIBRARY_PINS_HPP
#define BIT_LIBRARY_PINS_HPP

#define I2C_SDA 0
#define I2C_SCL 3

#define MOTOR_FL_A 33
#define MOTOR_FL_B 47
#define MOTOR_FR_A 37
#define MOTOR_FR_B 21
#define MOTOR_BR_A 18
#define MOTOR_BR_B 17
#define MOTOR_BL_A 16
#define MOTOR_BL_B 15

#define SERVO_1_PWM 13
#define SERVO_2_PWM 14
#define SERVO_3_PWM 12

#define CAM_PIN_RESET   -1 //software reset will be performed
#define CAM_PIN_XCLK    48
#define CAM_PIN_SIOD    37
#define CAM_PIN_SIOC    38

#define CAM_PIN_D7      39
#define CAM_PIN_D6      40
#define CAM_PIN_D5      41
#define CAM_PIN_D4      42
#define CAM_PIN_D3      44
#define CAM_PIN_D2      46
#define CAM_PIN_D1      45
#define CAM_PIN_D0      43
#define CAM_PIN_VSYNC   36
#define CAM_PIN_HREF    35
#define CAM_PIN_PCLK    34


//AW9523 pins:
#define EXP_CAM_PWDN 0
#define EXP_SHIFT_DATA 3
#define EXP_SHIFT_CLOCK 4
#define EXP_SHIFT_LATCH 5


//ShiftReg pins:
#define SHIFT_A_ADDR_1 7
#define SHIFT_A_ADDR_2 6
#define SHIFT_A_ADDR_3 5
#define SHIFT_A_ADDR_4 4
#define SHIFT_A_ADDR_5 3
#define SHIFT_A_ADDR_6 2
#define SHIFT_A_DET_1 1
#define SHIFT_A_DET_2 0
#define SHIFT_B_ADDR_1 15
#define SHIFT_B_ADDR_2 14
#define SHIFT_B_ADDR_3 13
#define SHIFT_B_ADDR_4 12
#define SHIFT_B_ADDR_5 11
#define SHIFT_B_ADDR_6 10
#define SHIFT_B_DET_1 9
#define SHIFT_B_DET_2 8


#endif //BIT_LIBRARY_PINS_HPP
