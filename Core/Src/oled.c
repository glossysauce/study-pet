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

static void oled_cmd(uint8_t c){
	  HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_RESET);
	  //spi transmit 8 bits
	  HAL_SPI_Transmit(&hspi1, &c, 1, 100);
}

static void oled_data(uint8_t d){
	//dc = 1
	HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_SET);
	HAL_SPI_Transmit(&hspi1, &d, 1, 100);
}

void oled_off(void){
	CS_LOW();
	  oled_cmd(0xAE); //off command for ssd1331
	  CS_HIGH();
}

void oled_on(void){
	CS_LOW();
	  oled_cmd(0xAF); //on command for ssd1331
	  CS_HIGH();
}

static void cmd_args(uint8_t c, const uint8_t *args, int n){
//	CS_LOW();
	oled_cmd(c);
	for(int i = 0; i < n; i++){
		oled_data(args[i]);
	}
//	CS_HIGH();
}

//0x15 col addr
//0x75 row addr
//[0-95] col
//[0,63] row
static void set_window(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1){
	  CS_LOW();
	  oled_cmd(0x15);
	  oled_data(x0);
	  oled_data(x1);

	  oled_cmd(0x75);
	  oled_data(y0);
	  oled_data(y1);
	  CS_HIGH();
}


//0x15 col addr
//0x75 row addr
//[0-95] col
//[0,63] row

void oled_init(void) {

    oled_reset();

    CS_LOW();
    oled_cmd(0xAE);// display off

    oled_cmd(0xFD); oled_data(0x12); // unlock

    oled_cmd(0x15); oled_data(0x00); oled_data(0x5F);
    oled_cmd(0x75); oled_data(0x00); oled_data(0x3F);

    oled_cmd(0x81); oled_data(0x80);
    oled_cmd(0x82); oled_data(0x60);
    oled_cmd(0x83); oled_data(0x80);

    oled_cmd(0x87); oled_data(0x08);

    oled_cmd(0x8A); oled_data(0x80);
    oled_cmd(0x8B); oled_data(0x60);
    oled_cmd(0x8C); oled_data(0x80);

    oled_cmd(0xA1); oled_data(0x00);
    oled_cmd(0xA2); oled_data(0x00);

    oled_cmd(0xA4);

    oled_cmd(0xA8); oled_data(0x3F);

    oled_cmd(0xAB); oled_data(0x8E);
    oled_cmd(0xB0); oled_data(0x0B);
    oled_cmd(0xB1); oled_data(0x74);
    oled_cmd(0xB3); oled_data(0xD0);

    oled_cmd(0xB9);

    oled_cmd(0xBB); oled_data(0x3E);
    oled_cmd(0xBE); oled_data(0x3E);

    oled_cmd(0xA0); oled_data(0x72); // remap 16-bit

    oled_cmd(0xAF);// display on
    CS_HIGH();

    HAL_Delay(100);
}

void oled_fill(uint16_t color){


    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    uint8_t px[2] = {hi, lo};

    CS_LOW();

    oled_cmd(0x15); oled_data(0); oled_data(95);
    oled_cmd(0x75); oled_data(0); oled_data(63);

    oled_cmd(0x5C);
    HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_SET);

    for(int i = 0; i < 96 * 64; i++){
        HAL_SPI_Transmit(&hspi1, px, 2, 100);
    }

    CS_HIGH();

}

void oled_write_data(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint16_t *img){
	oled_set_window(x, x+w-1, y, y+h-1);
	  CS_LOW();
	  oled_cmd(0x5C);
	  HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_SET);


}
void oled_clear_window(void) {
    CS_LOW();
    oled_cmd(0x25);
    oled_data(0);   // x0
    oled_data(0);   // y0
    oled_data(95);  // x1
    oled_data(63);  // y1
    CS_HIGH();
}

