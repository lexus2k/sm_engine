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
#include "../sme/iengine.h"
#include "../containers/stack.h"
#include "../containers/list.h"

#if SM_ENGINE_MULTITHREAD
#include <mutex>
#include <condition_variable>
#endif

#include <vector>
#include <stdint.h>

class SmEngine: public ISmEngine
{
public:
    SmEngine(int max_queue_size = 10)
        : ISmEngine( &m_states[0], max_queue_size )
    {}

    ~SmEngine();

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
    void add_state(StateUid id = SM_STATE_NONE)
    {
        T *p = new T();
        if ( id != SM_STATE_NONE )
        {
            p->setId( id );
        }
        register_state( *p, true );
    }
#endif

private:
    std::vector<SmStateInfo> m_states{};

    void register_state(ISmeState &state, bool auto_allocated);
};
