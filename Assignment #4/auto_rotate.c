// This example writes and reads registers in LIS2DH motion sensor


/***************************** Include Files **********************************/
#include "xparameters.h"
#include "xiicps.h"
#include "xil_printf.h"
#include "platform.h"
#include "sleep.h"
#include "xgpiops.h"

#include <stdio.h>
#include <math.h>


#define GPIO_DEVICE_ID  				XPAR_XGPIOPS_0_DEVICE_ID
#define SERVO_MOTOR_ADDRESS				0x41200000
#define LIS2DH_IIC_ADDRESS              0x18   //
#define WHOAMI_REG                      0x0F   //
#define OUT_TEMP_L                    	0x0C   //
#define OUT_TEMP_H                  	0x0D   //
#define TEMP_CFG_REG                    0x1F   //
#define CTRL_REG1                       0x20   //
#define CTRL_REG4                       0x23   //
#define OUT_X_L                   	    0x28   //
#define OUT_X_H                     	0x29   //
#define OUT_Y_L                   	    0x2A   //
#define OUT_Y_H                     	0x2B   //
#define OUT_Z_L                   	    0x2C   //
#define OUT_Z_H                     	0x2D   //

typedef struct { int8_t temperature; } Temp;
typedef struct { int16_t x, y, z; } Accel;

int init_i2c();
int LIS2DH_init();
int LIS2DH_write(u32 ZynqIicAddress, u8 register_offset, u8 write_value);
int LIS2DH_read(u32 ZynqIicAddress, u8 register_offset, u8 *read_value);
int rotate_servo(unsigned* motor, double degree);
int get_temp(Temp* temp);
int get_accel(Accel* accel, int output_bit);

XIicPs Iic;		/**< Instance of the IIC Device */
XGpioPs Gpio;	/* The driver instance for GPIO Device. */

int init_i2c() {
	/*
	 * Initialize the IIC driver so that it's ready to use
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
	XIicPs_Config* Config = XIicPs_LookupConfig(XPAR_XIICPS_0_DEVICE_ID);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	int Status = XIicPs_CfgInitialize(&Iic, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Perform a self-test to ensure that the hardware was built correctly.
	Status = XIicPs_SelfTest(&Iic);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	//Set the IIC serial clock rate.
	XIicPs_SetSClk(&Iic, 100000);

    printf("PS I2C Initialized\n\r");
    return 0;
}


int LIS2DH_init() {
	u8 ID = 0, ctrlreg1, ctrlreg4, tempcfg;

	LIS2DH_read(XPAR_PS7_I2C_0_BASEADDR, WHOAMI_REG, &ID);
	printf("Who AM I = 0x%x\r\n", ID);

	// ODR = 0101 100Hz, XYZ = enable
	LIS2DH_write(XPAR_PS7_I2C_0_DEVICE_ID, CTRL_REG1, 0x57);
	LIS2DH_read(XPAR_PS7_I2C_0_BASEADDR, CTRL_REG1, &ctrlreg1);
	printf("CTRL REG1 = 0x%x\r\n", ctrlreg1);

	// Enable Temperature Sensor
	LIS2DH_write(XPAR_PS7_I2C_0_DEVICE_ID, TEMP_CFG_REG, 0xC0);
	LIS2DH_read(XPAR_PS7_I2C_0_BASEADDR, TEMP_CFG_REG, &tempcfg);
	printf("TEMP_CFG_REG = 0x%x\r\n", tempcfg);

	// BDU = 1
	LIS2DH_write(XPAR_PS7_I2C_0_DEVICE_ID, CTRL_REG4, 0x80);
	LIS2DH_read(XPAR_PS7_I2C_0_BASEADDR, CTRL_REG4, &ctrlreg4);
	printf("CTRL REG4 = 0x%x\r\n", ctrlreg4);

	printf("LIS2DH initialized\r\n");
}


int LIS2DH_write(u32 ZynqIicAddress, u8 register_offset, u8 write_value) {
	/*
	* \brief       Use the Zynq IIC Controller to write a value to a
	*              LIS2DH register at a given offset
	*
	* \param[in]   ZynqIicAddress    - address of the I2C Controller
	* \param[in]   register_offset   - offset of register inside the LIS2DH
	* \param[in]   write_value       - value to be written to LIS2DH register
	*
	* \return      XST_SUCCESS if operation succeeded
	*/

	int Status = XST_SUCCESS;
	u8 TxBuffer[128]; // Only need this to be size 2, but making larger for future use

	TxBuffer[0] = register_offset;  // Offset of register to write
	TxBuffer[1] = write_value;  // value to write there
	/* ADD HERE
	 *  Add code to send the 2 bytes contained in TxBuffer over I2C to
	 *  the device responding to I2C address LIS2DH_IIC_ADDRESS.
	 *  The first byte of TxBuffer contains the offset to the register
	 *  inside the LIS2DH. The second byte contains the data to be
	 *  written to that register. If the transfer fails, then
	 *  set Status to XST_FAILURE
	 */
	Status = XIicPs_MasterSendPolled(&Iic, TxBuffer, 2, LIS2DH_IIC_ADDRESS);
	if (Status != XST_SUCCESS) return XST_FAILURE;
	//Wait until bus is idle to start another transfer.
	while (XIicPs_BusIsBusy(&Iic)) {/* NOP */}

	return(Status);
}


