/*
 * Copyright (c) 2017 james mathewson 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: James Mathewson <jlmathew@soe.ucsc.edu> 
 */


#ifndef GLOBALMODULENS3_H_
#define GLOBALMODULENS3_H_

class PktType;
class UtilNetEvent;
class TimeEvent;
class StoreAction;

#include <cstdlib>
#include <stdint.h>
#include <ostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <list>
#include <exception>
#include <vector>
#include <map>
#include "ns3/EventHandlerBase.h"
#include "ns3/IcnName.h"
#include "ns3/ContentName.h"
#include "ns3/UtilityConfigBase.h"
#include "ns3/GlobalModuleBase.h"
#include "ns3/timer.h"
#include "ns3/nstime.h"

namespace ns3 {
//need their own module
class GlobalModuleTimerNs3:public GlobalModuleTimer
{
public:
  GlobalModuleTimerNs3();
  virtual ~GlobalModuleTimerNs3();
  virtual timer_struct_t GetTime ();
  virtual int NanoSleep (const timer_struct_t & timeAdv);
private:
 uint64_t counter;
};

class Ns3InfoLog:public GlobalModuleLog
{
  virtual void log(const char *a, ...);
  template < typename T > void tLog (T t)
  {
    std::cout << t;
  }

};
}
#endif
