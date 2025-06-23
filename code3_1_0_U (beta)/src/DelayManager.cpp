/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "DelayManager.h"

void DelayManager::Stop(void)
{
    if (run_flag)
    {
        Delay_data = Delay_data_required; // forza l'attribuzione immediata dei valori finali
        run_flag = false;
    }
}

bool DelayManager::New_values(const Delay_data_struct *data)
{
    run_flag = false;
    Delay_data_required = *data;

    if (Delay_data_required.instrument_route != Delay_data.instrument_route)
    {
        if (true)
        {
            Serial.print(Delay_data.instrument_route);
            Serial.print(" - ");
            Serial.print(Delay_data_required.instrument_route);
            Serial.println(" old-new instrument_route");
        }

        flag[INSTRUMENT_ROUTE] = true;
        run_flag = true;
    }
    if (Delay_data_required.modulation_source != Delay_data.modulation_source)
    {
        if (false)
        {
            Serial.print(Delay_data.modulation_source);
            Serial.print(" - ");
            Serial.print(Delay_data_required.modulation_source);
            Serial.println(" old-new modulation_source");
        }

        flag[MODULATION_SOURCE] = true;
        run_flag = true;
    }

    if (Delay_data_required.samples != Delay_data.samples)
    {
        if (false)
        {
            Serial.print(Delay_data.samples);
            Serial.print(" - ");
            Serial.print(Delay_data_required.samples);
            Serial.println(" old-new samples");
        }

        Start_LPF(SAMPLES, Delay_values.samples, Calc_delay_samples(Delay_data_required.samples));

        // assegna subito il nuovo valore di regime
        Delay_data.samples = Delay_data_required.samples;

        flag[SAMPLES] = true;
        run_flag = true;
    }
    if (Delay_data_required.samples_LR != Delay_data.samples_LR)
    {
        if (false)
        {
            Serial.print(Delay_data.samples_LR);
            Serial.print(" - ");
            Serial.print(Delay_data_required.samples_LR);
            Serial.println(" old-new samples_LR");
        }

        Start_LPF(SAMPLES_LR, Delay_data.samples_LR, Delay_data_required.samples_LR);
        flag[SAMPLES_LR] = true;
        run_flag = true;
    }
    if (Delay_data_required.modulation_depth != Delay_data.modulation_depth)
    {
        if (false)
        {
            Serial.print(Delay_data.modulation_depth);
            Serial.print(" - ");
            Serial.print(Delay_data_required.modulation_depth);
            Serial.println(" old-new modulation_depth");
        }

        Start_LPF(MODULATION_DEPTH, Delay_data.modulation_depth, Delay_data_required.modulation_depth);
        flag[MODULATION_DEPTH] = true;
        run_flag = true;
    }
    if (Delay_data_required.modulation_frequency != Delay_data.modulation_frequency)
    {
        if (false)
        {
            Serial.print(Delay_data.modulation_frequency);
            Serial.print(" - ");
            Serial.print(Delay_data_required.modulation_frequency);
            Serial.println(" old-new modulation_frequency");
        }

        Start_LPF(MODULATION_FREQUENCY, Delay_data.modulation_frequency, Delay_data_required.modulation_frequency);
        flag[MODULATION_FREQUENCY] = true;
        run_flag = true;
    }
    if (Delay_data_required.modulation_phase_LR != Delay_data.modulation_phase_LR)
    {
        if (false)
        {
            Serial.print(Delay_data.modulation_phase_LR);
            Serial.print(" - ");
            Serial.print(Delay_data_required.modulation_phase_LR);
            Serial.println(" old-new modulation_phase_LR");
        }

        Start_LPF(MODULATION_PHASE_LR, Delay_data.modulation_phase_LR, Delay_data_required.modulation_phase_LR);
        flag[MODULATION_PHASE_LR] = true;
        run_flag = true;
    }
    if (Delay_data_required.loop_gain != Delay_data.loop_gain)
    {
        if (false)
        {
            Serial.print(Delay_data.loop_gain);
            Serial.print(" - ");
            Serial.print(Delay_data_required.loop_gain);
            Serial.println(" old-new loop_gain");
        }

        Start_LPF(LOOP_GAIN, Delay_values.loop_gain, Delay_feedback(Delay_data_required.loop_gain));

        // assegna subito il nuovo valore di regime
        Delay_data.loop_gain = Delay_data_required.loop_gain;

        flag[LOOP_GAIN] = true;
        run_flag = true;
    }

    if (run_flag)
    {
        step = steps;
    }

    // Serial.print("DelayManager::New_values? :");
    // Serial.println(run_flag);

    return run_flag;
}

