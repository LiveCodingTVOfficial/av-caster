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


#ifndef _AVCASTER_H_
#define _AVCASTER_H_

#include "Constants.h"
#include "AvCasterStore.h"
#include "MainContent.h"
#include "IrcClient.h"


/**
  Alert is a helper class for the AvCaster application.
  It queues and presents GUI message boxes to the user.
*/
class Alert
{
public:

  Alert(GUI::AlertType message_type , String message_text)
  {
    this->messageType = message_type ;
    this->messageText = message_text ;
  }

  GUI::AlertType messageType ;
  String         messageText ;
} ;


/**
  AvCaster is the controller class for the AvCaster application.
  It instantiates, initializes, and configures the runtime components
      and mediates communication amongst other parts of the application.
*/
class AvCaster
{
  friend class AvCasterApplication ;
  friend class AvCasterStore ;


public:

  // GUI dispatchers
  static void Warning    (String message_text) ;
  static void Error      (String message_text) ;
  static void AddChatLine(String prefix , String nick , String message) ;
  static void SendChat   (String chat_message) ;

  // callbacks and event handlers
  static ModalComponentManager::Callback* GetModalCb() ;
  static void                             OnModalDismissed(int result , int unused) ;

  // getters/setters
  static Rectangle<int> GetPreviewBounds    () ;
  static void           SetConfig           (const Identifier& a_key , var a_value) ;
  static ValueTree      GetConfigStore      () ;
  static void           DeactivateControl   (const Identifier& a_key) ;
  static void           StorePreset         (String preset_name) ;
  static void           RenamePreset        (String preset_name) ;
  static void           DeletePreset        () ;
  static void           ResetPreset         () ;
  static bool           SetPreset           (String preset_name , int option_n) ;
  static bool           RejectPresetChange  () ;
  static bool           IsConfigGuiSane     () ;
  static bool           IsStaticPreset      () ;
  static int            GetPresetIdx        () ;
  static String         GetPresetName       () ;
  static bool           GetIsPreviewActive  () ;
  static bool           GetIsConfigPending  () ;
  static StringArray    GetPresetsNames     () ;
  static StringArray    GetCameraNames      () ;
  static StringArray    GetAudioNames       () ;
  static StringArray    GetCameraResolutions() ;
  static String         GetCameraResolution () ;
  static String         GetCameraPath       () ;
  static int            GetCameraRate       () ;
  static String         GetVersionString    () ;
  static void           UpdateIrcHost       (StringArray alias_uris , String actual_host) ;
#ifdef PREFIX_CHAT_NICKS
  static void           UpdateChatNicks     (String host , String channel , StringArray nicks) ;
#else // PREFIX_CHAT_NICKS
  static void           UpdateChatNicks     (String host , StringArray nicks) ;
#endif // PREFIX_CHAT_NICKS
  static StringArray    GetChatNicks        (ValueTree chatters_store) ;


private:

  // setup
  static bool Initialize(JUCEApplicationBase* main_app , MainContent* main_content) ;
  static void Shutdown  () ;

  // callbacks and event handlers
  static void HandleTimer        (int timer_id) ;
  static void UpdateStatusGUI    () ;
  static void HandleConfigChanged(const Identifier& a_key) ;
  static void RefreshGui         () ;
  static void UpdateStatus      () ;

  // helpers
  static bool HandleCliParamsTerminating() ;
  static bool ProcessCliParams          () ;
  static bool ValidateEnvironment       () ;
  static void DisplayAlert              () ;


  static JUCEApplicationBase*     App ;
  static MainContent*             Gui ;
#ifndef DISABLE_CHAT
  static ScopedPointer<IrcClient> Irc ;
#endif // DISABLE_CHAT
  static StringArray              CliParams ;
  static Array<Alert*>            Alerts ;
  static bool                     IsAlertModal ;

  // persistence
  static ScopedPointer<AvCasterStore> Store ;

  // runtime features
  static bool IsInitialized ;
  static bool IsMediaEnabled ;
  static bool IsChatEnabled ;
} ;

#endif // _AVCASTER_H_
