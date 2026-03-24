/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "AudioADSR.h"

void AudioADSR::Set_identity(int id)
{
    identity = id;
}

int AudioADSR::Get_identity(void)
{
    return identity;
}

uint8_t AudioADSR::Get_phase(void)
{
    return phase;
}

void AudioADSR::Setup(const float &attack, const float &decay, const float &sustain, const float &release, const uint8_t &attack_type)
{
    Attack = ((attack > 0.01) ? attack : 0.01);    // [sec]; referred to pitch = 1.0
    Decay = ((decay > 0.05) ? decay : 0.05);       // [sec]; referred to pitch = 1.0
    Sustain = sustain;                             // [number]; referred to pitch = 1.0
    Release = ((release > 0.05) ? release : 0.05); // [sec]; referred to pitch = 1.0
    Attack_type = attack_type;                     // in AudioPlayer veniva assegnato in un secondo tempo

    /*
    Serial.print("AudioADSR::Setup - identity: ");
    Serial.print(identity);
    Serial.print(" ");
    Serial.print(Attack);
    Serial.print(" ");
    Serial.print(Decay);
    Serial.print(" ");
    Serial.print(Sustain);
    Serial.print(" ");
    Serial.print(Release);
    Serial.print(" ");
    Serial.println(Attack_type);
    */
}
/*

          |-> ATTACK               |-> DACAY                      |-> SUSTAIN                  |-> RELEASE                        END

sample    0----------------------(J1)---------------------------(J2)                         (J3)---------------------------------(J4)
                                                                                                              (DJ3_4)




*/
void AudioADSR::Set_parametrs(void)
{
    J1 = Attack * AUDIO_SAMPLE_RATE;           // [sample]
    J2 = (Attack + Decay) * AUDIO_SAMPLE_RATE; // [sample]
    DJ3_4 = Release * AUDIO_SAMPLE_RATE;       // [sample]
    K_Attack_step = 10.0f / J1;                // [1/sample] is a gain change-rate parameter
    K_Dacay_step = 10.0f / (J2 - J1);          // [1/sample] is a gain change-rate parameter
    K_Sustain_delta = 1.0f - Sustain;          // [gain]
    K_Release_step = 10.0f / DJ3_4;            // [1/sample] is a gain change-rate parameter
    phase = ATTACK;
    ADSR_point_0 = -K_Attack_step;

    /*
    Serial.print("AudioADSR::Set_parametrs(void) - identity: ");
    Serial.print(identity);
    Serial.print(" ");
    Serial.print(J1, 6);
    Serial.print(" ");
    Serial.print(J2, 6);
    Serial.print(" ");
    Serial.print(DJ3_4, 6);
    Serial.print(" ");
    Serial.print(K_Attack_step);
    Serial.print(" ");
    Serial.print(K_Dacay_step);
    Serial.print(" ");
    Serial.print(K_Sustain_delta);
    Serial.print(" ");
    Serial.print(K_Release_step);
    Serial.print(" ");
    Serial.print(phase);
    Serial.print(" ");
    Serial.println(ADSR_point_0);
    */
}

void AudioADSR::Release_note(void)
{
    K_Release_delta = ADSR_gain; // [gain]
    phase = RELEASE;     
    ADSR_point_0 = -K_Release_step;

    /*
    Serial.print("AudioADSR::Release_note() - identity: ");
    Serial.print(identity);
    Serial.print(" ADSR_gain: ");
    Serial.print(ADSR_gain);
    Serial.print(" ADSR_point_0: ");
    Serial.println(ADSR_point_0);
    */
}

void AudioADSR::Fast_stop(void)
{
    K_Release_delta = ADSR_gain; // [gain]
    phase = RELEASE;              // Release
    ADSR_point_0 = -K_Release_step;
}

float AudioADSR::Get_gain(void)
{

    if (phase == ATTACK)
    {
        ADSR_point = ADSR_point_0 + K_Attack_step;
        uint8_t adsr_point_int = floor(ADSR_point);

        if (adsr_point_int < 10)
        {
            ADSR_point_0 = ADSR_point;
            if (Attack_type == 1) // FAST Attack
            {
                ADSR_gain = (exp_table[adsr_point_int] + m_exp_table[adsr_point_int] * (ADSR_point - adsr_point_int));
                return ADSR_gain;
            }
            else // SLOW Attack
            {
                ADSR_gain = (sin_table[adsr_point_int] + m_sin_table[adsr_point_int] * (ADSR_point - adsr_point_int));
                return ADSR_gain;
            }
        }
        else
        {
            phase = DECAY;
            if ((adsr_point_int - 10) < K_Attack_step / 2.0)
            {
                ADSR_point = 0.0;
            }
            else
            {
                ADSR_point = K_Dacay_step;
            }
            adsr_point_int = floor(ADSR_point);
            ADSR_point_0 = ADSR_point;
            ADSR_gain = Sustain + K_Sustain_delta * (decay_table[adsr_point_int] + m_decay_table[adsr_point_int] * (ADSR_point - adsr_point_int));
            return ADSR_gain;
        }
    }

    else if (phase == DECAY) // Decay
    {
        ADSR_point = ADSR_point_0 + K_Dacay_step;
        uint8_t adsr_point_int = floor(ADSR_point);

        if (adsr_point_int < 10)
        {
            ADSR_point_0 = ADSR_point;
            ADSR_gain = Sustain + K_Sustain_delta * (decay_table[adsr_point_int] + m_decay_table[adsr_point_int] * (ADSR_point - adsr_point_int));
            return ADSR_gain;
        }
        else
        {
            phase = SUSTAIN;
            ADSR_point_0 = 0.0;
            ADSR_gain = Sustain;
            return ADSR_gain;
        }
    }

    else if (phase == SUSTAIN)
    {
        ADSR_gain = Sustain;
        return ADSR_gain;
    }

    else if (phase == RELEASE) // Release
    {
        ADSR_point = ADSR_point_0 + K_Release_step;
        uint8_t adsr_point_int = floor(ADSR_point);

        if (adsr_point_int < 10)
        {
            ADSR_point_0 = ADSR_point;
            ADSR_gain = K_Release_delta * (release_table[adsr_point_int] + m_release_table[adsr_point_int] * (ADSR_point - adsr_point_int));
            return ADSR_gain;
        }
        else
        {
            phase = STOP;
            ADSR_gain = 0.0f;
            return ADSR_gain;
        }
    }

    else // phase == STOP
    {
        ADSR_gain = 0.0f;
        return ADSR_gain;
    }
}