void DelayManager::Update(void)
{
    if (run_flag)
    {
        if (flag[INSTRUMENT_ROUTE])
        {
            // Change immediato
            Delay_data.instrument_route = Delay_data_required.instrument_route;

            // Calcola i nuovi valori
            Calc_delay_routing(Delay_data.instrument_route);

            for (auto i = 0; i < INSTRUMENTS_MAX; ++i)
            {
                // trasmetti i nuovi valori
                Players_Manager_ptr->MX_multicast_change_routing(Delay_values.instrument_route[i]);
            }

            flag[INSTRUMENT_ROUTE] = false;
            run_flag = false;
        }

        if (flag[MODULATION_SOURCE])
        {

            // Change immediato
            Delay_data.modulation_source = Delay_data_required.modulation_source;

            // calcola nuovo valore
            Delay_values.modulation_source = Delay_data.modulation_source;

            // trasmetti nuovo valore
            Delay_L_ptr->Set_delay_modulation_source(Delay_values.modulation_source); // Left channel
            Delay_R_ptr->Set_delay_modulation_source(Delay_values.modulation_source); // Right channel

            flag[MODULATION_SOURCE] = false;
            run_flag = false;
        }

        if (flag[SAMPLES])
        {
            Delay_values.samples = New_value(SAMPLES);
            if(Delay_values.samples < 0)
            {
               Delay_values.samples = 0; 
            }

            // trasmetti nuovo valore
            if (Delay_values.samples_LR >= 0) // Left channel
            {
                Delay_L_ptr->Set_delay_central_value(Delay_values.samples + Delay_values.samples_LR);
                Delay_R_ptr->Set_delay_central_value(Delay_values.samples);
            }
            else
            {
                Delay_R_ptr->Set_delay_central_value(Delay_values.samples - Delay_values.samples_LR);
                Delay_L_ptr->Set_delay_central_value(Delay_values.samples);
            }

            if (false)
            {
                Serial.print(F("Delay_values.samples: "));
                Serial.println(Delay_values.samples);
            }
            run_flag = true;
        }

        if (flag[SAMPLES_LR])
        {
            Delay_data.samples_LR = round(New_value(SAMPLES_LR));
            Delay_data.samples_LR = constrain(Delay_data.samples_LR, Delay_data_limits[SAMPLES_LR][0], Delay_data_limits[SAMPLES_LR][1]);

            // calcola nuovo valore
            Delay_values.samples_LR = Calc_delay_samples_LR(Delay_data.samples_LR);

            // trasmetti nuovo valore
            if (Delay_values.samples_LR >= 0) // Left channel
            {
                Delay_L_ptr->Set_delay_central_value(Delay_values.samples + Delay_values.samples_LR);
            }
            else
            {
                Delay_R_ptr->Set_delay_central_value(Delay_values.samples - Delay_values.samples_LR);
            }

            if (false)
            {
                Serial.print(F("Delay_values.samples_LR: "));
                Serial.println(Delay_values.samples_LR);
            }
            run_flag = true;
        }

        if (flag[MODULATION_DEPTH])
        {
            Delay_data.modulation_depth = round(New_value(MODULATION_DEPTH));
            Delay_data.modulation_depth = constrain(Delay_data.modulation_depth, Delay_data_limits[MODULATION_DEPTH][0], Delay_data_limits[MODULATION_DEPTH][1]);

            // calcola nuovo valore
            Delay_values.modulation_depth = Calc_delay_depth(Delay_data.modulation_depth);

            // trasmetti nuovo valore
            Delay_L_ptr->Set_delay_modulation_gain(Delay_values.modulation_depth);
            Delay_R_ptr->Set_delay_modulation_gain(Delay_values.modulation_depth);

            if (false)
            {
                Serial.print(F("Delay_values.modulation_depth: "));
                Serial.println(Delay_values.modulation_depth);
            }
            run_flag = true;
        }

        if (flag[MODULATION_FREQUENCY])
        {
            Delay_data.modulation_frequency = round(New_value(MODULATION_FREQUENCY));
            Delay_data.modulation_frequency = constrain(Delay_data.modulation_frequency, Delay_data_limits[MODULATION_FREQUENCY][0], Delay_data_limits[MODULATION_FREQUENCY][1]);

            // calcola nuovo valore
            Delay_values.modulation_frequency = Calc_delay_frequency(Delay_data.modulation_frequency);

            // trasmetti nuovo valore
            LFO_D_ptr[0]->Set_frequency(Delay_values.modulation_frequency);
            LFO_D_ptr[1]->Set_frequency(Delay_values.modulation_frequency);

            if (false)
            {
                Serial.print(F("Delay_values.modulation_frequency: "));
                Serial.println(Delay_values.modulation_frequency);
            }
            run_flag = true;
        }

        if (flag[MODULATION_PHASE_LR])
        {
            Delay_data.modulation_phase_LR = round(New_value(MODULATION_PHASE_LR));
            Delay_data.modulation_phase_LR = constrain(Delay_data.modulation_phase_LR, Delay_data_limits[MODULATION_PHASE_LR][0], Delay_data_limits[MODULATION_PHASE_LR][1]);

            // calcola nuovo valore
            Delay_values.modulation_phase_LR = Delay_data.modulation_phase_LR;

            // trasmetti nuovo valore
            LFO_D_ptr[0]->Set_phase(Delay_values.modulation_phase_LR);

            if (false)
            {
                Serial.print(F("Delay_values.modulation_phase_LR: "));
                Serial.println(Delay_values.modulation_phase_LR);
            }
            run_flag = true;
        }

        if (flag[LOOP_GAIN])
        {
            Delay_values.loop_gain = New_value(LOOP_GAIN);

            // trasmetti nuovo valore
            D_gain_L_feedback_ptr->Set_gain(Delay_values.loop_gain);
            D_gain_R_feedback_ptr->Set_gain(Delay_values.loop_gain);

            if (true)
            {
                Serial.print(F("Delay_values.loop_gain: "));
                Serial.println(Delay_values.loop_gain);
            }
            run_flag = true;
        }

        if (!run_flag)
        {
            return;
        }

        --step;
        if (step == 0)
        {
            run_flag = false;
            Serial.print(F("DelayManager::Update(void) - Update done."));
        }
    }
}

// private
bool DelayManager::Start_LPF(int item, double v_0, double v_1) // v_0: valore di partenza   v_1: valore desiderato
{
    if (item >= DELAY_LPF_ITEMS)
    {
        Serial.println(F("DelayManager::Start_LPF - errata inizializzazione!"));
        return false;
    }
    y_1[item] = v_0;
    y_2[item] = v_0;
    x_1[item] = v_0;
    x[item] = v_1;

    return true;
}

double DelayManager::New_value(int item)
{
    if (item >= DELAY_LPF_ITEMS)
    {
        Serial.println(F("DelayManager::New_value - item inesistente!"));
        return 0;
    }
    double y = d1 * y_1[item] + d2 * y_2[item] + n0 * x[item] + n1 * x_1[item];
    x_1[item] = x[item];
    y_2[item] = y_1[item];
    y_1[item] = y;

    return y;
}