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
/*
    FROM_STATE(source_id)
    {
        TRANSITION_PUSH(source_id, event_id, event_arg, func, dest_id)
        TRANSITION_POP(source_id, event_id, event_arg, func)
        TRANSITION(source_id, event_id, event_arg, func, type, dest_id)
    } */
    TRANSITION_TBL_END
}

static C_TRANSITION_TBL(state1Table)
{
    // FROM_STATE macro is not required here, since it is known state: state1
/*    TRANSITION_SIMPLE(event_id, func, dest_id)
    TRANSITION_PUSH(event_id, event_arg, func, dest_id)
    TRANSITION_POP(event_id, event_arg, func)
    TRANSITION(event_id, event_arg, func, type, dest_id) */
    TRANSITION_PUSH(EVENT_1, 0, sme::NO_FUNC, STATE_2)
    TRANSITION_TBL_END
}


static C_TRANSITION_TBL(state2Table)
{
    TRANSITION_POP(EVENT_2, 0, sme::NO_FUNC)
    TRANSITION_TBL_END
}

void state1_do_work()
{
}

TEST(ST, Simple)
{
    GenericState<sme::NO_ENTER, state1_do_work, sme::NO_EXIT, state1Table> state1(STATE_1);
    GenericState<sme::NO_ENTER, sme::NO_UPDATE, sme::NO_EXIT, state2Table> state2(STATE_2);
    SmStateInfo statesList[] =
    {
        STATE_LIST_ITEM(state1),
        STATE_LIST_ITEM(state2),
        STATE_LIST_END,
    };
    GenericStateEngine<masterTable> sm(statesList);

    sm.begin(STATE_1);
    sm.sendEvent( { EVENT_1, 0 } );
    sm.update();
    CHECK_EQUAL( STATE_2, sm.getId() );
    sm.update();
    CHECK_EQUAL( STATE_2, sm.getId() );
    sm.sendEvent( { EVENT_2, 0 } );
    sm.update();
    CHECK_EQUAL( STATE_1, sm.getId() );
    sm.end();
}






