#include "tinkertech.h"


// borrowing handles from main.c
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern DMA_HandleTypeDef hdma_adc2;

extern DAC_HandleTypeDef hdac1;
extern DMA_HandleTypeDef hdma_dac1_ch1;

extern TIM_HandleTypeDef htim6;

extern UART_HandleTypeDef huart2;

// tolerance of PI/6
const double t = M_PI / 6.0;

const double range_limit = 1800.0;

/* sinewave array */
const uint16_t sine_table[64] = {
        2048,2128,2208,2286,2361,2434,2503,2568,
        2627,2681,2729,2770,2805,2832,2851,2863,
        2867,2863,2851,2832,2805,2770,2729,2681,
        2627,2568,2503,2434,2361,2286,2208,2128,
        2048,1968,1888,1810,1735,1662,1593,1528,
        1469,1415,1367,1326,1291,1264,1245,1233,
        1229,1233,1245,1264,1291,1326,1367,1415,
        1469,1528,1593,1662,1735,1810,1888,1968
};

// volatile is for preventing random omitting from compiler

// array of baseline reading
uint16_t adc1_buffer[64];

// array of DUT reading
uint16_t adc2_buffer[64];

// message to console
char msg[100];

	// Which pin are we using for switch?
	#define SWITCH_PORT GPIOA
	#define SWITCH_PIN Switch_Pin


void setup() {
	GPIO RS = { LCD_RS_GPIO_Port, LCD_RS_Pin };
	GPIO E  = { LCD_E_GPIO_Port,  LCD_E_Pin  };
	GPIO D4 = { LCD_D4_GPIO_Port, LCD_D4_Pin };
	GPIO D5 = { LCD_D5_GPIO_Port, LCD_D5_Pin };
	GPIO D6 = { LCD_D6_GPIO_Port, LCD_D6_Pin };
	GPIO D7 = { LCD_D7_GPIO_Port, LCD_D7_Pin };

	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOFEN;

	lcd_init(RS, E, D4, D5, D6, D7);
	lcd_set_cursor(RS, E, D4, D5, D6, D7, 0, 0);
	lcd_write_string(RS, E, D4, D5, D6, D7, "Hello World!");


	// Start timer & DMA


	  HAL_DAC_Start_DMA(&hdac1, // dac handle
	                    DAC_CHANNEL_1, // dac channel
	                    (uint32_t*)sine_table, // pointing sinewave table, type conversion (temporary)
	                    64, // No. of samples = change if we use 64 version
	                    DAC_ALIGN_12B_R); // right alignment which should be default

	  HAL_Delay(100); // 1 sec delay for setup

	  HAL_ADC_Start_DMA(&hadc1,	// adc1 handle
			  	  	   (uint32_t*)adc1_buffer, // adc1 buffer table
					   64); // 64 readings

	  HAL_Delay(100); // 1 sec delay for setup

	  HAL_ADC_Start_DMA(&hadc2,	// adc2 handle
			  	  	   (uint32_t*)adc2_buffer,
					   64);

	  HAL_Delay(100); // 1 sec delay for setup

	  HAL_TIM_Base_Start(&htim6); // starts timer

	  HAL_Delay(1000); // 1 sec delay for setup
}



