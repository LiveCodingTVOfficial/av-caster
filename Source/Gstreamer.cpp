/*\
|*|  Copyright 2015 bill-auger <https://github.com/bill-auger/av-caster/issues>
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


#include <cstdlib>
#include <gst/video/videooverlay.h>
#include <gst/video/gstvideosink.h>

#include "AvCaster.h"
#include "Gstreamer.h"
#include "Trace/TraceGstreamer.h"


/* AvCaster private class variables */

GstElement* Gstreamer::Pipeline        = nullptr ;
GstElement* Gstreamer::ScreencapBin    = nullptr ;
GstElement* Gstreamer::CameraBin       = nullptr ;
GstElement* Gstreamer::TextBin         = nullptr ;
GstElement* Gstreamer::CompositorBin   = nullptr ;
GstElement* Gstreamer::AudioBin        = nullptr ;
GstElement* Gstreamer::InterstitialBin = nullptr ;
GstElement* Gstreamer::MuxBin          = nullptr ;
GstElement* Gstreamer::OutputBin       = nullptr ;
ValueTree   Gstreamer::ConfigStore ;               // Configure()
guintptr    Gstreamer::WindowHandle ;              // Initialize()


/* AvCaster private class methods */

bool Gstreamer::Initialize(void* x_window_handle)
{
  WindowHandle = (guintptr)x_window_handle ;

DEBUG_TRACE_GST_INIT_PHASE_1
/* TODO: not sure if we will need to handle signals
bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
gst_bus_set_sync_handler (bus, (GstBusSyncHandler) cbw, pipeline, NULL);
static GstBusSyncReply cb(GstBus* bus , GstMessage* message, GstPipeline* pipeline)
*/

  // instantiate pipeline
  gst_init(nullptr , nullptr) ;
  if (!(Pipeline = gst_pipeline_new("pipeline")))
  { AvCaster::Error(GUI::GST_INIT_ERROR_MSG) ; return false ; }

DEBUG_TRACE_GST_INIT_PHASE_2

  // configure elements
  if (!ConfigureTodo()) return false ;

DEBUG_TRACE_GST_INIT_PHASE_3

  // set rolling
  if (!SetState(Pipeline , GST_STATE_PLAYING)) return false ;

DEBUG_TRACE_GST_INIT_PHASE_4
DEBUG_MAKE_GRAPHVIZ

  return true ;
}

void Gstreamer::Shutdown()
{
  // cleanup gStreamer
  // TODO: to shut down correctly (flushing the buffers)
  //       gst_element_send_event(Pipeline , gst_event_eos()) ;
  //       then wait for EOS message on bus before setting pipeline state to NULL
  if (Pipeline != nullptr)
  {
    SetState(Pipeline , GST_STATE_NULL) ; gst_object_unref(Pipeline) ;
  }

  ConfigStore = ValueTree::invalid ;
}