//A = Blue
//B = Green
//C = Red
//TEST
void oled_rect_hw_red(void){
  CS_LOW();
  oled_cmd(0x26); oled_data(0x01);         // fill enable:contentReference[oaicite:4]{index=4}
  oled_cmd(0x22);                          // draw rectangle:contentReference[oaicite:5]{index=5}
  oled_data(0); oled_data(0);              // x0,y0
  oled_data(95); oled_data(63);            // x1,y1
  // outline C,B,A then fill C,B,A (datasheet calls them colors A/B/C)
  // just send same 16-bit value 3 times as you were doing (works on many modules)
//  oled_data(0xF8); oled_data(0x00);  // red
//  oled_data(0xF8); oled_data(0x00);
//  oled_data(0xF8); oled_data(0x00);
//  oled_data(0xF8); oled_data(0x00);
//  oled_data(0xF8); oled_data(0x00);
//  oled_data(0xF8); oled_data(0x00);
  // Outline color C,B,A  (set to red)
  oled_data(0xFF); // C
  oled_data(0x00); // B
  oled_data(0x00); // A

  // fill color C,B,A (set to red)
  oled_data(0xFF); // C
  oled_data(0x00); // B
  oled_data(0x00); // A
  CS_HIGH();
}

void oled_clear_hw(void){
  uint8_t a[4] = {0,0,95,63};
  CS_LOW();
  oled_cmd(0x25);
  for(int i=0;i<4;i++) oled_data(a[i]);
  CS_HIGH();
}

void oled_draw_pixel(uint8_t x, uint8_t y, uint16_t c){
  CS_LOW();
  oled_cmd(0x15); oled_data(x); oled_data(x);
  oled_cmd(0x75); oled_data(y); oled_data(y);
  oled_cmd(0x5C);
  HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_SET);
  uint8_t px[2] = { (uint8_t)(c>>8), (uint8_t)c };
  HAL_SPI_Transmit(&hspi1, px, 2, HAL_MAX_DELAY);
  CS_HIGH();
}

void oled_draw_block(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color)
{
    uint8_t hi = (uint8_t)(color >> 8);
    uint8_t lo = (uint8_t)(color & 0xFF);

    CS_LOW();

    // Set window (CS stays low)
    oled_cmd(0x15); oled_data(x);         oled_data(x + w - 1);
    oled_cmd(0x75); oled_data(y);         oled_data(y + h - 1);

    // Write RAM
    oled_cmd(0x5C);
    HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_SET);

    // Stream pixels
    uint8_t px[2] = {hi, lo};
    for (int i = 0; i < w * h; i++) {
        HAL_SPI_Transmit(&hspi1, px, 2, HAL_MAX_DELAY);
    }

    CS_HIGH();
}

void oled_block_rgb565(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color)
{
  uint8_t hi = (uint8_t)(color >> 8);
  uint8_t lo = (uint8_t)(color & 0xFF);

  CS_LOW();

  oled_cmd(0x15); oled_data(x);       oled_data(x + w - 1);
  oled_cmd(0x75); oled_data(y);       oled_data(y + h - 1);

  oled_cmd(0x5C);                     // write RAM
  HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_SET);

  uint8_t buf[256];
  for (int i = 0; i < 256; i += 2) { buf[i] = hi; buf[i+1] = lo; }

  int bytes = w * h * 2;
  while (bytes > 0) {
    int n = (bytes > 256) ? 256 : bytes;
    HAL_SPI_Transmit(&hspi1, buf, n, HAL_MAX_DELAY);
    bytes -= n;
  }

  CS_HIGH();
}

//working
void test(void){
	  CS_LOW(); oled_cmd(0xA5); CS_HIGH(); // all ON
	  HAL_Delay(800);
	  CS_LOW(); oled_cmd(0xA6); CS_HIGH(); // all OFF
	  HAL_Delay(800);
	  CS_LOW(); oled_cmd(0xA4); CS_HIGH(); // normal
	  HAL_Delay(200);
}

static inline void gac_color_rgb(uint8_t r,uint8_t g,uint8_t b){
  // your mapping: C=R, B=G, A=B (you measured it)
  oled_data(r); // C
  oled_data(g); // B
  oled_data(b); // A
}

void oled_gac_fill_rect(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1, uint8_t r,uint8_t g,uint8_t b){
  CS_LOW();
  oled_cmd(0x26); oled_data(0x01);  // fill enable
  oled_cmd(0x22);
  oled_data(x0); oled_data(y0);
  oled_data(x1); oled_data(y1);
  gac_color_rgb(r,g,b); // outline
  gac_color_rgb(r,g,b); // fill
  CS_HIGH();
}
