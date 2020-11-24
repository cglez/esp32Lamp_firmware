#define GPIO_INPUT_INT_1 (23)
#define GPIO_INPUT_INT_2 (22)

#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_INT_1) | (1ULL<<GPIO_INPUT_INT_2) | (1ULL<<GPIO_INPUT_INT_BUTTON))


#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define WRITE_BIT I2C_MASTER_WRITE  /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ    /*!< I2C master read */
#define ACK_CHECK_EN 0x1            /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0           /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                 /*!< I2C ack value */
#define NACK_VAL 0x1                /*!< I2C nack value */
#define ACK 0x0                     /*!< I2C ack value */
#define NACK 0x1                    /*!< I2C nack value */


static esp_err_t  i2c_gpio_sda  = 21;
static esp_err_t  i2c_gpio_scl  = 19;
static uint32_t   i2c_frequency = 100000;
static i2c_port_t i2c_port      = I2C_NUM_0;

// registros del acelerómetro


static uint8_t    dev_address   = 0x53;
static uint8_t    power_ctl_reg = 0x2D;
static uint8_t    who_i_am_reg  = 0x00;
static uint8_t    DATAX0        = 0x32;
// static uint8_t    DATAX1        = 0x33;
// static uint8_t    DATAY0        = 0x34;
// static uint8_t    DATAY1        = 0x35;
// static uint8_t    DATAZ0        = 0x36;
// static uint8_t    DATAZ1        = 0x37;

typedef struct{
    int16_t  FX;
    int16_t  FY;
    int16_t  FZ;
} fuerza;


typedef struct{
    uint8_t  FX0;
    uint8_t  FX1;
    uint8_t  FY0;
    uint8_t  FY1;
    uint8_t  FZ0;
    uint8_t  FZ1;
} force;

fuerza acc;
force acc2; 

uint8_t state_machine_position = 0;

/*
*    FUNCIONES DE INICIALIZACIÓN I2C
*/ 

static esp_err_t i2c_master_driver_initialize(void)
{
    i2c_config_t conf_i2c = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = i2c_gpio_sda,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_io_num = i2c_gpio_scl,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = i2c_frequency
    };
    return i2c_param_config(i2c_port, &conf_i2c);
}

/*
*   FIN DE FUNCIONES DE INICIALIZACIÓN I2C
*/ 

static void accelerometer_interrupt(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
            if(io_num ==0 && gpio_get_level(io_num)==1){

                state_machine_position +=1;
                if (state_machine_position == 6){
                    state_machine_position = 0;
                }

switch(state_machine_position){
int med;
                case 0:
                    // Demo 1
                    luminosity_bk.R  = luminosity.R;
                    luminosity_bk.G  = luminosity.G;
                    luminosity_bk.B  = luminosity.B;
                    luminosity_bk.T = (luminosity.R + luminosity.G + luminosity.B)/3;
                    luminosity.T = luminosity_bk.T;
                    vTaskResume( xDemo03Handle );
                    printf("Demo colors");
                    break;
                case 1:
                    //vTaskSuspend( xDemo01Handle );
                    vTaskSuspend( xDemo03Handle );
                    
                    // Whitew colors
                    // All PWM leds to luminosity value

                    // med = (luminosity.R + luminosity.G + luminosity.B)/3;
                    luminosity.R = luminosity_bk.T;
                    luminosity.G = luminosity_bk.T;
                    luminosity.B = luminosity_bk.T;
                    set_led_RGB();
                    printf("White colors");
                    break;
                case 2:
                    // Red color

                    luminosity_bk.R  = luminosity.R;
                    luminosity_bk.G  = luminosity.G;
                    luminosity_bk.B  = luminosity.B;
                    luminosity_bk.T = (luminosity.R + luminosity.G + luminosity.B)/3;
                    luminosity.G = 0;
                    luminosity.B = 0;
                    set_led_RGB();
                    printf("RED color");
                    break;
                case 3:
                    // Green color
                    luminosity_bk.R = luminosity.R;
                    luminosity.R = 0;
                    luminosity.G = luminosity_bk.G;
                    luminosity.B = 0;
                    set_led_RGB();
                    printf("Green color");
                    break;
                case 4:
                    // Blue color
                    luminosity_bk.G = luminosity.G;
                    luminosity.R = 0;
                    luminosity.G = 0;
                    luminosity.B = luminosity_bk.B;
                    set_led_RGB();
                    printf("Blue color");
                    break;
                case 5:
                    // RGB color
                    luminosity_bk.B = luminosity.B;
                    luminosity.R = luminosity_bk.R;
                    luminosity.G = luminosity_bk.G;
                    luminosity.B = luminosity_bk.B;
                                       
                    set_led_RGB();
                    printf("Blue color");
                    break;
            default:
                    // default statements
                    printf("fak");
                    //break;
            
            }  // end switch case
        


            printf("State machine position == %d \n", state_machine_position);
            } // end if
        }  // end if
    vTaskDelay(100 / portTICK_PERIOD_MS );
    } // end infinite bucle
} // end task function