bool Gstreamer::Configure() {} // TODO:
bool Gstreamer::ConfigureTodo() // TODO: rename this after reconfiguration is robust
{
  ConfigStore = AvCaster::GetConfigStore() ;

  String camera_device_path = AvCaster::GetCameraPath() ;

bool is_screencap_enabled            = true ; // TODO: GUI support
  bool   is_camera_enabled  = camera_device_path.isNotEmpty() ; // TODO: && GUI support
bool is_text_enabled                 = true ; // TODO: GUI support
bool is_interstitial_enabled         = true ; // TODO: GUI support
bool is_audio_enabled                = true ; // TODO: GUI support
bool is_output_enabled               = true ; // TODO: GUI support
is_screencap_enabled                 = is_screencap_enabled && is_interstitial_enabled ;
is_camera_enabled                    = is_camera_enabled    && is_interstitial_enabled ;
is_text_enabled                      = is_text_enabled      && is_interstitial_enabled ;
bool should_reconfigure_screencap    = is_screencap_enabled && true ;
bool should_reconfigure_camera       = is_camera_enabled    && true ;
bool should_reconfigure_text         = is_text_enabled      && true ;
bool should_reconfigure_interstitial = is_interstitial_enabled && true ;
bool should_reconfigure_compositor   = true ; // always enabled
bool should_reconfigure_audio        = is_audio_enabled        && true ;
bool should_reconfigure_mux          = is_output_enabled       && true ;
bool should_reconfigure_output       = is_output_enabled       && true ;
#ifdef DISABLE_SCREENCAP
is_screencap_enabled = should_reconfigure_screencap = false ;
#endif // DISABLE_SCREENCAP
#ifdef DISABLE_CAMERA
is_camera_enabled = should_reconfigure_camera = false ;
#endif // DISABLE_CAMERA
#ifdef DISABLE_TEXT
is_text_enabled = should_reconfigure_text = false ;
#endif // DISABLE_CAMERA

DEBUG_TRACE_CONFIGURE

#if CONFIGURE_SCREENCAP_BIN
  if (!is_screencap_enabled || should_reconfigure_screencap)
    if (!RemoveBin(ScreencapBin)) return false ;

  if (should_reconfigure_screencap)
  {
    DestroyElement(ScreencapBin) ;
    if (!(ScreencapBin = gst_bin_new(CHARSTAR(GST::SCREEN_BIN_ID))) ||
       !ConfigureScreencap()                                        ||
       !AddBin(ScreencapBin)                                         ) return false ;
  }
#else // CONFIGURE_SCREENCAP_BIN
#  ifdef DEBUG
Trace::TraceState("bypassing screencap configuration") ;
#  endif // DEBUG
#endif // CONFIGURE_SCREENCAP_BIN

#if CONFIGURE_CAMERA_BIN
  if (!is_camera_enabled || should_reconfigure_camera)
    if (!RemoveBin(CameraBin)) { AvCaster::Error(GUI::GST_CONFIG_ERROR_MSG) ; return false ; }

  if (should_reconfigure_camera)
  {
    DestroyElement(CameraBin) ;
    if (!(CameraBin = gst_bin_new(CHARSTAR(GST::CAMERA_BIN_ID))) ||
       !ConfigureCamera()                                        ||
       !AddBin(CameraBin)                                         )
    { AvCaster::Error(GUI::GST_CONFIG_ERROR_MSG) ; return false ; }
  }
#else // CONFIGURE_CAMERA_BIN
#  ifdef DEBUG
Trace::TraceState("bypassing camera configuration") ;
#  endif // DEBUG
#endif // CONFIGURE_CAMERA_BIN

#if CONFIGURE_TEXT_BIN
  if (!is_text_enabled || should_reconfigure_text)
    if (!RemoveBin(TextBin)) return false ;

  if (should_reconfigure_text)
  {
    DestroyElement(TextBin) ;
    if (!(TextBin = gst_bin_new(CHARSTAR(GST::TEXT_BIN_ID))) ||
       !ConfigureText()                                      ||
       !AddBin(TextBin)                                       )
    { AvCaster::Error(GUI::GST_CONFIG_ERROR_MSG) ; return false ; }
  }
#else // CONFIGURE_TEXT_BIN
#  ifdef DEBUG
Trace::TraceState("bypassing text configuration") ;
#  endif // DEBUG
#endif // CONFIGURE_TEXT_BIN

#if CONFIGURE_INTERSTITIAL_BIN
  if (!is_interstitial_enabled || should_reconfigure_interstitial)
    if (!RemoveBin(InterstitialBin)) { AvCaster::Error(GUI::GST_CONFIG_ERROR_MSG) ; return false ; }

  if (should_reconfigure_interstitial)
  {
    DestroyElement(InterstitialBin) ;
    if (!(InterstitialBin = gst_bin_new(CHARSTAR(GST::INTERSTITIAL_BIN_ID))) ||
       !ConfigureInterstitial()                                              ||
       !AddBin(InterstitialBin)                                               )
    { AvCaster::Error(GUI::GST_CONFIG_ERROR_MSG) ; return false ; }
  }
#else // CONFIGURE_INTERSTITIAL_BIN
#  ifdef DEBUG
Trace::TraceState("bypassing interstitial configuration") ;
#  endif // DEBUG
#endif // CONFIGURE_INTERSTITIAL_BIN

#if CONFIGURE_AUDIO_BIN
  if (!is_audio_enabled || should_reconfigure_audio)
    if (!RemoveBin(AudioBin)) { AvCaster::Error(GUI::GST_CONFIG_ERROR_MSG) ; return false ; }

  if (should_reconfigure_audio)
  {
    DestroyElement(AudioBin) ;
    if (!(AudioBin = gst_bin_new(CHARSTAR(GST::AUDIO_BIN_ID))) ||
       !ConfigureAudio()                                       ||
       !AddBin(AudioBin)                                        )
    { AvCaster::Error(GUI::GST_CONFIG_ERROR_MSG) ; return false ; }
  }
#else // CONFIGURE_AUDIO_BIN
#  ifdef DEBUG
Trace::TraceState("bypassing audio configuration") ;
#  endif // DEBUG
#endif // CONFIGURE_AUDIO_BIN

#if CONFIGURE_MUX_BIN
  if (!is_output_enabled || should_reconfigure_mux)
    if (!RemoveBin(MuxBin)) { AvCaster::Error(GUI::GST_CONFIG_ERROR_MSG) ; return false ; }

  if (should_reconfigure_mux)
  {
    DestroyElement(MuxBin) ;
    if (!(MuxBin = gst_bin_new(CHARSTAR(GST::MUX_BIN_ID))) ||
        !ConfigureMux()                                    ||
        !AddBin(MuxBin)                                     )
    { AvCaster::Error(GUI::GST_CONFIG_ERROR_MSG) ; return false ; }
  }

  if (!is_output_enabled || should_reconfigure_output)
    if (!RemoveBin(OutputBin)) { AvCaster::Error(GUI::GST_CONFIG_ERROR_MSG) ; return false ; }

  if (should_reconfigure_output)
  {
    DestroyElement(OutputBin) ;
    if (!(OutputBin = gst_bin_new(CHARSTAR(GST::OUTPUT_BIN_ID))) ||
        !ConfigureOutput()                                       ||
        !AddBin(OutputBin)                                        )
    { AvCaster::Error(GUI::GST_CONFIG_ERROR_MSG) ; return false ; }
  }
#else // CONFIGURE_MUX_BIN
#  ifdef DEBUG
Trace::TraceState("bypassing mux configuration") ;
Trace::TraceState("bypassing output configuration") ;
#  endif // DEBUG
#endif // CONFIGURE_MUX_BIN

  // configure compositor
  if (should_reconfigure_compositor)
  {
    if (!RemoveBin(CompositorBin)) { AvCaster::Error(GUI::GST_CONFIG_ERROR_MSG) ; return false ; }

    DestroyElement(CompositorBin) ;
    if (!(CompositorBin = gst_bin_new(CHARSTAR(GST::COMPOSITOR_BIN_ID))) ||
       !ConfigureCompositor()                                            ||
       !AddBin(CompositorBin)                                             )
    { AvCaster::Error(GUI::GST_CONFIG_ERROR_MSG) ; return false ; }
  }

  // connect enabled bins
  if ((IsInPipeline(ScreencapBin) && !LinkElements(ScreencapBin  , CompositorBin)) ||
      (IsInPipeline(CameraBin   ) && !LinkElements(CameraBin     , CompositorBin)) ||
      (IsInPipeline(MuxBin      ) && !LinkElements(CompositorBin , MuxBin       )) ||
      (IsInPipeline(MuxBin      ) && !LinkElements(AudioBin      , MuxBin       )) ||
      (IsInPipeline(MuxBin      ) && !LinkElements(MuxBin        , OutputBin    ))  )
    { AvCaster::Error(GUI::BIN_LINK_ERROR_MSG) ; return false ; }

  return true ;
}
/*
bool Gstreamer::ConfigureTodo() // TODO: rename this after reconfiguration is robust
{
  ConfigStore = AvCaster::GetConfigStore() ;

  // TODO: videotestsrc and audiotestsrc fallbacks
  GstElement* video_bin = (IsInPipeline(CompositorBin)) ? CompositorBin :
                          (IsInPipeline(ScreencapBin )) ? ScreencapBin  :
                          (IsInPipeline(CameraBin    )) ? CameraBin     : InterstitialBin ;
  if      (video_bin == InterstitialBin && !IsInPipeline(InterstitialBin)) AddBin   (InterstitialBin) ;
  else if (video_bin != InterstitialBin &&  IsInPipeline(InterstitialBin)) RemoveBin(InterstitialBin) ;

  int  has_screencap                 = ConfigureScreencap() ? 1 : 0 ;
  int  has_camera                    = ConfigureCamera   () ? 1 : 0 ;
  int  has_text                      = ConfigureText     () ? 1 : 0 ;
  bool should_configure_compositor   = has_screencap + has_camera + has_text > 1 ;
  bool should_configure_interstitial = has_screencap + has_camera + has_text < 1 ;

/ *
  // remove ScreencapBin and possibly CompositorBin from pipeline
  if (!is_screencap_enabled)
  {
    if ((IsInPipeline(ScreencapBin ) && !RemoveBin(ScreencapBin )) ||
        (IsInPipeline(CompositorBin) && !RemoveBin(CompositorBin))  )
    { DBG("can not remove ScreencapBin or CompositorBin from the Pipeline") ; return false ; }

    return true ;
  }

  // add ScreencapBin and possibly CompositorBin to pipeline
  if ((!IsInPipeline(ScreencapBin ) && !AddBin(ScreencapBin )) ||
     (!!IsInPipeline(CameraBin    )                         &&
       !IsInPipeline(CompositorBin) && !AddBin(CompositorBin))  )
  { DBG("can not add ScreencapBin or CompositorBin to the Pipeline") ; return false ; }

  if (!is_camera_enabled)
  {
    if ((IsInPipeline(CameraBin    ) && !RemoveBin(CameraBin    )) ||
        (IsInPipeline(CompositorBin) && !RemoveBin(CompositorBin))  )
    { DBG("can not remove CameraBin or CompositorBin from the Pipeline") ; return false ; }

    return true ;
  }

  if ((!IsInPipeline(CameraBin    ) && !AddBin(CameraBin    )) ||
     (!!IsInPipeline(ScreencapBin )                         &&
       !IsInPipeline(CompositorBin) && !AddBin(CompositorBin))  )
  { DBG("can not add CameraBin or CompositorBin to the Pipeline") ; return false ; }
* /

  if ((should_configure_compositor  && (!ConfigureCompositor() || !ConfigureAudio())) ||
     (should_configure_interstitial &&  !ConfigureInterstitial()                    ) ||
     !ConfigureMux() || !ConfigureOutput()                                             )
  { AvCaster::Error(GUI::GST_CONFIG_ERROR_MSG) ; return false ; }

  return true ;
}
*/
bool Gstreamer::ConfigureScreencap()
{
  GstElement *source , *capsfilter , *converter , *queue ;
  GstCaps    *caps ;

  int    screencap_w = int(ConfigStore[CONFIG::SCREENCAP_W_ID]) ;
  int    screencap_h = int(ConfigStore[CONFIG::SCREENCAP_H_ID]) ;
  int    framerate_n = int(ConfigStore[CONFIG::FRAMERATE_ID  ]) ;
  String framerate   = CONFIG::FRAMERATES[framerate_n] ;
#if JUCE_LINUX
  String plugin_id = "ximagesrc" ;
  String caps_str  = String("video/x-raw, ")                                +
                     "width=(int)"          + String(screencap_w ) + ", "   +
                     "height=(int)"         + String(screencap_h ) + ", "   +
                     "framerate=(fraction)" + String(framerate   ) + "/1, " +
                     "pixel-aspect-ratio=(fraction)1/1"                     ;
#endif // JUCE_LINUX

#ifdef FAUX_SCREEN_SRC
  plugin_id = "videotestsrc" ;
  caps_str  = String("video/x-raw, ")                                       +
                     "width=(int)"          + String(screencap_w ) + ", "   +
                     "height=(int)"         + String(screencap_h ) + ", "   +
                     "framerate=(fraction)" + String(framerate   ) + "/1, " +
                     "format=I420, "                                        +
                     "pixel-aspect-ratio=(fraction)1/1, "                   +
                     "interlace-mode=(string)progressive"                   ;
#endif // FAUX_SCREEN_SRC

DEBUG_TRACE_CONFIG_SCREENCAP

  if (!(source     = MakeElement(plugin_id      , GST::SCREEN_SOURCE_ID)) ||
      !(capsfilter = MakeElement("capsfilter"   , GST::SCREEN_CAPS_ID  )) ||
      !(converter  = MakeElement("videoconvert" , GST::SCREEN_CONV_ID  )) ||
      !(queue      = MakeElement("queue"        , GST::SCREEN_QUEUE_ID )) ||
      !(caps       = MakeCaps(caps_str)                                 )  )
  { AvCaster::Error(GUI::SCREENCAP_INIT_ERROR_MSG) ; return false ; }

#ifdef FAUX_SCREEN_SRC
  g_object_set(source , "is_live" , true , NULL) ; g_object_set(source , "pattern" , 1 , NULL) ;
#else // FAUX_SCREEN_SRC
  g_object_set(G_OBJECT(source    ) , "endx"       , screencap_w - 1 , NULL) ;
  g_object_set(G_OBJECT(source    ) , "endy"       , screencap_h - 1 , NULL) ;
  g_object_set(G_OBJECT(source    ) , "use-damage" , false           , NULL) ;
#endif // FAUX_SCREEN_SRC
  g_object_set(G_OBJECT(capsfilter) , "caps"       , caps            , NULL) ;
  gst_caps_unref(caps) ;

  if (!AddElement    (ScreencapBin , source    )            ||
      !AddElement    (ScreencapBin , capsfilter)            ||
      !AddElement    (ScreencapBin , converter )            ||
      !AddElement    (ScreencapBin , queue     )            ||
      !LinkElements  (source     , capsfilter)              ||
      !LinkElements  (capsfilter , converter )              ||
      !LinkElements  (converter  , queue     )              ||
      !AddGhostSrcPad(ScreencapBin , queue , "screen-source"))
  { AvCaster::Error(GUI::SCREENCAP_LINK_ERROR_MSG) ; return false ; }

  return true ;
}

