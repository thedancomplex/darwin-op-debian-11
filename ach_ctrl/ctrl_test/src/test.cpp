/*******************************************************************************
* Copyright 2022 Daniel M. Lofaro
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

/* Author: Daniel M. Lofaro */

#include "lofaro_darwin_ach.h"
#include <unistd.h>
int main()
{
  printf("make system object\n");
  DarwinAch da = DarwinAch();
  int r = 0;
  printf("use cmd to turn on system\n");
  r = da.cmd(DARWIN_CMD_ON, true);
  if( r == DARWIN_CMD_OK ) printf("Darwin Started\n");
  else printf("Darwin Fail to start\n");
  
  while(true)
  {
    da.getState();
    printf("ax= %f, ay=%f, az=%f\n",
            da.darwin_state.imu.acc_x,
            da.darwin_state.imu.acc_y,
            da.darwin_state.imu.acc_z);
    da.sleep(0.01);
  }
  return 0;
}