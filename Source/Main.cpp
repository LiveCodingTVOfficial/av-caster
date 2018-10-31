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


#include "Controllers/AvCaster.h"
#include "Trace/TraceMain.h"


class AvCasterApplication : public JUCEApplication , public MultiTimer
{
public:

  AvCasterApplication() {}

  void initialise(const String& command_line) override
  {
DEBUG_TRACE_INIT_VERSION

    this->mainWindow = new MainWindow() ;

    if (AvCaster::Initialize(this , this->mainWindow->mainContent))
    {
#ifdef JUCE_LINUX
      // create desktop launch file
      if (APP::desktopFile().loadFileAsString() != APP::desktopText())
        APP::desktopFile().replaceWithText(APP::desktopText()) ;

      // create desktop icon
      if (APP::iconFile().getSize() != APP::logoFile().getSize())
      {
        PNGImageFormat    image_format = PNGImageFormat() ;
        Image             icon_image   = ImageCache::getFromMemory(BinaryData::avcasterlogo48_png    ,
                                                                   BinaryData::avcasterlogo48_pngSize) ;
        FileOutputStream* icon_stream  = new FileOutputStream(APP::iconFile()) ;
        if (!icon_stream->failedToOpen())
          image_format.writeImageToStream(icon_image , *icon_stream) ;
        delete icon_stream ;
      }
#endif // JUCE_LINUX

      // start runtime timers
      startTimers() ;
    }
    else if (AvCaster::Alerts.size() > 0)
         { setApplicationReturnValue(255) ; startTimers() ;       } // defer shutdown
    else { setApplicationReturnValue(255) ; shutdown() ; quit() ; }
  }

  void startTimers()
  {
#ifndef DEBUG_QUIT_BEFORE_MAIN_LOOP
    for (int timer_n = 0 ; timer_n < APP::N_TIMERS ; ++timer_n)
      startTimer(APP::TIMER_IDS[timer_n] , APP::TIMER_IVLS[timer_n]) ;
#else // DEBUG_QUIT_BEFORE_MAIN_LOOP
    Trace::TraceEvent("forced quit") ; shutdown() ; quit() ;
#endif // DEBUG_QUIT_BEFORE_MAIN_LOOP
  }

  void stopTimers()
  {
    for (int timer_n = 0 ; timer_n > APP::N_TIMERS ; ++timer_n)
      stopTimer(APP::TIMER_IDS[timer_n]) ;
  }

  void shutdown() override
  {
DEBUG_TRACE_SHUTDOWN_IN

    stopTimers() ;
    AvCaster::Shutdown() ;

    this->mainWindow = nullptr ;

DEBUG_TRACE_SHUTDOWN_OUT
  }

  void anotherInstanceStarted(const String& command_line) override { }

  void         systemRequestedQuit()        override { this->quit() ; }
  const String getApplicationName()         override { return ProjectInfo::projectName ; }
  const String getApplicationVersion()      override { return ProjectInfo::versionString ; }
  bool         moreThanOneInstanceAllowed() override { return false ; }


  /**
    MainWindow is the top-level ancestor class of all Components.
    It implements the desktop window that contains an instance of our MainContent class.
  */
  class MainWindow : public DocumentWindow
  {
    friend class AvCasterApplication ;
    friend class MainContent ;


  public:

    MainWindow() : DocumentWindow(APP::APP_NAME , Colour(0xff202020) , GUI::TITLEBAR_BTNS)
    {
      // main content
      this->mainContent = new MainContent() ;
      setContentOwned(this->mainContent , true) ;

      // this main desktop window
      Image icon_image = ImageCache::getFromMemory(BinaryData::avcasterlogo48_png    ,
                                                   BinaryData::avcasterlogo48_pngSize) ;
      setIcon(icon_image) ; getPeer()->setIcon(icon_image) ;
#ifdef TRAY_ICON
        this->mainContent->trayIcon->setIconImage(icon_image) ;
#endif // TRAY_ICON
      setUsingNativeTitleBar(false) ;
#ifdef JUCE_MAC
      setTitleBarButtonsRequired(DocumentWindow::allButtons , true) ;
#endif // JUCE_MAC
      setTitleBarHeight(GUI::TITLEBAR_H) ;
      centreWithSize(getWidth() , getHeight()) ;
//       setResizable(true , false) ; // TODO: resizeable preview ?
      setVisible(true) ;
    }

    ~MainWindow() { this->mainContent = nullptr ; }

    void closeButtonPressed() override
    {
      JUCEApplicationBase::getInstance()->systemRequestedQuit() ;
    }

#ifdef TRAY_ICON
    void userTriedToCloseWindow()
    {
      JUCEApplicationBase::getInstance()->systemRequestedQuit() ;
    } // FIXME: this avoids assertion in juce_Component.cpp:737
#endif // TRAY_ICON


  private:

    ScopedPointer<MainContent> mainContent ;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
  } ;


private:

#ifndef DEBUG_QUIT_AFTER_MAIN_LOOP
  void timerCallback(int timer_id) override { AvCaster::HandleTimer(timer_id) ; }
#else // DEBUG_QUIT_AFTER_MAIN_LOOP
  void timerCallback(int timer_id) override
  {
    if (timer_id != APP::TIMER_LO_ID) AvCaster::HandleTimer(timer_id) ;
    else                              { Trace::TraceEvent("forced quit") ; quit() ; }
  }
#endif // DEBUG_QUIT_AFTER_MAIN_LOOP

  ScopedPointer<MainWindow> mainWindow ;
} ;


// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(AvCasterApplication)
