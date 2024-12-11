<p align="center">
<img width="500" src="/pics/logo_0.jpg")
</p>

LILLA is an audio sampler based on Teensy 4.1, designed and assembled in Italy.<br /> 
[Website](https://www.lillasampler.it/)<br /> 
[Facebook](https://www.facebook.com/Lilla.audio.sampler)

Here some images showing the project evolution since 2018.
<p align="center">
<img width="400" src="/pics/story_0.jpg")
</p>

<p align="center">
<img width="400" src="/pics/story_1.jpg")
</p>

<p align="center">
<img width="400" src="/pics/story_2.jpg")
</p>

<p align="center">
<img width="400" src="/pics/story_3.jpg")
</p>

<p align="center">
<img width="400" src="/pics/story_4.jpg")
</p>

<p align="center">
<img width="400" src="/pics/story_5.jpg")
</p>

<p align="center">
<img width="400" src="/pics/story_6.jpg")
</p>


Latest version is LILLA 2023; PCB is realized in Hong Kong, components assembling is done in Italy, enclosure (aluminium sheet case crafted with laser cutter and CNC bender press - wooden parts are crafted with CNC milling machine) is made in Italy.
Actually more versions are in progress of design.

Original and Modified libraries/classes included, with licensing notes.   

This following text describes the full #include tree for LILLA code. Each class/library mentioned is preceded by a number representing the level of dependency:
```
> level 1: explicitely included in LILLA code
>> level 2: included in a level-1 element
>>> level 3: included in a level-2 element
```
and so on.</p>

Arduino.h (header file)

