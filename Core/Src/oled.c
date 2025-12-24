#include "oled.h"
#include "main.h"

extern SPI_HandleTypeDef hspi1;
//oled
static inline void CS_LOW(void)  {
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
}
static inline void CS_HIGH(void) {
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
}

void oled_reset(void) {
	CS_HIGH();
	HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(20);
	HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET);
	HAL_Delay(20);
}

void oled_cmd(uint8_t c){
	  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	  //spi transmit 8 bits
	  HAL_SPI_Transmit(&hspi1, &c, 1, HAL_MAX_DELAY);
}

void oled_data(uint8_t d){
	//dc = 1
	HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_SET);
	HAL_SPI_Transmit(&hspi1, &d, 1, HAL_MAX_DELAY);
}

void oled_off(void){
	CS_LOW();
	  oled_cmd(0xAE); //off command for ssd1331
	  CS_HIGH();
}

void oled_on(void){
	CS_LOW();
	  oled_cmd(0xAF); //on command for ssd1331
	  S_HIGH();
}

//0x15 col addr
//0x75 row addr
//[0-95] col
//[0,63] row
void set_window(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1){
	uint8_t col[] = {x0, x1};
	uint8_t row[] = {y0, y1};

	cmd_args(0x15, col, 2);
	cmd_args(0x75, row, 2);
}

void cmd_args(uint8_t c, const uint8_t *args, int n){
	CS_LOW();
	oled_cmd(c);
	for(int i = 0; i < n - 1; i++){
		oled_data(args[i]);
	}
	CS_HIGH();
}

//0x15 col addr
//0x75 row addr
//[0-95] col
//[0,63] row

void fill(){

}
void oled_init(void){
	oled_reset();
	oled_off();
}

void oled_fill(uint16_t color){

}
