/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "SharedElements.h"

// SETUP
int key_step;
uint8_t optimization;
int8_t first_octave;

// GESTIONE EEPROM
EEPROM_Session_struct EEPROM_Session; // local copy of sessions

// FILES
const char name_file[RAW_FILES][10] =
    {
        // Normal files
        "0.raw", "1.raw", "2.raw", "3.raw", "4.raw", "5.raw", "6.raw", "7.raw", "8.raw", "9.raw", "10.raw", "11.raw", "12.raw", "13.raw", "14.raw", "15.raw", "16.raw", "17.raw", "18.raw", "19.raw",
        "20.raw", "21.raw", "22.raw", "23.raw", "24.raw", "25.raw", "26.raw", "27.raw", "28.raw", "29.raw", "30.raw", "31.raw", "32.raw", "33.raw", "34.raw", "35.raw", "36.raw", "37.raw", "38.raw", "39.raw",
        "40.raw", "41.raw", "42.raw", "43.raw", "44.raw", "45.raw", "46.raw", "47.raw", "48.raw", "49.raw", "50.raw", "51.raw", "52.raw", "53.raw", "54.raw", "55.raw", "56.raw", "57.raw", "58.raw", "59.raw",
        "60.raw", "61.raw", "62.raw", "63.raw", "64.raw", "65.raw", "66.raw", "67.raw", "68.raw", "69.raw", "70.raw", "71.raw", "72.raw", "73.raw", "74.raw", "75.raw", "76.raw", "77.raw", "78.raw", "79.raw",
        "80.raw", "81.raw", "82.raw", "83.raw", "84.raw", "85.raw", "86.raw", "87.raw", "88.raw", "89.raw", "90.raw", "91.raw", "92.raw", "93.raw", "94.raw", "95.raw", "96.raw", "97.raw", "98.raw", "99.raw",
        "100.raw", "101.raw", "102.raw", "103.raw", "104.raw", "105.raw", "106.raw", "107.raw", "108.raw", "109.raw", "110.raw", "111.raw", "112.raw", "113.raw", "114.raw", "115.raw", "116.raw", "117.raw", "118.raw", "119.raw",
        "120.raw", "121.raw", "122.raw", "123.raw", "124.raw", "125.raw", "126.raw", "127.raw", "128.raw", "129.raw", "130.raw", "131.raw", "132.raw", "133.raw", "134.raw", "135.raw", "136.raw", "137.raw", "138.raw", "139.raw",
        "140.raw", "141.raw", "142.raw", "143.raw", "144.raw", "145.raw", "146.raw", "147.raw", "148.raw", "149.raw", "150.raw", "151.raw", "152.raw", "153.raw", "154.raw", "155.raw", "156.raw", "157.raw", "158.raw", "159.raw",
        "160.raw", "161.raw", "162.raw", "163.raw", "164.raw", "165.raw", "166.raw", "167.raw", "168.raw", "169.raw", "170.raw", "171.raw", "172.raw", "173.raw", "174.raw", "175.raw", "176.raw", "177.raw", "178.raw", "179.raw",
        "180.raw", "181.raw", "182.raw", "183.raw", "184.raw", "185.raw", "186.raw", "187.raw", "188.raw", "189.raw", "190.raw", "191.raw", "192.raw", "193.raw", "194.raw", "195.raw", "196.raw", "197.raw", "198.raw", "199.raw",
        "200.raw", "201.raw", "202.raw", "203.raw", "204.raw", "205.raw", "206.raw", "207.raw", "208.raw", "209.raw", "210.raw", "211.raw", "212.raw", "213.raw", "214.raw", "215.raw", "216.raw", "217.raw", "218.raw", "219.raw",
        "220.raw", "221.raw", "222.raw", "223.raw", "224.raw", "225.raw", "226.raw", "227.raw", "228.raw", "229.raw", "230.raw", "231.raw", "232.raw", "233.raw", "234.raw", "235.raw", "236.raw", "237.raw", "238.raw", "239.raw",
        "240.raw", "241.raw", "242.raw", "243.raw", "244.raw", "245.raw", "246.raw", "247.raw", "248.raw", "249.raw", "250.raw", "251.raw", "252.raw", "253.raw", "254.raw", "255.raw", "256.raw", "257.raw", "258.raw", "259.raw",

        // Direct_Sampling (60 VFS-files)
        // FIRST_RECORDING_FILE = 260
        // 260 - 319 (2 files per each Recording)
        "0.rec", "1.rec", "2.rec", "3.rec", "4.rec", "5.rec", "6.rec", "7.rec", "8.rec", "9.rec", "10.rec", "11.rec", "12.rec", "13.rec", "14.rec", "15.rec", "16.rec", "17.rec", "18.rec", "19.rec",
        "20.rec", "21.rec", "22.rec", "23.rec", "24.rec", "25.rec", "26.rec", "27.rec", "28.rec", "29.rec", "30.rec", "31.rec", "32.rec", "33.rec", "34.rec", "35.rec", "36.rec", "37.rec", "38.rec", "39.rec",
        "40.rec", "41.rec", "42.rec", "43.rec", "44.rec", "45.rec", "46.rec", "47.rec", "48.rec", "49.rec", "50.rec", "51.rec", "52.rec", "53.rec", "54.rec", "55.rec", "56.rec", "57.rec", "58.rec", "59.rec",

        // Live_Sampling, arrays on PSRAM: int16_t* FIFO[FIFO_SAMPLES], _LS_buffer_L[FIFO_LR_SAMPLES], _LS_buffer_R[FIFO_LR_SAMPLES],
        // FIRST_LIVE_SAMPLING_FILE = 320
        // 320, 321, 322
        "Mono.liv", "Left.liv", "Right.liv"};

// GESTIONE DELLA MEMORIA FLASH ESTERNA
int flash_dimension_MB;