bool Gstreamer::ConfigureCamera()
{
  GstElement *source , *capsfilter , *converter , *queue ;
  GstCaps*    caps ;

  // TODO: query device for resolutions (eliminate CONFIG::CAMERA_RESOLUTIONS)
  String      device_path = AvCaster::GetCameraPath() ;
  int         framerate   = AvCaster::GetCameraRate() ;
  String      resolution  = AvCaster::GetCameraResolution() ;
  StringArray res_tokens  = StringArray::fromTokens(resolution , "x" , "") ;
  int         camera_w    = res_tokens[0].getIntValue() ;
  int         camera_h    = res_tokens[1].getIntValue() ;
#if JUCE_LINUX
  String      plugin_id   = "v4l2src" ;
  String      caps_str    = String("video/x-raw, ")                             +
// default is native camera resolution ?
//                             "width=(int)"          + String(camera_w ) + ", "   +
//                             "height=(int)"         + String(camera_h ) + ", "   +
//                             "framerate=(fraction)" + String(framerate) + "/1, " +
//                             "format=I420, "                                     +
                            "pixel-aspect-ratio=(fraction)1/1"                  ;
#endif // JUCE_LINUX

#ifdef FAUX_CAMERA_SRC
  n_devices = 0 ;
  plugin_id = "videotestsrc" ;
  caps_str  = String("video/x-raw, ")                                    +
                     "width=(int)"          + String(camera_w ) + ", "   +
                     "height=(int)"         + String(camera_h ) + ", "   +
                     "framerate=(fraction)" + String(framerate) + "/1, " +
                     "format=I420, "                                     +
                     "pixel-aspect-ratio=(fraction)1/1, "                +
                     "interlace-mode=(string)progressive"                ;
#endif // FAUX_CAMERA_SRC

DEBUG_TRACE_CONFIG_CAMERA

  if (!(source     = MakeElement(plugin_id      , GST::CAMERA_SOURCE_ID)) ||
      !(capsfilter = MakeElement("capsfilter"   , GST::CAMERA_CAPS_ID  )) ||
      !(converter  = MakeElement("videoconvert" , GST::CAMERA_CONV_ID  )) ||
      !(queue      = MakeElement("queue"        , GST::CAMERA_QUEUE_ID )) ||
      !(caps       = MakeCaps(caps_str)                                 )  )
  { AvCaster::Error(GUI::CAMERA_INIT_ERROR_MSG) ; return false ; }

    g_object_set(G_OBJECT(source    ) , "device" , CHARSTAR(device_path) , NULL) ;
    g_object_set(G_OBJECT(capsfilter) , "caps"   , caps                  , NULL) ;
    gst_caps_unref(caps) ;

  if (!AddElement    (CameraBin , source    )            ||
      !AddElement    (CameraBin , capsfilter)            ||
      !AddElement    (CameraBin , converter )            ||
      !AddElement    (CameraBin , queue     )            ||
      !LinkElements  (source     , capsfilter)           ||
      !LinkElements  (capsfilter , converter )           ||
      !LinkElements  (converter  , queue     )           ||
      !AddGhostSrcPad(CameraBin , queue , "camera-source"))
  { AvCaster::Error(GUI::CAMERA_LINK_ERROR_MSG) ; return false ; }

  return true ;
}

bool Gstreamer::ConfigureText()
{
  GstElement *filesrc , *subparser , *source , *converter , *queue ;

  String motd_text    = STRING(ConfigStore[CONFIG::MOTD_TEXT_ID    ]) ;
  int    text_style_n = int   (ConfigStore[CONFIG::TEXT_STYLE_ID   ]) ;
  int    text_pos_n   = int   (ConfigStore[CONFIG::TEXT_POSITION_ID]) ;

DEBUG_TRACE_CONFIG_TEXT

/* TODO: include custom font
#include <fontconfig/fontconfig.h>
std::string yourFontFilePath = "/home/testUser/bla.ttf"
const FcChar8 * file = (const FcChar8 *)yourFontFilePath.c_str();
FcBool fontAddStatus = FcConfigAppFOntAddFile(FcConfigGetCurrent(),file);
*/
// TODO: text from storage
  if (!(filesrc   = gst_element_factory_make("filesrc"      , "text-filesrc"  )) ||
      !(subparser = gst_element_factory_make("subparse"     , "text-subparser")) ||
      !(source    = gst_element_factory_make("textrender"   , "text-input"    )) ||
      !(converter = gst_element_factory_make("videoconvert" , "text-converter")) ||
      !(queue     = gst_element_factory_make("queue"        , "text-queue"    ))  )
    { AvCaster::Error(GUI::TEXT_INIT_ERROR_MSG) ; return false ; }

  g_object_set(source  , "font-desc" , "Purisa Normal 40"             , NULL) ;
  g_object_set(filesrc , "location"  , "/code/av-caster/deleteme.srt" , NULL) ;

  if (!AddElement    (TextBin , filesrc  )           ||
      !AddElement    (TextBin , subparser)           ||
      !AddElement    (TextBin , source   )           ||
      !AddElement    (TextBin , converter)           ||
      !AddElement    (TextBin , queue    )           ||
      !LinkElements  (filesrc   , subparser)         ||
      !LinkElements  (subparser , source   )         ||
      !LinkElements  (source    , converter)         ||
      !LinkElements  (converter , queue    )         ||
      !AddGhostSrcPad(TextBin , queue , "text-source"))
  { AvCaster::Error(GUI::TEXT_LINK_ERROR_MSG) ; return false ; }

  return true ;
}

