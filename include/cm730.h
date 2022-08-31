#if !defined(DYN_CM730)
#define DYN_CM730 1
#endif

#include <fcntl.h>
#include <termios.h>
#define STDIN_FILENO 0

#include <stdio.h>
#include <dynamixel_sdk.h>                                   // Uses Dynamixel SDK library
// Protocol version
#define PROTOCOL_VERSION                1.0                 // See which protocol version is used in the Dynamixel

// Default setting
int DXL_ID = 200;                   // Dynamixel ID: 200 - cm730
#define BAUDRATE                        1000000
#define DEVICENAME                      "/dev/ttyUSB0"      // Check which port is being used on your controller



// Enumbs and defines
#define CM730_ON  1
#define CM730_OFF 0
#define DARWIN_ON  CM730_ON
#define DARWIN_OFF CM730_OFF
#define IMU_ACC_SCALE 70.67723342939482
#define IMU_GYRO_SCALE 500.0
#define VOLTAGE_SCALE 10.0

// Motor IDs
#define ID_CM730 200
#define ID_DARWIN ID_CM730

// Addresses 
#define CM730_ADDRESS_DYN_POWER 24

#define CM730_ADDRESS_IMU_START 38
#define CM730_ADDRESS_IMU_LENGTH 12

#define CM730_ADDRESS_IMU_GYRO_Z 38
#define CM730_ADDRESS_IMU_GYRO_Y 40
#define CM730_ADDRESS_IMU_GYRO_X 42
#define CM730_ADDRESS_IMU_ACC_X 44
#define CM730_ADDRESS_IMU_ACC_Y 46
#define CM730_ADDRESS_IMU_ACC_Z 48

#define CM730_ADDRESS_VOLTAGE 50

namespace darwin {

  int close();
  int open();
  int getch();
  int on(int val);
  int off(int val);
  int kbhit(void);
  int ping(int val);
  int update_imu();
  int update_imu_setup();
  int update_imu_slow();
  double int2double(uint16_t val); 

  // IMU data
  double imu_gyro_x = -0.0; 
  double imu_gyro_y = -0.0; 
  double imu_gyro_z = -0.0; 
  double imu_acc_x  = -0.0; 
  double imu_acc_y  = -0.0; 
  double imu_acc_z  = -0.0; 

  // Voltage
  double voltage = -0.0;


  // Initialize PortHandler instance
  // Set the port path
  // Get methods and members of PortHandlerLinux or PortHandlerWindows
  dynamixel::PortHandler *portHandler = dynamixel::PortHandler::getPortHandler(DEVICENAME);