// VFS/Virtual File System
int VFS_FAT_table[VFS_PACKETS_DS] = {0};
int VFS_packets = 0; // number of Packets on FLASH (maximum is VFS_PACKETS_MAX)
int VFS_packets_max = 0;
const char PROGMEM name_packet[PACKETS][10] =
    {
        "P0.raw", "P1.raw", "P2.raw", "P3.raw", "P4.raw", "P5.raw", "P6.raw", "P7.raw", "P8.raw", "P9.raw", "P10.raw", "P11.raw", "P12.raw", "P13.raw", "P14.raw", "P15.raw", "P16.raw", "P17.raw", "P18.raw", "P19.raw",
        "P20.raw", "P21.raw", "P22.raw", "P23.raw", "P24.raw", "P25.raw", "P26.raw", "P27.raw", "P28.raw", "P29.raw", "P30.raw", "P31.raw", "P32.raw", "P33.raw", "P34.raw", "P35.raw", "P36.raw", "P37.raw", "P38.raw", "P39.raw",
        "P40.raw", "P41.raw", "P42.raw", "P43.raw", "P44.raw", "P45.raw", "P46.raw", "P47.raw", "P48.raw", "P49.raw", "P50.raw", "P51.raw", "P52.raw", "P53.raw", "P54.raw", "P55.raw", "P56.raw", "P57.raw", "P58.raw", "P59.raw",
        "P60.raw", "P61.raw", "P62.raw", "P63.raw", "P64.raw", "P65.raw", "P66.raw", "P67.raw", "P68.raw", "P69.raw", "P70.raw", "P71.raw", "P72.raw", "P73.raw", "P74.raw", "P75.raw", "P76.raw", "P77.raw", "P78.raw", "P79.raw",
        "P80.raw", "P81.raw", "P82.raw", "P83.raw", "P84.raw", "P85.raw", "P86.raw", "P87.raw", "P88.raw", "P89.raw", "P90.raw", "P91.raw", "P92.raw", "P93.raw", "P94.raw", "P95.raw", "P96.raw", "P97.raw", "P98.raw", "P99.raw",
        "P100.raw", "P101.raw", "P102.raw", "P103.raw", "P104.raw", "P105.raw", "P106.raw", "P107.raw", "P108.raw", "P109.raw", "P110.raw", "P111.raw", "P112.raw", "P113.raw", "P114.raw", "P115.raw", "P116.raw", "P117.raw", "P118.raw", "P119.raw",
        "P120.raw", "P121.raw", "P122.raw", "P123.raw", "P124.raw", "P125.raw", "P126.raw", "P127.raw", "P128.raw", "P129.raw", "P130.raw", "P131.raw", "P132.raw", "P133.raw", "P134.raw", "P135.raw", "P136.raw", "P137.raw", "P138.raw", "P139.raw",
        "P140.raw", "P141.raw", "P142.raw", "P143.raw", "P144.raw", "P145.raw", "P146.raw", "P147.raw", "P148.raw", "P149.raw", "P150.raw", "P151.raw", "P152.raw", "P153.raw", "P154.raw", "P155.raw", "P156.raw", "P157.raw", "P158.raw", "P159.raw",
        "P160.raw", "P161.raw", "P162.raw", "P163.raw", "P164.raw", "P165.raw", "P166.raw", "P167.raw", "P168.raw", "P169.raw", "P170.raw", "P171.raw", "P172.raw", "P173.raw", "P174.raw", "P175.raw", "P176.raw", "P177.raw", "P178.raw", "P179.raw",
        "P180.raw", "P181.raw", "P182.raw", "P183.raw", "P184.raw", "P185.raw", "P186.raw", "P187.raw", "P188.raw", "P189.raw", "P190.raw", "P191.raw", "P192.raw", "P193.raw", "P194.raw", "P195.raw", "P196.raw", "P197.raw", "P198.raw", "P199.raw",
        "P200.raw", "P201.raw", "P202.raw", "P203.raw", "P204.raw", "P205.raw", "P206.raw", "P207.raw", "P208.raw", "P209.raw", "P210.raw", "P211.raw", "P212.raw", "P213.raw", "P214.raw", "P215.raw", "P216.raw", "P217.raw", "P218.raw", "P219.raw",
        "P220.raw", "P221.raw", "P222.raw", "P223.raw", "P224.raw", "P225.raw", "P226.raw", "P227.raw", "P228.raw", "P229.raw", "P230.raw", "P231.raw", "P232.raw", "P233.raw", "P234.raw", "P235.raw", "P236.raw", "P237.raw", "P238.raw", "P239.raw",
        "P240.raw", "P241.raw", "P242.raw", "P243.raw", "P244.raw", "P245.raw", "P246.raw", "P247.raw", "P248.raw", "P249.raw", "P250.raw", "P251.raw", "P252.raw", "P253.raw", "P254.raw", "P255.raw", "P256.raw", "P257.raw", "P258.raw", "P259.raw",
        "P260.raw", "P261.raw", "P262.raw", "P263.raw", "P264.raw", "P265.raw", "P266.raw", "P267.raw", "P268.raw", "P269.raw", "P270.raw", "P271.raw", "P272.raw", "P273.raw", "P274.raw", "P275.raw", "P276.raw", "P277.raw", "P278.raw", "P279.raw",
        "P280.raw", "P281.raw", "P282.raw", "P283.raw", "P284.raw", "P285.raw", "P286.raw", "P287.raw", "P288.raw", "P289.raw", "P290.raw", "P291.raw", "P292.raw", "P293.raw", "P294.raw", "P295.raw", "P296.raw", "P297.raw", "P298.raw", "P299.raw",
        "P300.raw", "P301.raw", "P302.raw", "P303.raw", "P304.raw", "P305.raw", "P306.raw", "P307.raw", "P308.raw", "P309.raw", "P310.raw", "P311.raw", "P312.raw", "P313.raw", "P314.raw", "P315.raw", "P316.raw", "P317.raw", "P318.raw", "P319.raw",
        "P320.raw", "P321.raw", "P322.raw", "P323.raw", "P324.raw", "P325.raw", "P326.raw", "P327.raw", "P328.raw", "P329.raw", "P330.raw", "P331.raw", "P332.raw", "P333.raw", "P334.raw", "P335.raw", "P336.raw", "P337.raw", "P338.raw", "P339.raw",
        "P340.raw", "P341.raw", "P342.raw", "P343.raw", "P344.raw", "P345.raw", "P346.raw", "P347.raw", "P348.raw", "P349.raw", "P350.raw", "P351.raw", "P352.raw", "P353.raw", "P354.raw", "P355.raw", "P356.raw", "P357.raw", "P358.raw", "P359.raw",
        "P360.raw", "P361.raw", "P362.raw", "P363.raw", "P364.raw", "P365.raw", "P366.raw", "P367.raw", "P368.raw", "P369.raw", "P370.raw", "P371.raw", "P372.raw", "P373.raw", "P374.raw", "P375.raw", "P376.raw", "P377.raw", "P378.raw", "P379.raw",
        "P380.raw", "P381.raw", "P382.raw", "P383.raw", "P384.raw", "P385.raw", "P386.raw", "P387.raw", "P388.raw", "P389.raw", "P390.raw", "P391.raw", "P392.raw", "P393.raw", "P394.raw", "P395.raw", "P396.raw", "P397.raw", "P398.raw", "P399.raw",
        "P400.raw", "P401.raw", "P402.raw", "P403.raw", "P404.raw", "P405.raw", "P406.raw", "P407.raw", "P408.raw", "P409.raw", "P410.raw", "P411.raw", "P412.raw", "P413.raw", "P414.raw", "P415.raw", "P416.raw", "P417.raw", "P418.raw", "P419.raw",
        "P420.raw", "P421.raw", "P422.raw", "P423.raw", "P424.raw", "P425.raw", "P426.raw", "P427.raw", "P428.raw", "P429.raw", "P430.raw", "P431.raw", "P432.raw", "P433.raw", "P434.raw", "P435.raw", "P436.raw", "P437.raw", "P438.raw", "P439.raw",
        "P440.raw", "P441.raw", "P442.raw", "P443.raw", "P444.raw", "P445.raw", "P446.raw", "P447.raw", "P448.raw", "P449.raw", "P450.raw", "P451.raw", "P452.raw", "P453.raw", "P454.raw", "P455.raw", "P456.raw", "P457.raw", "P458.raw", "P459.raw",
        "P460.raw", "P461.raw", "P462.raw", "P463.raw", "P464.raw", "P465.raw", "P466.raw", "P467.raw", "P468.raw", "P469.raw", "P470.raw", "P471.raw", "P472.raw", "P473.raw", "P474.raw", "P475.raw", "P476.raw", "P477.raw", "P478.raw", "P479.raw",
        "P480.raw", "P481.raw", "P482.raw", "P483.raw", "P484.raw", "P485.raw", "P486.raw", "P487.raw", "P488.raw", "P489.raw", "P490.raw", "P491.raw", "P492.raw", "P493.raw", "P494.raw", "P495.raw", "P496.raw", "P497.raw", "P498.raw", "P499.raw",
        "P500.raw", "P501.raw", "P502.raw", "P503.raw", "P504.raw", "P505.raw", "P506.raw", "P507.raw", "P508.raw", "P509.raw", "P510.raw", "P511.raw", "P512.raw", "P513.raw", "P514.raw", "P515.raw", "P516.raw", "P517.raw", "P518.raw", "P519.raw",
        "P520.raw", "P521.raw", "P522.raw", "P523.raw", "P524.raw", "P525.raw", "P526.raw", "P527.raw", "P528.raw", "P529.raw", "P530.raw", "P531.raw", "P532.raw", "P533.raw", "P534.raw", "P535.raw", "P536.raw", "P537.raw", "P538.raw", "P539.raw",
        "P540.raw", "P541.raw", "P542.raw", "P543.raw", "P544.raw", "P545.raw", "P546.raw", "P547.raw", "P548.raw", "P549.raw", "P550.raw", "P551.raw", "P552.raw", "P553.raw", "P554.raw", "P555.raw", "P556.raw", "P557.raw", "P558.raw", "P559.raw",
        "P560.raw", "P561.raw", "P562.raw", "P563.raw", "P564.raw", "P565.raw", "P566.raw", "P567.raw", "P568.raw", "P569.raw", "P570.raw", "P571.raw", "P572.raw", "P573.raw", "P574.raw", "P575.raw", "P576.raw", "P577.raw", "P578.raw", "P579.raw",
        "P580.raw", "P581.raw", "P582.raw", "P583.raw", "P584.raw", "P585.raw", "P586.raw", "P587.raw", "P588.raw", "P589.raw", "P590.raw", "P591.raw", "P592.raw", "P593.raw", "P594.raw", "P595.raw", "P596.raw", "P597.raw", "P598.raw", "P599.raw",
        "P600.raw", "P601.raw", "P602.raw", "P603.raw", "P604.raw", "P605.raw", "P606.raw", "P607.raw", "P608.raw", "P609.raw", "P610.raw", "P611.raw", "P612.raw", "P613.raw", "P614.raw", "P615.raw", "P616.raw", "P617.raw", "P618.raw", "P619.raw",
        "P620.raw", "P621.raw", "P622.raw", "P623.raw", "P624.raw", "P625.raw", "P626.raw", "P627.raw", "P628.raw", "P629.raw", "P630.raw", "P631.raw", "P632.raw", "P633.raw", "P634.raw", "P635.raw", "P636.raw", "P637.raw", "P638.raw", "P639.raw",
        "P640.raw", "P641.raw", "P642.raw", "P643.raw", "P644.raw", "P645.raw", "P646.raw", "P647.raw", "P648.raw", "P649.raw", "P650.raw", "P651.raw", "P652.raw", "P653.raw", "P654.raw", "P655.raw", "P656.raw", "P657.raw", "P658.raw", "P659.raw",
        "P660.raw", "P661.raw", "P662.raw", "P663.raw", "P664.raw", "P665.raw", "P666.raw", "P667.raw", "P668.raw", "P669.raw", "P670.raw", "P671.raw", "P672.raw", "P673.raw", "P674.raw", "P675.raw", "P676.raw", "P677.raw", "P678.raw", "P679.raw",
        "P680.raw", "P681.raw", "P682.raw", "P683.raw", "P684.raw", "P685.raw", "P686.raw", "P687.raw", "P688.raw", "P689.raw", "P690.raw", "P691.raw", "P692.raw", "P693.raw", "P694.raw", "P695.raw", "P696.raw", "P697.raw", "P698.raw", "P699.raw",
        "P700.raw", "P701.raw", "P702.raw", "P703.raw", "P704.raw", "P705.raw", "P706.raw", "P707.raw", "P708.raw", "P709.raw", "P710.raw", "P711.raw", "P712.raw", "P713.raw", "P714.raw", "P715.raw", "P716.raw", "P717.raw", "P718.raw", "P719.raw",
        "P720.raw", "P721.raw", "P722.raw", "P723.raw", "P724.raw", "P725.raw", "P726.raw", "P727.raw", "P728.raw", "P729.raw", "P730.raw", "P731.raw", "P732.raw", "P733.raw", "P734.raw", "P735.raw", "P736.raw", "P737.raw", "P738.raw", "P739.raw",
        "P740.raw", "P741.raw", "P742.raw", "P743.raw", "P744.raw", "P745.raw", "P746.raw", "P747.raw", "P748.raw", "P749.raw", "P750.raw", "P751.raw", "P752.raw", "P753.raw", "P754.raw", "P755.raw", "P756.raw", "P757.raw", "P758.raw", "P759.raw",
        "P760.raw", "P761.raw", "P762.raw", "P763.raw", "P764.raw", "P765.raw", "P766.raw", "P767.raw", "P768.raw", "P769.raw", "P770.raw", "P771.raw", "P772.raw", "P773.raw", "P774.raw", "P775.raw", "P776.raw", "P777.raw", "P778.raw", "P779.raw",
        "P780.raw", "P781.raw", "P782.raw", "P783.raw", "P784.raw", "P785.raw", "P786.raw", "P787.raw", "P788.raw", "P789.raw", "P790.raw", "P791.raw", "P792.raw", "P793.raw", "P794.raw", "P795.raw", "P796.raw", "P797.raw", "P798.raw", "P799.raw",
        "P800.raw", "P801.raw", "P802.raw", "P803.raw", "P804.raw", "P805.raw", "P806.raw", "P807.raw", "P808.raw", "P809.raw", "P810.raw", "P811.raw", "P812.raw", "P813.raw", "P814.raw", "P815.raw", "P816.raw", "P817.raw", "P818.raw", "P819.raw",
        "P820.raw", "P821.raw", "P822.raw", "P823.raw", "P824.raw", "P825.raw", "P826.raw", "P827.raw", "P828.raw", "P829.raw", "P830.raw", "P831.raw", "P832.raw", "P833.raw", "P834.raw", "P835.raw", "P836.raw", "P837.raw", "P838.raw", "P839.raw",
        "P840.raw", "P841.raw", "P842.raw", "P843.raw", "P844.raw", "P845.raw", "P846.raw", "P847.raw", "P848.raw", "P849.raw", "P850.raw", "P851.raw", "P852.raw", "P853.raw", "P854.raw", "P855.raw", "P856.raw", "P857.raw", "P858.raw", "P859.raw",
        "P860.raw", "P861.raw", "P862.raw", "P863.raw", "P864.raw", "P865.raw", "P866.raw", "P867.raw", "P868.raw", "P869.raw", "P870.raw", "P871.raw", "P872.raw", "P873.raw", "P874.raw", "P875.raw", "P876.raw", "P877.raw", "P878.raw", "P879.raw",
        "P880.raw", "P881.raw", "P882.raw", "P883.raw", "P884.raw", "P885.raw", "P886.raw", "P887.raw", "P888.raw", "P889.raw", "P890.raw", "P891.raw", "P892.raw", "P893.raw", "P894.raw", "P895.raw", "P896.raw", "P897.raw", "P898.raw", "P899.raw",
        "P900.raw", "P901.raw", "P902.raw", "P903.raw", "P904.raw", "P905.raw", "P906.raw", "P907.raw", "P908.raw", "P909.raw", "P910.raw", "P911.raw", "P912.raw", "P913.raw", "P914.raw", "P915.raw", "P916.raw", "P917.raw", "P918.raw", "P919.raw",
        "P920.raw", "P921.raw", "P922.raw", "P923.raw", "P924.raw", "P925.raw", "P926.raw", "P927.raw", "P928.raw", "P929.raw", "P930.raw", "P931.raw", "P932.raw", "P933.raw", "P934.raw", "P935.raw", "P936.raw", "P937.raw", "P938.raw", "P939.raw",
        "P940.raw", "P941.raw", "P942.raw", "P943.raw", "P944.raw", "P945.raw", "P946.raw", "P947.raw", "P948.raw", "P949.raw", "P950.raw", "P951.raw", "P952.raw", "P953.raw", "P954.raw", "P955.raw", "P956.raw", "P957.raw", "P958.raw", "P959.raw",
        "P960.raw", "P961.raw", "P962.raw", "P963.raw", "P964.raw", "P965.raw", "P966.raw", "P967.raw", "P968.raw", "P969.raw", "P970.raw", "P971.raw", "P972.raw", "P973.raw", "P974.raw", "P975.raw", "P976.raw", "P977.raw", "P978.raw", "P979.raw",
        "P980.raw", "P981.raw", "P982.raw", "P983.raw", "P984.raw", "P985.raw", "P986.raw", "P987.raw", "P988.raw", "P989.raw", "P990.raw", "P991.raw", "P992.raw", "P993.raw", "P994.raw", "P995.raw", "P996.raw", "P997.raw", "P998.raw", "P999.raw",
        "P1000.raw", "P1001.raw", "P1002.raw", "P1003.raw", "P1004.raw", "P1005.raw", "P1006.raw", "P1007.raw", "P1008.raw", "P1009.raw", "P1010.raw", "P1011.raw", "P1012.raw", "P1013.raw", "P1014.raw", "P1015.raw", "P1016.raw", "P1017.raw", "P1018.raw", "P1019.raw",
        "P1020.raw", "P1021.raw", "P1022.raw", "P1023.raw"};

