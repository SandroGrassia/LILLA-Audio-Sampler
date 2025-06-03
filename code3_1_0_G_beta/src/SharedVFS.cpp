/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "SharedVFS.h"


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