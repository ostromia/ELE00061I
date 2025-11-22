#include "tinkertech.h"

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



	  /* Initialize all configured peripherals */
	  MX_GPIO_Init();
	  MX_DMA_Init();
	  MX_DAC1_Init();
	  MX_TIM6_Init();
	  MX_ADC1_Init();
	  MX_ADC2_Init();
	  /* USER CODE BEGIN 2 */
	  HAL_TIM_Base_Start(&htim6); // starts timer

	  HAL_DAC_Start_DMA(&hdac1, // dac handle
	                    DAC_CHANNEL_1, // dac channel
	                    (uint32_t*)sine_table, // pointing sinewave table, type conversion (temporary)
	                    64, // No. of samples = change if we use 64 version
	                    DAC_ALIGN_12B_R); // right alignment which should be default

	  HAL_ADC_Start_DMA(&hadc1,	// adc1 handle
			  	  	   (uint32_t*)adc1_buffer, // adc1 buffer table
					   64); // 64 readings

	  HAL_ADC_Start_DMA(&hadc2,	// adc2 handle
			  	  	   (uint32_t*)adc2_buffer,
					   64);
	  /* USER CODE END 2 */

	  /* Infinite loop */
	  /* USER CODE BEGIN WHILE */
	  /* USER CODE BEGIN 3 */
	  const double t = M_PI / 12.0; // tolerance of PI/6
}

void loop() {
	 while (1)
	  {// taking 4 samples per period
		// subtract 2048 to omit the dc property

		    // Vbase = adc1
		    double vbase_a0 = (double)adc1_buffer[0] - 2048.0;
		    double vbase_a1 = (double)adc1_buffer[16] - 2048.0;
		    double vbase_a2 = (double)adc1_buffer[32] - 2048.0;
		    double vbase_a3 = (double)adc1_buffer[48] - 2048.0;

		    // Vout = adc2
		    double vout_a0 = (double)adc2_buffer[0] - 2048.0;
		    double vout_a1 = (double)adc2_buffer[16] - 2048.0;
		    double vout_a2 = (double)adc2_buffer[32] - 2048.0;
		    double vout_a3 = (double)adc2_buffer[48] - 2048.0;

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

		    // keeping phase diff in range (-PI - +PI)
			if (phase_diff > M_PI) {
			  phase_diff -= (2 * M_PI);
			} else if (phase_diff < -M_PI) {
			  phase_diff += (2 * M_PI);
			}


			// Inductor
			if (phase_diff > (M_PI_2 - t) && // P.D. is around +PI/2
				phase_diff < (M_PI_2 + t))
			{
			   //printf("This is an Inductor (L)\r\n");
			}

			// Capacitor
			else if (phase_diff > (-M_PI_2 - t) && // P.D. is around -PI/2
					 phase_diff < (-M_PI_2 + t))
			{
			  //printf("This is a Capacitor (C)\r\n");
			}

			// Resistor
			else if (phase_diff > (M_PI - t) || // PI - t
					 phase_diff < (-M_PI + t))   // - PI + t
			{
			  //printf("This is a Resistor (R)\r\n");
			}

			// Exception
			else
			{
			  //printf("No Component\r\n");
			}

			// print every 500ms
			HAL_Delay(500);



}