void loop() {
	 	 // static: variable keeps its value after function call
		static int mode = 0;	// 0 = 4.7kOhm
								// 1 = 107Ohm

	 while (1)
	  {		// For better reading, dc_offset is produced which replacing 2048
		 	HAL_Delay(100);
		 	double dc_offset = (adc1_buffer[0] + adc1_buffer[16] + adc1_buffer[32] + adc1_buffer[48]) / 4.0;
		 	// taking 4 samples per period
		 	// subtract dc_offset to omit the dc property

		    // Vbase = adc1
		    double vbase_a0 = (double)adc1_buffer[0] - dc_offset;
		    double vbase_a1 = (double)adc1_buffer[16] - dc_offset;
		    double vbase_a2 = (double)adc1_buffer[32] - dc_offset;
		    double vbase_a3 = (double)adc1_buffer[48] - dc_offset;

		    // Vout = adc2
		    double vout_a0 = (double)adc2_buffer[0] - dc_offset;
		    double vout_a1 = (double)adc2_buffer[16] - dc_offset;
		    double vout_a2 = (double)adc2_buffer[32] - dc_offset;
		    double vout_a3 = (double)adc2_buffer[48] - dc_offset;

		    // calculating sin property and cos property for each wave
		    double Vbase_sin = vbase_a0 - vbase_a2;
		    double Vbase_cos = vbase_a1 - vbase_a3;

		    double Vout_sin = vout_a0 - vout_a2;
		    double Vout_cos = vout_a1 - vout_a3;

		    // calculating phase of each wave
		    // atan2: Returns the principal value of the arc tangent of y/x, expressed in radians.
		    double phase_Vbase = atan2(Vbase_sin, Vbase_cos); // atan2(sin property, cos property)
		    double phase_Vout = atan2(Vout_sin, Vout_cos); // atan2(sin property, cos property)

		    // final phase diff
		    double phase_diff = phase_Vout - phase_Vbase;

		    // Magnitude (amplitude) of Vout peak to peak
		    double mag_Vbase = sqrt((Vbase_sin*Vbase_sin)+(Vbase_cos*Vbase_cos));
		    double mag_Vout = sqrt((Vout_sin*Vout_sin)+(Vout_cos*Vout_cos));

		    // Reference resistors
		    const double rRef_high = 4700.0;	// when switch is OFF
		    const double rRef_low = 107.0;	// when switch is ON

		    // rRef is not determined yet
		    double rRef = 4700.0;

		    HAL_Delay(100);
		    /*
		    if (mode == 0) {
		    	// if mode is OFF
		    	// turn OFF the switch
		    	HAL_GPIO_WritePin(SWITCH_PORT,SWITCH_PIN, GPIO_PIN_RESET);
		    	// and therefore Reference resistance = 4k7
		    	rRef = rRef_high;
		    } else {
		    	// if mode is ON
		    	// turn ON the switch
		    	HAL_GPIO_WritePin(SWITCH_PORT,SWITCH_PIN, GPIO_PIN_SET);
		    	// so Ref is 107
		    	rRef = rRef_low;
		    }
			*/
		    HAL_Delay(100);

		    // Auto ranging
		    /*
		    if (mode == 0 && mag_Vout > range_limit) { // if signal is too strong
		    	mode = 1;							// Turn ON the switch
		    	HAL_Delay(100);
		    	continue;
		    }

		    if (mode == 1 && mag_Vout < 200.0) {	// if signal is too weak
		    	mode = 0;							// Turn OFF the switch
		    	HAL_Delay(100);
		    	continue;
		    }
		    */

		    HAL_Delay(100); //

		    if (mag_Vout < 10.0) {
			sprintf(msg, "Type: No Component (Open) | Magnitude: %.2f | Attempt:%.0f\r\n", mag_Vout);
			HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
			HAL_Delay(500);
			// continue; // continue without phase difference calculation
		        }

		    double impedance = rRef * (mag_Vbase / mag_Vout);	// Impedance calculation

		    // freq through DUT
		    double freq = 10000.0;

		    //calculating each RLC value for DUT
		    double R = impedance / 10;
		    double L = impedance/(2.0 * M_PI * freq);
		    double C = 1.0/(2.0 * M_PI * freq * impedance);

		    double Cu = C * 1000000.0;
		    double Cn = C * 1000000000.0;


		    // keeping phase difference in range (-PI - +PI)
			if (phase_diff > M_PI) {
			  phase_diff -= (2.0 * M_PI);
			} else if (phase_diff < -M_PI) {
			  phase_diff += (2.0 * M_PI);
			}

			// check circuit functionality


			sprintf(msg, "mag_Vbase: %f | mag_Vout: %f | \r\n", mag_Vbase, mag_Vout);
			HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
			HAL_Delay(500);

			sprintf(msg, "Vbase_sin: %f | Vbase_cos: %f | \r\n", Vbase_sin, Vbase_cos);
			HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
			HAL_Delay(500);

			sprintf(msg, "Vout_sin: %f | Vout_cos: %f | \r\n", Vout_sin, Vout_cos);
			HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
			HAL_Delay(500);


			// Print measurement
			// Inductor
			if (phase_diff > (M_PI_2 - t) && // P.D. is around +PI/2
				phase_diff < (M_PI_2 + t))
			{

				sprintf(msg, "Type: Inductor (L) | Phase:%.2f | %.2fH\r\n", phase_diff, L);
			}

			// Capacitor
			else if (phase_diff > (-M_PI_2 - t) && // P.D. is around -PI/2
					 phase_diff < (-M_PI_2 + t))
			{
				if (Cn > 999.9) // if C > 1uF
					{
					sprintf(msg, "Type: Capacitor (C) | Phase:%.2f | %.2fuF\r\n", phase_diff, Cu);	// printf in uF
					}
				else
					{
					sprintf(msg, "Type: Capacitor (C) | Phase:%.2f | %.2fnF\r\n", phase_diff, Cn);	// printf in nF
					}
			}

			// Resistor
			else if (phase_diff > (M_PI - t) || //p.d.is around 0
					 phase_diff < (-M_PI + t))
			{

				sprintf(msg, "Type: Resistor (R) | Phase:%.2f | %.2fOhm\r\n", phase_diff, R);
			}

			// Exception
			else
			{
				sprintf(msg, "Type: Other Component | Phase: %.2f | Impedance:%.2f\r\n ", phase_diff, impedance);
			}

			HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 200);
			// print every 500ms
			HAL_Delay(500);



	  }

}