  // Initialize PacketHandler instance
  // Set the protocol version
  // Get methods and members of Protocol1PacketHandler or Protocol2PacketHandler
  dynamixel::PacketHandler *packetHandler = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);

  // Initialize GroupBulkRead instance
  dynamixel::GroupBulkRead groupBulkReadImu(portHandler, packetHandler);


  double int2double(uint16_t val)
  {
    double the_out = (double)((int32_t)val - 512) / 1023.0;
    return the_out;
  }


  bool update_imu_setup_first = true;
  int update_imu_setup()
  {
    if(update_imu_setup_first)
    {
      bool dxl_addparam_result = false;               // addParam result
      // Add parameter storage for Dynamixel#1 present position value
      // +1 is added to read the voltage
      dxl_addparam_result = groupBulkReadImu.addParam(ID_CM730, CM730_ADDRESS_IMU_START, CM730_ADDRESS_IMU_LENGTH+1);
      if (dxl_addparam_result != true) return 1;

      update_imu_setup_first = false;
      return 0;
    }
    return 1;
  }

  int update_imu()
  {
    update_imu_setup();
    bool dxl_getdata_result = false;                // GetParam result
    uint8_t dxl_error = 0;                          // Dynamixel error

    int dxl_comm_result = COMM_TX_FAIL;             // Communication result

    dxl_comm_result = groupBulkReadImu.txRxPacket();
    packetHandler->getTxRxResult(dxl_comm_result);
    if (groupBulkReadImu.getError(ID_CM730, &dxl_error)) return 1;

    // Check if data is avaliable
    dxl_getdata_result = groupBulkReadImu.isAvailable(ID_CM730, CM730_ADDRESS_IMU_START, CM730_ADDRESS_IMU_LENGTH);
    if (dxl_getdata_result != true) return 1;
    
    // Assign the data
    uint16_t buff_gyro_x = groupBulkReadImu.getData(ID_CM730, CM730_ADDRESS_IMU_GYRO_X, 2);
    uint16_t buff_gyro_y = groupBulkReadImu.getData(ID_CM730, CM730_ADDRESS_IMU_GYRO_Y, 2);
    uint16_t buff_gyro_z = groupBulkReadImu.getData(ID_CM730, CM730_ADDRESS_IMU_GYRO_Z, 2);
    uint16_t buff_acc_x = groupBulkReadImu.getData(ID_CM730, CM730_ADDRESS_IMU_ACC_X, 2);
    uint16_t buff_acc_y = groupBulkReadImu.getData(ID_CM730, CM730_ADDRESS_IMU_ACC_Y, 2);
    uint16_t buff_acc_z = groupBulkReadImu.getData(ID_CM730, CM730_ADDRESS_IMU_ACC_Z, 2);
    uint8_t buff_voltage = groupBulkReadImu.getData(ID_CM730, CM730_ADDRESS_VOLTAGE, 1);

    imu_gyro_x = int2double(buff_gyro_x) * IMU_GYRO_SCALE;
    imu_gyro_y = int2double(buff_gyro_y) * IMU_GYRO_SCALE;
    imu_gyro_z = int2double(buff_gyro_z) * IMU_GYRO_SCALE;
    imu_acc_x  = int2double(buff_acc_x)  * IMU_ACC_SCALE;
    imu_acc_y  = int2double(buff_acc_y)  * IMU_ACC_SCALE;
    imu_acc_z  = int2double(buff_acc_z)  * IMU_ACC_SCALE;
    voltage    = (double)buff_voltage / VOLTAGE_SCALE;


    return 0;
  }

  int update_imu_slow()
  {
    // Read IMU info
    int dxl_comm_result = COMM_TX_FAIL;             // Communication result
    uint8_t dxl_error = 0;                          // Dynamixel error
    uint16_t imu_tmp = 0;

    int imu_i[] = {CM730_ADDRESS_IMU_GYRO_Z, CM730_ADDRESS_IMU_GYRO_Y, CM730_ADDRESS_IMU_GYRO_X,
                   CM730_ADDRESS_IMU_ACC_X,  CM730_ADDRESS_IMU_ACC_Y,  CM730_ADDRESS_IMU_ACC_Z};
    int imu_max = sizeof(imu_i) / sizeof(imu_i[0]);
    int ret = 0;
    for( int i = 0; i < imu_max; i++ )
    {
      int the_imu_val = imu_i[i];
      dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, ID_CM730, the_imu_val, &imu_tmp, &dxl_error);
      //if (dxl_comm_result != COMM_SUCCESS)
      if (dxl_error == 0)
      {
        packetHandler->getTxRxResult(dxl_comm_result);
        //printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
        if     ( the_imu_val == CM730_ADDRESS_IMU_GYRO_Z ) imu_gyro_z = int2double(imu_tmp) * IMU_GYRO_SCALE; // +-500 deg/sec
        else if( the_imu_val == CM730_ADDRESS_IMU_GYRO_Y ) imu_gyro_y = int2double(imu_tmp) * IMU_GYRO_SCALE; // +-500 deg/sec
        else if( the_imu_val == CM730_ADDRESS_IMU_GYRO_X ) imu_gyro_x = int2double(imu_tmp) * IMU_GYRO_SCALE; // +- 500 deg/sec
        else if( the_imu_val == CM730_ADDRESS_IMU_ACC_X  ) imu_acc_x  = int2double(imu_tmp) * IMU_ACC_SCALE; // +-4g
        else if( the_imu_val == CM730_ADDRESS_IMU_ACC_Y  ) imu_acc_y  = int2double(imu_tmp) * IMU_ACC_SCALE; // +-4g
        else if( the_imu_val == CM730_ADDRESS_IMU_ACC_Z  ) imu_acc_z  = int2double(imu_tmp) * IMU_ACC_SCALE; // +-4g
        else ret = 1;
      }
      else ret = 1;
    }


    return ret;
  }


  int close()
  {
    portHandler->closePort();
    return 0;
  }

  int open()
  {
    // Open port
    if (portHandler->openPort())
    {
      printf("Succeeded to open the port!\n");
    }
    else
    {
      printf("Failed to open the port!\n");
      printf("Press any key to terminate...\n");
      getch();
      return 1;
    }

    // Set port baudrate
    if (portHandler->setBaudRate(BAUDRATE))
    {
      printf("Succeeded to change the baudrate!\n");
    }
    else
    {
      printf("Failed to change the baudrate!\n");
      printf("Press any key to terminate...\n");
      getch();
      return 1;
    }
    return 0;
  }


  int getch()
  {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
  }

  int kbhit(void)
  {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
      ungetc(ch, stdin);
      return 1;
    }

    return 0;
  }



  int off(int val)
  {
    int dxl_comm_result = COMM_TX_FAIL;             // Communication result
    uint8_t dxl_error = 0;                          // Dynamixel error

    dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, val, CM730_ADDRESS_DYN_POWER, CM730_OFF, &dxl_error);
    if (dxl_comm_result != COMM_SUCCESS)
    {
      printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
    }
    else if (dxl_error != 0)
    {
      printf("%s\n", packetHandler->getRxPacketError(dxl_error));
      return 1;
    }
    else
    {
      printf("Dynamixel has been successfully turned off \n");
    }
    return 0;
  }

  int on(int val)
  {
    int dxl_comm_result = COMM_TX_FAIL;             // Communication result
    uint8_t dxl_error = 0;                          // Dynamixel error

    dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, val, CM730_ADDRESS_DYN_POWER, CM730_ON, &dxl_error);
    if (dxl_comm_result != COMM_SUCCESS)
    {
      printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
    }
    else if (dxl_error != 0)
    {
      printf("%s\n", packetHandler->getRxPacketError(dxl_error));
      return 1;
    }
    else
    {
      printf("Dynamixel has been successfully turned on \n");
    }
    return 0;
  }

  int ping(int val)
  {
    int dxl_comm_result = COMM_TX_FAIL;             // Communication result
    uint8_t dxl_error = 0;                          // Dynamixel error
    uint16_t dxl_model_number;                      // Dynamixel model number

    dxl_comm_result = packetHandler->ping(portHandler, val, &dxl_model_number, &dxl_error);
    if (dxl_comm_result != COMM_SUCCESS) {
      printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
    }
    else if (dxl_error != 0) {
      printf("%s\n", packetHandler->getRxPacketError(dxl_error));
      return 1;
    }
    else {
      printf("[ID:%03d] ping Succeeded. Dynamixel model number : %d\n", val, dxl_model_number);
    }
    return 0;
  }



}
