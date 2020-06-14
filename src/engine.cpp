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

#include "sme/engine.h"
#include "sme/state.h"
#include "sm_engine_logger.h"
#if SM_ENGINE_USE_STL
#include <chrono>
#endif

#define MAX_APP_QUEUE_SIZE   10

static const char* TAG = "SME";

SmEngine::~SmEngine()
{
    m_states.clear();
}

void SmEngine::addState(ISmeState &state)
{
    registerState( state, false );
}

void SmEngine::registerState(ISmeState &state, bool autoAllocated)
{
    SmStateInfo info =
    {
        .state = &state,
#if SM_ENGINE_USE_STL
        .autoAllocated = autoAllocated
#endif
    };
    state.setParent( this );
    if ( m_states.size() == 0 )
        m_states.push_back( info );
    else
        m_states.back() = info;
    SmStateInfo end =
    {
        nullptr,
#if SM_ENGINE_USE_STL
        false
#endif
    };
    m_states.push_back( end );
    setStates( &m_states[0] );
}
