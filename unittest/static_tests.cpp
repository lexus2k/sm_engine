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

#include <CppUTest/TestHarness.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <thread>

#include "sme/engine.h"
#include "sme/generic_state.h"
#include "sme/generic_state_engine.h"

TEST_GROUP(ST)
{
    void setup()
    {
        // ...
    }

    void teardown()
    {
        // ...
    }
};

// ------------------------------- SIMPLE

enum
{
    EVENT_1,
    EVENT_2,
    EVENT_3,
};

enum
{
    STATE_1,
    STATE_2,
    STATE_3
};

static C_TRANSITION_TBL(masterTable)
{
    FROM_STATE(STATE_1) TRANSITION_SWITCH(EVENT_1, 1, sme::NO_FUNC, STATE_2)
    FROM_STATE(STATE_2)
    {
        TRANSITION_SWITCH(EVENT_2, 1, sme::NO_FUNC, STATE_1)
        TRANSITION_SWITCH(EVENT_3, 1, sme::NO_FUNC, STATE_3)
    }
    TRANSITION_TBL_END
}

static C_TRANSITION_TBL(state1Table)
{
    // FROM_STATE macro is not required here, since it is known state: state1
/*    TRANSITION_SIMPLE(event_id, func, dest_id)
    TRANSITION(event_id, event_arg, func, type, dest_id) */
    TRANSITION_PUSH  (EVENT_2, 2, sme::NO_FUNC, STATE_2)
    TRANSITION_SWITCH(EVENT_2, 0, sme::NO_FUNC, STATE_2)
    TRANSITION_SWITCH(EVENT_3, 0, sme::NO_FUNC, STATE_3)
    TRANSITION_SIMPLE(EVENT_3, sme::NO_FUNC, STATE_3)
    TRANSITION_TBL_END
}


static C_TRANSITION_TBL(state2Table)
{
    TRANSITION_POP(EVENT_1, 2, sme::NO_FUNC)
    TRANSITION_PUSH(EVENT_3, 2, sme::NO_FUNC, STATE_3)
    TRANSITION_SWITCH(EVENT_1, 0, sme::NO_FUNC, STATE_2)
    TRANSITION_SWITCH(EVENT_3, 0, sme::NO_FUNC, STATE_3)
    TRANSITION_TBL_END
}

static C_TRANSITION_TBL(state3Table)
{
    TRANSITION_POP(EVENT_2, 2, sme::NO_FUNC)
    TRANSITION_SWITCH(EVENT_1, 0, sme::NO_FUNC, STATE_1)
    TRANSITION_SWITCH(EVENT_2, 0, sme::NO_FUNC, STATE_2)
    TRANSITION_SIMPLE(EVENT_2, sme::NO_FUNC, STATE_2)
    TRANSITION_TBL_END
}

void state1_do_work()
{
}

TEST(ST, checkPushPop)
{
    GenericState<sme::NO_ENTER, state1_do_work, sme::NO_EXIT, state1Table> state1(STATE_1);
    GenericState<sme::NO_ENTER, sme::NO_UPDATE, sme::NO_EXIT, state2Table> state2(STATE_2);
    GenericState<sme::NO_ENTER, sme::NO_UPDATE, sme::NO_EXIT, state3Table> state3(STATE_3);
    SmStateInfo statesList[] =
    {
        STATE_LIST_ITEM(state1),
        STATE_LIST_ITEM(state2),
        STATE_LIST_ITEM(state3),
        STATE_LIST_END,
    };
    GenericStateEngine<masterTable> sm(statesList);

    sm.begin(STATE_1);
    sm.sendEvent( { EVENT_1, 2 } );
    sm.update();
    CHECK_EQUAL( STATE_1, sm.getActiveId() );
    sm.sendEvent( { EVENT_2, 2 } );
    sm.update();
    sm.update();
    CHECK_EQUAL( STATE_2, sm.getActiveId() );
    sm.sendEvent( { EVENT_3, 2 } );
    sm.update();
    CHECK_EQUAL( STATE_3, sm.getActiveId() );
    sm.sendEvent( { EVENT_2, 2 } );
    sm.update();
    CHECK_EQUAL( STATE_2, sm.getActiveId() );
    sm.sendEvent( { EVENT_1, 2 } );
    sm.update();
    CHECK_EQUAL( STATE_1, sm.getActiveId() );
    sm.end();
}

