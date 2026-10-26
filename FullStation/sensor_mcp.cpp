/*
 * ======================================================================================================================
 * sensor_mcp.cpp - MicroChip mcp9808 I2C Sensors
 * ======================================================================================================================
 */
#include "include/qc.h"
#include "include/output.h"
#include "include/support.h"
#include "include/main.h"
#include "include/nvcf.h"
#include "include/obs.h"
#include "include/sensor_mcp.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 *  MCP9808 - I2C - Temperature sensor
 * ======================================================================================================================
 */
Adafruit_MCP9808 mcp1;
Adafruit_MCP9808 mcp2;
Adafruit_MCP9808 mcp3;
Adafruit_MCP9808 mcp4;

bool MCP_1_exists = false;
bool MCP_2_exists = false;
bool MCP_3_exists = false;
bool MCP_4_exists = false;

float mcp3_temp = 0.0;  // Used with Derived Observation (wbgt)

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/* 
 *=======================================================================================================================
 * mcp_obs_do() - 
 *=======================================================================================================================
 */
void mcp_obs_do(int &sidx) {
  if (MCP_1_exists) {
    float t = 0.0;
   
    // MCP1 Temperature
    strcpy (obs.sensor[sidx].id, "mt1");
    obs.sensor[sidx].type = F_OBS;
    t = mcp1.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
  }

  if (MCP_2_exists) {
    float t = 0.0;
    
    // MCP2 Temperature
    strcpy (obs.sensor[sidx].id, "mt2");
    obs.sensor[sidx].type = F_OBS;
    t = mcp2.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
  }

  if (MCP_3_exists) { // globe temperature 1
    float t = 0.0;

    // MCP3 Globe Temperature
    strcpy (obs.sensor[sidx].id, "gt1");
    obs.sensor[sidx].type = F_OBS;
    t = mcp3.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;

    mcp3_temp = t; // globe temperature
  }

  if (MCP_4_exists) { // globe temperature 2
    float t = 0.0;

    // MCP4 Globe Temperature
    strcpy (obs.sensor[sidx].id, "gt2");
    obs.sensor[sidx].type = F_OBS;
    t = mcp4.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
  }
}

/* 
 *=======================================================================================================================
 * mcp_info() - add to msg mcp sensors, Passing msg ptr, msg bubber sz, and pointer to comma
 *=======================================================================================================================
 */
void mcp_info(char *rest, int size, const char *&comma) {

  if (MCP_1_exists) {
    int used = strlen(rest);
    int n = sprintf (Buffer32Bytes, "%sMCP1", comma);

    // can we add with out over flowing?
    if ((used + n) < size) {
      sprintf(rest + used, "%s", Buffer32Bytes);
      comma = ",";
    }
    else {
      return;
    }
  }

  if (MCP_2_exists) {
    int used = strlen(rest);
    int n = sprintf (Buffer32Bytes, "%sMCP2", comma);

    // can we add with out over flowing?
    if ((used + n) < size) {
      sprintf(rest + used, "%s", Buffer32Bytes);
      comma = ",";
    }
    else {
      return;
    }
  }

  if (MCP_3_exists) {
    int used = strlen(rest);
    int n = sprintf (Buffer32Bytes, "%sMCP3/gt1", comma);

    // can we add with out over flowing?
    if ((used + n) < size) {
      sprintf(rest + used, "%s", Buffer32Bytes);
      comma = ",";
    }
    else {
      return;
    }
  }

  if (MCP_4_exists) {
    int used = strlen(rest);
    int n = sprintf (Buffer32Bytes, "%sMCP4/gt2", comma);

    // can we add with out over flowing?
    if ((used + n) < size) {
      sprintf(rest + used, "%s", Buffer32Bytes);
      comma = ",";
    }
    else {
      return;
    }
  }
}

/* 
 *=======================================================================================================================
 * mcp_initialize() - MicroChip MCP9808 sensor initialize
 *=======================================================================================================================
 */
void mcp_initialize() {
  Output("MCP:INIT");
  
  // 1st MCP9808 Precision I2C Temperature Sensor (I2C ADDRESS = 0x18)
  mcp1 = Adafruit_MCP9808();
  if (!mcp1.begin(MCP_ADDRESS_1)) {
    msgp = (char *) "MCP1 NF";
    MCP_1_exists = false;
  }
  else {
    MCP_1_exists = true;
    msgp = (char *) "MCP1 OK";
  }
  Output (msgp);

  // 2nd MCP9808 Precision I2C Temperature Sensor (I2C ADDRESS = 0x19)
  mcp2 = Adafruit_MCP9808();
  if (!mcp2.begin(MCP_ADDRESS_2)) {
    msgp = (char *) "MCP2 NF";
    MCP_2_exists = false;
  }
  else {
    MCP_2_exists = true;
    msgp = (char *) "MCP2 OK";
  }
  Output (msgp);

  // 3rd MCP9808 Precision I2C Temperature Sensor (I2C ADDRESS = 0x20)
  mcp3 = Adafruit_MCP9808();
  if (!mcp3.begin(MCP_ADDRESS_3)) {
    msgp = (char *) "MCP3 NF";
    MCP_3_exists = false;
  }
  else {
    MCP_3_exists = true;
    msgp = (char *) "MCP3 OK";
  }
  Output (msgp);

  // 4th MCP9808 Precision I2C Temperature Sensor (I2C ADDRESS = 0x21)
  mcp4 = Adafruit_MCP9808();
  if (!mcp4.begin(MCP_ADDRESS_4)) {
    msgp = (char *) "MCP4 NF";
    MCP_4_exists = false;
  }
  else {
    MCP_4_exists = true;
    msgp = (char *) "MCP4 OK";
  }
  Output (msgp);
}