bool Gstreamer::ConfigureInterstitial()
{
//#define STATIC_INTERSTITIAL
#  ifdef STATIC_INTERSTITIAL
  GstElement *source , *decoder , *capsfilter , *converter , *scaler , *freezer , *queue ;
  GstCaps    *caps ;

  int    interstitial_w = int(ConfigStore[CONFIG::SCREENCAP_W_ID]) ;
  int    interstitial_h = int(ConfigStore[CONFIG::SCREENCAP_H_ID]) ;
  int    framerate_n    = int(ConfigStore[CONFIG::FRAMERATE_ID  ]) ;
  String framerate      = CONFIG::FRAMERATES[framerate_n] ;
  String image_filename = "/home/bill/img/tech-diff.png" ;
interstitial_w = 1280 ;
interstitial_h = 720 ;
  String caps_str       = String("video/x-raw, ")                                  +
                          "width=(int)"          + String(interstitial_w) + ", "   +
                          "height=(int)"         + String(interstitial_h) + ", "   +
                          "framerate=(fraction)" + String(framerate     ) + "/1, " +
                          "format=I420"                                            ;

DEBUG_TRACE_CONFIG_INTERSTITIAL

  if (!(source     = MakeElement("filesrc"      , "interstitial-real-source")) ||
      !(decoder    = MakeElement("pngdec"       , "interstitial-decoder"    )) ||
      !(capsfilter = MakeElement("capsfilter"   , "interstitial-capsfilter" )) ||
      !(converter  = MakeElement("videoconvert" , "interstitial-converter"  )) ||
      !(scaler     = MakeElement("videoscale"   , "interstitial-scaler"     )) ||
      !(freezer    = MakeElement("imagefreeze"  , "interstitial-freezer"    )) ||
      !(queue      = MakeElement("queue"        , "interstitial-queue"      )) ||
      !(caps       = MakeCaps(caps_str)                                      )  )
  { AvCaster::Error(GUI::INTERSTITIAL_INIT_ERROR_MSG) ; return false ; }

    g_object_set(G_OBJECT(source    ) , "location" , CHARSTAR(image_filename) , NULL) ;
    g_object_set(G_OBJECT(capsfilter) , "caps"     , caps                     , NULL) ;
    gst_caps_unref(caps) ;

  if (!AddElement    (InterstitialBin , source   )                     ||
      !AddElement    (InterstitialBin , decoder  )                     ||
      !AddElement    (InterstitialBin , capsfilter)                    ||
      !AddElement    (InterstitialBin , converter)                     ||
      !AddElement    (InterstitialBin , scaler   )                     ||
      !AddElement    (InterstitialBin , freezer  )                     ||
      !AddElement    (InterstitialBin , queue    )                     ||
      !LinkElements  (source     , decoder   )                         ||
      !LinkElements  (decoder    , converter )                         ||
      !LinkElements  (converter  , capsfilter)                         ||
      !LinkElements  (capsfilter , scaler    )                         ||
      !LinkElements  (scaler     , freezer   )                         ||
      !LinkElements  (freezer    , queue     )                         ||
      !AddGhostSrcPad(InterstitialBin   , queue , "interstitial-source"))
  { AvCaster::Error(GUI::INTERSTITIAL_LINK_ERROR_MSG) ; return false ; }

#  else // STATIC_INTERSTITIAL

  GstElement *source , *capsfilter , *converter , *queue ;
  GstCaps    *caps ;

  // TODO: static image src
  int    interstitial_w = int(ConfigStore[CONFIG::SCREENCAP_W_ID]) ;
  int    interstitial_h = int(ConfigStore[CONFIG::SCREENCAP_H_ID]) ;
  int    framerate_n    = int(ConfigStore[CONFIG::FRAMERATE_ID  ]) ;
  String framerate      = CONFIG::FRAMERATES[framerate_n] ;
  String plugin_id      = "videotestsrc" ;
  String caps_str       = String("video/x-raw, ")                                  +
                          "width=(int)"          + String(interstitial_w) + ", "   +
                          "height=(int)"         + String(interstitial_h) + ", "   +
                          "framerate=(fraction)" + String(framerate     ) + "/1, " +
                          "format=I420, "                                          +
                          "pixel-aspect-ratio=(fraction)1/1, "                     +
                          "interlace-mode=(string)progressive"                     ;

//DEBUG_TRACE_CONFIG_INTERSTITIAL

  if (!(source     = MakeElement(plugin_id      , "interstitial-real-source")) ||
      !(capsfilter = MakeElement("capsfilter"   , "interstitial-capsfilter" )) ||
      !(converter  = MakeElement("videoconvert" , "interstitial-converter"  )) ||
      !(queue      = MakeElement("queue"        , "interstitial-queue"      )) ||
      !(caps       = MakeCaps(caps_str)                                      )  )
  { AvCaster::Error(GUI::INTERSTITIAL_INIT_ERROR_MSG) ; return false ; }

    g_object_set(G_OBJECT(source    ) , "is_live" , true , NULL) ;
    g_object_set(G_OBJECT(source    ) , "pattern" , 1    , NULL) ;
    g_object_set(G_OBJECT(capsfilter) , "caps"    , caps , NULL) ; gst_caps_unref(caps) ;

  if (!AddElement    (InterstitialBin , source    )                    ||
      !AddElement    (InterstitialBin , capsfilter)                    ||
      !AddElement    (InterstitialBin , converter )                    ||
      !AddElement    (InterstitialBin , queue     )                    ||
      !LinkElements  (source     , capsfilter)                         ||
      !LinkElements  (capsfilter , converter )                         ||
      !LinkElements  (converter  , queue     )                         ||
      !AddGhostSrcPad(InterstitialBin   , queue , "interstitial-source"))
  { AvCaster::Error(GUI::INTERSTITIAL_LINK_ERROR_MSG) ; return false ; }

#  endif // STATIC_INTERSTITIAL

  return true ;
}

bool Gstreamer::ConfigureCompositor()
{
  GstElement *compositor , *capsfilter , *converter , *queue ;
  GstCaps*    caps ;
  GstPad     *compositor_fullscreen_sinkpad , *compositor_overlay_sinkpad ;

  int         fullscreen_w = int(ConfigStore[CONFIG::SCREENCAP_W_ID]) ;
  int         fullscreen_h = int(ConfigStore[CONFIG::SCREENCAP_H_ID]) ;
  int         output_w     = int(ConfigStore[CONFIG::OUTPUT_W_ID   ]) ;
  int         output_h     = int(ConfigStore[CONFIG::OUTPUT_H_ID   ]) ;
  int         framerate_n  = int(ConfigStore[CONFIG::FRAMERATE_ID  ]) ;
  int         framerate    = CONFIG::FRAMERATES[framerate_n].getIntValue() ;
  String      overlay_res  = AvCaster::GetCameraResolution() ;
  StringArray res_tokens   = StringArray::fromTokens(overlay_res , "x" , "") ;
  int         overlay_w    = res_tokens[0].getIntValue() ;
  int         overlay_h    = res_tokens[1].getIntValue() ;
  int         overlay_x    = fullscreen_w - overlay_w ;
  int         overlay_y    = fullscreen_h - overlay_h ;
  String      caps_str     = String("video/x-raw, ")                             +
                             "width=(int)"          + String(output_w ) + ", "   +
                             "height=(int)"         + String(output_h ) + ", "   +
                             "framerate=(fraction)" + String(framerate) + "/1, " +
                             "format=I420, "                                     +
                             "interlace-mode=(string)progressive, "              +
                             "pixel-aspect-ratio=(fraction)1/1"                  ;

DEBUG_TRACE_CONFIG_COMPOSITOR

#  if CONFIGURE_TEES
  GstElement *fullscreen_queue     , *overlay_queue        , *composite_tee  ,
             *composite_sink_queue , *composite_thru_queue , *composite_sink ;

GstElement *fake_fullscreen_src        , *fake_overlay_src        ,
           *fake_fullscreen_capsfilter , *fake_overlay_capsfilter ,
           *fake_fullscreen_thru_sink  , *fake_overlay_thru_sink  , *fake_composite_thru_sink  ;
GstCaps*   test_caps = MakeCaps(caps_str) ;
bool has_real_fullscreen_source = true ;
bool has_real_overlay_source = true ;
bool has_real_output_sink = true ;
#    if CONFIGURE_SCREENCAP_BIN
  if (!IsInPipeline(ScreencapBin))
#    endif // CONFIGURE_SCREENCAP_BIN
  {
DBG("fake_fullscreen_src") ;
has_real_fullscreen_source = false ;
fake_fullscreen_src        = MakeElement("videotestsrc" , "compositor-fullscreen-src"       ) ;
fake_fullscreen_capsfilter = MakeElement("capsfilter"   , "compositor-fullscreen-capsfilter") ;
g_object_set(fake_fullscreen_src , "is_live" , true , NULL) ;
g_object_set(fake_fullscreen_src , "pattern" , 1    , NULL) ;
g_object_set(fake_fullscreen_capsfilter , "caps" , test_caps , NULL) ;
if (!AddElement  (CompositorBin , fake_fullscreen_src      )) { DBG("fake_fullscreen_src ERR") ; return false ; }
//fake_fullscreen_capsfilter unused
  }
#    if CONFIGURE_CAMERA_BIN
  if (!IsInPipeline(CameraBin))
#    endif // CONFIGURE_CAMERA_BIN
  {
DBG("fake_overlay_src") ;
has_real_overlay_source = false ;
fake_overlay_src           = MakeElement("videotestsrc" , "compositor-overlay-src"          ) ;
fake_overlay_capsfilter    = MakeElement("capsfilter"   , "compositor-overlay-capsfilter"   ) ;
g_object_set(fake_overlay_src , "is_live" , true , NULL) ;
g_object_set(fake_overlay_src , "pattern" , 1    , NULL) ;
g_object_set(fake_overlay_capsfilter    , "caps" , test_caps , NULL) ;
if (!AddElement  (CompositorBin , fake_overlay_src         )) { DBG("fake_overlay_src ERR") ; return false ; }
//fake_overlay_capsfilter unused
  }
gst_caps_unref(test_caps) ;
#    if CONFIGURE_MUX_BIN
  if (!IsInPipeline(MuxBin))
#    endif // CONFIGURE_MUX_BIN
  {
DBG("fake_composite_thru_sink") ;
has_real_output_sink = false ;
fake_composite_thru_sink = MakeElement("fakesink" , "compositor-thru-sink" ) ;
if (!AddElement  (CompositorBin , fake_composite_thru_sink )) { DBG("fake_composite_thru_sink ERR") ; return false ; }
  }

  // instantiate elements
  if (!(fullscreen_queue     = MakeElement("queue"        , "compositor-fullscreen-queue")) ||
      !(overlay_queue        = MakeElement("queue"        , "compositor-overlay-queue"   )) ||
      !(compositor           = MakeElement("compositor"   , "compositor"                 )) ||
      !(capsfilter           = MakeElement("capsfilter"   , "compositor-capsfilter"      )) ||
      !(converter            = MakeElement("videoconvert" , "compositor-converter"       )) ||
      !(composite_tee        = MakeElement("tee"          , "compositor-tee"             )) ||
      !(composite_sink_queue = MakeElement("queue"        , "compositor-sink-queue"      )) ||
      !(composite_thru_queue = MakeElement("queue"        , "compositor-thru-queue"      )) ||
      !(composite_sink       = MakeElement("xvimagesink"  , "compositor-sink"            )) ||
      !(caps                 = MakeCaps   (caps_str)                                      )  )
#  else // CONFIGURE_TEES
  if (!(compositor = MakeElement("compositor"   , "compositor"          )) ||
      !(capsfilter = MakeElement("capsfilter"   , "compositor-caps"     )) ||
      !(converter  = MakeElement("videoconvert" , "compositor-converter")) ||
      !(queue      = MakeElement("queue"        , "compositor-queue"    )) ||
      !(caps       = MakeCaps   (caps_str)                               )  )
#  endif // CONFIGURE_TEES
  { AvCaster::Error(GUI::MIXER_INIT_ERROR_MSG) ; return false ; }

  // configure elements
  g_object_set(compositor           , "background"       , 1          , NULL) ;
  g_object_set(capsfilter           , "caps"             , caps       , NULL) ;
  g_object_set(composite_sink_queue , "max-size-bytes"   , (guint  )0 , NULL) ;
  g_object_set(composite_sink_queue , "max-size-time"    , (guint64)0 , NULL) ;
  g_object_set(composite_sink_queue , "max-size-buffers" , (guint  )0 , NULL) ;
  gst_caps_unref(caps) ;

#if CONFIGURE_TEES
  // attach native xwindow to gStreamer video sink
  if (has_real_output_sink)
  {
    gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(composite_sink) , WindowHandle) ;
    if (!gst_video_overlay_set_render_rectangle(GST_VIDEO_OVERLAY(composite_sink) ,
                                                GUI::PREVIEW_X , GUI::PREVIEW_Y  ,
                                                GUI::PREVIEW_W , GUI::PREVIEW_H  ))
    { AvCaster::Error(GUI::GST_XWIN_ERROR_MSG) ; return false ; }
  }
