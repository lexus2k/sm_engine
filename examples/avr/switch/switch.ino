/*
BSD 3-Clause License

Copyright (c) 2020, Aleksei Dynda
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "sme_engine.h"
#include "sme/engine.h"
#include "sme/generic_state.h"
#include "sme/generic_state_engine.h"
#include "Arduino.h"

// Here are our events
enum
{
    EVENT_BUTTON_PRESS,
};

// This is state unique ids
enum
{
    STATE_ON,
    STATE_OFF,
};

// Transition table for off state
C_TRANSITION_TBL(stateOffTable)
{
    TRANSITION_SWITCH(EVENT_BUTTON_PRESS, SM_EVENT_ARG_ANY, sme::NO_FUNC, STATE_ON)
    TRANSITION_TBL_END
}

// Transition table for on state
C_TRANSITION_TBL(stateOnTable)
{
    TRANSITION_SWITCH(EVENT_BUTTON_PRESS, SM_EVENT_ARG_ANY, sme::NO_FUNC, STATE_OFF)
    TRANSITION_TBL_END
}

void enterOnState(SEventData *event)
{
    Serial.println( "Switch is turned On" );
}

void enterOffState(SEventData *event)
{
    Serial.println( "Switch is turned Off" );
}


// Create switch states
GenericState<enterOffState, sme::NO_UPDATE, sme::NO_EXIT, stateOffTable> stateOff(STATE_OFF);
GenericState<enterOnState,  sme::NO_UPDATE, sme::NO_EXIT, stateOnTable> stateOn(STATE_ON);

SmStateInfo statesList[] =
{
    STATE_LIST_ITEM(stateOff),
    STATE_LIST_ITEM(stateOn),
    STATE_LIST_END,
};

GenericStateEngine<sme::NO_TABLE> switchSm(statesList);

void setup()
{
    Serial.begin( 115200 );
    // This will call begin method for all registered states
    // and will call enter() function for state OFF
    switchSm.begin(STATE_OFF);
    // This code runs in single thread, so do not block if there no events to process
    switchSm.setWaitEventTimeout( 0 );
}

void loop()
{
    // update() method will run single iteration/
    // it will call update() method of active state
    switchSm.update( );

    static unsigned long lastTime = millis();
    // Send button press event every 1 second
    if ( static_cast<unsigned long>( millis() - lastTime ) >= 1000 )
    {
         switchSm.sendEvent( { EVENT_BUTTON_PRESS, 0 } );
         lastTime = millis();
    }
}