/*
    // sent 200 small packets
    for (nsent = 0; nsent < 200; nsent++)
    {
        uint8_t      txbuf[4] = { 0xAA, 0xFF, 0xCC, 0x66 };
        int result = helper2.send( txbuf, sizeof(txbuf) );
        CHECK_EQUAL( TINY_SUCCESS, result );
    }
    // wait until last frame arrives
    helper1.wait_until_rx_count( 200, 1000 );
    CHECK_EQUAL( 200, helper1.rx_count() );

TEST(FD, arduino_to_pc)
{
    std::atomic<int> low_device_frames{};
    FakeConnection conn( 4096, 32 ); // PC side has larger UART buffer: 4096
    TinyHelperFd pc( &conn.endpoint1(), 4096, nullptr );
    TinyHelperFd arduino( &conn.endpoint2(), tiny_fd_buffer_size_by_mtu(64,4),
                          [&arduino, &low_device_frames](uint16_t,uint8_t*b,int s)->void
                          { if ( arduino.send(b, s) == TINY_ERR_TIMEOUT ) low_device_frames++; }, 4, 0 );
    conn.endpoint2().setTimeout( 0 );
    conn.endpoint2().disable();
    conn.setSpeed( 115200 );
    pc.run(true);
    pc.send( 100, "Generated frame. test in progress" );
    // Usually arduino starts later by 2 seconds due to reboot on UART-2-USB access, emulate al teast 100ms delay
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    conn.endpoint2().enable();
    // sent 200 small packets
    auto startTs = std::chrono::steady_clock::now();
    do
    {
        arduino.run_rx();
        arduino.run_tx();
    } while ( pc.rx_count() + low_device_frames != 100 && std::chrono::steady_clock::now() - startTs <= std::chrono::seconds(4) );
    // Not lost bytes check for now, we admit, that FD hdlc can restransmit frames on bad lines
//    CHECK_EQUAL( 0, conn.lostBytes() );
    CHECK_EQUAL( 100 - low_device_frames, pc.rx_count() );
}

TEST(FD, errors_on_tx_line)
{
    FakeConnection conn;
    uint16_t     nsent = 0;
    TinyHelperFd helper1( &conn.endpoint1(), 4096, nullptr );
    TinyHelperFd helper2( &conn.endpoint2(), 4096, nullptr );
    conn.line2().generate_error_every_n_byte( 200 );
    helper1.run(true);
    helper2.run(true);

    for (nsent = 0; nsent < 200; nsent++)
    {
        uint8_t      txbuf[4] = { 0xAA, 0xFF, 0xCC, 0x66 };
        int result = helper2.send( txbuf, sizeof(txbuf) );
        CHECK_EQUAL( TINY_SUCCESS, result );
    }
    // wait until last frame arrives
    helper1.wait_until_rx_count( 200, 1000 );
    CHECK_EQUAL( 200, helper1.rx_count() );
}

TEST(FD, error_on_single_I_send)
{
    // Each U-frame or S-frame is 6 bytes or more: 7F, ADDR, CTL, FSC16, 7F
    // TX1: U, U, R
    // TX2: U, U, I,
    FakeConnection conn;
    uint16_t     nsent = 0;
    TinyHelperFd helper1( &conn.endpoint1(), 4096, nullptr, 7, 2000 );
    TinyHelperFd helper2( &conn.endpoint2(), 4096, nullptr, 7, 2000 );
    conn.line2().generate_single_error( 6 + 6 + 3 ); // Put error on I-frame
    helper1.run(true);
    helper2.run(true);

    for (nsent = 0; nsent < 1; nsent++)
    {
        uint8_t      txbuf[4] = { 0xAA, 0xFF, 0xCC, 0x66 };
        int result = helper2.send( txbuf, sizeof(txbuf) );
        CHECK_EQUAL( TINY_SUCCESS, result );
    }
    // wait until last frame arrives
    helper1.wait_until_rx_count( 1, 2000 );
    CHECK_EQUAL( 1, helper1.rx_count() );
}

TEST(FD, error_on_rej)
{
    // Each U-frame or S-frame is 6 bytes or more: 7F, ADDR, CTL, FSC16, 7F
    // TX1: U, U, R
    // TX2: U, U, I,
    FakeConnection conn;
    uint16_t     nsent = 0;
    TinyHelperFd helper1( &conn.endpoint1(), 4096, nullptr, 7, 2000 );
    TinyHelperFd helper2( &conn.endpoint2(), 4096, nullptr, 7, 2000 );
    conn.line2().generate_single_error( 6 + 6 + 4 ); // Put error on first I-frame
    conn.line1().generate_single_error( 6 + 6 + 3 ); // Put error on S-frame REJ
    helper1.run(true);
    helper2.run(true);

    for (nsent = 0; nsent < 2; nsent++)
    {
        uint8_t      txbuf[4] = { 0xAA, 0xFF, 0xCC, 0x66 };
        int result = helper2.send( txbuf, sizeof(txbuf) );
        CHECK_EQUAL( TINY_SUCCESS, result );
    }
    // wait until last frame arrives
    helper1.wait_until_rx_count( 2, 2000 );
    CHECK_EQUAL( 2, helper1.rx_count() );
}

TEST(FD, no_ka_switch_to_disconnected)
{
    FakeConnection conn(32, 32);
    TinyHelperFd helper1( &conn.endpoint1(), 4096, nullptr, 4, 100 );
    TinyHelperFd helper2( &conn.endpoint2(), 4096, nullptr, 4, 100 );
    conn.endpoint1().setTimeout( 30 );
    conn.endpoint2().setTimeout( 30 );
    helper1.set_ka_timeout( 100 );
    helper2.set_ka_timeout( 100 );
    helper1.run(true);
    helper2.run(true);

    // Consider FD use keep alive to keep connection during 50 milliseconds
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    // Stop remote side, and sleep again for 150 milliseconds, to get disconnected state
    helper2.stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    // Stop both endpoints to do clean flush
    helper1.stop();
    conn.endpoint1().flush();
    conn.endpoint2().flush();
    // Run local endpoint again.
    helper1.run(true);
    // At this step, we should be in disconnected state, and should see SABM frames
    // Sleep for 100 milliseconds to get at least one Keep Alive
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    uint8_t buffer[32];
    int len = conn.endpoint2().read( buffer, sizeof(buffer) );
    const uint8_t sabm_request[] = { 0x7E, 0xFF, 0x3F, 0xF3, 0x39, 0x7E };
    if ( (size_t)len < sizeof(sabm_request) )
    {
         CHECK_EQUAL( sizeof(sabm_request), len );
    }
    MEMCMP_EQUAL( sabm_request, buffer, sizeof(sabm_request) );
}

TEST(FD, resend_timeout)
{
    FakeConnection conn(128, 128);
    TinyHelperFd helper1( &conn.endpoint1(), 4096, nullptr, 4, 70 );
    TinyHelperFd helper2( &conn.endpoint2(), 4096, nullptr, 4, 70 );
    conn.endpoint1().setTimeout( 30 );
    conn.endpoint2().setTimeout( 30 );
    helper1.run(true);
    helper2.run(true);

    // Consider FD use keep alive to keep connection during 50 milliseconds
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    // Stop remote side, and try to send something
    helper2.stop();
    conn.endpoint1().flush();
    conn.endpoint2().flush();
    helper1.send("#");
    std::this_thread::sleep_for(std::chrono::milliseconds(70*2 + 100));
    helper1.stop();
    const uint8_t reconnect_dat[] = { 0x7E, 0xFF, 0x00, '#', 0xA6, 0x13, 0x7E, // 1-st attempt
                                      0x7E, 0xFF, 0x00, '#', 0xA6, 0x13, 0x7E, // 2-nd attempt (1st retry)
                                      0x7E, 0xFF, 0x00, '#', 0xA6, 0x13, 0x7E, // 3-rd attempt (2nd retry)
                                      0x7E, 0xFF, 0x3F, 0xF3, 0x39, 0x7E }; // Attempt to reconnect (SABM)
    uint8_t buffer[64]{};
    conn.endpoint2().read( buffer, sizeof(buffer) );
    MEMCMP_EQUAL( reconnect_dat, buffer, sizeof(reconnect_dat) );
}

TEST(FD, singlethread_basic)
{
    // TODO:
    CHECK_EQUAL( 0, 0 );
}
*/
