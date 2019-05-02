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

#include <time.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include "GlobalModulesNs3.h"
#include "ns3/log.h"

namespace ns3 
{

GlobalModuleTimerNs3::GlobalModuleTimerNs3()
{
  counter = 0;
}
GlobalModuleTimerNs3::~GlobalModuleTimerNs3(){}
//need their own module
timer_struct_t
GlobalModuleTimerNs3::GetTime ()
{
  uint64_t ns3_time = ns3::Simulator::Now().GetNanoSeconds(); 
  //uint64_t ns3_time = ns3::Simulator::Now().GetInteger();
  timer_struct_t time;
  time.tv_sec = ns3_time/1000000000;
  time.tv_nsec = ns3_time-time.tv_sec;
  return time;

}

int
GlobalModuleTimerNs3::NanoSleep (const timer_struct_t & timeAdv)
{
  assert(0);
  return 0;
}

using namespace ns3;
void
Ns3InfoLog::log(const char*a, ...) {
  char buffer[512];
  va_list (ap);
  va_start (ap, a);
  vsprintf (buffer, a, ap);
  va_end (ap);
  std::string msg(buffer);
  //NS_LOG_INFO(msg);  //not working atm
}

}