TEST(ST, checkSwitch)
{
    GenericState<sme::NO_ENTER, state1_do_work, sme::NO_EXIT, state1Table> state1(STATE_1);
    GenericState<sme::NO_ENTER, sme::NO_UPDATE, sme::NO_EXIT, state2Table> state2(STATE_2);
    GenericState<sme::NO_ENTER, sme::NO_UPDATE, sme::NO_EXIT, state3Table> state3(STATE_3);
    SmStateInfo statesList[] =
    {
        STATE_LIST_ITEM(state1),
        STATE_LIST_ITEM(state2),
        STATE_LIST_ITEM(state3),
        STATE_LIST_END,
    };
    GenericStateEngine<masterTable> sm(statesList);

    sm.begin(STATE_1);
    sm.sendEvent( { EVENT_1, 0 } );
    sm.update();
    CHECK_EQUAL( STATE_1, sm.getActiveId() );
    sm.sendEvent( { EVENT_2, 0 } );
    sm.update();
    CHECK_EQUAL( STATE_2, sm.getActiveId() );
    sm.sendEvent( { EVENT_3, 0 } );
    sm.update();
    CHECK_EQUAL( STATE_3, sm.getActiveId() );
    sm.sendEvent( { EVENT_1, 0 } );
    sm.update();
    CHECK_EQUAL( STATE_1, sm.getActiveId() );
    sm.sendEvent( { EVENT_3, 0 } );
    sm.update();
    CHECK_EQUAL( STATE_3, sm.getActiveId() );
    sm.sendEvent( { EVENT_2, 0 } );
    sm.update();
    CHECK_EQUAL( STATE_2, sm.getActiveId() );
    sm.end();
}

TEST(ST, checkSimple)
{
    GenericState<sme::NO_ENTER, state1_do_work, sme::NO_EXIT, state1Table> state1(STATE_1);
    GenericState<sme::NO_ENTER, sme::NO_UPDATE, sme::NO_EXIT, state2Table> state2(STATE_2);
    GenericState<sme::NO_ENTER, sme::NO_UPDATE, sme::NO_EXIT, state3Table> state3(STATE_3);
    SmStateInfo statesList[] =
    {
        STATE_LIST_ITEM(state1),
        STATE_LIST_ITEM(state2),
        STATE_LIST_ITEM(state3),
        STATE_LIST_END,
    };
    GenericStateEngine<masterTable> sm(statesList);

    sm.begin(STATE_1);
    sm.sendEvent( { EVENT_3, 500 } );
    sm.update();
    CHECK_EQUAL( STATE_3, sm.getActiveId() );
    sm.sendEvent( { EVENT_2, 300 } );
    sm.update();
    CHECK_EQUAL( STATE_2, sm.getActiveId() );
    sm.sendEvent( { EVENT_1, 300 } );
    sm.update();
    CHECK_EQUAL( STATE_2, sm.getActiveId() );
    sm.end();
}

TEST(ST, checkMasterTable)
{
    GenericState<sme::NO_ENTER, state1_do_work, sme::NO_EXIT, state1Table> state1(STATE_1);
    GenericState<sme::NO_ENTER, sme::NO_UPDATE, sme::NO_EXIT, state2Table> state2(STATE_2);
    GenericState<sme::NO_ENTER, sme::NO_UPDATE, sme::NO_EXIT, state3Table> state3(STATE_3);
    SmStateInfo statesList[] =
    {
        STATE_LIST_ITEM(state1),
        STATE_LIST_ITEM(state2),
        STATE_LIST_ITEM(state3),
        STATE_LIST_END,
    };
    GenericStateEngine<masterTable> sm(statesList);

    sm.begin(STATE_1);
    sm.sendEvent( { EVENT_1, 1 } );
    sm.update();
    CHECK_EQUAL( STATE_2, sm.getActiveId() );
    sm.sendEvent( { EVENT_2, 1 } );
    sm.update();
    CHECK_EQUAL( STATE_1, sm.getActiveId() );
    sm.sendEvent( { EVENT_1, 1 } );
    sm.sendEvent( { EVENT_3, 1 } );
    sm.update();
    sm.update();
    CHECK_EQUAL( STATE_3, sm.getActiveId() );
    sm.end();
}