EEPROM_VFS_Recording EEPROM_Recording[RECORDINGS];
VFS_Recording Recording[RECORDINGS];

// LILLA STATE
uint8_t Lilla_state;
uint8_t Lilla_state_0;

// SESSION
Session_struct Session[SESSIONS_MAX + 1]; // local copy of sessions
Sound_struct Sound[SOUNDS_MAX + 2];

// PERFORMANCE
uint8_t session = 0;
int volume_session = 29;
uint8_t map_instrument_for_note[16][128] = {0};
bool key_state[16][128] = {0}; // usato solo a fini statistici; key premuti su ciascun canale midi; rilevato attaverso il conteggio dei NoteOn
bool file_midi_ch_flag = true;
int8_t position_of_Instrument[INSTRUMENTS_MAX];
const char note_name[12][3] = {{"C"}, {"C#"}, {"D"}, {"D#"}, {"E"}, {"F"}, {"F#"}, {"G"}, {"G#"}, {"A"}, {"A#"}, {"B"}};
float pitch_from_note[128] = {0};
bool display_instrument_volume_flag = false;
uint8_t instrument_volume_changed = 0;

// menu
bool instrument_editing_flag = false;
uint8_t choice_performance_menu;
char Menu_P_char[5][12] = {{"EXIT"}, {"SAVE"}, {"CLONE"}, {"SAVE_AS_NEW"}, {"DROP"}};
uint8_t dimension_voice_Menu_P[5] = {4, 4, 5, 11, 4};
int X_position_Menu_P[5];   // argument is position
int Y_position_Menu_P[5];   // argument is position
uint8_t element_Menu_P[5];  // argument is position
uint8_t position_Menu_P[5]; // argument is element
bool Menu_P[5];

