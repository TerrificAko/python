auto_cali_cfg
//phy_debug_on
2E 20 00 03 55 03 01

//atest sw, rfsw, trx on
2E 20 00 09 55 01 10 60 00 06 00 0b aa

//rx vbgr ldoh1p8
2E 20 00 09 55 01 10 5c 1c 40 00 00 aa

//adc tune
2E 20 00 09 55 01 10 3c e0 00 00 00 aa

//xo_freq_change
2E 20 00 09 55 01 10 14 18 41 00 18 aa


//cfg analog            cal_clk_div_en
2E 20 00 09 55 01 10 40 00 00 00 02 aa

//cali_wait_time vga_gain_sel & vga_gain_sel_ovrd  sel1&sel2 map value :1.22
2E 20 00 09 55 01 11 44 0F F1 00 00 aa

//cfg i/q keep on   analog no used
2E 20 00 09 55 01 11 40 00 00 00 44 aa

//open auto cali_en
2E 20 00 09 55 00 11 40 00 00 00 44 aa
2E 20 00 09 55 01 11 40 00 00 00 55 aa

//check auto cali done [5:4]
2E 20 00 09 55 00 11 48 00 00 00 00 aa


//review I cali_value
2E 20 00 09 55 00 11 70 00 00 00 00 aa
//review Q cali_value
2E 20 00 09 55 00 11 74 00 00 00 00 aa

//read adc_mem

//manual calibration value register I
2E 20 00 09 55 00 11 50 00 00 00 00 aa
2E 20 00 09 55 01 11 50 2b 2d 27 21 aa
//manual calibration value register Q
2E 20 00 09 55 00 11 54 00 00 00 00 aa
2E 20 00 09 55 01 11 54 27 27 17 15 aa
//vga_word_ovrd ,at this point,auto_cali_en selected values to be overwritten
2E 20 00 09 55 00 11 4c 00 00 00 00 aa
2E 20 00 09 55 01 11 4c 00 00 00 03 aa

//read adc_mem

//close auto cali_en
2E 20 00 09 55 00 11 40 00 00 00 44 aa
2E 20 00 09 55 01 11 40 00 00 00 44 aa