#endif // CONFIGURE_TEES

  // link elements
#  if CONFIGURE_TEES
  if (!AddElement  (CompositorBin , fullscreen_queue    )  ||
      !AddElement  (CompositorBin , overlay_queue       )  ||
      !AddElement  (CompositorBin , compositor          )  ||
      !AddElement  (CompositorBin , capsfilter          )  ||
      !AddElement  (CompositorBin , converter           )  ||
      !AddElement  (CompositorBin , composite_tee       )  ||
      !AddElement  (CompositorBin , composite_sink_queue)  ||
      !AddElement  (CompositorBin , composite_thru_queue)  ||
      !AddElement  (CompositorBin , composite_sink      )  ||
      !LinkElements(compositor           , capsfilter    ) ||
      !LinkElements(capsfilter           , converter     ) ||
      !LinkElements(converter            , composite_tee ) ||
      !LinkElements(composite_sink_queue , composite_sink)  )
#  else // CONFIGURE_TEES
  if (!AddElement  (CompositorBin , compositor) ||
      !AddElement  (CompositorBin , capsfilter) ||
      !AddElement  (CompositorBin , converter ) ||
      !AddElement  (CompositorBin , queue     ) ||
      !LinkElements(compositor , capsfilter)    ||
      !LinkElements(capsfilter , converter )    ||
      !LinkElements(converter  , queue     )     )
#  endif // CONFIGURE_TEES
    { AvCaster::Error(GUI::MIXER_LINK_ERROR_MSG) ; return false ; }

#  if ! CONFIGURE_TEES
  // instantiate ghost pads and request pads
  compositor_fullscreen_sinkpad = MakeRequestSinkPad(CompositorBin , compositor , "compositor-fullscreen-sink") ;
  compositor_overlay_sinkpad    = MakeRequestSinkPad(CompositorBin , compositor , "compositor-overlay-sink"   ) ;
#  else // CONFIGURE_TEES
  GstPad *composite_tee_monitor_srcpad , *composite_tee_thru_srcpad ;

  if ((has_real_fullscreen_source                                                       &&
        !AddGhostSinkPad(CompositorBin , fullscreen_queue , "compositor-fullscreen-sink")) ||
      (!has_real_fullscreen_source                                                      &&
        !LinkElements(fake_fullscreen_src , fullscreen_queue)                            ) ||
      (has_real_overlay_source                                                          &&
        !AddGhostSinkPad(CompositorBin , overlay_queue    , "compositor-overlay-sink"   )) ||
      (!has_real_overlay_source                                                         &&
        !LinkElements(fake_overlay_src    , overlay_queue   )                            ) ||
      !(compositor_fullscreen_sinkpad = NewRequestSinkPad(compositor  )                  ) ||
      !(compositor_overlay_sinkpad    = NewRequestSinkPad(compositor  )                  ) ||
      !(composite_tee_monitor_srcpad  = NewRequestSrcPad (composite_tee)                 ) ||
      !(composite_tee_thru_srcpad     = NewRequestSrcPad (composite_tee)                 )  )
  { DBG("ERR instantiate ghost pads and request pads") ;  false ; }
#  endif // CONFIGURE_TEES

  // configure request pads
  g_object_set(compositor_fullscreen_sinkpad , "width"  , fullscreen_w , NULL) ;
  g_object_set(compositor_fullscreen_sinkpad , "height" , fullscreen_h , NULL) ;
  g_object_set(compositor_fullscreen_sinkpad , "xpos"   , 0            , NULL) ;
  g_object_set(compositor_fullscreen_sinkpad , "ypos"   , 0            , NULL) ;
  g_object_set(compositor_overlay_sinkpad    , "width"  , overlay_w    , NULL) ;
  g_object_set(compositor_overlay_sinkpad    , "height" , overlay_h    , NULL) ;
  g_object_set(compositor_overlay_sinkpad    , "xpos"   , overlay_x    , NULL) ;
  g_object_set(compositor_overlay_sinkpad    , "ypos"   , overlay_y    , NULL) ;

#  if CONFIGURE_TEES
  // link ghost pads and request pads
  GstPad *fullscreen_thru_srcpad , *overlay_thru_srcpad    ,
         *composite_sink_sinkpad , *composite_thru_sinkpad ;
  if (!(fullscreen_thru_srcpad = NewStaticSrcPad (fullscreen_queue    )            ) ||
      !(overlay_thru_srcpad    = NewStaticSrcPad (overlay_queue       )            ) ||
      !(composite_sink_sinkpad = NewStaticSinkPad(composite_sink_queue)            ) ||
      !(composite_thru_sinkpad = NewStaticSinkPad(composite_thru_queue)            ) ||
      !LinkPads       (fullscreen_thru_srcpad       , compositor_fullscreen_sinkpad) ||
      !LinkPads       (overlay_thru_srcpad          , compositor_overlay_sinkpad   ) ||
      !LinkPads       (composite_tee_monitor_srcpad , composite_sink_sinkpad       ) ||
      !LinkPads       (composite_tee_thru_srcpad    , composite_thru_sinkpad       ) ||
     (has_real_output_sink                                                        &&
       !AddGhostSrcPad(CompositorBin , composite_thru_queue , "compositor-source") ) ||
     (!has_real_output_sink                                                       &&
       !LinkElements  (composite_thru_queue , fake_composite_thru_sink)            )  )
  { AvCaster::Error(GUI::MIXER_LINK_ERROR_MSG) ; return false ; }
#  else // CONFIGURE_TEES
  if (!LinkElements  (ScreencapBin , CompositorBin)              ||
      !LinkElements  (CameraBin    , CompositorBin)              ||
      !AddGhostSrcPad(CompositorBin , queue , "compositor-source"))
  { AvCaster::Error(GUI::MIXER_LINK_ERROR_MSG) ; return false ; }
