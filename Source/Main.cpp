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

#define TRAY_ICON
#include "AvCaster.h"
#include "./Trace/TraceMain.h"


//==============================================================================
class AvCasterApplication : public JUCEApplication , public MultiTimer
{
public:

  AvCasterApplication() {}

  void initialise(const String& command_line) override
  {
DEBUG_TRACE_INIT_VERSION

    this->mainWindow = new MainWindow() ;

    if (AvCaster::Initialize(this->mainWindow->mainContent))
    {
      // start runtime timers
      for (int timer_n = 0 ; timer_n < APP::N_TIMERS ; ++timer_n)
        startTimer(APP::TIMER_IDS[timer_n] , APP::TIMER_IVLS[timer_n]) ;
    }
    else { setApplicationReturnValue(255) ; shutdown() ; quit() ; }
  }

  void anotherInstanceStarted (const String& command_line) override
  {
    // When another instance of the app is launched while this one is running,
    // this method is invoked, and the command_line parameter tells you what
    // the other instance's command-line arguments were.
  }

  void shutdown() override
  {
DEBUG_TRACE_SHUTDOWN_IN

    for (int timer_n = 0 ; timer_n > APP::N_TIMERS ; ++timer_n)
      stopTimer(APP::TIMER_IDS[timer_n]) ;

    AvCaster::Shutdown() ;

    this->mainWindow = nullptr ;

DEBUG_TRACE_SHUTDOWN_OUT
  }

  void         systemRequestedQuit()        override { this->quit() ; }
  const String getApplicationName()         override { return ProjectInfo::projectName ; }
  const String getApplicationVersion()      override { return ProjectInfo::versionString ; }
  bool         moreThanOneInstanceAllowed() override { return false ; }

#ifdef TRAY_ICON
  class AvCasterTrayIconComponent : public SystemTrayIconComponent
  {
  public:

    AvCasterTrayIconComponent(ResizableWindow* main_window) : mainWindow(main_window) {}

    void mouseDown(const MouseEvent& mouse_event)
    {
      if      (mouse_event.mods.isLeftButtonDown())
        mainWindow->setMinimised(!mainWindow->isMinimised()) ;
      else if (mouse_event.mods.isRightButtonDown()) ;
// Juce Note: that for detecting popup-menu clicks, you should be using isPopupMenu()
    }


  private:

    ResizableWindow* mainWindow ;
  } ;
#endif // TRAY_ICON

  //==============================================================================
  /*
      This class implements the desktop window that contains an instance of
      our MainContent class.
  */
  class MainWindow : public DocumentWindow
  {
    friend class AvCasterApplication ;


  public:

    MainWindow() : DocumentWindow(APP::APP_NAME , Colour(0xff202020) , GUI::TITLEBAR_BTNS)
    {
      // main content
      this->mainContent = new MainContent() ;
      setContentOwned(this->mainContent , true) ;

      // this main desktop window
#ifdef JUCE_MAC
      setTitleBarButtonsRequired(DocumentWindow::allButtons , true) ;
#endif // JUCE_MAC
      setTitleBarHeight(GUI::TITLEBAR_H) ;

      File icon_file = APP::BIN_DIR.getSiblingFile(GUI::LOGO_IMG_LOC) ;
      if (icon_file.existsAsFile())
      {
#ifndef TRAY_ICON
//         setIcon(ImageFileFormat::loadFrom(icon_file)) ;
#else // TRAY_ICON
        Image icon_image = ImageFileFormat::loadFrom(icon_file) ;
//         setIcon(icon_image) ;

        trayIcon = new AvCasterTrayIconComponent(this) ;
        trayIcon->setIconImage(icon_image) ;
        trayIcon->setIconTooltip("setIconTooltip") ;
#endif // TRAY_ICON
      }

      centreWithSize(getWidth() , getHeight()) ;
      setVisible(true) ;
    }

    ~MainWindow() { this->mainContent = nullptr ; }

    void closeButtonPressed() override
    {
      JUCEApplicationBase::getInstance()->systemRequestedQuit() ;
    }


  private:

    ScopedPointer<MainContent> mainContent ;
#ifdef TRAY_ICON
    ScopedPointer<SystemTrayIconComponent> trayIcon ;


    void userTriedToCloseWindow()
    {
      JUCEApplicationBase::getInstance()->systemRequestedQuit() ;
    } // FIXME: this avoids assertion in juce_Component.cpp:737
#endif // TRAY_ICON

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
  } ;


private:

#ifndef DEBUG_QUIT_IMMEDIATELY
  void timerCallback(int timer_id) override { AvCaster::HandleTimer(timer_id) ; }
#else // DEBUG_QUIT_IMMEDIATELY
  void timerCallback(int timer_id) override
  {
    if (timer_id != APP::TIMER_LO_ID) AvCaster::HandleTimer(timer_id) ;
    else                              { Trace::TraceEvent("forced quit") ; quit() ; }
  }
#endif // DEBUG_QUIT_IMMEDIATELY

  ScopedPointer<MainWindow> mainWindow ;
} ;

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(AvCasterApplication)