>WProgram.h
>Copyright (c) 2017 PJRC.COM, LLC.
>Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the"Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish,
>distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
>1. The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
>2. If the Software is incorporated into a build system that allows selection among a list of target devices, then similar target devices manufactured by PJRC.COM must be included in the list of target devices and selectable in the same manner.
>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
>HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.




        
            2 WProgram.h    Copyright (c) 2017 PJRC.COM, LLC.
                            Permission is hereby granted, free of charge, to any person obtaining
                            a copy of this software and associated documentation files (the
                            "Software"), to deal in the Software without restriction, including
                            without limitation the rights to use, copy, modify, merge, publish,
                            distribute, sublicense, and/or sell copies of the Software, and to
                            permit persons to whom the Software is furnished to do so, subject to
                            the following conditions:
                            1. The above copyright notice and this permission notice shall be
                            included in all copies or substantial portions of the Software.
                            2. If the Software is incorporated into a build system that allows
                            selection among a list of target devices, then similar target
                            devices manufactured by PJRC.COM must be included in the list of
                            target devices and selectable in the same manner.
                            THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
                            EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
                            MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
                            NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
                            BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
                            ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
                            CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
                            SOFTWARE.
        
                            notes:
                            1. Il codice è pubblicato in forma di main e classi compilate; i sorgenti non
                            sono pubblici, non ci sono commenti leggibili; 
                            2. Il software NON consente di utilizzare un'altra piattaforma diversa dal
                               Teensy 4.1.


                3 stdlib.h  Copyright (C) 2002-2024 Free Software Foundation, Inc.
                            This file is part of the GNU ISO C++ Library.  This library is free
                            software; you can redistribute it and/or modify it under the
                            terms of the GNU General Public License as published by the
                            Free Software Foundation; either version 3, or (at your option)
                            any later version.
                            This library is distributed in the hope that it will be useful,
                            but WITHOUT ANY WARRANTY; without even the implied warranty of
                            MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
                            GNU General Public License for more details.
                            Under Section 7 of GPL version 3, you are granted additional
                            permissions described in the GCC Runtime Library Exception, version
                            3.1, as published by the Free Software Foundation.
                            You should have received a copy of the GNU General Public License and
                            a copy of the GCC Runtime Library Exception along with this program;
                            see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
                            <http://www.gnu.org/licenses/>.
                            
                            notes:
                                Licenza GNU GPL è una licenza libera, che permette di redistribuire e/o modificare
                                il software, ma richiede che le stesse libertà siano mantenute per chi riceve il
                                software. In altre parole il software modificato deve mantenere la stessa licenza
                                GPL.
                                Se il progetto utilizza codice coperto dalla licenza GPLv3 con l'eccezione della
                                GCC Runtime Library Exception 3.1, posso distribuire il software come closed
                                source senza essere obbligato a rilasciare il codice sorgente del progetto.

                                


            3 string.h  Copyright (C) 2002-2024 Free Software Foundation, Inc. (see above for licensing)
            3 math.h    Copyright (C) 2006-2015 Free Software Foundation, Inc.  (see above for licensing)
        2 pins_arduino.h    Copyright (c) 2018 PJRC.COM, LLC.  (see above for licensing)
            3 stdint.h  Copyright (C) 2002-2024 Free Software Foundation, Inc.  (see above for licensing)
    
    1 DMAChannel    Copyright (c) 2017 PJRC.COM, LLC.
        2 imxrt.h   ARM documentation

    1 control_sgtl5000.h    Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
                            Development of this audio library was funded by PJRC.COM, LLC by sales of
                            Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
                            open source software by purchasing Teensy or other PJRC products.
                            Permission is hereby granted, free of charge, to any person obtaining a copy
                            of this software and associated documentation files (the "Software"), to deal
                            in the Software without restriction, including without limitation the rights
                            to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
                            copies of the Software, and to permit persons to whom the Software is
                            furnished to do so, subject to the following conditions:
                            The above copyright notice, development funding notice, and this permission
                            notice shall be included in all copies or substantial portions of the Software.
                            THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
                            IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
                            FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
                            AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
                            LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
                            OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
                            THE SOFTWARE.

                            notes:
                            1. Il codice è pubblicato in forma di main e classi compilate; i sorgenti non
                            sono pubblici, non ci sono commenti leggibili

        2 AudioStream.h     Copyright (c) 2017 PJRC.COM, LLC. (see above for licensing)
            3 stdio.h       Copyright (C) 2002-2024 Free Software Foundation, Inc. (see above for licensing)
            3 string.h      (see above)
        2 AudioControl.h    Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com (see above for licensing)
            3 stdint.h      (see above)
    
    1 input_i2s.h       Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com (see above for licensing)
        2 Arduino.h     (see above)
        2 AudioStream.h (see above)
        2 DMAChannel.h  Copyright (c) 2017 PJRC.COM, LLC. (see above for licensing)
            3 imxrt.h   (see above)
    
    1 output_i2s.h      Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com (see above for licensing)
        2 Arduino.h         (see above)
        2 AudioStream.h     (see above)
        2 DMAChannel.h      (see above)
            3 imxrt.h       (see above)
    
    1 record_queue.h    Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
        2 Arduino.h         (see above)
        2 AudioStream.h     (see above)

    1 SPI.h     Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
        Copyright (c) 2014 by Paul Stoffregen <paul@pjrc.com> (Transaction API)
        Copyright (c) 2014 by Matthijs Kooijman <matthijs@stdin.nl> (SPISettings AVR)
        SPI Master library for arduino.
        This file is free software; you can redistribute it and/or modify
        it under the terms of either the GNU General Public License version 2
        or the GNU Lesser General Public License version 2.1, both as
        published by the Free Software Foundation.

        notes:
        GNU General Public License version 2 (GPL): usando software con licenza GPL in un progetto closed
        source devo rilasciare l'intero codice sorgente del tuo progetto sotto GPL.
        GNU Lesser General Public License version 2.1 (LGPL): se utilizzo la libreria senza modificarla,
        devo permettere agli utenti di sostituire la libreria con una versione modificata. In pratica,
        questo significa che il software deve essere collegato dinamicamente alla libreria, in modo che
        gli utenti possano facilmente sostituirla con una versione diversa se lo desiderano.

        2 Arduino.h     (see above)
        2 DMAChannel.h  (see above)
        2 EventResponder.h  Copyright 2017 Paul Stoffregen (see above for licensing)

    1 spi_interrupt.h   Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com (see above for licensing)
        2 Arduino.h       (see above)
        2 AudioStream.h   (see above)
        2 SPI.h     (see above)
    
    1 Wire.h    Copyright (c) 2006 Nicholas Zambetti.  All right reserved.
                This library is free software; you can redistribute it and/or
                modify it under the terms of the GNU Lesser General Public
                License as published by the Free Software Foundation; either
                version 2.1 of the License, or (at your option) any later version.
                This library is distributed in the hope that it will be useful,
                but WITHOUT ANY WARRANTY; without even the implied warranty of
                MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
                Lesser General Public License for more details.
                You should have received a copy of the GNU Lesser General Public
                License along with this library; if not, write to the Free Software
                Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
                Modified 2012 by Todd Krein (todd@krein.org) to implement repeated starts

                notes:
                GNU General Public License version 2 (GPL): usando software con licenza GPL in un
                progetto closed source devo rilasciare l'intero codice sorgente del tuo progetto
                sotto GPL.
                GNU Lesser General Public License version 2.1 (LGPL): se utilizzo la libreria senza
                modificarla, devo permettere agli utenti di sostituire la libreria con una versione
                modificata. In pratica, questo significa che il software deve essere collegato
                dinamicamente alla libreria, in modo che gli utenti possano facilmente sostituirla
                con una versione diversa se lo desiderano.

        2 WireIMXRT.h   Copyright (c) 2014-2017, Paul Stoffregen, paul@pjrc.com (see above for licensing)
    
    1 EEPROM.h  Copyright (c) 2015 Paul Stoffregen <paul@pjrc.com> (see above for licensing)
        2 stddef.h          (see above)
        2 stdint.h          (see above)
        2 avr_functions.h Copyright (c) 2017 PJRC.COM, LLC. (see above for licensing)           
            3 inttypes.h  Copyright (C) 2006-2019 Free Software Foundation, Inc. (see above for licensing)               
                4 tr1/cinttypes Copyright (C) 2006-2019 Free Software Foundation, Inc. (see above
                                for licensing)

    1 SD.h      Copyright (c) 2020, Paul Stoffregen, paul@pjrc.com (see above for licensing)               
        2 Arduino.h     (see above)
        2 SdFat.h   Copyright (c) 2011-2021 Bill Greiman
                    MIT License
                    Permission is hereby granted, free of charge, to any person obtaining a
                    copy of this software and associated documentation files (the "Software"),
                    to deal in the Software without restriction, including without limitation
                    the rights to use, copy, modify, merge, publish, distribute, sublicense,
                    and/or sell copies of the Software, and to permit persons to whom the
                    Software is furnished to do so, subject to the following conditions:
                    The above copyright notice and this permission notice shall be included
                    in all copies or substantial portions of the Software.
                    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
                    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
                    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
                    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
                    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
                    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
                    DEALINGS IN THE SOFTWARE.

                    notes:
                    Il codice è pubblicato in forma di main e classi compilate; i sorgenti non sono
                    pubblici, non ci sono commenti leggibili. 

            3 common/SysCall.h  Copyright (c) 2011-2021 Bill Greiman (see above for licensing)
                4 stdint.h      (see above)    
                4 stddef.h      (see above)
                4 SdFatConfig.h     Copyright (c) 2011-2021 Bill Greiman (see above for licensing)
                    5 stdint.h      (see above)
                    5 avr/io.h      (see above)
                    5 Arduino.h     (see above)
                4 PrintBasic.h      Copyright (c) 2011-2021 Bill Greiman (see above for licensing)
            3 SdCard/SdCard.h   Copyright (c) 2011-2021 Bill Greiman (see above for licensing)
                4 SdioCard.h    Copyright (c) 2011-2021 Bill Greiman (see above for licensing)
                    5 SysCall.h (see above)      
                        6 stdint.h      (see above)
                        6 stddef.h      (see above)
                        6 SdFatConfig.h (see above)
                            7 stdint.h      (see above)
                            7 avr/io.h      Copyright (c) 2015 Paul Stoffregen <paul@pjrc.com> (see above
                                            for licensing)   
                                8 avr_emulation.h   Copyright (c) 2019 PJRC.COM, LLC. (see above for
                                                    licensing)
                    5 SdCardInterface.h     Copyright (c) 2011-2021 Bill Greiman (see above for licensing)
                        6 common/FsBlockDeviceInterface.h   Copyright (c) 2011-2021 Bill Greiman
                                                            (see above for licensing)
                            7 stdint.h  (see above)
                            7 stddef.h  (see above)
                        6 SdCardInfo.h  Copyright (c) 2011-2021 Bill Greiman (see above for licensing)
                            7 common/SysCall.h  (see above)
                                8 stdint.h      (see above)
                                8 stddef.h      (see above)
                                8 SdFatConfig.h (see above)
                4 SdSpiCard.h       Copyright (c) 2011-2021 Bill Greiman (see above for licensing)
                    5 stddef.h      (see above)
                    5 common/SysCall.h      (see above)
                    5 SdCardInfo.h          (see above)
                    5 SdCardInterface.h     (see above)
                    5 SpiDriver/SdSpiDriver.h   Copyright (c) 2011-2021 Bill Greiman (see above for
                                                licensing)
                        6 common/SysCall.h      (see above)
            3 ExFatLib/ExFatLib.h   Copyright (c) 2011-2021 Bill Greiman (see above for licensing)
                4 ExFatVolume.h     Copyright (c) 2011-2021 Bill Greiman (see above for licensing)
                    5 ExFatFile.h   Copyright (c) 2011-2021 Bill Greiman (see above for licensing)
                        6 limits.h  Copyright (C) 1992-2019 Free Software Foundation, Inc.
                                    This file is part of GCC. GCC is free software; you can redistribute
                                    it and/or modify it under the terms of the GNU General Public License
                                    as published by the Free Software Foundation; either version 3, or 
                                    (at your option) any later version.
                                    GCC is distributed in the hope that it will be useful, but WITHOUT ANY
                                    WARRANTY; without even the implied warranty of MERCHANTABILITY or
                                    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
                                    for more details.
                                    Under Section 7 of GPL version 3, you are granted additional
                                    permissions described in the GCC Runtime Library Exception, version
                                    3.1, as published by the Free Software Foundation.
                                    You should have received a copy of the GNU General Public License and
                                    a copy of the GCC Runtime Library Exception along with this program;
                                    see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
                                    <http://www.gnu.org/licenses/>.

                                    notes:
                                    il software è concesso in licenza sotto la GNU General Public License
                                    (GPL) versione 3 con l'aggiunta di un'eccezione specifica, nota come
                                    GCC Runtime Library Exception, versione 3.1. Questa eccezione è importante
                                    perché modifica alcuni degli obblighi imposti dalla GPL, rendendo più
                                    flessibile l'utilizzo del software in progetti closed source.
                                    L'eccezione della Runtime Library di GCC è progettata per consentire
                                    l'uso delle librerie di runtime di GCC in un contesto più ampio senza
                                    richiedere che tutto il software collegato a tali librerie sia rilasciato
                                    sotto la GPL.
                                    In altre parole, se utilizzi una libreria di runtime di GCC coperta da questa
                                    eccezione in un progetto closed source, non sei obbligato a rilasciare il
                                    codice sorgente del tuo progetto sotto la GPL, a condizione di rispettare
                                    determinate condizioni.
                                    
                                    Dettagli dell'Eccezione:
                                    Linking Permesso: La GCC Runtime Library Exception permette di collegare
                                    dinamicamente o staticamente il proprio software con le librerie di runtime
                                    di GCC senza che l'intero software finale debba essere rilasciato sotto GPL.
                                    Questo significa che puoi utilizzare le librerie di runtime di GCC in un
                                    progetto closed source senza dover rendere open source l'intero progetto.
                                    
                                    Distribuzione: Quando distribuisci il software che include le librerie di
                                    runtime di GCC, non sei obbligato a rilasciare il codice sorgente del tuo
                                    progetto principale sotto GPL.
                                    Tuttavia, devi comunque rispettare i termini della GPL per la distribuzione
                                    delle librerie stesse, mantenendo gli avvisi di copyright e la licenza.

                                    Modifiche alle Librerie: Se modifichi le librerie di runtime di GCC, le
                                    modifiche devono essere rilasciate sotto la GPL. Tuttavia, il tuo software
                                    che si collega alle librerie (senza modificarle) può rimanere closed source.

                        6 string.h  Copyright (c) 2006 The NetBSD Foundation, Inc. All rights reserved.
                                    This code is derived from software contributed to The NetBSD Foundation
                                    by Christos Zoulas.
                                    Redistribution and use in source and binary forms, with or without
                                    modification, are permitted provided that the following conditions
                                    are met:
                                    1. Redistributions of source code must retain the above copyright
                                    notice, this list of conditions and the following disclaimer.
                                    2. Redistributions in binary form must reproduce the above copyright
                                    notice, this list of conditions and the following disclaimer in the
                                    documentation and/or other materials provided with the distribution.
                                    THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
                                    ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
                                    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
                                    PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
                                    BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
                                    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
                                    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
                                    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
                                    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
                                    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
                                    POSSIBILITY OF SUCH DAMAGE.

                                    notes:
                                    devo mantenere intatti gli avvisi di copyright e le condizioni della licenza
                                    sia nel codice sorgente che nei binari distribuiti, e assicurarti che queste
                                    informazioni siano incluse nella documentazione o nei materiali distribuiti
                                    con il software.

                        6 common/FsDateTime.h   Copyright (c) 2011-2021 Bill Greiman (see above for licensing)
                            7 stdint.h              (see above)
                            7 CompileDateTime.h     Copyright (c) 2011-2021 Bill Greiman (see above for licensing)
                            7 SysCall.h     (see above)
                        6 common/FsApiConstants.h   Copyright (c) 2011-2021 Bill Greiman (see above for licensing)
                            7 SysCall.h (see above)
                            7 fcntl.h   (??)
                        6 common/FmtNumber.h    Copyright (c) 2011-2021 Bill Greiman (see above for licensing)
                            7 math.h    (see above)  
                            7 stdint.h  (see above)
                            7 stddef.h  (see above)
                        6 common/FsName.h   Copyright (c) 2011-2021 Bill Greiman (see above for licensing)
                            7 SysCall.h     (see above)
                            7 stdint.h      (see above)
                        6 ExFatPartition.h  Copyright (c) 2011-2021 Bill Greiman (see above for licensing)
                            7 SysCall.h     (see above)
                            7 common/FsBlockDevice.h    Copyright (c) 2011-2021 Bill Greiman (see above for
                                                        licensing)
                            7 common/FsCache.h          Copyright (c) 2011-2021 Bill Greiman (see above for
                                                        licensing)
                            7 common/FsStructs.h        Copyright (c) 2011-2021 Bill Greiman   (see above for
                                                        licensing)                    
                4 ExFatFormatter.h  Copyright (c) 2011-2022 Bill Greiman (see above for licensing)
                    5 common/FsBlockDevice.h    (see above)          
            3 FatLib/FatLib.h   Copyright (c) 2011-2022Bill Greiman (see above for licensing)
                4 FatVolume.h   Copyright (c) 2011-2022 Bill Greiman (see above for licensing)
                4 FatFormatter.h    Copyright (c) 2011-2022 Bill Greiman (see above for licensing)
            3 FsLib/FsLib.h     Copyright (c) 2011-2022 Bill Greiman (see above for licensing)
                4 FsVolume.h    Copyright (c) 2011-2022 Bill Greiman (see above for licensing)
                    5 FsNew.h   Copyright (c) 2011-2022 Bill Greiman (see above for licensing)
                        6 stddef.h      (see above)
                        6 stdint.h      (see above)
                    5 FatLib/FatLib.h   (see above)
                    5 ExFatLib/ExFatLib.h   (see above)     
                4 FsFile.h  Copyright (c) 2011-2024 Bill Greiman (see above for licensing)
                    5 FsNew.h           (see above)
                    5 FatLib/FatLib.h   (see above)
                    5 ExFatLib/ExFatLib.h   (see above)
                4 FsFormatter.h Copyright (c) 2011-2022 Bill Greiman (see above for licensing)
                    5 FatLib/FatLib.h       (see above)
                    5 ExFatLib/ExFatLib.h   (see above) 
            3 sdios.h   (see above)
        2 FS.h  Copyright (c) 2021 PJRC.COM, LLC. (see above for licensing)

    1 SerialFlash.h Copyright (C) 2015, Paul Stoffregen, paul@pjrc.com (see above for licensing)
        2 Arduino.h (see above)
        2 SPI.h     (see above)

    1 dspinst.h Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com (see above for licensing)
        2 stdint.h  (see above)
    
    1  MIDI.h  Copyright (c) 2015 Francois Best (see above for licensing)
        2 midi_Defs.h         Copyright (c) 2015 Francois Best (see above for licensing)
        2 midi_Platform.h     Copyright (c) 2015 Francois Best (see above for licensing)
        2 midi_Settings.h     Copyright (c) 2015 Francois Best (see above for licensing)
        2 midi_Message.h      Copyright (c) 2015 Francois Best (see above for licensing)
        2 serialMIDI.h        Copyright (c) 2015 Francois Best (see above for licensing)    
    
    1 Adafruit_GFX.h    Copyright (c) 2012 Adafruit Industries.  All rights reserved.
                        Redistribution and use in source and binary forms, with or without
                        modification, are permitted provided that the following conditions are met:
                        - Redistributions of source code must retain the above copyright notice,
                        this list of conditions and the following disclaimer.
                        - Redistributions in binary form must reproduce the above copyright notice,
                        this list of conditions and the following disclaimer in the documentation
                        and/or other materials provided with the distribution.
                        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
                        AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
                        IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
                        ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
                        LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
                        CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
                        SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
                        INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
                        CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
                        ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
                        POSSIBILITY OF SUCH DAMAGE.

                        notes:
                        devo mantenere intatti gli avvisi di copyright e le condizioni della licenza, e
                        assicurare che queste informazioni siano incluse nella documentazione o nei materiali
                        distribuiti con il software.

        2 Arduino.h (see above)
        2 Print.h   Copyright (c) 2017 PJRC.COM, LLC. (see above for licensing)
        2 gfxfont.h
        2 Adafruit_I2CDevice.h
        2 Adafruit_SPIDevice.h

    1 Adafruit_ILI9341.h    Copyright (c) 2012 Adafruit Industries.  All rights reserved.
                            
                            @file Adafruit_ILI9341.h
                            This is the documentation for Adafruit's ILI9341 driver for the
                            Arduino platform.
                            This library works with the Adafruit 2.8" Touch Shield V2 (SPI)
                            http://www.adafruit.com/products/1651
                            Adafruit 2.4" TFT LCD with Touchscreen Breakout w/MicroSD Socket - ILI9341
                            https://www.adafruit.com/product/2478
                            2.8" TFT LCD with Touchscreen Breakout Board w/MicroSD Socket - ILI9341
                            https://www.adafruit.com/product/1770
                            2.2" 18-bit color TFT LCD display with microSD card breakout - ILI9340
                            https://www.adafruit.com/product/1770
                            TFT FeatherWing - 2.4" 320x240 Touchscreen For All Feathers
                            https://www.adafruit.com/product/3315
                            These displays use SPI to communicate, 4 or 5 pins are required
                            to interface (RST is optional).
                            Adafruit invests time and resources providing this open source code,
                            please support Adafruit and open-source hardware by purchasing
                            products from Adafruit!
                            This library depends on <a href="https://github.com/adafruit/Adafruit_GFX">
                            Adafruit_GFX</a> being present on your system. Please make sure you have
                            installed the latest version before using this library.
                            Written by Limor "ladyada" Fried for Adafruit Industries.
                            BSD license, all text here must be included in any redistribution.

                            notes:
                            Chi modifica un programma protetto da licenze BSD può ridistribuirlo usando la stessa
                            o qualunque altra licenza (anche non libera), senza avere l'obbligo di redistribuire
                            le modifiche apportate al codice sorgente. 

        2 Adafruit_GFX.h    (see above)
        2 Arduino.h (see above)
        2 Print.h (see above)
        2 Adafruit_SPITFT.h
                                @file Adafruit_SPITFT.h
                                Part of Adafruit's GFX graphics library. Originally this class was
                                written to handle a range of color TFT displays connected via SPI,
                                but over time this library and some display-specific subclasses have
                                mutated to include some color OLEDs as well as parallel-interfaced
                                displays. The name's been kept for the sake of older code.

                                Adafruit invests time and resources providing this open source code,
                                please support Adafruit and open-source hardware by purchasing
                                products from Adafruit!
                                Written by Limor "ladyada" Fried for Adafruit Industries,
                                with contributions from the open source community.
                                BSD license, all text here must be included in any redistribution.

                                notes:
                                Chi modifica un programma protetto da licenze BSD può ridistribuirlo usando
                                la stessa o qualunque altra licenza (anche non libera), senza avere l'obbligo
                                di redistribuire le modifiche apportate al codice sorgente.

        2 SPI.h (see above)

    1 memcpy_audio.h    Copyright (c) 2016 Frank Bösing
                        Permission is hereby granted, free of charge, to any person obtaining
                        a copy of this software and associated documentation files (the
                        "Software"), to deal in the Software without restriction, including
                        without limitation the rights to use, copy, modify, merge, publish,
                        distribute, sublicense, and/or sell copies of the Software, and to
                        permit persons to whom the Software is furnished to do so, subject to
                        the following conditions:
                        1. The above copyright notice and this permission notice shall be
                        included in all copies or substantial portions of the Software.
                        2. If the Software is incorporated into a build system that allows
                        selection among a list of target devices, then similar target
                        devices manufactured by PJRC.COM must be included in the list of
                        target devices and selectable in the same manner.
                        THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
                        EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
                        MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
                        NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
                        BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
                        ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
                        CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
                        SOFTWARE.

                        notes:
                        1. Il codice è pubblicato in forma di main e classi compilate; i sorgenti non
                           sono pubblici, non ci sono commenti leggibili; 
                        2. Il software NON consente di utilizzare un'altra piattaforma diversa del
                           Teensy 4.1.

    1 output_noiseshaped_pwm.h  Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com  (see above for
                                licensing)
        2 Arduino.h       (see above)
        2 AudioStream.h   (see above)
        2 DMAChannel.h    (see above)
        
    1 mixer.h   Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com (see above for licensing)
                notes: class modified

        2 Arduino.h         (see above)
        2 AudioStream.h     (see above)

    1 filter_biquad.h   Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com (see above for licensing)
                        notes: class modified

        2 Arduino.h         (see above)
        2 AudioStream.h     (see above)
