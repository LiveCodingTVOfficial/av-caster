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


#ifndef _PRESETS_H_
#define _PRESETS_H_

//[Headers]     -- You can add your own extra header files here --

#include "JuceHeader.h"


class MainContent ;

//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
  Presets is the preset management GUI for the AvCaster application.
  It has controls for adding, removing, and selecting preset stream configurations.
                                                                    //[/Comments]
*/
class Presets  : public Component,
                 public ButtonListener,
                 public ComboBoxListener
{
public:
    //==============================================================================
    Presets (MainContent* main_content);
    ~Presets();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.

  void broughtToFront() override ;

  void handleNewButton     () ;
  void handleDeleteButton  () ;
  bool handleConfigButton  () ;
  void handlePresetsCombo  () ;
  void toggleControls      () ;
  void setCreatePresetMode (bool is_pending_new_preset_name) ;
  bool isCreatePresetMode  () ;
  bool createOrUpdatePreset() ;

    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.

  MainContent* mainContent ;

    //[/UserVariables]

    //==============================================================================
    ScopedPointer<GroupComponent> presetsGroup;
    ScopedPointer<TextButton> newPresetButton;
    ScopedPointer<TextButton> deletePresetButton;
    ScopedPointer<Label> presetLabel;
    ScopedPointer<ComboBox> presetsCombo;
    ScopedPointer<ImageButton> configButton;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Presets)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // _PRESETS_H_
