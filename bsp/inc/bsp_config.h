
#ifndef BSP_CONFIG_H_
#define BSP_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ENABLE              1
#define DISABLE             0

#define HIGH                1
#define LOW                 0

#define ON                  0
#define OFF                 1

#define DELAY_TIME          5000000

#define PORTA_PIN0          0U
#define PORTA_PIN1          1U
#define PORTA_PIN2          2U
#define PORTA_PIN3          3U
#define PORTA_PIN6          6U
#define PORTA_PIN7          7U
#define PORTA_PIN8          8U
#define PORTA_PIN9          9U
#define PORTA_PIN10         10U
#define PORTA_PIN11         11U
#define PORTA_PIN12         12U

#define PORTB_PIN0          16U
#define PORTB_PIN6          22U
#define PORTB_PIN14         30U
#define PORTB_PIN15         31U

#define PORTC_PIN13         45U

#define LED                 PORTC_PIN13
#define BUTTON              PORTA_PIN3

#define MOTOR_PWM_PIN       PORTA_PIN8
#define MOTOR_IN1           PORTB_PIN14
#define MOTOR_IN2           PORTB_PIN15
#define PWM_FREQ            20000U


#define MOTOR_ENCODER_A     PORTA_PIN6
#define MOTOR_ENCODER_B     PORTA_PIN7

#define SERVO_PWM_PIN       PORTB_PIN6
#define SERVO_FREQ          50U

#define USART1_TX_PIN       PORTA_PIN9
#define USART1_RX_PIN       PORTA_PIN10

#define CAN_BAUDRATE_SUP    500000U
#define CAN_RX_PIN          PORTA_PIN11
#define CAN_TX_PIN          PORTA_PIN12


void BSP_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_CONFIG_H_ */