#include <AHT10.h>

uint8_t AHT10_Original_Data[6]={0};

/// @brief 初始化AHT10传感器
/// @param  None
void AHT10_Init(void){
    uint8_t Init_Commands[3]={0};
    Init_Commands[0]=Initialize_Command; //指令组来自Datasheet
    Init_Commands[1]=0x08;
    Init_Commands[2]=0x00;
    HAL_I2C_Master_Transmit(&AHT10_I2C,AHT10_ADDRESS,Init_Commands,3,100);
}



/// @brief 软复位AHT10传感器
/// @param  None
void AHT10_Reset(void){
    uint8_t Reset_Command=Soft_Reset_Command;
    HAL_I2C_Master_Transmit(&AHT10_I2C,AHT10_ADDRESS,&Reset_Command,1,100);
    HAL_Delay(20);
}



/// @brief 触发AHT10传感器，进行一次温湿度传感检测
/// @param  None
void AHT10_Trig_Measure(void){
    uint8_t Trig_Commands[3]={0};
    Trig_Commands[0]=Measure_Trig_Command; //指令组来自Datasheet
    Trig_Commands[1]=0x33;
    Trig_Commands[2]=0x00;
    HAL_I2C_Master_Transmit(&AHT10_I2C,AHT10_ADDRESS,Trig_Commands,3,100);
}



/// @brief 读取传感器原始数据（6字节）
/// @param pOriginData 
/// @return 0-读取成功，数据有效 1-读取失败，传感器报告数据无效
uint8_t AHT10_Read_Data(uint8_t *pOriginData){
    HAL_I2C_Master_Receive(&AHT10_I2C,AHT10_ADDRESS,pOriginData,6,100);
    uint8_t checkTemp=pOriginData[0];
    checkTemp &= 0x80;  //只留下bit7,用于判断数据是否有效
    if(checkTemp==0x00){ //如果数据有效就返回0
        return 0;
    }else{
        return 1;
    }
}



/// @brief 计算获得的原始数据，返回float型的温湿度数据
/// @param pTemperature 
/// @param pHumidity 
/// @return 0-读取完毕 1-读取失败
uint8_t AHT10_Measure(float *pTemperature, float *pHumidity){
    uint8_t readFlag = 0;

    AHT10_Trig_Measure(); //触发一次测量
    HAL_Delay(100); //触发后等待转换，转换结束以后才能够读取
    readFlag=AHT10_Read_Data(AHT10_Original_Data); //读取原始数据

    if(readFlag==0){    //如果原始数据有效
        *pHumidity=AHT10_GetHumidity(); //读取湿度
        *pTemperature=AHT10_GetTemperature(); //读取温度
    }

    return readFlag;
}



/// @brief 从原始数据中获得湿度数据
/// @param  None
/// @return 湿度（浮点型）
float AHT10_GetHumidity(void){
    uint8_t humiTemp[3]={0};
    uint32_t humi=0;
    float tempHumidity=0;

    humiTemp[0]=AHT10_Original_Data[1];
    humiTemp[1]=AHT10_Original_Data[2];
    humiTemp[2]=AHT10_Original_Data[3]&0xF0; //根据Datasheet，原始数据第3字节，高4位是湿度，低4位为温度。所以需要将温度滤去

    for(int i=0;i<3;i++){  //将湿度原始数据字节整合到一个变量中
        humi+=humiTemp[i];
        humi<<=8;
    }   
    humi>>=12;  //抛弃低位无用0数据（有效数据只有20bit）
    tempHumidity=(humi/1048576.0f)*100.0f; //进行最终数据计算

    for(int i=0;i<3;i++){ //清空临时数据，避免数据错乱
        humiTemp[i]=0;
    }
    humi=0;

    return tempHumidity;
}



/// @brief 从原始数据中获得温度信息
/// @param  None   
/// @return 温度（浮点型）
float AHT10_GetTemperature(void){
    uint8_t tempTemp[3]={0};
    uint32_t temp=0;
    float tempTemperature=0;

    tempTemp[0]=(AHT10_Original_Data[3]&0x0F)<<4;  //因为原始数据字节3低4位才对应温度，这里需要将其变为高4位
    tempTemp[0]+=(AHT10_Original_Data[4]&0xF0)>>4; //这样的变换导致后面的高低位全部错位，需要进行一些移位操作
    tempTemp[1]=(AHT10_Original_Data[4]&0x0F)<<4;
    tempTemp[1]+=(AHT10_Original_Data[5]&0xF0)>>4;
    tempTemp[2]=(AHT10_Original_Data[5]&0x0F)<<4;
    tempTemp[2]+=(AHT10_Original_Data[6]&0xF0)>>4;

    for(int i=0;i<3;i++){ //将数组整合为一整个变量
        temp+=tempTemp[i];
        temp<<=8;
    }
    temp>>=12; //抛弃低位0
    tempTemperature=((temp/1048576.0f)*200.0f)-50.0f; //进行最终计算

    for(int i=0;i<3;i++){ //清空缓存
        tempTemp[i]=0;
    }
    temp=0;

    return tempTemperature;
}