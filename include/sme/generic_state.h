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

#include "../sme/state.h"
#include "../sme/event.h"
#include "../sme/state_uid.h"


typedef void (*TSmeFunction)(void);
typedef STransitionData (*TSmeTable)(StateUid, SEventData);

template <TSmeFunction enterFunc, TSmeFunction updateFunc, TSmeFunction exitFunc, TSmeTable table>
class GenericState: public SmState
{
public:
    GenericState(const char *name): SmState( name ) { }
    GenericState(StateUid uid, const char *name = nullptr): SmState( name ) { setId( uid ); }

private:
    void enter() override final { enterFunc(); }
    void update() override final { updateFunc(); }
    void exit() override final { exitFunc(); }
    STransitionData onEvent(SEventData event) override final { return table( getId(), event); }
};

namespace sme
{
    static inline void NO_ENTER() { }
    static inline void NO_UPDATE() { }
    static inline void NO_EXIT() { }
    static inline void NO_FUNC() { }
    static inline C_TRANSITION_TBL(NO_TABLE)
    {
        TRANSITION_TBL_END
    }
}
