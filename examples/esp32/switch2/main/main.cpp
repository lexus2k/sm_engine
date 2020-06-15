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

#include <stdio.h>
#include "sme/engine.h"

#include <thread>
#include <chrono>

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

class StateOff: public SmState
{
public:
    StateOff(): SmState("off") {}

    void enter(SEventData *event) override
    {
        fprintf( stderr, "Switch is turned Off\n" );
    }

    // Transition table for off state
    STransitionData onEvent(SEventData event) override
    {
        TRANSITION_SWITCH(EVENT_BUTTON_PRESS, SM_EVENT_ARG_ANY, sme::NO_FUNC, STATE_ON)
        TRANSITION_TBL_END
    }
};

class StateOn: public SmState
{
public:
    StateOn(): SmState("on") {}

    void enter(SEventData *event) override
    {
        fprintf( stderr, "Switch is turned On\n" );
    }

    // Transition table for on state
    STransitionData onEvent(SEventData event) override
    {
        TRANSITION_SWITCH(EVENT_BUTTON_PRESS, SM_EVENT_ARG_ANY, sme::NO_FUNC, STATE_OFF)
        TRANSITION_TBL_END
    }
};

class SwitchFsm: public SmEngine
{
public:
     SwitchFsm(): SmEngine()
     {
         SM_STATE( StateOff, STATE_OFF );
         SM_STATE( StateOn,  STATE_ON );
     }
};

extern "C" void app_main()
{
    SwitchFsm switchSm;

    std::thread timer([&]() {
        // Send 10 times EVENT_BUTTON_PRESS and then stop state machine
        for (int i=0; i<10; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            switchSm.sendEvent( { EVENT_BUTTON_PRESS, 0 } );
        }
        switchSm.stop();
    });

    // This will call begin method for all registered states
    // and will call enter() function for state OFF
    switchSm.begin(STATE_OFF);
    // run() method will run infinite loop until state machine is stopped
    // in this infinite loop, it will call update() method of active state every 100 milliseconds
    switchSm.loop( 100 );
    // This will call end methods for all registered states
    switchSm.end();
    timer.join();
}


