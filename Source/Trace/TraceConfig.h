/*\
|*|  Copyright 2015-2016 bill-auger <https://github.com/bill-auger/av-caster/issues>
|*|
|*|  This file is part of the AvCaster program.
|*|
|*|  AvCaster is free software: you can redistribute it and/or modify
|*|  it under the terms of the GNU Lesser General Public License version 3
|*|  as published by the Free Software Foundation.
|*|
|*|  AvCaster is distributed in the hope that it will be useful,
|*|  but WITHOUT ANY WARRANTY; without even the implied warranty of
|*|  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|*|  GNU Lesser General Public License for more details.
|*|
|*|  You should have received a copy of the GNU Lesser General Public License
|*|  along with AvCaster.  If not, see <http://www.gnu.org/licenses/>.
\*/


#ifndef _TRACECONFIG_H_
#define _TRACECONFIG_H_

#ifdef DEBUG_TRACE

#  include "Trace.h"


#  define DEBUG_TRACE_CONFIG_LOAD_CONFIG                                                        \
  Trace::TraceConfig("loading preset '" + STRING(config_store[CONFIG::PRESET_NAME_ID]) + "'") ; \
  Trace::DumpConfig(config_store , "volatile") ;

#else // DEBUG_TRACE

#  define DEBUG_TRACE_CONFIG_LOAD_CONFIG   ;

#endif // DEBUG_TRACE
#endif // _TRACECONFIG_H_
