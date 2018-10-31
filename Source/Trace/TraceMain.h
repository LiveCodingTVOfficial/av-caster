/*\
|*|  Copyright 2015-2016 bill-auger <https://github.com/bill-auger/av-caster/issues>
|*|
|*|  This file is part of the AvCaster program.
|*|
|*|  AvCaster is free software: you can redistribute it and/or modify
|*|  it under the terms of the GNU General Public License version 3
|*|  as published by the Free Software Foundation.
|*|
|*|  AvCaster is distributed in the hope that it will be useful,
|*|  but WITHOUT ANY WARRANTY; without even the implied warranty of
|*|  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|*|  GNU General Public License for more details.
|*|
|*|  You should have received a copy of the GNU General Public License
|*|  along with AvCaster.  If not, see <http://www.gnu.org/licenses/>.
\*/


#ifndef _TRACEMAIN_H_
#define _TRACEMAIN_H_

#ifdef DEBUG_TRACE

#  include "Trace.h"


/* state */

#  define DEBUG_TRACE_INIT_VERSION LOG(AvCaster::GstVersionMsg()) ; LOG(APP::CLI_VERSION_MSG) ;

#  define DEBUG_TRACE_SHUTDOWN_IN  Trace::TraceState("shutting down") ;

#  define DEBUG_TRACE_SHUTDOWN_OUT Trace::TraceState("clean shutdown - bye") ;

#else // DEBUG_TRACE

#  define DEBUG_TRACE_INIT_VERSION ;
#  define DEBUG_TRACE_SHUTDOWN_IN  ;
#  define DEBUG_TRACE_SHUTDOWN_OUT ;

#endif // DEBUG_TRACE
#endif // _TRACEMAIN_H_
