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

#ifndef _TRACEIRCCLIENT_H_
#define _TRACEIRCCLIENT_H_

#ifdef DEBUG_TRACE

#  include "Trace.h"


#  define DUMP_SERVER_PARAMS String dbg = "" ;                    \
  for (int i = 0 ; i < count ; ++i)                               \
    dbg += "\n\tparams[" + String(i) + "]=" + String(params[i]) ; \
  Trace::TraceChat(dbg) ;                                         \

#  define DEBUG_TRACE_SERVER_INFO                                                                                         \
  bool is_session_valid = a_server_info->session != 0 ;                                                                   \
  bool is_host_valid    = a_server_info->host.isNotEmpty() ;                                                              \
  bool is_port_valid    = a_server_info->port > 0 ;                                                                       \
  bool is_nick_valid    = a_server_info->nick.isNotEmpty() ;                                                              \
  if (DEBUG_TRACE_VB) Trace::TraceChat(String("a_server_info =>") +                                                       \
    "\n\tsession='" + String(!!a_server_info->session) + "' (" + String((is_session_valid) ? "valid" : "invalid") + ")" + \
    "\n\thost='"    + String(a_server_info   ->host  ) + "' (" + String((is_host_valid   ) ? "valid" : "invalid") + ")" + \
    "\n\tport='"    + String(a_server_info   ->port  ) + "' (" + String((is_port_valid   ) ? "valid" : "invalid") + ")" + \
    "\n\tnick='"    + String(a_server_info   ->nick  ) + "' (" + String((is_nick_valid   ) ? "valid" : "invalid") + ")" ) ;

#  define DEBUG_TRACE_CREATE_SESSION                                        \
  String dbg = " session for host '" + String(server_info.host) + "'" ;     \
  if (server_info.session == 0) Trace::TraceError("error creating" + dbg) ; \
  else                          Trace::TraceChat("created" + dbg)           ;

#  define DEBUG_TRACE_LOGIN                                                              \
  String dbg = "connecting to " + String(a_server_info->host) +                          \
               ":"              + String(a_server_info->port) +                          \
               " as '"          + String(a_server_info->nick) + "'" ;                    \
  String err = (is_err) ? String(irc_strerror(irc_errno(a_server_info->session))) : "" ; \
  if (is_err) Trace::TraceError("error " + dbg + " - " + err) ;                          \
  else        Trace::TraceChat(dbg)                                                      ;

#  define DEBUG_TRACE_CONNECTED Trace::TraceState("connected to " + host) ;

#  define DEBUG_TRACE_SERVER_MSG if (DEBUG_TRACE_VB)                                          \
  { if      (event == LIBIRC_RFC_RPL_NAMREPLY  ) Trace::TraceChat("received names reply") ;   \
    else if (event == LIBIRC_RFC_RPL_ENDOFNAMES) Trace::TraceChat("received end of names") ;  \
    else { Trace::TraceChat(String("received code: ") + String(event)) ; DUMP_SERVER_PARAMS } }

#  define DEBUG_TRACE_NICKS                                                          \
  Trace::TraceChat("got " + String(nicks.size())                                   + \
                   " nicks for '" + host + "' channel: " + channel                 + \
                   ((DEBUG_TRACE_VB) ? " [" + nicks.joinIntoString(",") + "]" : "")) ;

#  define DEBUG_TRACE_CHAT_MSG_VB                                                           \
  if (DEBUG_TRACE_VB && count == 2)                                                         \
    Trace::TraceChat("'" + String((!!origin) ? origin : "someone") + "' said in channel " + \
                     String(params[0]) + ": " + String(params[1]) + "")                     ;

#  define DEBUG_TRACE_CHAT_MSG                                    \
  bool has_kicked_self = message.endsWith(IRC::KICKED_SELF_MSG) ; \
  if (is_root_channel ) Trace::TraceVerbose("is_root_channel" ) ; \
  if (is_root_user    ) Trace::TraceVerbose("is_root_user"    ) ; \
  if (is_xmpp_channel ) Trace::TraceVerbose("is_my_channel"   ) ; \
  if (is_login_blocked) Trace::TraceVerbose("is_login_blocked") ; \
  if (is_logged_in    ) Trace::TraceVerbose("is_logged_in"    ) ; \
  if (has_kicked_self ) Trace::TraceVerbose("has_kicked_self" ) ; \
  String dbg = ((is_root_user && is_root_channel  ) ?                                        \
               ((is_logged_in                     ) ? "logged into bitlbee"              :   \
               ((is_login_blocked                 ) ? "already logged into bitlbee" : "")) : \
               ((!is_root_user && !is_root_channel) ? nick + " said: " + message :           \
               ((DEBUG_TRACE_VB                   ) ? "unhandled msg" : "")      )      ) ;  \
  if (dbg.isNotEmpty()) Trace::TraceChat(dbg)                                                ;

#  define DEBUG_TRACE_ONJOIN                                      \
  Trace::TraceChat(nick + " just joined channel " + channel_name) ;

#  define DEBUG_TRACE_ONPART                                      \
  Trace::TraceChat(nick + " just parted channel " + channel_name) ;

#  define DEBUG_TRACE_NICK_CHANGE                                                                \
  Trace::TraceChat(from_nick + "' changed nick to '" + to_nick + "' on TODO(host)TODO(channel)") ;

#  define DEBUG_TRACE_THREAD_RUN_IN                                    \
  Trace::TraceState("starting '" + this->getThreadName() + "' thread") ;

#  define DEBUG_TRACE_THREAD_RUN_OUT                                   \
  Trace::TraceState("stopping '" + this->getThreadName() + "' thread") ;

#else // DEBUG_TRACE

#  define DEBUG_TRACE_SERVER_INFO    ;
#  define DEBUG_TRACE_CREATE_SESSION ;
#  define DEBUG_TRACE_LOGIN          ;
#  define DEBUG_TRACE_CONNECTED      ;
#  define DEBUG_TRACE_SERVER_MSG     ;
#  define DEBUG_TRACE_NICKS          ;
#  define DEBUG_TRACE_CHAT_MSG_VB    ;
#  define DEBUG_TRACE_CHAT_MSG       ;
#  define DEBUG_TRACE_ONJOIN         ;
#  define DEBUG_TRACE_ONPART         ;
#  define DEBUG_TRACE_NICK_CHANGE    ;
#  define DEBUG_TRACE_THREAD_RUN_IN  ;
#  define DEBUG_TRACE_THREAD_RUN_OUT ;

#endif // DEBUG_TRACE
#endif // _TRACEIRCCLIENT_H_