// Tuning tone
int tuning_tone_volume = 10;
uint8_t tuning_tone_last_note = 0;
bool tuning_tone_flag = false;
bool TT_playing = false;
bool TT_led_flag = false;

// SOUND EDIT
uint8_t trim_speed = 5;
uint16_t Noclick_max;
bool solo_flag = false;
bool slicing_mode = true; // true: slicing AB  - false: slicing A-Samples
const char name_mode[6][8] = {{"FWD"}, {"REV"}, {"FWD"}, {"FWD-REV"}, {"REV-FWD"}, {"REV"}};
char loop_mode[6][5] = {{"once"}, {"once"}, {"loop"}, {"loop"}, {"loop"}, {"loop"}};

// Menu
uint8_t choice_sound_menu;
char Menu_So_char[3][7] = {{"RETURN"}, {"CLONE"}, {"DROP"}};
uint8_t dimension_voice_Menu_So[3] = {6, 5, 4};
uint8_t X_position_Menu_So[3]; // argument is position
uint8_t element_Menu_So[3];    // argument is position
uint8_t position_Menu_So[3];   // argument is element
bool Menu_So[3];

// array di costanti
const int16_t Sine_wave[257] =
    {0, 804, 1608, 2410, 3212, 4011, 4808, 5602, 6393, 7179,
     7962, 8739, 9512, 10278, 11039, 11793, 12539, 13279, 14010, 14732,
     15446, 16151, 16846, 17530, 18204, 18868, 19519, 20159, 20787, 21403,
     22005, 22594, 23170, 23731, 24279, 24811, 25329, 25832, 26319, 26790,
     27245, 27683, 28105, 28510, 28898, 29268, 29621, 29956, 30273, 30571,
     30852, 31113, 31356, 31580, 31785, 31971, 32137, 32285, 32412, 32521,
     32609, 32678, 32728, 32757, 32767, 32757, 32728, 32678, 32609, 32521,
     32412, 32285, 32137, 31971, 31785, 31580, 31356, 31113, 30852, 30571,
     30273, 29956, 29621, 29268, 28898, 28510, 28105, 27683, 27245, 26790,
     26319, 25832, 25329, 24811, 24279, 23731, 23170, 22594, 22005, 21403,
     20787, 20159, 19519, 18868, 18204, 17530, 16846, 16151, 15446, 14732,
     14010, 13279, 12539, 11793, 11039, 10278, 9512, 8739, 7962, 7179,
     6393, 5602, 4808, 4011, 3212, 2410, 1608, 804,
     0, -804, -1608, -2410, -3212, -4011, -4808, -5602, -6393, -7179,
     -7962, -8739, -9512, -10278, -11039, -11793, -12539, -13279, -14010, -14732,
     -15446, -16151, -16846, -17530, -18204, -18868, -19519, -20159, -20787, -21403,
     -22005, -22594, -23170, -23731, -24279, -24811, -25329, -25832, -26319, -26790,
     -27245, -27683, -28105, -28510, -28898, -29268, -29621, -29956, -30273, -30571,
     -30852, -31113, -31356, -31580, -31785, -31971, -32137, -32285, -32412, -32521,
     -32609, -32678, -32728, -32757, -32767, -32757, -32728, -32678, -32609, -32521,
     -32412, -32285, -32137, -31971, -31785, -31580, -31356, -31113, -30852, -30571,
     -30273, -29956, -29621, -29268, -28898, -28510, -28105, -27683, -27245, -26790,
     -26319, -25832, -25329, -24811, -24279, -23731, -23170, -22594, -22005, -21403,
     -20787, -20159, -19519, -18868, -18204, -17530, -16846, -16151, -15446, -14732,
     -14010, -13279, -12539, -11793, -11039, -10278, -9512, -8739, -7962, -7179,
     -6393, -5602, -4808, -4011, -3212, -2410, -1608, -804, 0};
