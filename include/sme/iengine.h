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

#include "../sme/istate.h"
#include "../sme/state.h"
#include "../containers/stack.h"
#include "../containers/list.h"

#if SM_ENGINE_MULTITHREAD
#include <mutex>
#include <condition_variable>
#endif

#include <stdint.h>

#define SM_FUNC_NONE

#define SM_STATE(state,id) addState<state>(id)

class ISmEngine: public ISmeState
{
public:

    ISmEngine(const SmStateInfo *states, int max_queue_size = 10)
        : ISmeState( "engine" )
        , m_max_event_queue_size( max_queue_size )
        , m_states( states )
    {
    }

    ~ISmEngine();

    /**
     * Calls begin() method of all state machine states
     */
    bool begin() override final;

    bool begin(StateUid id);

    /**
     * Calls end() method of all state machine states
     */
    void end() override final;

    /**
     * @brief Starts infinite loop.
     *
     * Enters infinite loop and runs state machine. If you want
     * to do anything else outside state machine functions, please,
     * do not use this function. Refer to run() function instead.
     *
     * @param eventWaitTimeoutMs timeout to wait for incoming event. Do not use 0,
     *        since loop() will occupy 100% cpu.
     */
    void loop(uint32_t eventWaitTimeoutMs);

    /**
     * @brief sets event wait timeout
     *
     * If no events in state machine engine queue, then sme will wait for specified time
     *
     * @param eventWaitTimeoutMs event timeout in milliseconds
     */
    void setWaitEventTimeout( uint32_t eventWaitTimeoutMs ) { m_eventWaitTimeoutMs = eventWaitTimeoutMs; }

    /**
     * @brief Runs single iteration of state machine.
     *
     * Runs single iteration of state machine and exits.
     *
     * @param eventWaitTimeoutMs timeout to wait for incoming event. If 0, the function
     *        will return immediately.
     */
    void update() override final;

    /**
     * @brief sends event state machine event queue
     *
     * Sends event to state machine event queue
     *
     * @param event event to put to queue
     */
    bool sendEvent(SEventData event) override final;

    /**
     * @brief sends event state machine event queue after ms timeout
     *
     * Sends event to state machine event queue after ms timeout
     *
     * @param event event to put to queue
     * @param ms timeout in milliseconds
     */
    bool sendEvent(SEventData event, uint32_t ms);

    /**
     * Terminates state machine. This causes loop() method to exit.
     */
    void stop() { m_stopped = true; }

    /**
     * Returns timestamp in microseconds
     */
    virtual uint64_t getMicros();

    /**
     * Returns true if timeout happens after entering new state
     * @param timeout timeout in microseconds
     * @param generate_event set to true if state machine timeout event needs to be generated
     */
    bool timeoutEvent(uint64_t timeout, bool generate_event = false) override final;

    /**
     * Reset timeout timer
     */
    void resetTimeout() override final;

    /**
     * Returns pointer to state object by id
     */
    ISmeState *getById(StateUid id);


    StateUid getActiveId() { return m_activeId; }

protected:

    /**
     * The method can be used to perform additional initialization
     * before any state objects starts to work
     */
    virtual bool onBegin();

    /**
     * The method can be used to hook all events for all states
     */
    virtual STransitionData onEvent(SEventData event);

    /**
     * This method can be used to perform actions in all states.
     */
    virtual void onUpdate();

    /**
     * The method is called after all states are ended
     */
    virtual void onEnd();

private:
    ISmeState *m_active = nullptr;

#if SM_ENGINE_MULTITHREAD
    std::condition_variable m_cond{};
    std::mutex m_mutex{};
#endif

    int m_max_event_queue_size = 10;

    sme::stack<ISmeState*> m_stack{};
    sme::list<__SDeferredEventData> m_events{};
    const SmStateInfo *m_states = nullptr;

    bool m_stopped = false;
    uint32_t m_lastUpdateTimeMs = 0;
    uint64_t m_stateStartTs = 0;
    uint32_t m_eventWaitTimeoutMs = 0;
    StateUid m_activeId = SM_STATE_NONE;

    EEventResult processAppEvent(SEventData &event);

    void registerState(ISmeState &state, bool autoAllocated);

    void waitForNextEvent();

    /**
     * @brief change current state to new one
     *
     * Changes current state to new one. For current state method exit()
     * will be called, for new state method enter() will be called.
     *
     * @param newState id of new state to switch to
     */
    bool switchState(StateUid newState, SEventData *event);

    /**
     * @brief change current state to new one, but stores current state
     *
     * Changes current state to new one, but stores current state id.
     * For current state method exit() will be called, for new state method
     * enter() will be called. To return to stored state use pop_state() method.
     *
     * @param new_state id of new state to switch to
     */
    bool pushState(StateUid newState, SEventData *event);

    /**
     * @brief returns to last stored state.
     *
     * returns to last stored state.
     * @see pushState
     */
    bool popState(SEventData *event);
};
