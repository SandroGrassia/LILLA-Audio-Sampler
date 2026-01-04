/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once

#include <Arduino.h>
#include "config.h"
#include "UserInterface.h"

class Encoders
{
private:
    int8_t rotation[ENCODERS];
    enum State
    {
        a,
        b,
        c,
        d,
        e,
        f,
        g,
        x
    };
    State state[ENCODERS];
    struct NextStateAndRotation
    {
        State next_state;
        int rotation;
    };
    static constexpr NextStateAndRotation matrix[4][8] =
        {
            //  a       b       c       d       e       f       g       x         DT-CLK
            {{x, 0}, {c, 0}, {c, 0}, {c, 0}, {f, 0}, {f, 0}, {f, 0}, {x, 0}},  // 0 (00)
            {{e, 0}, {d, 0}, {d, 0}, {d, 0}, {e, 0}, {e, 0}, {e, 0}, {d, 0}},  // 1 (01)
            {{b, 0}, {b, 0}, {b, 0}, {b, 0}, {g, 0}, {g, 0}, {g, 0}, {g, 0}},  // 2 (10)
            {{a, 0}, {a, 0}, {a, 1}, {a, 1}, {a, 0}, {a, -1}, {a, -1}, {a, 0}} // 3 (11)
    };

    // Graph:
    // https://magjac.com/graphviz-visual-editor/?dot=digraph%7B%0Anode%20%5Bstyle%3Dfilled%20fontname%3D%22arial%22%20fontsize%3D24%20width%3D0%5D%3B%0Aedge%20%5Barrowhead%3Dopen%20fontsize%3D12%20alingment%3Dleft%20fontname%3D%22arial%20narrow%22%20fillcolor%3D%22%23ffffff%22%5D%3B%0A%20%20%20%0A%20%20%20%20a%20%5Bshape%3D%22circle%22%20style%3D%22filled%22%20fillcolor%3D%22%23ffffff%22%5D%0A%20%20%20%20b%20%5Bshape%3D%22circle%22%20style%3D%22filled%22%20fillcolor%3D%22%23ffffff%22%5D%0A%20%20%20%20c%20%5Bshape%3D%22circle%22%20style%3D%22filled%22%20fillcolor%3D%22%23ffffff%22%5D%0A%20%20%20%20d%20%5Bshape%3D%22circle%22%20style%3D%22filled%22%20fillcolor%3D%22%23ffffff%22%5D%0A%20%20%20%20e%20%5Bshape%3D%22circle%22%20style%3D%22filled%22%20fillcolor%3D%22%23ffffff%22%5D%0A%20%20%20%20f%20%5Bshape%3D%22circle%22%20style%3D%22filled%22%20fillcolor%3D%22%23ffffff%22%5D%0A%20%20%20%20g%20%5Bshape%3D%22circle%22%20style%3D%22filled%22%20fillcolor%3D%22%23ffffff%22%5D%0A%20%20%20%20x%20%5Bshape%3D%22circle%22%20style%3D%22filled%22%20fillcolor%3D%22%23ffffff%22%5D%0A%20%20%20%20%0A%20%20%20%20a%20-%3E%20x%20%5Blabel%20%3D%20%220%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20a%20-%3E%20e%20%5Blabel%20%3D%20%221%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20a%20-%3E%20b%20%5Blabel%20%3D%20%222%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20a%20-%3E%20a%20%5Blabel%20%3D%20%223%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20%0A%20%20%20%20b%20-%3E%20c%20%5Blabel%20%3D%20%220%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20b%20-%3E%20d%20%5Blabel%20%3D%20%221%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20b%20-%3E%20b%20%5Blabel%20%3D%20%222%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20b%20-%3E%20a%20%5Blabel%20%3D%20%223%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20%0A%20%20%20%20c%20-%3E%20c%20%5Blabel%20%3D%20%220%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20c%20-%3E%20d%20%5Blabel%20%3D%20%221%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20c%20-%3E%20b%20%5Blabel%20%3D%20%222%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20c%20-%3E%20a%20%5Blabel%20%3D%20%223%2F%2B1%22%20fillcolor%3D%22%23000000%22%20color%3D%22%230000FF%22%5D%0A%20%20%20%20%0A%20%20%20%20d%20-%3E%20c%20%5Blabel%20%3D%20%220%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20d%20-%3E%20d%20%5Blabel%20%3D%20%221%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20d%20-%3E%20b%20%5Blabel%20%3D%20%222%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20d%20-%3E%20a%20%5Blabel%20%3D%20%223%2F%2B1%22%20fillcolor%3D%22%230000FF%22%20color%3D%22%230000FF%22%5D%0A%20%20%20%20%0A%20%20%20%20e%20-%3E%20f%20%5Blabel%20%3D%20%220%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20e%20-%3E%20e%20%5Blabel%20%3D%20%221%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20e%20-%3E%20g%20%5Blabel%20%3D%20%222%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20e%20-%3E%20a%20%5Blabel%20%3D%20%223%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20%0A%20%20%20%20f%20-%3E%20f%20%5Blabel%20%3D%20%220%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20f%20-%3E%20e%20%5Blabel%20%3D%20%221%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20f%20-%3E%20g%20%5Blabel%20%3D%20%222%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20f%20-%3E%20a%20%5Blabel%20%3D%20%223%2F-1%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23FF0000%22%5D%0A%20%20%20%20%0A%20%20%20%20g%20-%3E%20f%20%5Blabel%20%3D%20%220%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20g%20-%3E%20e%20%5Blabel%20%3D%20%221%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20g%20-%3E%20g%20%5Blabel%20%3D%20%222%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20g%20-%3E%20a%20%5Blabel%20%3D%20%223%2F-1%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23FF0000%22%5D%0A%0A%20%20%20%20x%20-%3E%20x%20%5Blabel%20%3D%20%220%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20x%20-%3E%20d%20%5Blabel%20%3D%20%221%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20x%20-%3E%20g%20%5Blabel%20%3D%20%222%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%20%20%20%20x%20-%3E%20a%20%5Blabel%20%3D%20%223%22%20fillcolor%3D%22%23000000%22%20color%3D%22%23000000%22%5D%0A%7D
    
public:
    Encoders()
    {
        Reset();
    }

    void Transmit_DT_CLK(const uint8_t& encoder, const uint8_t& DT, const uint8_t& CLK);
    int Get_state(const uint8_t& encoder);
    int Get_rotation(const uint8_t& encoder);
    void Reset(void);
};