const float waveform_decay[11] = {1.0, 0.600024, 0.34, 0.189978, 0.079989, 0.029511, 0.013886, 0.006012, 0.002106, 0.0002136, 0};  // {32767,19661,11141,6225,2621,967,455,197,69,7,0};
const float waveform_attack[11] = {0, 0.0002136, 0.002106, 0.006012, 0.013886, 0.029511, 0.079989, 0.189978, 0.34, 0.600024, 1.0}; // {32767,19661,11141,6225,2621,967,455,197,69,7,0};
const float Volume_float[41] =
    {0, 0.00015, 0.0005, 0.002, 0.008, 0.02, 0.037, 0.058, 0.08, 0.10125,
     0.125, 0.15125, 0.18, 0.21125, 0.245, 0.28125, 0.32, 0.36125, 0.405, 0.45125,
     0.5, 0.55125, 0.605, 0.66125, 0.72, 0.78125, 0.845, 0.91125, 0.98, 1.0,
     1.125, 1.20125, 1.28, 1.36125, 1.445, 1.53125, 1.62, 1.71125, 1.805, 1.90125, 2};
const float exp_table[11] = {0.0, 0.5000, 0.7500, 0.8750, 0.9375, 0.9688, 0.9844, 0.9922, 0.9961, 0.9980, 1.0};
const float sin_table[11] = {0.0, 0.0245, 0.0955, 0.2061, 0.3455, 0.5, 0.6545, 0.7939, 0.9045, 0.9755, 1.0};
const float decay_table[11] = {1.0, 0.9755, 0.9045, 0.7939, 0.6545, 0.5, 0.3455, 0.2061, 0.0955, 0.0245, 0.0};
const float release_table[11] = {1.0, 0.4991, 0.2487, 0.1234, 0.0608, 0.0295, 0.0139, 0.0060, 0.0021, 0.0002, 0.0};