int LIS2DH_read(u32 ZynqIicAddress, u8 register_offset, u8 *read_value) {
	/*
	* \brief       Use the Zynq IIC Controller to read a value from a
	*              LIS2DH register at a given offset
	*
	* \param[in]   ZynqIicAddress    - address of the I2C Controller
	* \param[in]   register_offset   - offset of register inside the LIS2DH
	* \param[in]   *read_value       - pointer to data read from LIS2DH register
	*
	* \return      XST_SUCCESS if operation succeeded
	*/

	int Status = XST_SUCCESS;
	u8 TxBuffer[128]; // Only need this to be size 1, but making larger for future use
	u8 RxBuffer[128]; // Only need this to be size 1, but making larger for future use

	TxBuffer[0] = register_offset;
	/* ADD HERE
	 *  Add code to send 1 byte contained in TxBuffer over I2C to
	 *  the device responding to I2C address LIS2DH_IIC_ADDRESS.
	 *  TxBuffer contains the offset to the register
	 *  inside the LIS2DH. If the transfer fails, then
	 *  set Status to XST_FAILURE
	 */
	Status = XIicPs_MasterSendPolled(&Iic, TxBuffer, 1, LIS2DH_IIC_ADDRESS);
	if (Status != XST_SUCCESS) return XST_FAILURE;
	//Wait until bus is idle to start another transfer.
	while (XIicPs_BusIsBusy(&Iic)) {/* NOP */}

	/* ADD HERE
	 *  Add code to receive 1 byte into RxBuffer over I2C from
	 *  the device responding to I2C address LIS2DH_IIC_ADDRESS.
	 *  Since we previously sent the offset to the register
	 *  inside the LIS2DH, the LIS2DH will now return the data
	 *  contained within that register. If the transfer fails, then
	 *  set Status to XST_FAILURE
	 */
	Status = XIicPs_MasterRecvPolled(&Iic, RxBuffer, 1, LIS2DH_IIC_ADDRESS);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	if(Status==XST_SUCCESS)
		*read_value = RxBuffer[0];

	usleep(100000); // Delay 100 ms, which is 100K us
	return(Status);
}


int get_temp(Temp* temp) {
	u8 tempH, tempL;

	LIS2DH_read(XPAR_PS7_I2C_0_BASEADDR, OUT_TEMP_H, &tempH);
	LIS2DH_read(XPAR_PS7_I2C_0_BASEADDR, OUT_TEMP_L, &tempL);

	temp->temperature = 25 + tempH;
	printf("Current Temperature = %d\r\n", temp->temperature);

	return 0;
}


int get_accel(Accel* accel, int output_bit) {
	u8 xH, xL, yH, yL, zH, zL;
	s16 x, y, z;

	LIS2DH_read(XPAR_PS7_I2C_0_BASEADDR, OUT_X_H, &xH);
	LIS2DH_read(XPAR_PS7_I2C_0_BASEADDR, OUT_X_L, &xL);
	LIS2DH_read(XPAR_PS7_I2C_0_BASEADDR, OUT_Y_H, &yH);
	LIS2DH_read(XPAR_PS7_I2C_0_BASEADDR, OUT_Y_L, &yL);
	LIS2DH_read(XPAR_PS7_I2C_0_BASEADDR, OUT_Z_H, &zH);
	LIS2DH_read(XPAR_PS7_I2C_0_BASEADDR, OUT_Z_L, &zL);

	// low-power mode			8-bit data output
	// normal mode				10-bit data output
	// high-resolution mode		12-bit data output
	int shift = 16 - output_bit;

	x = (s8)xH;
	x <<= 8 - shift;
	x |= xL >> shift;

	y = (s8)yH;
	y <<= 8 - shift;
	y |= yL >> shift;

	z = (s8)zH;
	z <<= 8 - shift;
	z |= zL >> shift;

	accel->x = x;
	accel->y = -y;
	accel->z = z;

	printf("(x, y, z) = (%4d, %4d, %4d)\r\n", accel->x, accel->y, accel->z);

	return 0;
}


int rotate_servo(unsigned* motor, double rad) {
	int degree = (int)(rad * 180 * M_1_PI);		// M_1_PI: 1/PI

    int total_duration = 2100000;
    int degree_0 	=  30000;
    int degree_90 	= 110000;
    int degree_180 	= 190000;

    printf("Degree: %d\n", degree);

    int high_duration = degree_0 + (degree_180 - degree_0) * degree;

    int i, j;
    for (j = 0; j < 20; j++) {
		*motor = 0x1; // HIGH
		for (i = 0; i < high_duration; i++);
		*motor = 0x0; // LOW
		for (i = 0; i < total_duration - high_duration; i++);
	}

    return 0;
}


int main() {
    init_platform();

    init_i2c();
    LIS2DH_init();
    unsigned* motor = (unsigned*)SERVO_MOTOR_ADDRESS;

    // Get Temperature
    Temp* temp = NULL;
    get_temp(temp);

    while (1) {
		// Get Accel
		Accel* accel = NULL;
		get_accel(accel, 10);


		// Rotate motor
		rotate_servo(motor, atan2(accel->y, accel->x));
    }

    cleanup_platform();
    return 0;
}


