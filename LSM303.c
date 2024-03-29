#include <stdio.h>

#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "stm32f10x_i2c.h"

#include "LSM303.h"
#include "vector.h"

int OffAccX, OffAccY, OffAccZ;	 	// acceclerometer offset obtained from calibration
int OffMagnX, OffMagnY, OffMagnZ;		// magnetometer offset obtained from calibration
float GainAccX, GainAccY, GainAccZ;	// accelerometer gain obtained from calibration
float GainMagnX, GainMagnY, GainMagnZ;// magnetometer gain obtained from calibration

int MagnX, MagnY, MagnZ;			// 3-axis magnetometer 
int AccX, AccY, AccZ;				// 3-axis accelerometer 

float vectorH[3];
float vectorA[3];

float uVectorH[3];
float uVectorA[3];

float fPitch, fRoll, fHeading;
int PitchInDegree, RollInDegree, HeadingInDegree;

struct vector3 uH, uA, v;
struct matrix m;
struct vector3 uVectorTemp;

//*****************************************************************************
// @brief  Writes one byte to the  LSM303.
// @param  slAddr : slave address LSM_A_ADDRESS or LSM_M_ADDRESS 
// @param  pBuffer : pointer to the buffer containing the data to be written 
//		to the LSM303.
// @param  WriteAddr : address of the register in which the data will 
//		be written
// @retval None
//*****************************************************************************
static void ByteWrite(uint8_t slAddr, uint8_t* pBuffer, uint8_t WriteAddr)
{
	/* Send START condition */
	GenerateSTART(I2C1, ENABLE);

	/* Test on EV5 and clear it */
	while(!CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

	/* Send address for write */
	Send7bitAddress(I2C1, slAddr, I2C_Direction_Transmitter);

	/* Test on EV6 and clear it */
	while(!CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	/* Send the internal address to write to */
	SendData(I2C1, WriteAddr);

	/* Test on EV8 and clear it */
	while(!CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	/* Send the byte to be written */
	SendData(I2C1, *pBuffer);

	/* Test on EV8 and clear it */
	while(!CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	/* Send STOP condition */
	GenerateSTOP(I2C1, ENABLE);

}

//*****************************************************************************
// @brief  Reads a block of data from the LSM303
// @param  slAddr  : slave address LSM_A_ADDRESS or LSM_M_ADDRESS 
// @param  pBuffer : pointer to the buffer that receives the data read 
//		from the LSM303.
// @param  ReadAddr : LSM303's internal address to read from.
// @param  NumByteToRead : number of bytes to read from the LSM303 
//						( NumByteToRead > 1  only for the Mgnetometer readinf).
// @retval None
//*****************************************************************************
static void BufferRead(uint8_t slAddr, uint8_t* pBuffer, uint8_t ReadAddr
		, uint16_t NumByteToRead)
{

	/* While the bus is busy */
	while(GetFlagStatus(I2C1, I2C_FLAG_BUSY));

	/* Send START condition */
	GenerateSTART(I2C1, ENABLE);

	/* Test on EV5 and clear it */
	while(!CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

	/* Send address for write */
	Send7bitAddress(I2C1, slAddr, I2C_Direction_Transmitter);

	/* Test on EV6 and clear it */
	while(!CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	/* Clear EV6 by setting again the PE bit */
	Cmd(I2C1, ENABLE);

	/* Send the internal address to read from */
	SendData(I2C1, ReadAddr);

	/* Test on EV8 and clear it */
	while(!CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	/* Send STRAT condition a second time */
	GenerateSTART(I2C1, ENABLE);

	/* Test on EV5 and clear it */
	while(!CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

	/* Send LSM303 address for read */
	Send7bitAddress(I2C1, slAddr, I2C_Direction_Receiver);

	/* Test on EV6 and clear it */
	while(!CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

	/* While there is data to be read */
	while(NumByteToRead)
	{
		if(NumByteToRead == 1) {
			/* Disable Acknowledgement */
			AcknowledgeConfig(I2C1, DISABLE);

			/* Send STOP Condition */
			GenerateSTOP(I2C1, ENABLE);
		}

		/* Test on EV7 and clear it */
		if(CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED)) {
			/* Read a byte from the sensor */
			*pBuffer = ReceiveData(I2C1);

			/* Point to the next location where the byte read will be saved */
			pBuffer++;

			/* Decrement the read bytes counter */
			NumByteToRead--;
		}
	}

	/* Enable Acknowledgement to be ready for another reception */
	AcknowledgeConfig(I2C1, ENABLE);

}

//*****************************************************************************
// @brief Set configuration of Linear Acceleration measurement of LSM303
// @param LSM_Acc_Config_Struct : pointer to a LSM_Acc_ConfigTypeDef structure 
//		 that contains the configuration setting for the Accelerometer LSM303.
// @retval None
//*****************************************************************************
void LSM303_Acc_Config(LSM_Acc_ConfigTypeDef *LSM_Acc_Config_Struct)
{
	uint8_t CRTL1 = 0x00;
	uint8_t CRTL4  =  0x00;

	CRTL1 |= (uint8_t) (LSM_Acc_Config_Struct->Power_Mode 
			| LSM_Acc_Config_Struct->ODR
			| LSM_Acc_Config_Struct->Axes_Enable);

	CRTL4 |= (uint8_t) (LSM_Acc_Config_Struct->FS 
			| LSM_Acc_Config_Struct->Data_Update
			| LSM_Acc_Config_Struct->Endianess);

	ByteWrite(LSM_A_ADDRESS, &CRTL1, LSM_A_CTRL_REG1_ADDR);
	ByteWrite(LSM_A_ADDRESS, &CRTL4, LSM_A_CTRL_REG4_ADDR);
}

//*****************************************************************************
// @brief Set configuration of Internal High Pass Filter of LSM303 for 
//		the linear acceleration
// @param LSM303_Filter_ConfigTypeDef : pointer to a 
//		LSM303_ConfigTypeDef structure that contains the configuration 
//		setting for the LSM303.
// @retval None
//*****************************************************************************
void LSM303_Acc_Filter_Config(LSM_Acc_Filter_ConfigTypeDef 
		* LSM_Acc_Filter_Config_Struct)
{
	uint8_t CRTL2 = 0x00;
	uint8_t REF  =  0x00;

	CRTL2 |= (uint8_t) (LSM_Acc_Filter_Config_Struct->HPF_Enable 
			| LSM_Acc_Filter_Config_Struct->HPF_Mode
			| LSM_Acc_Filter_Config_Struct->HPF_Frequency);

	REF |= (uint8_t) (LSM_Acc_Filter_Config_Struct->HPF_Reference);

	ByteWrite(LSM_A_ADDRESS, &CRTL2, LSM_A_CTRL_REG2_ADDR);
	ByteWrite(LSM_A_ADDRESS, &REF, LSM_A_REFERENCE_REG_ADDR);
}

//*****************************************************************************
// @brief  Change the lowpower mode for Accelerometer of LSM303
// @param  LowPowerMode : new state for the lowpower mode. 
//		This parameter can be: LSM303_Lowpower_x see LSM303_SPI.h file
// @retval None
//*****************************************************************************
void LSM303_Acc_Lowpower_Cmd(uint8_t LowPowerMode)
{
	uint8_t tmpreg;
	BufferRead(LSM_A_ADDRESS, &tmpreg, LSM_A_CTRL_REG1_ADDR, 1);
	tmpreg &= 0x1F;
	tmpreg |= LowPowerMode;
	ByteWrite(LSM_A_ADDRESS,&tmpreg, LSM_A_CTRL_REG1_ADDR);
}

//*****************************************************************************
// @brief  Change the ODR(Output data rate) for Acceleromter of LSM303
// @param  DataRateValue : new ODR value. This parameter can be: 
//		LSM303_ODR_x see LSM303_SPI.h file
// @retval None
//*****************************************************************************
void LSM303_Acc_DataRate_Cmd(uint8_t DataRateValue)
{
	uint8_t tmpreg;
	BufferRead(LSM_A_ADDRESS, &tmpreg, LSM_A_CTRL_REG1_ADDR, 1);
	tmpreg &= 0xE7;
	tmpreg |= DataRateValue;
	ByteWrite(LSM_A_ADDRESS,&tmpreg, LSM_A_CTRL_REG1_ADDR);
}

//*****************************************************************************
// @brief  Change the Full Scale of LSM303
// @param  FS_value : new full scale value. This parameter can be: 
//		LSM303_FS_x  see LSM303_SPI.h file
// @retval None
//*****************************************************************************
void LSM303_Acc_FullScale_Cmd(uint8_t FS_value)
{
	uint8_t tmpreg;
	BufferRead(LSM_A_ADDRESS, &tmpreg, LSM_A_CTRL_REG4_ADDR, 1);
	tmpreg &= 0xCF;
	tmpreg |= FS_value;
	ByteWrite(LSM_A_ADDRESS,&tmpreg, LSM_A_CTRL_REG4_ADDR);
}

//*****************************************************************************
// @brief  Reboot memory content of LSM303
// @param  None
// @retval None
//*****************************************************************************
void LSM303_Acc_Reboot_Cmd(void)
{
	uint8_t tmpreg;
	BufferRead(LSM_A_ADDRESS, &tmpreg, LSM_A_CTRL_REG2_ADDR, 1);
	tmpreg |= 0x80;
	ByteWrite(LSM_A_ADDRESS, &tmpreg, LSM_A_CTRL_REG2_ADDR);
}

//*****************************************************************************
// @brief  Read LSM303 linear acceleration output register
// @param  out : buffer to store data
// @retval None
//*****************************************************************************
void I2CLSM_Acc_Read_OutReg(uint8_t* out)
{
	BufferRead(LSM_A_ADDRESS, out, (LSM_A_OUT_X_L_ADDR | 0x80), 6);
}

//*****************************************************************************
// @brief   Read LSM303 output register, and calculate the raw  
//			acceleration [LSB] ACC= (out_h*256+out_l)/16 (12 bit rappresentation)
// @param  out : buffer to store data
// @retval None
//*****************************************************************************
static uint8_t LSM303_Acc_Read_RawData(int16_t* out)
{
	uint8_t buffer[6];
	uint8_t crtl4;
	int i;

	BufferRead(LSM_A_ADDRESS, &crtl4, LSM_A_CTRL_REG4_ADDR, 1);
	BufferRead(LSM_A_ADDRESS, &buffer[0], LSM_A_OUT_X_L_ADDR, 6);

	/* check in the control register4 the data alignment*/
	if(crtl4 & 0x40) {
		for(i = 0; i < 3; i++){
			out[i] = (((uint16_t)buffer[2 * i + 1] << 8) | buffer[2 * i]);
		}
	} else {
		for(i=0; i<3; i++){
			out[i]=((int16_t)((uint16_t)buffer[2*i] << 8) | buffer[2 * i + 1]) >> 4;
		}
	}
	return crtl4;
}


//*****************************************************************************
// @brief   Read LSM303 output register, and calculate the acceleration 
//				ACC=SENSITIVITY* (out_h*256+out_l)/16 (12 bit rappresentation)
// @param  out : buffer to store data
// @retval None
//*****************************************************************************
void LSM303_Acc_Read_Acc(int16_t* out)
{
	uint8_t crtl4;
	uint8_t scale;
	uint8_t i;

	crtl4 = LSM303_Acc_Read_RawData( out );
	switch(crtl4 & 0x30){
		/*--------------------------------------------------
		* case 0x00:
		* 	scale = LSM_Acc_Sensitivity_2g_left_rotate;
		* 	break;
		*--------------------------------------------------*/
		case 0x10:
			scale = LSM_Acc_Sensitivity_4g_left_rotate;
			break;
		case 0x30:
			scale = LSM_Acc_Sensitivity_8g_left_rotate;
			break;
		default:
			return;
	}

	for(i = 0; i < 3; i++){
		out[i] <<= scale;
	}
}

//*****************************************************************************
// @brief  Set configuration of Magnetic field measurement of LSM303
// @param  LSM_Magn_Config_Struct :  pointer to LSM_Magn_ConfigTypeDef 
//		structure that contains the configuration setting for the LSM303_Magn.
// @retval None
//*****************************************************************************
void LSM303_Magn_Config(LSM_Magn_ConfigTypeDef *LSM_Magn_Config_Struct)
{
	uint8_t CRTLA = 0x00;
	uint8_t CRTLB = 0x00;
	uint8_t MODE = 0x00;

	CRTLA |= (uint8_t) (LSM_Magn_Config_Struct->M_ODR 
			| LSM_Magn_Config_Struct->Meas_Conf);

	CRTLB |= (uint8_t) (LSM_Magn_Config_Struct->Gain);
	MODE  |= (uint8_t) (LSM_Magn_Config_Struct->Mode);

	ByteWrite(LSM_M_ADDRESS, &CRTLA, LSM_M_CRA_REG_ADDR);  //CRTL_REGA
	ByteWrite(LSM_M_ADDRESS, &CRTLB, LSM_M_CRB_REG_ADDR);  //CRTL_REGB
	ByteWrite(LSM_M_ADDRESS, &MODE, LSM_M_MR_REG_ADDR);    //Mode register
}

//*****************************************************************************
// @brief  Read LSM303 magnetic field output register and compute the int16_t value
// @param  out : buffer to store data
// @retval None
//*****************************************************************************
static void LSM303_Magn_Read_RawData(int16_t* out)
{
	uint8_t buffer[6];
	int i;

	BufferRead(LSM_M_ADDRESS, buffer, LSM_M_OUT_X_H_ADDR, 6);

	for(i = 0; i < 3; i++){
		out[i] = ((uint16_t)buffer[2 * i] << 8) | buffer[2 * i + 1];
	}
}

//*****************************************************************************
// @brief  Read LSM303 output register, and calculate the magnetic field 
//		   Magn[Ga]=(out_h*256+out_l)*1000/ SENSITIVITY
// @param  out : buffer to store data
// @retval None
//*****************************************************************************
void LSM303_Magn_Read_Magn(int16_t* out)
{
	uint8_t buffer[6];
	uint8_t crtlB;
	int i;
	double scaleXY, scaleZ;

	BufferRead(LSM_M_ADDRESS, &crtlB, LSM_M_CRB_REG_ADDR, 1);
	
 	switch(crtlB & 0xE0) {
	 	case 0x40:
			scaleXY = LSM_Magn_Sensitivity_XY_1_3Ga;
			scaleZ = LSM_Magn_Sensitivity_Z_1_3Ga;
			break;

	 	case 0x60:
			scaleXY = LSM_Magn_Sensitivity_XY_1_9Ga;
			scaleZ = LSM_Magn_Sensitivity_Z_1_9Ga;
			break;

	 	case 0x80:
			scaleXY = LSM_Magn_Sensitivity_XY_2_5Ga;
			scaleZ = LSM_Magn_Sensitivity_Z_2_5Ga;
			break;

	 	case 0xA0:
			scaleXY = LSM_Magn_Sensitivity_XY_4Ga;
			scaleZ = LSM_Magn_Sensitivity_Z_4Ga;
			break;

	 	case 0xB0:
			scaleXY = LSM_Magn_Sensitivity_XY_4_7Ga;
			scaleZ = LSM_Magn_Sensitivity_Z_4_7Ga;
			break;

	 	case 0xC0:
			scaleXY = LSM_Magn_Sensitivity_XY_5_6Ga;
			scaleZ = LSM_Magn_Sensitivity_Z_5_6Ga;
			break;

	 	case 0xE0:
			scaleXY = LSM_Magn_Sensitivity_XY_8_1Ga;
			scaleZ = LSM_Magn_Sensitivity_Z_8_1Ga;
			break;
	}

 	LSM303_Magn_Read_RawData(out);

	for(i = 0; i < 2; i++){
		out[i] = (uint16_t)(out[i] * scaleXY);
	}

	out[2] *= (uint16_t)(out[2] * scaleZ);
}




//*****************************************************************************
// @brief  Head LSM303 Data Init	with calibration value
// @param  None
// @retval None
//*****************************************************************************
void LSM303_Data_Init(void) 	
{
	OffAccX = -19;
	OffAccY = -30;
	OffAccZ = 44;

	OffMagnX = -146 ;
	OffMagnY = 8;
	OffMagnZ = 6;

	GainAccX = 0.9671;
	GainAccY = 0.9804;
	GainAccZ = 0.9671;

	GainMagnX = 2.8248;
	GainMagnY = 2.8777;
	GainMagnZ = 3.3956;

}
//*************************************
// @brief  Head LSM303 Config
// @param  None
// @retval None
//**************************************
void LSM303_Configuration(void)
{
	LSM_Acc_ConfigTypeDef  LSM_Acc_InitStructure;
	LSM_Magn_ConfigTypeDef LSM_Magn_InitStructure;

	LSM_Acc_Filter_ConfigTypeDef LSM_Acc_FilterStructure;
	LSM_Acc_InitStructure.Power_Mode = LSM_Acc_Lowpower_NormalMode;
	LSM_Acc_InitStructure.ODR = LSM_Acc_ODR_50;
	LSM_Acc_InitStructure.Axes_Enable= LSM_Acc_XYZEN;
	LSM_Acc_InitStructure.FS = LSM_Acc_FS_2;
	LSM_Acc_InitStructure.Data_Update = LSM_Acc_BDU_Continuos;
	LSM_Acc_InitStructure.Endianess=LSM_Acc_Big_Endian;

	LSM_Acc_FilterStructure.HPF_Enable=LSM_Acc_Filter_Disable;
	LSM_Acc_FilterStructure.HPF_Mode=LSM_Acc_FilterMode_Normal;
	LSM_Acc_FilterStructure.HPF_Reference=0x00;
	LSM_Acc_FilterStructure.HPF_Frequency=LSM_Acc_Filter_HPc8;

	LSM303_Acc_Config(&LSM_Acc_InitStructure);
	LSM303_Acc_Filter_Config(&LSM_Acc_FilterStructure);

	LSM_Magn_InitStructure.M_ODR = LSM_Magn_ODR_30;
	LSM_Magn_InitStructure.Meas_Conf = LSM_Magn_MEASCONF_NORMAL;
	LSM_Magn_InitStructure.Gain = LSM_Magn_GAIN_1_3;
	LSM_Magn_InitStructure.Mode = LSM_Magn_MODE_CONTINUOS ;
	LSM303_Magn_Config(&LSM_Magn_InitStructure);
}

//*******************************************
// Calculate magnetic field and acceleration vector
//*******************************************
void LSM303_CalVhVa(void)
{
	// Form magnetic vector and change sign for matching right hand rule
	vectorH[0] = (float) (MagnX - OffMagnX) * GainMagnX;		// Hx
	vectorH[1] = (float) (MagnY - OffMagnY) * GainMagnY;		// Hy
	vectorH[2] = (float) (MagnZ - OffMagnZ) * GainMagnZ;		// Hz

	// Form acceleration vector and change sign for matching right hand rule
	vectorA[0] = (float) (AccX - OffAccX) * GainAccX;	// Ax
	vectorA[1] = (float) (AccY - OffAccY) * GainAccY;	// Ay
	vectorA[2] = (float) (AccZ - OffAccZ) * GainAccZ;	// Az
}

//*******************************************
//	Calculate unit vector of magnetic and acceleration
//*******************************************
void LSM303_CalUhUa(void)
{
	uVectorTemp = Normalize(vectorH[0], vectorH[1], vectorH[2]);	
	uVectorH[0] = uVectorTemp.x;
	uVectorH[1] = uVectorTemp.y;
	uVectorH[2] = uVectorTemp.z;

	uVectorTemp = Normalize(vectorA[0], vectorA[1], vectorA[2]);
	uVectorA[0] = uVectorTemp.x;
	uVectorA[1] = uVectorTemp.y;
	uVectorA[2] = uVectorTemp.z;
}

//*******************************************
// Make unit vector of magnetic and acceleration
//*******************************************
void LSM303_MakeAllVector(void)
{
	uH = MakeVector(uVectorH[0], uVectorH[1], uVectorH[2]);	
	uA = MakeVector(uVectorA[0], uVectorA[1], uVectorA[2]);	
}

//********************************************************
// Calculate Pitch, Roll, and Heading (Compass direction)
//********************************************************
void LSM303_CalPitchRollHeading(void)
{
	float TempHeading;
	int16_t AccelRaw[3];
	int16_t MagRaw[3];

	//Reading data from the sensor
	LSM303_Acc_Read_RawData(AccelRaw);
	LSM303_Magn_Read_RawData(MagRaw);

	AccX = (int)AccelRaw[0];
	AccY = (int)AccelRaw[1];
	AccZ = (int)AccelRaw[2];

	MagnX = (int)MagRaw[0];
	MagnY = (int)MagRaw[1];
	MagnZ =	(int)MagRaw[2];

	LSM303_Data_Init(); 	// Set calibration data
	LSM303_CalVhVa();		// Calculate magnetic vector H and acceleration vector A
	LSM303_CalUhUa();		// Calculate unit vector H and A
	LSM303_MakeAllVector();	// Make vector

	fPitch = asin(uA.x);				// Pitch angle in radian
	fRoll = asin(uA.y);					// Roll angle in radian

	m = SetupRotationMatrix(1, -fRoll);//-fPitch);
	v = RotationMatrixObjectToInertial(m, uH);  // Rotate unit vector H along X-axis

	m = SetupRotationMatrix(2, fPitch);//fRoll);
	v = RotationMatrixObjectToInertial(m, v);   // Rotate vector v along Y-axis

	TempHeading = -atan(v.y/v.x);			   //

	// Make heading range from -180 to +180 degree
	if(v.y < 0)
	{
		if(TempHeading < 0) 
			fHeading = 3.1416f + TempHeading;
		else	
			fHeading = TempHeading;
	}
	else
	{
		if(TempHeading > 0) 
			fHeading = -3.1416f + TempHeading;
		else	
			fHeading = TempHeading;
	} 

	PitchInDegree = (int)(fPitch * 57.2956); 		//kRadToDeg);
	RollInDegree = (int)(fRoll * 57.2956);			//kRadToDeg);
	HeadingInDegree = (int)(-fHeading * 57.2956); 	//kRadToDeg);
}

//********************************************************
// Get Pitch
//********************************************************
int LSM303_GetPitch(void)
{
	return PitchInDegree;
}

//********************************************************
// Get Roll
//********************************************************
int LSM303_GetRoll(void)
{
	return RollInDegree;
}

//********************************************************
// Get Heading
//********************************************************
int LSM303_GetHeading(void)
{
	return HeadingInDegree;
}