// array compilati al setup()
float m_exp_table[10];
float m_sin_table[10];
float m_decay_table[10];
float m_release_table[10];
float pan_gain_L_table[33];
float pan_gain_R_table[33];

// funzioni
uint8_t Get_midi_channel(uint8_t session, uint8_t instrument)
{
    // .data contains midi channel in its bits: 7 6 5 M I D I 0
    return ((Sound[Session[session].Instrument[instrument].id_sound].data & 30) >> 1);
}
float Calc_pitch(float value)
{
    return pow(2.0f, value / 192.0f); // 0: no shift
}
float Calc_attack(uint8_t &n)
{
    return (n / 100.0f);
}
float CALC_decay(uint8_t &n)
{
    return (n / 25.0f);
}
float Calc_sustain(uint8_t &n)
{
    return (n / 50.0f);
}
float Calc_release(uint8_t &n)

{
    return (n / 2.0f);
}

// PRESET
Preset_struct Preset[INSTRUMENTS_MAX];

// DIRECT SAMPLING
bool Menu_DS[DS_MV];
const char Menu_DS_char[][19] = {{"CANCEL_RECORDING"}, {"PAUSE+REC"}, {"MONO_REC"}, {"STEREO_REC"}, {"STOP"}, {"MAKE_RAW"}, {"CANCEL"}, {"MAKE_RAW"}, {"MAKE_LEFT"}, {"MAKE_RIGHT"}, {"MAKE_BOTH"}, {"EXPORT_RAW_TO_SD"}};
const uint8_t dimension_voice_Menu_DS[DS_MV] = {16, 9, 8, 10, 4, 8, 6, 8, 9, 10, 9, 16};
uint8_t X_position_Menu_DS[DS_MV]; // argument is position
uint8_t Y_position_Menu_DS[DS_MV]; // argument is position
uint8_t element_Menu_DS[DS_MV];    // argument is position
uint8_t position_Menu_DS[DS_MV];   // argument is element
uint8_t choice_DS_menu;

// variabili
int recordings;
int recording = -1; // recording id online
int DS_gain;
elapsedMillis DS_blink_timer;
bool DS_blink_ON = false;

// LIVE SAMPLING
// Menu
bool Menu_LS[LS_MV];
const char Menu_LS_char[][12] = {{"REC"}, {"STOP"}, {"MONO/STEREO"}, {"ERASE"}};
const uint8_t dimension_voice_Menu_LS[LS_MV] = {3, 4, 11, 5};
uint8_t X_position_Menu_LS[LS_MV]; // argument is position
uint8_t element_Menu_LS[LS_MV];    // argument is position
uint8_t position_Menu_LS[LS_MV];   // argument is element

// variabili
int LS_state;
int LS_feedback;
uint8_t LS_mode; // playing mode
int LS_X_step;
int LS_window_width; // samples from LS_window_A_sample to LS_window_B_sample
bool LS_stereo;
int LS_Y_sample;
int LS_Q_sample;
int LS_buffer_dim;
bool LS_XY_lock;
int LS_X_sample;
int LS_X_delta;
int LS_XY_delta;

// waveform
int LS_constrain_position(int value)
{
    while (value < 0)
        value += LS_buffer_dim;
    while (value > LS_buffer_dim - 1)
        value -= LS_buffer_dim;
    return value;
}

// DELAY
Delay_values_struct Delay_values;
Delay_data_struct Delay_data;