void write_ADXL343(uint8_t address, uint8_t register1, uint8_t data){
   // printf("read function");
   
           /*
	 * COMUNICACION I2C
	 *   SDA-> GPIO21
	 *   SCL-> GPIO19
	 */


    // uint8_t DATAX1 = 0x33;
    int ret;
    

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	//vTaskDelay(5 / portTICK_RATE_MS);
    i2c_master_write_byte(cmd, (address<<1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, register1, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    
	i2c_master_cmd_begin(i2c_port, cmd, 100 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	

	/*
	 * FIN DE COMUNICACIÓN I2C
	 */

}


uint8_t read_ADXL343(uint8_t address, uint8_t register1, uint8_t len){
   // printf("read function");
   
           /*
	 * COMUNICACION I2C
	 *   SDA-> GPIO21
	 *   SCL-> GPIO19
	 */

    int ret;
    uint8_t data[len];

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	//vTaskDelay(5 / portTICK_RATE_MS);
    i2c_master_write_byte(cmd, (address<<1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, register1, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    
	i2c_master_cmd_begin(i2c_port, cmd, 100 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
 	i2c_master_write_byte(cmd, (address<<1) | READ_BIT, ACK_CHECK_EN);

    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data + len - 1, NACK_VAL);
    
    
 
    //vTaskDelay(5 / portTICK_RATE_MS);
    i2c_master_stop(cmd);

	ret = i2c_master_cmd_begin(i2c_port, cmd, 100 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
	
	// Se espera 0xE5
	
	// printf("write_add = 0x%02x \n", (dev_address<<1) | WRITE_BIT);
	// printf("read_add = 0x%02x \n", (dev_address<<1) | READ_BIT);
	if(ret == ESP_ERR_TIMEOUT){
	    printf("BUS IS BUSY \n");
	    }
	// printf("ret: %i \n", ret);
	
	for (int i = 0; i < len; i++) {
        printf("0x%02x ;", data[i]);

        printf("int: %i \n", data[i]);
        if ((i + 1) % 6 == 0) {
            printf("\r\n");
        }  //endif
    }  // endfor
    
    if (len==6){

        acc2.FX0 = data[0];
        acc2.FX1 = data[1];
        acc2.FY0 = data[2];
        acc2.FY1 = data[3];
        acc2.FZ0 = data[4];
        acc2.FZ1 = data[5];

        acc.FX = data[1];
        acc.FX <<=8;
        acc.FX |= data[0]; 
        acc.FY = data[3];
        acc.FY <<=8;
        acc.FY |= data[2];
        acc.FZ = data[5];
        acc.FZ <<=8;
        acc.FZ |= data[4];
    }
    //    printf("0x%02x ", dato);
    //    printf("int: %i ", dato);
	
	// free(data);
	/*
	 * FIN DE COMUNICACIÓN I2C
	 */
    return data;
}


// Accelerometer Task

void getAccelerometer(void *pvParameters)
{
    // int i = 0;
    // uint8_t WIAM = 0;
    // int DATA_LENGTH = 1;
    read_ADXL343(dev_address, who_i_am_reg, 1);
    // printf("Resp acc: 0x%02x \n", response);
    write_ADXL343(dev_address, power_ctl_reg, 0x08);
    for( ; ; ){

    read_ADXL343(dev_address, DATAX0, 6);
    
    // printf("X1 :: X0 -> 0x%02X :: 0x%02X\n", acc2.FX1, acc2.FX0);
    // printf("Y1 :: Y0 -> 0x%02X :: 0x%02X\n", acc2.FY1, acc2.FY0);
    // printf("Z1 :: Z0 -> 0x%02X :: 0x%02X\n", acc2.FZ1, acc2.FZ0);
    printf("DATAX:  0x%02x OR %i; \n", acc.FX, acc.FX);
    //printf("DATAX1:  0x%02x OR %d; \n", acc.X1, acc.X1);
    printf("DATAY:  0x%02x OR %i; \n", acc.FY, acc.FY);
    //printf("DATAY1:  0x%02x OR %d; \n", acc.Y1, acc.Y1);
    printf("DATAZ:  0x%02x OR %i; \n\n", acc.FZ, acc.FZ);
    //printf("DATAZ1:  0x%02x OR %d; \n", acc.Z1, acc.Z1);
    
    if (acc.FY < threshold_low){

        switch(state_machine_position){         

        case 0:
            luminosity.T = (luminosity.T << 1 | 0x01);
            luminosity.T = luminosity.T & 0x3ff;
            break;

        case 1:
            luminosity.T = (luminosity.T << 1 | 0x01);
            luminosity.T = luminosity.T & 0x3ff;
            luminosity.R = luminosity.T;
            luminosity.G = luminosity.T;
            luminosity.B = luminosity.T;
            set_led_RGB();
            break;

        case 2:
            luminosity.R = (luminosity.R << 1 | 0x01);
            luminosity.R = luminosity.R & 0x3ff;
        //luminosity.G = luminosity.R;
        //luminosity.B = luminosity.R;
            set_led_RGB();
            break;
        case 3:
            luminosity.G = (luminosity.G << 1 | 0x01);
            luminosity.G = luminosity.G & 0x3ff;
            set_led_RGB();
            break;
        case 4:
            luminosity.B = (luminosity.B << 1 | 0x01);
            luminosity.B = luminosity.B & 0x3ff;
            set_led_RGB();
            break;
        case 5:
            luminosity.R = (luminosity.R*2);
            //luminosity.R = luminosity.R & 0x3ff;
            if (luminosity.R > 0x3ff){
                luminosity.R = 0x2ff;
            }
            
            luminosity.G = (luminosity.G*2);
            // luminosity.G = luminosity.G & 0x3ff;
            if (luminosity.G > 0x3ff){
                luminosity.G = 0x3ff;
            }

            luminosity.B = (luminosity.B*2);
            //luminosity.B = luminosity.B & 0x3ff;
            if (luminosity.B > 0x3ff){
                luminosity.B = 0x3ff;
            }
            
            set_led_RGB();
            break;
        }; // end switch case
    // printf("threshold_high; luminosity = 0x%02X; \n ", luminosity.R);

    }

    if (acc.FY > threshold_high){

        switch(state_machine_position){  
        case 0:
            luminosity.T = (luminosity.T >> 1 );
            break;
        case 1:
            luminosity.T = (luminosity.T >> 1 );
            luminosity.R  = luminosity.T;
            luminosity.G  = luminosity.T;  
            luminosity.B  = luminosity.T; 
            set_led_RGB();
            break;
        case 2:
            luminosity.R = (luminosity.R >> 1 );
            set_led_RGB();
            break;
        case 3:
            luminosity.G = (luminosity.G >> 1 );
            set_led_RGB();
            break;
        case 4:
            luminosity.B = (luminosity.B >> 1 );
            set_led_RGB();
            break;
        case 5:
            luminosity.R = (luminosity.R/2 );
            luminosity.G = (luminosity.G/2 );
            luminosity.B = (luminosity.B/2 );
            set_led_RGB();
            break;
        }

    }

    // vTaskDelay(5 / portTICK_RATE_MS);
    // response = read_ADXL343(dev_address, DATAX0, 6);

                /*  BLOQUEADO: CÓDIGO  POR ELIMINAR
                    // RUTINA DE SLEEP 
            
            	esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
            	esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_ON);
            	esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_ON);
            	esp_sleep_pd_config(ESP_PD_DOMAIN_MAX, ESP_PD_OPTION_ON);
            	int t = 10000000;
            	esp_sleep_enable_timer_wakeup(t);
                    
            	i = i + 1;
                    printf("getAccelerometer task %d.\n", i);
            	
            	if(i%10 == 0){
            	printf("OK\n");
                    // esp_light_sleep_start();
                    // esp_deep_sleep(t);
            	} //endif
            
                */  // FIN DE RUTINA DE SLEEP

    vTaskDelay( 1000 / portTICK_PERIOD_MS );
    } //end infiniteloop

    vTaskDelete(NULL);
} //end task