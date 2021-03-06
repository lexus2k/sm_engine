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

#include "sme/iengine.h"
#include "sme/state.h"
#include "sm_engine_logger.h"
#if SM_ENGINE_USE_STL
#include <chrono>
#endif

#define MAX_APP_QUEUE_SIZE   10

static const char* TAG = "SME";

ISmEngine::~ISmEngine()
{
    const SmStateInfo *state = m_states;
    while ( state->state != nullptr )
    {
#if SM_ENGINE_DYNAMIC_ALLOC
        if (state->autoAllocated)
        {
            delete state->state;
        }
#endif
        state++;
    }
}

bool ISmEngine::sendEvent(SEventData event)
{
    return sendEvent(event, 0);
}

bool ISmEngine::sendEvent(SEventData event, uint32_t ms)
{
    __SDeferredEventData ev =
    {
        .event = event,
        .micros = ms * 1000
    };
#if SM_ENGINE_MULTITHREAD
    std::unique_lock<std::mutex> lock(m_mutex);
#endif
    if ( m_events.size() >= MAX_APP_QUEUE_SIZE )
    {
        ESP_LOGE( TAG, "Failed to put new event: %02X", event.event );
        return false;
    }
    ESP_LOGI( TAG, "New event arrived: %02X", event.event );
    m_events.push_back( ev );
#if SM_ENGINE_MULTITHREAD
    m_cond.notify_one();
#endif
    return true;
}

void ISmEngine::loop(uint32_t eventWaitTimeoutMs)
{
    setWaitEventTimeout( eventWaitTimeoutMs );
    while (!m_stopped)
    {
        update();
    }
}

EEventResult ISmEngine::processAppEvent(SEventData &event)
{
    ESP_LOGD( TAG, "Processing event: %02X", event.event );
    STransitionData status = onEvent( event );
    if ( status.result == EEventResult::NOT_PROCESSED && m_active )
    {
        status = m_active->onEvent( event );
    }
    ESP_LOGD( TAG, "Processing result 1: %02X", static_cast<uint8_t>(status.result) );
    if ( status.result == EEventResult::NOT_PROCESSED )
    {
        ESP_LOGW(TAG, "Event is not processed: %i, %X",
                 event.event, event.arg );
    }
    else
    {
        switch ( status.result )
        {
            case EEventResult::SWITCH_STATE: switchState( status.stateId, &event ); break;
            case EEventResult::PUSH_STATE: pushState( status.stateId, &event ); break;
            case EEventResult::POP_STATE: popState( &event ); break;
            default: break;
        };
    }
    return status.result;
}

void ISmEngine::waitForNextEvent()
{
#if SM_ENGINE_MULTITHREAD
    std::unique_lock<std::mutex> lock( m_mutex );
    m_cond.wait_for( lock, std::chrono::milliseconds( m_eventWaitTimeoutMs ),
                     [this]()->bool{ return m_events.size() > 0; } );
#endif
}


void ISmEngine::update()
{
    onUpdate();

    waitForNextEvent();

    uint32_t ts = getMicros();
    uint32_t delta = static_cast<uint32_t>( ts - m_lastUpdateTimeMs );
    m_lastUpdateTimeMs = ts;

    auto it = m_events.begin();
    while ( it != m_events.end() )
    {
        if ( it->micros <= delta )
        {
            processAppEvent( it->event );
#if SM_ENGINE_MULTITHREAD
            std::unique_lock<std::mutex> lock( m_mutex );
#endif
            it = m_events.erase( it );
        }
        else
        {
            it->micros -= delta;
            it++;
        }
    }
    if (m_active)
        m_active->update();
    else
        ESP_LOGE(TAG, "Initial state is not specified!");
}

STransitionData ISmEngine::onEvent(SEventData event)
{
    return { EEventResult::NOT_PROCESSED, SM_STATE_ANY };
}

void ISmEngine::onUpdate()
{
}

bool ISmEngine::begin()
{
    bool result = onBegin();
    if ( result )
    {
        const SmStateInfo * state = m_states;
        while ( state->state != nullptr )
        {
            result = state->state->begin();
            if ( !result )
            {
                break;
            }
            state++;
        }
    }
    m_lastUpdateTimeMs = getMicros();
    m_stopped = false;
    return result;
}

bool ISmEngine::begin( StateUid id )
{
    bool result = begin();
    if ( result )
    {
        result = switchState( id, nullptr );
    }
    return result;
}

bool ISmEngine::onBegin()
{
    return true;
}

void ISmEngine::end()
{
    if (m_active)
    {
        m_active->exit( nullptr );
    }
    const SmStateInfo * state = m_states;
    while ( state->state != nullptr )
    {
        state->state->end();
        state++;
    };
    onEnd();
}

void ISmEngine::onEnd()
{
}

ISmeState *ISmEngine::getById(StateUid id)
{
    const SmStateInfo * state = m_states;
    while ( state->state != nullptr )
    {
        if ( state->state->getId() == id )
        {
            return state->state;
        }
        state++;
    };
    return nullptr;
}

bool ISmEngine::switchState(StateUid id, SEventData *event)
{
    if ( id == SM_STATE_NONE )
    {
        return true;
    }
    ISmeState * newState = getById( id );
    if ( newState )
    {
        if ( m_active )
        {
            if ( m_active->getId() == id )
            {
                return false;
            }
            m_active->exit(event);
        }
        ESP_LOGI(TAG, "Switching to state %s", newState->getName());
        m_active = newState;

        m_stateStartTs = getMicros();
        m_active->enter( event );
        m_activeId = id;
        return true;
    }
    ESP_LOGE(TAG, "Switching to state 0x%02X failed, state not found", id);
    return false;
}

bool ISmEngine::pushState(StateUid newState, SEventData *event)
{
    m_stack.push(m_active);
    bool result = switchState(newState, event);
    if (!result)
    {
        m_stack.pop();
        ESP_LOGE(TAG, "Failed to push state: %02X", newState);
    }
    else
    {
        ESP_LOGI(TAG, "Push state successful: %02X", newState);
    }
    return result;
}

bool ISmEngine::popState(SEventData *event)
{
    bool result = false;
    if (!m_stack.empty())
    {
        auto state = m_stack.top();
        m_stack.pop();
        result = switchState( state->getId(), event );
        if (!result)
        {
            m_stack.push(state);
        }
    }
    else
    {
        ESP_LOGE(TAG, "Failed to pop state: stack is empty");
    }
    return result;
}

uint64_t ISmEngine::getMicros()
{
#if SM_ENGINE_USE_STL
    return std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()).time_since_epoch().count();
#else
    return 0;
#endif
}

bool ISmEngine::timeoutEvent(uint64_t timeout, bool generateEvent)
{
    bool event = static_cast<uint64_t>( getMicros() - m_stateStartTs ) >= timeout;
    if ( event && generateEvent )
    {
         sendEvent( { SM_EVENT_TIMEOUT, static_cast<uintptr_t>(timeout) } );
    }
    return event;
}

void ISmEngine::resetTimeout()
{
    m_stateStartTs = getMicros();
}


