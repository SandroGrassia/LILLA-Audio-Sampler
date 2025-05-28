/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <Arduino.h>
#include "output_noiseshaped_pwm.h"

#if defined(__IMXRT1062__)

#define PWM_RATIO 6
#define BUFSIZE  (PWM_RATIO * AUDIO_BLOCK_SAMPLES)
#define PWM_RATE (PWM_RATIO * AUDIO_SAMPLE_RATE_EXACT)
extern uint8_t analog_write_res;

extern const struct _nspwm_pin_info_struct pwm_pin_info[];


static AudioOutputNoiseShapedPWM * AudioOutputNoiseShapedPWM::instances[4] ;
static int AudioOutputNoiseShapedPWM::units = 0 ;
static bool AudioOutputNoiseShapedPWM::pins_in_use[64] = { false };


void AudioOutputNoiseShapedPWM::begin (uint8_t pin)
{
	active = false ;
	block = NULL ;
	update_responsibility = false ;

	if (pin >= 64)
		return ;
	if (pins_in_use[pin])
		return ;
	
	int unit = allocate_unit () ;
	if (unit == -1)
		return ;
	pins_in_use [pin] = true ;

	pwm_tx_buffer = (uint16_t *) malloc (2 * BUFSIZE * sizeof (uint16_t)) ; // in DMAMEM, double buffer
	if (pwm_tx_buffer == NULL)
		return ;
	
	for (int i = 0 ; i < 4 ; i++)
		accum [i] = 0;

  analogWriteResolution(8);
	analogWriteFrequency (pin, PWM_RATE);
	analogWrite (pin, 0x80);

	apin.pin = pin;
	apin.info = pwm_pin_info[apin.pin];

	uint8_t dmamux_source;

	if (apin.info.type == 1)
	{ //only for valid flexPWM pin:
		unsigned module = (apin.info.module >> 4) & 3;
		unsigned submodule = apin.info.module & 3;
		switch (module)
			{
			case 0:
				{
					apin.flexpwm = &IMXRT_FLEXPWM1;
					switch (submodule)
					{
					case 0: dmamux_source = DMAMUX_SOURCE_FLEXPWM1_WRITE0; break;
					case 1: dmamux_source = DMAMUX_SOURCE_FLEXPWM1_WRITE1; break;
					case 2: dmamux_source = DMAMUX_SOURCE_FLEXPWM1_WRITE2; break;
					default: dmamux_source = DMAMUX_SOURCE_FLEXPWM1_WRITE3;
					}
					break;
				}
			case 1:
				{
					apin.flexpwm = &IMXRT_FLEXPWM2;
					switch (submodule)
					{
					case 0: dmamux_source = DMAMUX_SOURCE_FLEXPWM2_WRITE0; break;
					case 1: dmamux_source = DMAMUX_SOURCE_FLEXPWM2_WRITE1; break;
					case 2: dmamux_source = DMAMUX_SOURCE_FLEXPWM2_WRITE2; break;
					default: dmamux_source = DMAMUX_SOURCE_FLEXPWM2_WRITE3;
					}
					break;
				}
			case 2:
				{
					apin.flexpwm = &IMXRT_FLEXPWM3;
					switch (submodule)
					{
					case 0: dmamux_source = DMAMUX_SOURCE_FLEXPWM3_WRITE0; break;
					case 1: dmamux_source = DMAMUX_SOURCE_FLEXPWM3_WRITE1; break;
					case 2: dmamux_source = DMAMUX_SOURCE_FLEXPWM3_WRITE2; break;
					default: dmamux_source = DMAMUX_SOURCE_FLEXPWM3_WRITE3;
					}
					break;
				}
        default:
				{
					apin.flexpwm = &IMXRT_FLEXPWM4;
					switch (submodule) {
					case 0: dmamux_source = DMAMUX_SOURCE_FLEXPWM4_WRITE0; break;
					case 1: dmamux_source = DMAMUX_SOURCE_FLEXPWM4_WRITE1; break;
					case 2: dmamux_source = DMAMUX_SOURCE_FLEXPWM4_WRITE2; break;
					default: dmamux_source = DMAMUX_SOURCE_FLEXPWM4_WRITE3;
					}
				}
			}

		volatile uint16_t * valReg ;
	
		switch (apin.info.channel)
			{
			case 0:  valReg = &apin.flexpwm->SM[submodule].VAL0; break;
			case 1:  valReg = &apin.flexpwm->SM[submodule].VAL3; break;
			default: valReg = &apin.flexpwm->SM[submodule].VAL5; break;
			}

		dma.begin(true);
		dma.TCD->SADDR = pwm_tx_buffer;
		dma.TCD->SOFF = 2;
		dma.TCD->ATTR = DMA_TCD_ATTR_SSIZE(1) | DMA_TCD_ATTR_DSIZE(1);
		dma.TCD->NBYTES_MLNO = 2;
		dma.TCD->SLAST = - 2 * BUFSIZE * sizeof (uint16_t);
		dma.TCD->DOFF = 0;
		dma.TCD->CITER_ELINKNO = BUFSIZE * sizeof (uint16_t);
		dma.TCD->DLASTSGA = 0;
		dma.TCD->BITER_ELINKNO = BUFSIZE * sizeof (uint16_t);
		dma.TCD->DADDR = valReg;
		dma.triggerAtHardwareEvent(dmamux_source);
		dma.TCD->CSR = DMA_TCD_CSR_INTHALF | DMA_TCD_CSR_INTMAJOR;
		dma.attachInterrupt(unit == 0 ? isr0 :
												unit == 1 ? isr1 :
												unit == 2 ? isr2 :
												unit == 3 ? isr3 : NULL) ;

		//set PWM-DMA-Enable
		apin.flexpwm->SM[submodule].DMAEN = FLEXPWM_SMDMAEN_VALDE;

		//clear inital dma data:
		int16_t modulo = apin.flexpwm->SM[apin.info.module & 3].VAL1;
		for (unsigned j = 0 ; j < 2*BUFSIZE ; j++)
		{
			pwm_tx_buffer[j] = modulo / 2;
		}
		arm_dcache_flush_delete (pwm_tx_buffer, 2 * BUFSIZE * sizeof (int16_t));
			
		update_responsibility = update_setup();

		dma.enable();
		active = true ;
	}
}

