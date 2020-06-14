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

#pragma once

#include <stdint.h>
#include "../sme/config.h"
#include "../sme/event.h"
#include "../sme/state_uid.h"

enum class EEventResult: uint8_t
{
    NOT_PROCESSED,
    PROCESSED_AND_HOOKED,
    SWITCH_STATE,
    POP_STATE,
    PUSH_STATE,
};

typedef struct
{
    EEventResult result;
    StateUid stateId;
} STransitionData;

#define C_TRANSITION_TBL(x) STransitionData  x(StateUid sid, SEventData event)

#define TRANSITION_TBL_END return { EEventResult::NOT_PROCESSED, SM_STATE_NONE };

#define FROM_STATE( source_id ) \
             if ( sid == source_id )


/**
 * transition macro. if condition is successful, it exits onEvent function immediately with
 * EEventResult::PROCESSED_AND_HOOKED result.
 * @param source_id source state number, which event should be processed in. Can be equal to SM_STATE_NONE, which means
 *                  event will be processed in any state
 * @param event_id user event id to process
 * @param event_arg argument (uintptr_t) of user event id, can be equal to SM_EVENT_ARG_NONE, which means that events
 *                  with any argument values should be processed
 * @param func function or method to call with the list of arguments, can be SM_FUNC_NONE
 * @param type type of event processing ( see EEventResult ).
 * @param dest_id new state number, or SM_STATE_NONE to remain in the same state
 */
#define TRANSITION(eventId, event_arg, func, type, dest_id) \
             if ( (event.event == eventId && ( event_arg == SM_EVENT_ARG_ANY || event_arg == event.arg)) ) \
             { \
                 func; \
                 return { type, dest_id }; \
             } \


#define TRANSITION_SIMPLE(event_id, func, dest_id) \
             TRANSITION(event_id, SM_EVENT_ARG_ANY, func, EEventResult::SWITCH_STATE, dest_id)

#define TRANSITION_PUSH(event_id, event_arg, func, dest_id) \
             TRANSITION(event_id, event_arg, func, EEventResult::PUSH_STATE, dest_id)

#define TRANSITION_SWITCH(event_id, event_arg, func, dest_id) \
             TRANSITION(event_id, event_arg, func, EEventResult::SWITCH_STATE, dest_id)

#define TRANSITION_POP(event_id, event_arg, func) \
             TRANSITION(event_id, event_arg, func, EEventResult::POP_STATE, SM_STATE_NONE)


#define NO_TRANSITION(event_id, event_arg, func) \
             TRANSITION(event_id, event_arg, func, EEventResult::PROCESSED_AND_HOOKED, SM_STATE_NONE )


namespace sme
{
    static inline void NO_FUNC() { }
}