const int PROGMEM delay_samples_table[100] =
    {0, 11, 22, 33, 44, 55, 66, 77, 88, 110, 132, 154, 176, 221,
     265, 309, 353, 397, 485, 573, 662, 750, 838, 1058, 1279, 1499,
     1720, 1940, 2381, 2822, 3263, 3704, 4145, 4586, 5027, 5468, 5909,
     6350, 7232, 8114, 8996, 9878, 10760, 11642, 12524, 13406, 14876,
     16346, 17816, 19286, 20756, 22226, 23696, 25166, 26636, 28106,
     29576, 31046, 32516, 34721, 36926, 39131, 41336, 43541, 45746,
     47951, 50156, 52361, 54566, 56771, 58976, 61181, 63386, 65591,
     67796, 70001, 72206, 74411, 76616, 81026, 85436, 89846, 94256,
     98666, 103076, 107486, 111896, 116306, 120716, 125126, 132476,
     139826, 147176, 154526, 161876, 169226, 176576, 183926, 196000, 220500};

float Delay_feedback(int8_t value) // feedback
{
    const float answer[] = {0, -0.07, -0.18, -0.4, -0.6, -0.65, -0.71, -0.80, -0.92, -0.98};
    value = constrain(value, 0, 10);
    return answer[value];
}

void Calc_Delay_values(Delay_data_struct data)
{
    Calc_delay_routing(data.instrument_route);
    Delay_values.samples = Calc_delay_samples(data.samples);
    Delay_values.samples_LR = Calc_delay_samples_LR(data.samples_LR);
    Delay_values.modulation_source = data.modulation_source;
    Delay_values.modulation_depth = Calc_delay_depth(data.modulation_depth);
    Delay_values.modulation_frequency = Calc_delay_frequency(data.modulation_frequency);
    Delay_values.modulation_phase_LR = data.modulation_phase_LR;
    Delay_values.loop_gain = Delay_feedback(data.loop_gain); // feedback

    if (true)
        Print_Delay_values(Delay_values);
}

FLASHMEM
void Print_Delay_values(Delay_values_struct Delay_values)
{
    Serial.println();
    Serial.println("Print Delay_values");
    Serial.print("Delay_values.samples: ");
    Serial.println(Delay_values.samples);
    Serial.print("Delay_values.samples_LR: ");
    Serial.println(Delay_values.samples_LR);
    Serial.print("Delay_values.modulation_source: ");
    Serial.println(Delay_values.modulation_source);
    Serial.print("Delay_values.modulation_depth: ");
    Serial.println(Delay_values.modulation_depth);
    Serial.print("Delay_values.modulation_frequency: ");
    Serial.println(Delay_values.modulation_frequency);
    Serial.print("Delay_values.modulation_phase_LR: ");
    Serial.println(Delay_values.modulation_phase_LR);
    Serial.print("Delay_values.loop_gain: ");
    Serial.println(Delay_values.loop_gain);
    for (int i = 0; i < INSTRUMENTS_MAX; ++i)
    {
        Serial.print("Delay_values.instrument_route[");
        Serial.print(i);
        Serial.print("]: ");
        Serial.println(Delay_values.instrument_route[i]);
    }
    Serial.println();
}

FLASHMEM
void Print_Delay_data(const byte *data)
{       
        Serial.println();
        Serial.println(F("Print Delay_data"));
        byte data_LSB;
        byte data_MSB;
        int16_t result_int;
        uint16_t result_uint;
        int i = 0;

        Serial.print("uint16_t samples: ");
        data_LSB = *(data + i++);
        data_MSB = *(data + i++);
        result_uint = data_MSB << 8 | data_LSB;
        Serial.println(result_uint);

        Serial.print("int16_t samples_LR: ");
        data_LSB = *(data + i++);
        data_MSB = *(data + i++);
        result_int = data_MSB << 8 | data_LSB;
        Serial.println(result_int);

        Serial.print("instrument_route: ");
        Serial.println(*(data + i++));

        Serial.print("modulation: ");
        Serial.println(*(data + i++));

        Serial.print("depth: ");
        Serial.println(*(data + i++));

        Serial.print("frequency: ");
        Serial.println(*(data + i++));

        Serial.print("uint16_t phase_LR: ");
        data_LSB = *(data + i++);
        data_MSB = *(data + i++);
        result_uint = data_MSB << 8 | data_LSB;
        Serial.println(result_uint);

        Serial.print("uint16_t loop_gain: ");
        data_LSB = *(data + i++);
        data_MSB = *(data + i++);
        result_uint = data_MSB << 8 | data_LSB;
        Serial.println(result_uint);
        Serial.println();
}

void Calc_delay_routing(uint8_t value)
{
    for (int i = 0; i < INSTRUMENTS_MAX; ++i)
    {
        Delay_values.instrument_route[i] = (bitRead(value, i) == 1 ? true : false);
        if (false)
        {
            Serial.print("Calc_delay_routing(uint8_t value) - Delay_values.instrument_route[i]: ");
            Serial.println(Delay_values.instrument_route[i]);
        }
    }
}

void Turn_ON_Delay(bool ON) // switch on/off Delay (using Instrument routing)
{
    static int instrument_route[INSTRUMENTS_MAX];
    if (!ON)
        for (int i = 0; i < INSTRUMENTS_MAX; ++i)
        {
            instrument_route[i] = Delay_values.instrument_route[i];
            Delay_values.instrument_route[i] = 0;
        }
    else
        for (int i = 0; i < INSTRUMENTS_MAX; ++i)
        {
            Delay_values.instrument_route[i] = instrument_route[i];
        }
}

float Calc_delay_samples(int value) // 0 --> 99
{
    value = constrain(value, 0, 99);
    return delay_samples_table[value];
}

float Calc_delay_samples_LR(int value) // 0 --> 50
{
    return value * 44.1;
}

float Calc_delay_depth(uint8_t value)
{
    const float read_depth_array[40] = {
        0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 1.0,
        2, 3, 4, 5, 6, 8, 10, 12, 14, 16,
        18, 20, 24, 28, 32, 36, 40, 44, 48, 52,
        56, 60, 65, 70, 75, 80, 85, 90, 95, 100};
    return read_depth_array[value] / 100.0f; // 0 <= value <= 39
}