int AudioOutputNoiseShapedPWM::allocate_unit (void)
{
	__disable_irq() ;  // is this needed?
	if (units >= 4)
	{
		__enable_irq() ;
		return -1 ;
	}
	int res = units ;
	instances [res] = this ;
	units += 1 ;
	__enable_irq() ;
	return res ;
}


void AudioOutputNoiseShapedPWM::isr0 (void) {	if (instances[0]) instances[0]->handle_interrupt () ; }
void AudioOutputNoiseShapedPWM::isr1 (void) {	if (instances[1]) instances[1]->handle_interrupt () ; }
void AudioOutputNoiseShapedPWM::isr2 (void) {	if (instances[2]) instances[2]->handle_interrupt () ; }
void AudioOutputNoiseShapedPWM::isr3 (void) {	if (instances[3]) instances[3]->handle_interrupt () ; }

void AudioOutputNoiseShapedPWM::handle_interrupt (void)
{
	// capture DMA progress
	uint16_t * saddr = (uint16_t *)(dma.TCD->SADDR);
	uint16_t * midpoint = pwm_tx_buffer + BUFSIZE ;

	// let other interrupts progress
	dma.clearInterrupt();

	uint16_t * dest = (uint32_t) saddr < (uint32_t) midpoint ? midpoint : pwm_tx_buffer ;

	const uint32_t modulo = apin.flexpwm->SM[apin.info.module & 3].VAL1 ;

	if (block)
	{
		for (unsigned i = 0 ; i < AUDIO_BLOCK_SAMPLES ; i++)
		{
			int16_t top ;
			int32_t feedb ;
			for (unsigned j = 0 ; j < PWM_RATIO ; j++)
			{
				accum[0] += block->data[i] ;
				accum[1] += accum[0] ;
				accum[2] += accum[1] ;
				accum[3] += accum[2] ;

				top = accum[3] >> 8 ;
				feedb = top << 8 ;

				accum[0] -= feedb ;
				accum[1] -= feedb ;
				accum[2] -= feedb ;
				accum[3] -= feedb ;
			
				dest [PWM_RATIO*i+j] = top + modulo / 2 ;
			}
		}
	}
	else
	{	// fill with silence when no data available
		for (unsigned i = 0 ; i < BUFSIZE ; i++)
			dest[i] = modulo / 2 ;
	}
	arm_dcache_flush_delete (dest, BUFSIZE * sizeof (uint16_t));
	

	if (block)
	{
		release (block);
		block = NULL;
	}
	
	if (update_responsibility)
		update_all();
}

void AudioOutputNoiseShapedPWM::update (void)
{
	if (!active)
		return ;
	audio_block_t * new_block = receiveReadOnly();
	__disable_irq();
	audio_block_t * old_block = block ;
	block = new_block ;
	__enable_irq();
	if (old_block)
		release (old_block) ;
}

#endif // __IMXRT1062__