#  endif // CONFIGURE_TEES

//  gst_object_unref(compositor_fullscreen_sinkpad) ; gst_object_unref(compositor_overlay_sinkpad) ;
#  if CONFIGURE_TEES
//   gst_object_unref(fullscreen_tee_monitor_srcpad   ) ; gst_object_unref(fullscreen_tee_monitor_srcpad   ) ;
//   gst_object_unref(overlay_tee_monitor_srcpad) ; gst_object_unref(overlay_tee_monitor_srcpad) ;
//   gst_object_unref(composite_tee_monitor_srcpad   ) ; gst_object_unref(composite_tee_monitor_srcpad   ) ;
#  endif // CONFIGURE_TEES

  return true ;
}

bool Gstreamer::ConfigureAudio()
{
  GstElement *source , *capsfilter , *converter , *queue ;
  GstCaps*    caps ;

  int    audio_api        = int(ConfigStore[CONFIG::AUDIO_API_ID ]) ;
  int    n_channels       = int(ConfigStore[CONFIG::N_CHANNELS_ID]) ;
  int    samplerate_n     = int(ConfigStore[CONFIG::SAMPLERATE_ID]) ;
  int    samplerate       = CONFIG::AUDIO_SAMPLERATES[samplerate_n].getIntValue() ;
  String audio16_caps_str = String("audio/x-raw, "               )               +
                            String("format=(string)S16LE, "      )               +
                            String("layout=(string)interleaved, ")               +
                            String("rate=(int)"    ) + String(samplerate) + ", " +
                            String("channels=(int)") + String(n_channels)        ;
  String audio32_caps_str = String("audio/x-raw, "               )               +
                            String("format=(string)F32LE, "      )               +
                            String("layout=(string)interleaved, ")               +
                            String("rate=(int)"    ) + String(samplerate) + ", " +
                            String("channels=(int)") + String(n_channels)        ;
  String plugin_id , caps_str ;

  switch ((AvCasterStore::AudioApi)audio_api)
  {
    case AvCasterStore::ALSA_AUDIO:  plugin_id = GST::ALSA_PLUGIN_ID ;
                                     caps_str  = audio16_caps_str ;              break ;
    case AvCasterStore::PULSE_AUDIO: plugin_id = GST::PULSE_PLUGIN_ID ;
                                     caps_str  = audio16_caps_str ;              break ;
    case AvCasterStore::JACK_AUDIO:  plugin_id = GST::JACK_PLUGIN_ID ;
                                     caps_str  = audio32_caps_str ;              break ;
    default:                         AvCaster::Error(GUI::AUDIO_CFG_ERROR_MSG) ;
                                     return false ;                              break ;
  }

#ifdef FAUX_AUDIO_SRC
  String test_caps_str = "audio/x-raw, format=(string)S16LE, endianness=(int)1234, signed=(boolean)true, width=(int)16, depth=(int)16, rate=(int)44100, channels=(int)2" ;
  plugin_id = "audiotestsrc" ; caps_str = test_caps_str ;
#endif // FAUX_AUDIO_SRC

DEBUG_TRACE_CONFIG_AUDIO

  if (!(source     = MakeElement(plugin_id      , "audio-real-source")) ||
      !(capsfilter = MakeElement("capsfilter"   , "audio-capsfilter" )) ||
      !(converter  = MakeElement("audioconvert" , "audio-converter"  )) ||
      !(queue      = MakeElement("queue"        , "audio-queue"      )) ||
      !(caps       = MakeCaps(caps_str)                               )  )
  { AvCaster::Error(GUI::AUDIO_INIT_ERROR_MSG) ; return false ; }

  g_object_set(G_OBJECT(capsfilter) , "caps" , caps , NULL) ; gst_caps_unref(caps) ;

#ifdef FAUX_AUDIO_SRC
  g_object_set(source , "is_live" , true , NULL) ;
#endif // FAUX_AUDIO_SRC

  if (!AddElement    (AudioBin , source    )             ||
      !AddElement    (AudioBin , capsfilter)             ||
      !AddElement    (AudioBin , converter )             ||
      !AddElement    (AudioBin , queue     )             ||
      !LinkElements  (source     , capsfilter)           ||
      !LinkElements  (capsfilter , converter )           ||
      !LinkElements  (converter  , queue     )           ||
      !AddGhostSrcPad(AudioBin   , queue , "audio-source"))
  { AvCaster::Error(GUI::AUDIO_LINK_ERROR_MSG) ; return false ; }

  return true ;
}

bool Gstreamer::ConfigureMux()
{
  GstElement *video_in_queue , *video_converter , *video_encoder , *video_parser ,
             *video_enc_caps , *video_enc_queue                                  ;
  GstElement *audio_in_queue , *audio_encoder , *audio_parser , *audio_enc_queue ;
  GstElement *mux ;
  GstCaps    *video_caps ;

  int   output_w        = int(ConfigStore[CONFIG::OUTPUT_W_ID     ]) ;
  int   output_h        = int(ConfigStore[CONFIG::OUTPUT_H_ID     ]) ;
  int   video_bitrate_n = int(ConfigStore[CONFIG::VIDEO_BITRATE_ID]) ;
  int   audio_bitrate_n = int(ConfigStore[CONFIG::AUDIO_BITRATE_ID]) ;
  int   framerate_n     = int(ConfigStore[CONFIG::FRAMERATE_ID    ]) ;
  int   n_channels      = int(ConfigStore[CONFIG::N_CHANNELS_ID   ]) ;
  int   samplerate_n    = int(ConfigStore[CONFIG::SAMPLERATE_ID   ]) ;
  guint video_bitrate   = CONFIG::VIDEO_BITRATES   [video_bitrate_n].getIntValue() ;
  guint audio_bitrate   = CONFIG::AUDIO_BITRATES   [audio_bitrate_n].getIntValue() ;
  int   framerate       = CONFIG::FRAMERATES       [framerate_n    ].getIntValue() ;
  int   samplerate      = CONFIG::AUDIO_SAMPLERATES[samplerate_n   ].getIntValue() ;
//   String h264_caps_str  = "video/x-h264, level=(string)4.1, profile=main" ;
  String h264_caps_str  = String("video/x-h264, ")                            +
                          "width=(int)"          + String(output_w ) + ", "   +
                          "height=(int)"         + String(output_h ) + ", "   +
                          "framerate=(fraction)" + String(framerate) + "/1, " +
                          "stream-format=avc, alignment=au, profile=main"     ;
//   String mp3_caps_str   = String("audio/mpeg, mpegversion=1, layer=3, mpegaudioversion=3, ") +
  String mp3_caps_str   = String("audio/mpeg, mpegversion=1, layer=3, ")       +
                          String("rate=(int)"    ) + String(samplerate) + ", " +
                          String("channels=(int)") + String(n_channels)        ;
  String video_caps_str = h264_caps_str ;
  String audio_caps_str = mp3_caps_str ;

DEBUG_TRACE_CONFIG_MUX

// TODO: some or all of these may not be necessary
// audio_enc_caps is either mis-configured or unfit for this specified insertion
GstElement *audio_converter , *audio_enc_queue2 , *audio_enc_queue3 , *audio_enc_queue4 ;
GstCaps *audio_caps ; GstElement* audio_enc_caps ;

  if (!(video_in_queue  = MakeElement("queue"          , "mux-video-queue"    )) ||
      !(video_converter = MakeElement("videoconvert"   , "mux-video-converter")) ||
      !(video_encoder   = MakeElement("x264enc"        , "mux-video-encoder"  )) ||
      !(video_parser    = MakeElement("h264parse"      , "mux-video-parser"   )) ||
      !(video_caps      = MakeCaps(video_caps_str)                             ) ||
      !(video_enc_caps  = MakeElement("capsfilter"     , "mux-video-enc-caps" )) ||
      !(video_enc_queue = MakeElement("queue"          , "mux-video-enc-queue")) ||
      !(audio_in_queue  = MakeElement("queue"          , "mux-audio-queue"    )) ||
!(audio_converter = MakeElement("audioconvert" , "audio-converter")) ||
      !(audio_encoder   = MakeElement("lamemp3enc"     , "mux-audio-encoder"  )) ||
      !(audio_parser    = MakeElement("mpegaudioparse" , "mux-audio-parser"   )) ||
!(audio_caps      = MakeCaps(audio_caps_str)                          ) ||
!(audio_enc_caps  = MakeElement("capsfilter"     , "mux-audio-enc-caps" )) ||
      !(audio_enc_queue = MakeElement("queue"          , "mux-audio-enc-queue")) ||
!(audio_enc_queue2 = MakeElement("queue"          , "mux-audio-enc-queue2")) ||
!(audio_enc_queue3 = MakeElement("queue"          , "mux-audio-enc-queue3")) ||
!(audio_enc_queue4 = MakeElement("queue"          , "mux-audio-enc-queue4")) ||
      !(mux             = MakeElement("flvmux"         , "mux-flvmux"         ))  )
  { AvCaster::Error(GUI::MUX_INIT_ERROR_MSG) ; return false ; }

  g_object_set(video_encoder  , "bitrate"    , video_bitrate  , NULL) ;
//   g_object_set(video_encoder  , "tune"       , 0x00000004     , NULL) ; // may lower quality in favor of latency
  g_object_set(video_enc_caps , "caps"       , video_caps     , NULL) ;
  g_object_set(audio_encoder  , "target"     , 1              , NULL) ;
  g_object_set(audio_encoder  , "cbr"        , true           , NULL) ; // CBR
  g_object_set(audio_encoder  , "bitrate"    , audio_bitrate  , NULL) ; // CBR
// g_object_set(audio_encoder , "quality" , 2 , NULL) ; // VBR (default is 4) // VBR
  g_object_set(audio_enc_caps , "caps"       , audio_caps     , NULL) ;
  g_object_set(mux            , "streamable" , true           , NULL) ;
  gst_caps_unref(video_caps) ; gst_caps_unref(audio_caps) ;

#  ifdef FAKE_MUX_ENCODER_SRC_AND_SINK
DBG("FAKE_MUX_ENCODER_SRC_AND_SINK") ;
GstElement* fake_enc_sink = MakeElement("xvimagesink"  , "debug-muxer-enc-sink") ;
GstElement* fake_enc_src  = MakeElement("videotestsrc" , "debug-muxer-enc-src" ) ;
g_object_set(fake_enc_src , "is_live" , true , NULL) ;
g_object_set(fake_enc_src , "pattern" , 0    , NULL) ;
if (!AddElement(MuxBin , fake_enc_sink)) return false ;
if (!AddElement(MuxBin , fake_enc_src )) return false ;
gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(fake_enc_sink) , WindowHandle) ;
if (!gst_video_overlay_set_render_rectangle(GST_VIDEO_OVERLAY(fake_enc_sink)                   ,
                                            GUI::COMPOSITE_MONITOR_X + GUI::MONITORS_W ,
                                            GUI::MONITORS_Y                            ,
                                            GUI::MONITORS_W , GUI::MONITORS_H)         ) return false ;