float Calc_delay_frequency(uint8_t value)
{
    return (value * value / 90.0f); // 0 <= value <= 90
}

// MIDI LOOP
LOOP_struct LOOP_element[TRACKS][LOOP_EVENTS];
byte LOOP_events[TRACKS] = {0};
int LOOP_slide[TRACKS] = {0};
float LOOP_stretch = 1.0;

// Menu
int Loop_menu_max;
uint8_t choice_loop_menu;
const char Menu_Loop_char[4][12] = {{"NEW"}, {"SAVE"}, {"SAVE_AS_NEW"}, {"DELETE"}};
const uint8_t dimension_voice_Menu_Loop[4] = {3, 4, 11, 6};
uint8_t X_position_Menu_Loop[4]; // argument is position
bool Menu_Loop[4];
uint8_t element_Menu_Loop[4];  // argument is position
uint8_t position_Menu_Loop[4]; // argument is element

// LOOP play/stop
bool LOOP_track_run[TRACKS] = {false};
uint32_t LOOP_play_time[TRACKS] = {0};
int LOOP_play_event[TRACKS] = {0};
float LOOP_volume[TRACKS] = {0};
uint16_t LOOP_time;
int LOOP_pitch_int[TRACKS] = {0};

// LOOP learn
int LOOP_learning_track;
bool LOOP_learn_flag;
int LOOP_elements;
elapsedMillis LOOP_learn_clock;
int LOOP_clock_memo;
int LOOP_last_event;

// LOOP leds
bool LOOP_led_flag[TRACKS] = {false};

// LOOP_metronomo
bool LOOP_metronomo_run = false; // il metronomo e' running

// LOOP metronomo, richieste da MidiReader a Main
bool LOOP_metronomo_flag_IN[2] = {false, false}; // accendi led_0, switch led del metronomo

// LOOP metronomo, richieste da Main a MidiReader
// bool LOOP_metronomo_flag_OUT = {false}; // run metronomo
// unsigned long LOOP_metronomo_update_time = 0;  // prossimo istante di aggiornamento

// LOOP salvataggio su SD
int LOOP_id;

// MIXER
uint8_t MX_source = 0; // 0-->7: Sound 8: InputDevice
uint8_t PWM_volume = VOLUME_1;
uint8_t MAIN_volume = VOLUME_1;
uint8_t MX_routing_source[9] = {3, 3, 3, 3, 3, 3, 3, 3, 3}; // 0-->7: Sound 8: InputDevice; 1 --> source routed to PWM output (monitor); 2 --> source routed to Audio Board output;  3 --> source routed to both
bool MX_mute[9] = {false, false, false, false, false, false, false, false, false};

// MIDI MONITOR
uint8_t MM_midi_channel = 0;
uint8_t MM_note_number = 0;
uint8_t MM_velocity = 0;
uint8_t MM_midi_controller = 0;
uint8_t MM_midi_value = 0;
uint8_t MM_pitch_bend_least = 0;
uint8_t MM_pitch_bend_most = 0;
uint8_t MM_least_bits = 0;
uint8_t MM_most_bits = 0;
uint8_t midi_message_received = 0;
bool display_wait = false; // used in MIDI_MONITOR, CC_SETTING

// PLAYER
elapsedMicros big_T = 0;
int16_t *Noclick_pointer[INSTRUMENTS_MAX] = {0};   // each Noclick instance contains 2 arrays
int16_t *Wavetable_pointer[INSTRUMENTS_MAX] = {0}; // each Wavetable instance contains 2 arrays

// funzioni
void Update_map_Instrument_for_notes(uint8_t from_note, uint8_t to_note, uint8_t instrument) // aggiorna la mappatura tra tutte Instrument e le coppie midi_channel/note_number e relative
{
    for (uint8_t note = 0; note < 128; note++)
    {
        if (note >= from_note && note <= to_note)
            bitWrite(map_instrument_for_note[Get_midi_channel(session, instrument)][note], instrument, 1);
        else
            bitWrite(map_instrument_for_note[Get_midi_channel(session, instrument)][note], instrument, 0);
    }
}
void Reset_keys_state()
{
    for (uint8_t a = 0; a < 16; a++)
        for (uint8_t b = 0; b < 128; b++)
            key_state[a][b] = false; // true: key pressed down
}

// LPF FILTER RESOLUTION DOWNSAMPLING
bool lowpass_flag = false;
bool lowpass_direction = false;
int lowpass = LPF_MAX;
int lowpass_target = LPF_MAX;
const int lowpass_value[40] =
    {40, 48, 56, 65, 75, 90, 100, 125, 145, 170,
     200, 230, 270, 320, 380, 440, 520, 600, 720, 840,
     980, 1150, 1350, 1600, 1900, 2200, 2500, 3000, 3500, 4100,
     4900, 5700, 6700, 7900, 9200, 10000, 12500, 14500, 17500, 20000}; // 0 -> 39
bool display_lowpass_flag = false;
int resolution = 68;
int downsampling = 1;
float Calc_resolution(int value) // 0<= value <= 68 ; 1.0bit <= return <= 16.0bit
{
    if (value < 62)
        return 1.0 + 9 * (value / 62.0f);
    else
        return value - 52;
}

// PITCH BEND - AFTER TOUCH - VIBRATO
float pitch_bend_value[16] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
float after_touch_channel_value[16] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

// CONTROL CHANGE
uint8_t CC_Sound_gain[INSTRUMENTS_MAX] = {0};
uint8_t CC_lowpass_filter = 0;
uint8_t CC_midi_controller = 0;

// LED (NoteOn/Off da tastiera)
bool led_flag = false;

// STAMPA
void PRINT(String who, String what, float value)
{
    Serial.print(who);
    Serial.print(" ");
    Serial.print(what);
    Serial.print(": ");
    Serial.println(value);
}