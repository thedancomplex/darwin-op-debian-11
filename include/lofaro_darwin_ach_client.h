#define DARWIN_LOFARO_ACH 1

#include "lofaro_includes_ach.h"
#include "lofaro_defines_ach.h"
#include "lofaro_defines.h"
#include "lofaro_utils.h"
#include "lofaro_types.h"
#include <cstring>
#include <chrono>
#include <functional>
#include <memory>
#include <string>



class DarwinAchClient
{
  public:
    DarwinAchClient();
    int cmd(int cmd);
    int cmd(int cmd, bool block);
    int sleep(double val);
    double time();

    /* Update Methods */
    int getState();
    int stageRefPos(int mot, double val);
    int stageRefPosD(int mot, double val);
    int stageRefVel(int mot, double val);
    int stageRefTorque(int mot, double val);
    int stagePGain(int mot, double val);
    int stageIGain(int mot, double val);
    int stageDGain(int mot, double val);
    int postRef();


    /* Data types */
    darwin_data_def_t darwin_ref;
    darwin_data_def_t darwin_state;
    darwin_cmd_def_t  darwin_cmd;
    darwin_cmd_def_t  darwin_cmd_return;

  private:
    int stageGain(int mot, double val);

    LofaroUtils* lu = new LofaroUtils();

    bool run_loop = false;

    /* Reference Channel */
    ach_channel_t chan_darwin_ref;  

    /* State Feedback Channel */
    ach_channel_t chan_darwin_state;

    /* Command channel */
    ach_channel_t chan_darwin_cmd;

    /* Command Channel Return */
    ach_channel_t chan_darwin_cmd_return;
};

DarwinAchClient::DarwinAchClient()
{
  /* Zero Data */
  memset(&this->darwin_ref,          0, sizeof(this->darwin_ref));
  memset(&this->darwin_state,        0, sizeof(this->darwin_state));
  memset(&this->darwin_cmd,          0, sizeof(this->darwin_cmd));
  memset(&this->darwin_cmd_return,   0, sizeof(this->darwin_cmd_return));

  for( int i = 0; i <= DARWIN_MOTOR_MAX; i++ )
  {
    this->darwin_ref.motor_ref[i].pos    = DARWIN_REF_POS_0;
    this->darwin_ref.motor_ref[i].speed  = DARWIN_REF_VEL_0;
    this->darwin_ref.motor_ref[i].torque = DARWIN_REF_TOR_0;
  }


  /* Make Ach Channels */
  ach_status_t r = ACH_OK;

  /* Open Channels */
  r = ach_open(&this->chan_darwin_ref,        DARWIN_ACH_CHAN_REF,        NULL);
  r = ach_open(&this->chan_darwin_state,      DARWIN_ACH_CHAN_STATE,      NULL);
  r = ach_open(&this->chan_darwin_cmd,        DARWIN_ACH_CHAN_CMD,        NULL);
  r = ach_open(&this->chan_darwin_cmd_return, DARWIN_ACH_CHAN_CMD_RETURN, NULL);

  /* Do initial put on the channel to make sure the exist */
  ach_put(&this->chan_darwin_ref,        &this->darwin_ref,        sizeof(this->darwin_ref));
  ach_put(&this->chan_darwin_state,      &this->darwin_state,      sizeof(this->darwin_state));
  ach_put(&this->chan_darwin_cmd,        &this->darwin_cmd,        sizeof(this->darwin_cmd));
  ach_put(&this->chan_darwin_cmd_return, &this->darwin_cmd_return, sizeof(this->darwin_cmd_return));
  return;
}

double DarwinAchClient::time()
{
  return this->lu->getTime();
}

int DarwinAchClient::sleep(double val)
{
  return this->lu->sleep(val);
}

int DarwinAchClient::getState()
{
  size_t fs;
  ach_status_t r = ach_get( &this->chan_darwin_state, &this->darwin_state, sizeof(this->darwin_state), &fs, NULL, ACH_O_LAST );
  return (int)r;
}


int DarwinAchClient::cmd(int cmd)
{
    return this->cmd(cmd,false);
}
int DarwinAchClient::cmd(int cmd, bool block)
{
  size_t fs;
  ach_status_t r = ACH_OK;
  memset(&this->darwin_cmd,   0, sizeof(this->darwin_cmd));
  this->darwin_cmd.cmd = cmd;
  r = ach_put(&this->chan_darwin_cmd, &this->darwin_cmd, sizeof(this->darwin_cmd));

  /* Waits until return of cmd if the a block of "ture" is sent */
  if(block)
  {
    ach_flush(&this->chan_darwin_cmd_return);
    memset(&this->darwin_cmd_return,   0, sizeof(this->darwin_cmd_return));
    ach_status_t r = ach_get( &this->chan_darwin_cmd_return, &this->darwin_cmd_return, sizeof(this->darwin_cmd_return), &fs, NULL, ACH_O_WAIT );
  }
  return (int)r;
}

int DarwinAchClient::stagePGain(int mot, double val)
{
  if( mot > DARWIN_MOTOR_MAX ) return 1;
  if( mot < DARWIN_MOTOR_MIN ) return 1;
  this->darwin_ref.motor_ref[mot].p_gain;
  return 0;
}

int DarwinAchClient::stageIGain(int mot, double val)
{
  if( mot > DARWIN_MOTOR_MAX ) return 1;
  if( mot < DARWIN_MOTOR_MIN ) return 1;
  this->darwin_ref.motor_ref[mot].i_gain;
  return 0;
}

int DarwinAchClient::stageDGain(int mot, double val)
{
  if( mot > DARWIN_MOTOR_MAX ) return 1;
  if( mot < DARWIN_MOTOR_MIN ) return 1;
  this->darwin_ref.motor_ref[mot].d_gain;
  return 0;
}

int DarwinAchClient::stageRefPosD(int mot, double val)
{
  val = val / 180.0 * M_PI;
  return this->stageRefPos(mot, val);
}

int DarwinAchClient::stageRefPos(int mot, double val)
{
  if(mot > DARWIN_MOTOR_MAX) return 1;
  this->darwin_ref.motor_ref[mot].pos = val;
  return 0;
}

int DarwinAchClient::stageRefVel(int mot, double val)
{
  if(mot > DARWIN_MOTOR_MAX) return 1;
  this->darwin_ref.motor_ref[mot].speed = val;
  return 0;
}

int DarwinAchClient::stageRefTorque(int mot, double val)
{
  if(mot > DARWIN_MOTOR_MAX) return 1;
  this->darwin_ref.motor_ref[mot].torque = val;
  return 0;
}

int DarwinAchClient::postRef()
{
  ach_status_t r = ACH_OK;
  r = ach_put(&this->chan_darwin_ref, &this->darwin_ref, sizeof(this->darwin_ref));
  return (int)r;
}
