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


//[Headers] You can add your own extra header files here...
//[/Headers]

#include "ConfigCamera.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
ConfigCamera::ConfigCamera ()
{
    addAndMakeVisible (cameraDevLabel = new Label ("cameraDevLabel",
                                                   TRANS("Device:")));
    cameraDevLabel->setFont (Font (15.00f, Font::plain));
    cameraDevLabel->setJustificationType (Justification::centredLeft);
    cameraDevLabel->setEditable (false, false, false);
    cameraDevLabel->setColour (Label::textColourId, Colours::white);
    cameraDevLabel->setColour (TextEditor::textColourId, Colours::black);
    cameraDevLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (cameraDevCombo = new ComboBox ("cameraDevCombo"));
    cameraDevCombo->setExplicitFocusOrder (1);
    cameraDevCombo->setEditableText (false);
    cameraDevCombo->setJustificationType (Justification::centredLeft);
    cameraDevCombo->setTextWhenNothingSelected (TRANS("(no camera devices)"));
    cameraDevCombo->setTextWhenNoChoicesAvailable (TRANS("(no camera devices)"));
    cameraDevCombo->addListener (this);

    addAndMakeVisible (cameraResLabel = new Label ("cameraResLabel",
                                                   TRANS("Resolution:")));
    cameraResLabel->setFont (Font (15.00f, Font::plain));
    cameraResLabel->setJustificationType (Justification::centredLeft);
    cameraResLabel->setEditable (false, false, false);
    cameraResLabel->setColour (Label::textColourId, Colours::white);
    cameraResLabel->setColour (TextEditor::textColourId, Colours::black);
    cameraResLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (cameraResCombo = new ComboBox ("cameraResCombo"));
    cameraResCombo->setExplicitFocusOrder (2);
    cameraResCombo->setEditableText (false);
    cameraResCombo->setJustificationType (Justification::centredLeft);
    cameraResCombo->setTextWhenNothingSelected (String::empty);
    cameraResCombo->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    cameraResCombo->addListener (this);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (1, 1);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

ConfigCamera::~ConfigCamera()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    cameraDevLabel = nullptr;
    cameraDevCombo = nullptr;
    cameraResLabel = nullptr;
    cameraResCombo = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void ConfigCamera::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void ConfigCamera::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    cameraDevLabel->setBounds (0, 8, 80, 24);
    cameraDevCombo->setBounds (88, 8, 200, 24);
    cameraResLabel->setBounds (0, 48, 80, 24);
    cameraResCombo->setBounds (88, 48, 200, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void ConfigCamera::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == cameraDevCombo)
    {
        //[UserComboBoxCode_cameraDevCombo] -- add your combo box handling code here..
        //[/UserComboBoxCode_cameraDevCombo]
    }
    else if (comboBoxThatHasChanged == cameraResCombo)
    {
        //[UserComboBoxCode_cameraResCombo] -- add your combo box handling code here..
        //[/UserComboBoxCode_cameraResCombo]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="ConfigCamera" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="4" snapActive="0" snapShown="0" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="1" initialHeight="1">
  <BACKGROUND backgroundColour="0"/>
  <LABEL name="cameraDevLabel" id="b00161e3a7f27d06" memberName="cameraDevLabel"
         virtualName="" explicitFocusOrder="0" pos="0 8 80 24" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Device:" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <COMBOBOX name="cameraDevCombo" id="f143a9d8fad92dd2" memberName="cameraDevCombo"
            virtualName="" explicitFocusOrder="1" pos="88 8 200 24" editable="0"
            layout="33" items="" textWhenNonSelected="(no camera devices)"
            textWhenNoItems="(no camera devices)"/>
  <LABEL name="cameraResLabel" id="e2a00639ad344d6" memberName="cameraResLabel"
         virtualName="" explicitFocusOrder="0" pos="0 48 80 24" textCol="ffffffff"
         edTextCol="ff000000" edBkgCol="0" labelText="Resolution:" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <COMBOBOX name="cameraResCombo" id="bcc0c59e13c46f76" memberName="cameraResCombo"
            virtualName="" explicitFocusOrder="2" pos="88 48 200 24" editable="0"
            layout="33" items="" textWhenNonSelected="" textWhenNoItems="(no choices)"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
