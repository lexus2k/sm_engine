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

#include "sme_interface.h"
#include "sme_state.h"
#include "containers/stack.h"
#include "containers/list.h"

#if SM_ENGINE_MULTITHREAD
#include <mutex>
#include <condition_variable>
#endif

#include <stdint.h>

#define SM_STATE_ANY       0xFF

#define SM_FUNC_NONE

enum
{
    SM_PUSH = 0,
    SM_SWITCH,
    SM_POP,
    SM_NONE,
};

#define SM_STATE(state,id) add_state<state>(id)

/**
 * transition macro. if condition is successful, it exits on_event function immediately with 
 * EEventResult::PROCESSED_AND_HOOKED result.
 * @param source_id source state number, which event should be processed in. Can be equal to SM_STATE_NONE, which means
 *                  event will be processed in any state
 * @param event_id user event id to process
 * @param event_arg argument (uintptr_t) of user event id, can be equal to SM_EVENT_ARG_NONE, which means that events
 *                  with any argument values should be processed
 * @param func function or method to call with the list of arguments, can be SM_FUNC_NONE
 * @param type type of transition. Can be one of SM_NONE, SM_PUSH, SM_POP, SM_SWITCH which means the type of SME action
 *              switch will take place.
 * @param dest_id new state number, or SM_STATE_NONE to remain in the same state
 */
#define SM_TRANSITION(source_id, event_id, event_arg, func, type, dest_id) \
             if ( (source_id == SM_STATE_ANY || (source_id != SM_STATE_ANY && get_id() == source_id)) && \
                  (event.event == event_id && (event_arg == SM_EVENT_ARG_ANY || event_arg == event.arg)) ) \
             { \
                 if ( type == SM_POP ) { \
                     pop_state(); \
                 } else if ( type != SM_NONE && dest_id != SM_STATE_NONE ) { \
                     if ( type == SM_PUSH ) push_state( dest_id ); \
                     else if ( type == SM_SWITCH ) switch_state( dest_id ); \
                 } \
                 func; \
                 return EEventResult::PROCESSED_AND_HOOKED; \
             } \

#define SM_PUSH_TRANSITION(event_id, event_arg, func, dest_id) \
          SM_TRANSITION(SM_STATE_ANY, event_id, event_arg, func, SM_PUSH, dest_id)

#define SM_POP_TRANSITION(event_id, event_arg, func) \
          SM_TRANSITION(SM_STATE_ANY, event_id, event_arg, func, SM_POP, SM_STATE_NONE)

#define SM_NO_TRANSITION(event_id, event_arg, func) \
          SM_TRANSITION(SM_STATE_ANY, event_id, event_arg, func, SM_NONE, SM_STATE_NONE)

class SmEngine: public ISmeState
{
public:
    SmEngine(int max_queue_size = 10): ISmeState( "engine" ), m_max_event_queue_size( max_queue_size ) { }
    virtual ~SmEngine();

    /**
     * Calls begin() method of all state machine states
     */
    bool begin() override final;

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
     * @param event_wait_timeout_ms timeout to wait for incoming event. Do not use 0,
     *        since loop() will occupy 100% cpu.
     */
    void loop(uint32_t event_wait_timeout_ms);

    /**
     * @brief sets event wait timeout
     *
     * If no events in state machine engine queue, then sme will wait for specified time
     *
     * @param event_wait_timeout_ms event timeout in milliseconds
     */
    void set_wait_event_timeout( uint32_t event_wait_timeout_ms ) { m_event_wait_timeout_ms = event_wait_timeout_ms; }

    /**
     * @brief Runs single iteration of state machine.
     *
     * Runs single iteration of state machine and exits.
     *
     * @param event_wait_timeout_ms timeout to wait for incoming event. If 0, the function
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
    bool send_event(SEventData event) override final;

    /**
     * @brief sends event state machine event queue after ms timeout
     *
     * Sends event to state machine event queue after ms timeout
     *
     * @param event event to put to queue
     * @param ms timeout in milliseconds
     */
    bool send_delayed_event(SEventData event, uint32_t ms);

    /**
     * @brief change current state to new one
     *
     * Changes current state to new one. For current state method exit()
     * will be called, for new state method enter() will be called.
     *
     * @param new_state id of new state to switch to
     */
    bool switch_state(uint8_t new_state) override final;

    /**
     * @brief change current state to new one, but stores current state
     *
     * Changes current state to new one, but stores current state id.
     * For current state method exit() will be called, for new state method
     * enter() will be called. To return to stored state use pop_state() method.
     *
     * @param new_state id of new state to switch to
     */
    bool push_state(uint8_t new_state) override final;

    /**
     * @brief returns to last stored state.
     *
     * returns to last stored state.
     * @see push_state
     */
    bool pop_state() override final;

    /**
     * @brief registers new state in state machine memory
     *
     * Registers new state in state machine memory
     *
     * @param state reference to SmState-based object
     */
    void add_state(ISmeState &state);

#if SM_ENGINE_DYNAMIC_ALLOC
    /**
     * @brief registers new state in state machine memory
     *
     * Registers new state in state machine memory. SmState-based
     * object will be automatically allocated and freed by state machine
     *
     */
    template <class T>
    void add_state(uint8_t id = SM_STATE_NONE)
    {
        T *p = new T();
        if ( id != SM_STATE_NONE )
        {
            p->set_id( id );
        }
        register_state( *p, true );
    }
#endif

    /**
     * Terminates state machine. This causes loop() method to exit.
     */
    void stop() { m_stopped = true; }

    /**
     * Returns timestamp in microseconds
     */
    virtual uint64_t get_micros();

    /**
     * Returns true if timeout happens after entering new state
     * @param timeout timeout in microseconds
     * @param generate_event set to true if state machine timeout event needs to be generated
     */
    bool timeout_event(uint64_t timeout, bool generate_event = false) override final;

    /**
     * Reset timeout timer
     */
    void reset_timeout() override final;

    /**
     * Returns pointer to state object by id
     */
    ISmeState *getById(uint8_t id);

protected:

    /**
     * The method can be used to perform additional initialization
     * before any state objects starts to work
     */
    virtual bool on_begin();

    /**
     * The method can be used to hook all events for all states
     */
    virtual EEventResult on_event(SEventData event);

    /**
     * This method can be used to perform actions in all states.
     */
    virtual void on_update();

    /**
     * The method is called after all states are ended
     */
    virtual void on_end();

private:
    typedef struct
    {
        ISmeState *state;
#if SM_ENGINE_DYNAMIC_ALLOC
        bool auto_allocated;
#endif
    } SmStateInfo;

    ISmeState *m_active = nullptr;

#if SM_ENGINE_MULTITHREAD
    std::condition_variable m_cond{};
    std::mutex m_mutex{};
#endif


    sme::stack<ISmeState*> m_stack{};
    sme::list<__SDeferredEventData> m_events{};
    sme::list<SmStateInfo> m_states{};

    bool m_stopped = false;
    uint32_t m_last_update_time_ms = 0;
    uint64_t m_state_start_ts = 0;
    uint32_t m_event_wait_timeout_ms = 0;
    int m_max_event_queue_size = 10;

    EEventResult process_app_event(SEventData &event);

    void register_state(ISmeState &state, bool auto_allocated);

    void wait_for_next_event();
};
