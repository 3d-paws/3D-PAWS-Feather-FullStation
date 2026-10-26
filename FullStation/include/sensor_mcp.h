/*
 * ======================================================================================================================
 *  sensor_mcp.h - MicroChip mcp9808 I2C Sensors
 * ======================================================================================================================
 */
#include <Adafruit_MCP9808.h>

/*
 * ======================================================================================================================
 *  MCP9808 - I2C - Temperature sensor
 * 
 * I2C Address is:  0011,A2,A1,A0
 *                  0011000 = 0x18  where A2,1,0 = 0 MCP9808_I2CADDR_DEFAULT  
 *                  0011001 = 0x19  where A0 = 1
 *                  0011001 = 0x1A  where A1 = 1
 *                  0011011 = 0x1B  where A0 & A1 = 1
 * ======================================================================================================================
 */
#define MCP_ADDRESS_1     0x18
#define MCP_ADDRESS_2     0x19        // A0 set high, VDD
#define MCP_ADDRESS_3     0x1A        // A1 set high, VDD
#define MCP_ADDRESS_4     0x1B        // A0 & A1 set high, VDD

// Extern declarations for sensor objects
extern Adafruit_MCP9808 mcp1;
extern Adafruit_MCP9808 mcp2;
extern Adafruit_MCP9808 mcp3;
extern Adafruit_MCP9808 mcp4;

// Extern declarations for variables
extern bool MCP_1_exists;
extern bool MCP_2_exists;
extern bool MCP_3_exists;
extern bool MCP_4_exists;
extern float mcp3_temp;  // Used with Derived Observation (wbgt)

// Function prototype
void mcp_obs_do(int &sidx);
void mcp_info(char *rest, int size, const char *&comma);
void mcp_initialize();