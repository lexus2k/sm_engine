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
#include "../sme/state_uid.h"
#include "../sme/event.h"
#include "../sme/transition.h"

class ISmeState;

typedef struct
{
    ISmeState *state;
#if SM_ENGINE_USE_STL
    bool auto_allocated;
#endif
} SmStateInfo;

#define STATE_LIST_ITEM(x) { &x }

#define STATE_LIST_END { nullptr }

class ISmeState
{
public:
    ISmeState(const char *name): m_name( name ) { }

    virtual ~ISmeState() = default;

    /**
     * State initialization (is called, when new state is being added to state machine)
     */
    virtual bool begin() { return true; }

    /**
     * State deinitialization (is called, when state machine is destroyed)
     */
    virtual void end() { }

    /**
     * enter function is called, when state is being activated
     */
    virtual void enter(SEventData *event) { }

    /**
     * update() is called in loop, when state is active
     */
    virtual void update() { }

    /**
     * exit function is called, when state is being deactivated.
     */
    virtual void exit(SEventData *event) { }

    /**
     * onEvent method is called, when new event arrives in active state
     */
    virtual STransitionData onEvent(SEventData event) { return { EEventResult::NOT_PROCESSED, SM_STATE_NONE }; }

    /**
     * Returns state id
     */
    StateUid getId() { return m_id; }

    /**
     * Returns state name
     */
    const char *getName() { return m_name; }

    /**
     * Sets state id. Id is changed only if it was not previously specified for the state.
     */
    void setId(StateUid id) { if ( m_id == SM_STATE_NONE ) m_id = id; }

    void setParent( ISmeState * parent ) { m_parent = parent; }

protected:

    /**
     * @brief sends event state machine event queue
     *
     * Sends event to state machine event queue
     *
     * @param event event to put to queue
     */
    virtual bool sendEvent(SEventData event) { return m_parent ? m_parent->sendEvent( event ) : false; }

    /**
     * Returns timestamp in microseconds, since system is up
     */
//    uint64_t get_micros();

    /**
     * Returns true, when timeout takes place, and sends timeout event to queue
     * if generate_event is set to true
     * @param timeout timeout in microseconds
     * @param generate_event true if state machine timeout event should be generated
     * @return true if timeout, false otherwise
     */
    virtual bool timeoutEvent(uint64_t timeout, bool generate_event = false)
    {
        return m_parent ? m_parent->timeoutEvent( timeout, generate_event ) : false;
    }

    /**
     * Resets internal state timer
     */
    virtual void resetTimeout() { if (m_parent) m_parent->resetTimeout(); }

private:

    StateUid m_id = SM_STATE_NONE;

    const char * m_name = nullptr;

    ISmeState * m_parent = nullptr;
};

