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

#include "../sme/config.h"

#if SM_ENGINE_USE_STL

#include <vector>
namespace sme {

template <typename T>
using vector = std::vector<T>;

}
#else

namespace sme {

static constexpr int MAX_VECTOR_EL = 4;

template <typename T>
class vector
{
public:
    vector() {}

    void push_back( T &e ) { if ( m_ptr < MAX_VECTOR_EL) m_elem[m_ptr++] = e; }

    T* begin() { return &m_elem[0]; }

    T* end() { return &m_elem[m_ptr]; }

    void clear() { m_ptr = 0; }

    int size() { return m_ptr; }

    bool empty() { return m_ptr == 0; }

    void emplace_back( T &e ) { if ( m_ptr < MAX_VECTOR_EL) m_elem[m_ptr] = e; }

    T& operator[] (int n) { return m_elem[n]; }

    const T& operator[] (int n) const { return m_elem[n]; }

private:
    T m_elem[MAX_VECTOR_EL];
    int m_ptr = 0;
};

}
#endif