#  endif // FAKE_MUX_ENCODER_SRC_AND_SINK
#  if FAKE_MUX_ENCODER_SRC_AND_SINK || (! CONFIGURE_OUTPUT_BIN)
GstElement* fake_thru_sink = MakeElement("fakesink" , "fake-muxer-thru-sink") ;
if (!AddElement(MuxBin , fake_thru_sink)) return false ;
#  endif // FAKE_MUX_ENCODER_SRC_AND_SINK || (! CONFIGURE_OUTPUT_BIN)

  // TODO: videotestsrc and audiotestsrc fallbacks

  if (!AddElement     (MuxBin , video_in_queue )                   ||
      !AddElement     (MuxBin , video_converter)                   ||
      !AddElement     (MuxBin , video_encoder  )                   ||
      !AddElement     (MuxBin , video_enc_caps )                   ||
      !AddElement     (MuxBin , video_parser   )                   ||
      !AddElement     (MuxBin , video_enc_queue)                   ||
      !AddElement     (MuxBin , audio_in_queue )                   ||
!AddElement(MuxBin , audio_converter ) ||
      !AddElement     (MuxBin , audio_encoder  )                   ||
      !AddElement     (MuxBin , audio_parser   )                   ||
!AddElement(MuxBin , audio_enc_caps )      ||
      !AddElement     (MuxBin , audio_enc_queue)                   ||
!AddElement(MuxBin , audio_enc_queue2) ||
!AddElement(MuxBin , audio_enc_queue3) ||
!AddElement(MuxBin , audio_enc_queue4) ||
      !AddElement     (MuxBin , mux            )                   ||
      !LinkElements   (video_in_queue  , video_converter)          ||
#  ifdef FAKE_MUX_ENCODER_SRC_AND_SINK
      !LinkElements   (video_converter , fake_enc_sink  )          ||
      !LinkElements   (fake_enc_src    , video_encoder  )          ||
#  else // FAKE_MUX_ENCODER_SRC_AND_SINK
      !LinkElements   (video_converter , video_encoder  )          ||
#  endif // FAKE_MUX_ENCODER_SRC_AND_SINK
      !LinkElements   (video_encoder   , video_enc_caps )          ||
      !LinkElements   (video_enc_caps  , video_parser   )          ||
      !LinkElements   (video_parser    , video_enc_queue)          ||
      !LinkElements   (video_enc_queue , mux            )          ||
      !AddGhostSinkPad(MuxBin , video_in_queue , "mux-video-sink") ||
!LinkElements   (audio_in_queue , audio_converter   ) ||
      !LinkElements   (audio_converter , audio_encoder  )          ||
      !LinkElements   (audio_encoder   , audio_enc_caps)          ||
!LinkElements   (audio_enc_caps , audio_parser   )         ||
!LinkElements   (audio_parser   , audio_enc_queue)         ||
//       !LinkElements   (audio_parser    , audio_enc_queue)          ||
!LinkElements   (audio_enc_queue  , audio_enc_queue2) ||
!LinkElements   (audio_enc_queue2 , audio_enc_queue3) ||
!LinkElements   (audio_enc_queue3 , audio_enc_queue4) ||
      !LinkElements   (audio_enc_queue4 , mux           )          ||
      !AddGhostSinkPad(MuxBin , audio_in_queue , "mux-audio-sink") ||
#  if CONFIGURE_OUTPUT_BIN
      !AddGhostSrcPad (MuxBin , mux            , "mux-source"    )  )
#  else // CONFIGURE_OUTPUT_BIN
      !LinkElements   (mux   , fake_thru_sink           )           )
#  endif // CONFIGURE_OUTPUT_BIN
  { AvCaster::Error(GUI::MUX_LINK_ERROR_MSG) ; return false ; }

  return true ;
}

bool Gstreamer::ConfigureOutput()
{
  GstElement *queue , *sink ;

  int    stream_idx  = int   (ConfigStore[CONFIG::OUTPUT_STREAM_ID]) ;
  String dest        = STRING(ConfigStore[CONFIG::OUTPUT_DEST_ID  ]) ;
  String stream      = CONFIG::OUTPUT_STREAMS[stream_idx] ;
  String file_ext    = CONFIG::FLV_CONTAINER ;
  String filename    = dest.upToLastOccurrenceOf(file_ext , false , true) ;
  File   output_file = APP::VIDEOS_DIR.getNonexistentChildFile(filename , file_ext , false) ;
  String file_url    = output_file.getFullPathName() ;
  bool   is_lctv     = AvCaster::GetPresetIdx() == CONFIG::LCTV_PRESET_IDX ;
  String rtmp_url    = (is_lctv) ? MakeLctvUrl(dest) : dest ;

  String plugin_id , output_url ;
  if      (stream == CONFIG::FILE_OUTPUT) { plugin_id = "filesink" ; output_url = file_url ; }
  else if (stream == CONFIG::RTMP_OUTPUT) { plugin_id = "rtmpsink" ; output_url = rtmp_url ; }
  else                                    { AvCaster::Error(GUI::OUTPUT_CFG_ERROR_MSG) ; return false ; }

DEBUG_TRACE_CONFIG_OUTPUT
#ifdef DEBUG
if (stream == CONFIG::RTMP_OUTPUT) output_url = GST::LCTV_RTMP_URL + String(std::getenv("LIVECODING_STREAM_KEY")) + " live=1" ;
#endif // DEBUG

  if (!(queue = MakeElement("queue"   , "output-queue"    )) ||
      !(sink  = MakeElement(plugin_id , "output-real-sink"))  )
  { AvCaster::Error(GUI::OUTPUT_INIT_ERROR_MSG) ; return false ; }

  g_object_set(G_OBJECT(sink) , "location" , CHARSTAR(output_url) , NULL) ;

  if (!AddElement     (OutputBin , queue)               ||
      !AddElement     (OutputBin , sink )               ||
      !LinkElements   (queue  , sink)                   ||
      !AddGhostSinkPad(OutputBin , queue , "output-sink"))
  { AvCaster::Error(GUI::OUTPUT_LINK_ERROR_MSG) ; return false ; }

  return true ;
}

