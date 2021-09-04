#include "Buttons.h"

Buttons::Buttons(DisplayManagement & display, Presets & presets, DDS & dds, Calibrate & calibrate): display(display), presets(presets), dds(dds), calibrate(calibrate) {}

/*****
  Purpose: To execute the Button1 Preset

  Argument list:
    void
  Return value:
    void
*****/
void Buttons::executeButton1() {
  whichBandOption = display.SelectBand();            // Select the band to be used
  submenuIndex = 0;
  presets.ProcessPresets(whichBandOption, submenuIndex);         // Select a preselected frequency
  menuIndex = FREQMENU;                         // When done, start over...
  //ShowMainDisplay(menuIndex, SWR);   TEMPORARILY COMMENTED
  //ShowSubmenuData(SWR);              TEMPORARILY COMMENTED
  dds.SendFrequency(currentFrequency);
  //SWR = ReadNewSWRValue();           TEMPORARILY COMMENTED
  //EraseBelowMenu();                  TEMPORARILY COMMENTED

}

/*****
  Purpose: To execute the Button3 Preset Select Menu Option

  Argument list:
    void
  Return value:
    void
*****/
void Buttons::executeButton3() {
  calibrate.DoNewCalibrate2();
}

/******
  Purpose: The ISR to detect the QuickCal Button push

  Parameter list:
    void

  Return value:
    void
*****/
void Buttons::quickCalISR() {
  quickCalFlag = 1;
}
