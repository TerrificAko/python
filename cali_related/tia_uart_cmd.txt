auto_cali_cfg
//phy_debug_on
2E 20 00 03 55 03 01

//atest sw, rfsw, trx on
2E 20 00 09 55 01 10 60 00 06 00 0b aa

//rx vbgr ldoh1p8
2E 20 00 09 55 01 10 5c 1c 40 00 00 aa


//cfg analog            cal_clk_div_en
2E 20 00 09 55 01 10 40 00 00 00 02 aa

tia test

//cfg lna agc ovrd
2E 20 00 09 55 01 10 2c 00 00 01 c0 aa

//cfg lna stage1 gain setup
2E 20 00 09 55 01 10 30 00 00 00 0f aa
//cfg lna stage2 gain setup
2E 20 00 09 55 01 10 2c 00 18 01 c0 aa
//cfg i_tia_fb_res[5:0]
2E 20 00 09 55 01 10 34 00 01 00 00 aa
//cfg q_tia_fb_res[5:0]
2E 20 00 09 55 01 10 38 00 00 00 80 aa




//cfg analog_top I channel i_atest_en & clk38M4
2E 20 00 09 55 01 10 40 00 00 01 02 aa

//cfg keep_dcoc dcoc_en i_2ua_en add
2E 20 00 09 55 01 10 34 00 01 03 80 aa

//cfg cali_I en
2E 20 00 09 55 01 11 20 00 00 0f f0 aa
2E 20 00 09 55 01 11 20 00 00 0f f1 aa

//read cali I value
2E 20 00 09 55 00 11 24 00 00 00 00 aa
2E 20 00 09 55 00 12 70 00 00 00 00 aa

//close I keep_dcoc dcoc_en i_2ua_en
2E 20 00 09 55 00 10 34 00 00 00 00 aa
2E 20 00 09 55 01 10 34 00 01 00 00 aa





//open Q analog_top q_atest_en & clk38M4
2E 20 00 09 55 00 10 40 00 00 00 00 aa
2E 20 00 09 55 01 10 40 00 00 10 02 aa

//q dcoc_en keep_dcoc q_2ua_en add
2E 20 00 09 55 00 10 38 00 00 00 80 aa
2E 20 00 09 55 01 10 38 06 00 00 81 aa

//cfg cali_Q en
2E 20 00 09 55 01 11 20 00 00 0f f0 aa
2E 20 00 09 55 01 11 20 00 00 0f f2 aa

//read cali Q value
2E 20 00 09 55 00 11 24 00 00 00 00 aa
2E 20 00 09 55 00 12 70 00 00 00 00 aa



//cali_off
//close Q analog_top q_atest_en
2E 20 00 09 55 00 10 40 00 00 00 00 aa
2E 20 00 09 55 01 10 40 00 00 00 02 aa

//close q dcoc_en keep_dcoc q_2ua_en
2E 20 00 09 55 00 10 38 00 00 00 80 aa
2E 20 00 09 55 01 10 38 00 00 00 80 aa

//close cali_Q en
2E 20 00 09 55 01 11 20 00 00 0f f0 aa


//close tia_agc_ovrd
2E 20 00 09 55 00 10 2c 00 18 01 c0 aa
2E 20 00 09 55 01 10 2c 00 18 00 00 aa

//en_agc_sel_tia_ovrd
2E 20 00 09 55 00 11 20 00 00 0f f0 aa
2E 20 00 09 55 01 11 20 00 3c 0f f0 aa

//tia calib ovrd word
2E 20 00 09 55 00 11 2c 3e 41 42 3f aa
2E 20 00 09 55 01 11 2c 3e 41 42 3f aa
//tia_calib_load_ovrd
2E 20 00 09 55 00 11 20 00 00 00 00 aa
2E 20 00 09 55 01 11 20 00 00 c0 00 aa