bool Gstreamer::TogglePreview()
{
  bool     is_preview_on = bool(ConfigStore[CONFIG::IS_PREVIEW_ON_ID]) ;
  GstState next_state    = (is_preview_on) ? GST_STATE_PLAYING : GST_STATE_PAUSED ;
  // TODO: howto
}

/*
bool Gstreamer::ToggleOutput()
{
  bool     is_output_on = bool(ConfigStore[CONFIG::IS_OUTPUT_ON_ID]) ;
  GstState next_state   = (is_output_on) ? GST_STATE_PLAYING : GST_STATE_PAUSED ;

  // TODO: disable the muxer - swapping in fakesink in parallel with preview
#if CONFIGURE_OUTPUT_BIN
  RemoveBin(OutputBin , next_state) ;
#endif // CONFIGURE_OUTPUT_BIN

  return is_output_on ;
}
*/

bool Gstreamer::SetState(GstElement* an_element , GstState next_state)
{
  bool is_err = an_element != nullptr                                                     &&
                gst_element_set_state(an_element , next_state) == GST_STATE_CHANGE_FAILURE ;

DEBUG_TRACE_SET_GST_STATE

  if (is_err) AvCaster::Error(GUI::GST_STATE_ERROR_MSG) ;

  return !is_err ;
}

GstElement* Gstreamer::MakeElement(String plugin_id , String element_id)
{
  GstElement* new_element = gst_element_factory_make(CHARSTAR(plugin_id) , CHARSTAR(element_id)) ;

DEBUG_TRACE_MAKE_ELEMENT

  return new_element ;
}

GstCaps* Gstreamer::MakeCaps(String caps_str)
{
  GstCaps* new_caps = gst_caps_from_string(CHARSTAR(caps_str)) ;

DEBUG_TRACE_MAKE_CAPS

  return new_caps ;
}

bool Gstreamer::AddElement(GstElement* a_bin , GstElement* an_element)
{
  bool is_err = !gst_bin_add(GST_BIN(a_bin) , an_element) ;

DEBUG_TRACE_ADD_ELEMENT

  return !is_err ;
}

bool Gstreamer::AddBin(GstElement* a_bin)
{
  bool is_err = !gst_bin_add(GST_BIN(Pipeline) , a_bin)   ||
                !gst_element_sync_state_with_parent(a_bin) ;

DEBUG_TRACE_ADD_BIN

  return !is_err ;
}

bool Gstreamer::RemoveBin(GstElement* a_bin)
{
DEBUG_TRACE_REMOVE_BIN_IN

  bool is_err = IsInPipeline(a_bin) && !gst_bin_remove(GST_BIN(Pipeline) , a_bin) ;

DEBUG_TRACE_REMOVE_BIN_OUT

  return !is_err ;
}

bool Gstreamer::LinkElements(GstElement* source , GstElement* sink)
{
  bool is_err = !gst_element_link(source , sink) ;

DEBUG_TRACE_LINK_ELEMENTS

  return !is_err ;
}

bool Gstreamer::LinkPads(GstPad* srcpad , GstPad* sinkpad)
{
  bool is_err = gst_pad_link(srcpad , sinkpad) != GST_PAD_LINK_OK ;

DEBUG_TRACE_LINK_PADS

  return !is_err ;
}

GstPad* Gstreamer::AddGhostSrcPad(GstElement* a_bin         , GstElement* an_element ,
                                  String      public_pad_id                          )
{
  return AddGhostPad(a_bin , an_element , "src" , public_pad_id) ;
}

GstPad* Gstreamer::AddGhostSinkPad(GstElement* a_bin         , GstElement* an_element ,
                                   String      public_pad_id                          )
{
  return AddGhostPad(a_bin , an_element , "sink" , public_pad_id) ;
}

GstPad* Gstreamer::AddGhostPad(GstElement* a_bin       , GstElement* an_element   ,
                               String      template_id , String      public_pad_id)
{
  const gchar*  private_id = UTF8(template_id  ) ;
  const gchar*  public_id  = UTF8(public_pad_id) ;
  GstPad       *private_pad , *public_pad ;

  bool is_err = !(private_pad = gst_element_get_static_pad(an_element , private_id )) ||
                !(public_pad  = gst_ghost_pad_new         (public_id  , private_pad)) ||
                !gst_pad_set_active(public_pad , TRUE)                                 ;
  gst_object_unref(private_pad) ;

DEBUG_TRACE_MAKE_GHOST_PAD

  if (is_err) return nullptr ;

  if (a_bin != nullptr) is_err = !AddGhostPad(a_bin , public_pad) ;

  return public_pad ;
}

#if ! CONFIGURE_TEES
GstPad* Gstreamer::MakeRequestSrcPad(GstElement* a_bin         , GstElement* an_element ,
                                     String      public_pad_id                          )
{
  return MakeRequestGhostPad(a_bin , an_element , "src_%u" , public_pad_id) ;
}

GstPad* Gstreamer::MakeRequestSinkPad(GstElement* a_bin         , GstElement* an_element ,
                                      String      public_pad_id                          )
{
  return MakeRequestGhostPad(a_bin , an_element , "sink_%u" , public_pad_id) ;
}

GstPad* Gstreamer::MakeRequestGhostPad(GstElement* a_bin       , GstElement* an_element   ,
                                       String      template_id , String      public_pad_id)
{
  const gchar*  private_id = UTF8(template_id  ) ;
  const gchar*  public_id  = UTF8(public_pad_id) ;
  GstPad       *private_pad , *public_pad ;

  bool is_err = !(private_pad = gst_element_get_request_pad(an_element , private_id )) ||
                !(public_pad  = gst_ghost_pad_new          (public_id  , private_pad)) ||
                !gst_pad_set_active(public_pad , TRUE)                                  ;

DEBUG_TRACE_MAKE_GHOST_PAD

  if (is_err || !AddGhostPad(a_bin , public_pad))
  {
    if (private_pad != nullptr) gst_object_unref(private_pad) ;
    if (public_pad  != nullptr) gst_object_unref(public_pad ) ;
    return nullptr ;
  }
  else return private_pad ;
}
#endif // CONFIGURE_TEES
bool Gstreamer::AddGhostPad(GstElement* a_bin , GstPad* public_pad)
{
  bool is_err = !gst_element_add_pad(a_bin , public_pad) ;

DEBUG_TRACE_ADD_GHOST_PAD

  return !is_err ;
}

GstPad* Gstreamer::NewStaticSinkPad(GstElement* an_element)
{
  return NewStaticPad(an_element , "sink") ;
}

GstPad* Gstreamer::NewStaticSrcPad(GstElement* an_element)
{
  return NewStaticPad(an_element , "src") ;
}

GstPad* Gstreamer::NewStaticPad(GstElement* an_element , String template_id)
{
  const gchar*  private_id = UTF8(template_id) ;
  GstPad*       private_pad ;

  bool is_err = !(private_pad = gst_element_get_static_pad(an_element , private_id)) ;

DEBUG_TRACE_GET_STATIC_PAD

  return private_pad ;
}

GstPad* Gstreamer::NewRequestSinkPad(GstElement* an_element)
{
  return NewRequestPad(an_element , "sink_%u") ;
}

GstPad* Gstreamer::NewRequestSrcPad(GstElement* an_element)
{
  return NewRequestPad(an_element , "src_%u") ;
}

GstPad* Gstreamer::NewRequestPad(GstElement* an_element , String template_id)
{
  const gchar*  private_id = UTF8(template_id) ;
  GstPad*       private_pad ;

  bool is_err = !(private_pad = gst_element_get_request_pad(an_element , private_id)) ;

DEBUG_TRACE_GET_REQUEST_PAD

  return private_pad ;
}

bool Gstreamer::IsInPipeline(GstElement* an_element)
{
  return !!an_element && GST_ELEMENT_PARENT(an_element) == Pipeline ;
}

void Gstreamer::DestroyElement(GstElement* an_element)
{
  if (an_element != nullptr) gst_object_unref(an_element) ;
}

String Gstreamer::MakeLctvUrl(String dest)
{
  return GST::LCTV_RTMP_URL                                            +
         dest.fromFirstOccurrenceOf(GST::LCTV_RTMP_URL , false , true) +
         dest.upToLastOccurrenceOf (" live=1"          , false , true) +
         " live=1"                                                     ;
}
