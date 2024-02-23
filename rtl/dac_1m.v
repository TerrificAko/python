module dac_1m(
    input   clk,
    input   rst_n,
    input   axis_tready,
    output  reg [127:0] dac_data,
    output  reg         dac_data_valid
);
reg [6:0]   cnt128;

always@(posedge clk or negedge rst_n)begin
    if(!rst_n)
        cnt128   <=  'd0;
    else
        cnt128   <=  cnt128   +   'd1;
end

always@(posedge clk or negedge rst_n)begin
    if(!rst_n)begin
        dac_data  <=  'd0;
        dac_data_valid <=  'd0;
    end
    else if (axis_tready)begin
        dac_data_valid  <=  'd1;
        case(cnt128)
'h00:   dac_data    <= {16'h15E0,16'h12C8,16'h0FAC,16'h0C8C,16'h096C,16'h0648,16'h0324,16'h0000};
'h01:   dac_data    <= {16'h2E10,16'h2B1C,16'h2824,16'h2528,16'h2224,16'h1F18,16'h1C0C,16'h18F8};
'h02:   dac_data    <= {16'h4478,16'h41CC,16'h3F14,16'h3C54,16'h398C,16'h36B8,16'h33DC,16'h30FC};
'h03:   dac_data    <= {16'h5840,16'h55F4,16'h5398,16'h5130,16'h4EBC,16'h4C3C,16'h49B0,16'h471C};
'h04:   dac_data    <= {16'h68A4,16'h66CC,16'h64E4,16'h62F0,16'h60E8,16'h5ED4,16'h5CB0,16'h5A80};
'h05:   dac_data    <= {16'h7500,16'h73B4,16'h7250,16'h70E0,16'h6F5C,16'h6DC8,16'h6C20,16'h6A6C};
'h06:   dac_data    <= {16'h7CE0,16'h7C28,16'h7B58,16'h7A78,16'h7988,16'h7880,16'h7768,16'h763C};
'h07:   dac_data    <= {16'h7FF4,16'h7FD4,16'h7FA4,16'h7F60,16'h7F04,16'h7E98,16'h7E18,16'h7D88};
'h08:   dac_data    <= {16'h7E18,16'h7E98,16'h7F04,16'h7F60,16'h7FA4,16'h7FD4,16'h7FF4,16'h7FFC};
'h09:   dac_data    <= {16'h7768,16'h7880,16'h7988,16'h7A78,16'h7B58,16'h7C28,16'h7CE0,16'h7D88};
'h0a:   dac_data    <= {16'h6C20,16'h6DC8,16'h6F5C,16'h70E0,16'h7250,16'h73B4,16'h7500,16'h763C};
'h0b:   dac_data    <= {16'h5CB0,16'h5ED4,16'h60E8,16'h62F0,16'h64E4,16'h66CC,16'h68A4,16'h6A6C};
'h0c:   dac_data    <= {16'h49B0,16'h4C3C,16'h4EBC,16'h5130,16'h5398,16'h55F4,16'h5840,16'h5A80};
'h0d:   dac_data    <= {16'h33DC,16'h36B8,16'h398C,16'h3C54,16'h3F14,16'h41CC,16'h4478,16'h471C};
'h0e:   dac_data    <= {16'h1C0C,16'h1F18,16'h2224,16'h2528,16'h2824,16'h2B1C,16'h2E10,16'h30FC};
'h0f:   dac_data    <= {16'h0324,16'h0648,16'h096C,16'h0C8C,16'h0FAC,16'h12C8,16'h15E0,16'h18F8};
'h10:   dac_data    <= {16'hEA20,16'hED38,16'hF054,16'hF374,16'hF694,16'hF9B8,16'hFCDC,16'h0000};
'h11:   dac_data    <= {16'hD1F0,16'hD4E4,16'hD7DC,16'hDAD8,16'hDDDC,16'hE0E8,16'hE3F4,16'hE708};
'h12:   dac_data    <= {16'hBB88,16'hBE34,16'hC0EC,16'hC3AC,16'hC674,16'hC948,16'hCC24,16'hCF04};
'h13:   dac_data    <= {16'hA7C0,16'hAA0C,16'hAC68,16'hAED0,16'hB144,16'hB3C4,16'hB650,16'hB8E4};
'h14:   dac_data    <= {16'h975C,16'h9934,16'h9B1C,16'h9D10,16'h9F18,16'hA12C,16'hA350,16'hA580};
'h15:   dac_data    <= {16'h8B00,16'h8C4C,16'h8DB0,16'h8F20,16'h90A4,16'h9238,16'h93E0,16'h9594};
'h16:   dac_data    <= {16'h8320,16'h83D8,16'h84A8,16'h8588,16'h8678,16'h8780,16'h8898,16'h89C4};
'h17:   dac_data    <= {16'h800C,16'h802C,16'h805C,16'h80A0,16'h80FC,16'h8168,16'h81E8,16'h8278};
'h18:   dac_data    <= {16'h81E8,16'h8168,16'h80FC,16'h80A0,16'h805C,16'h802C,16'h800C,16'h8004};
'h19:   dac_data    <= {16'h8898,16'h8780,16'h8678,16'h8588,16'h84A8,16'h83D8,16'h8320,16'h8278};
'h1a:   dac_data    <= {16'h93E0,16'h9238,16'h90A4,16'h8F20,16'h8DB0,16'h8C4C,16'h8B00,16'h89C4};
'h1b:   dac_data    <= {16'hA350,16'hA12C,16'h9F18,16'h9D10,16'h9B1C,16'h9934,16'h975C,16'h9594};
'h1c:   dac_data    <= {16'hB650,16'hB3C4,16'hB144,16'hAED0,16'hAC68,16'hAA0C,16'hA7C0,16'hA580};
'h1d:   dac_data    <= {16'hCC24,16'hC948,16'hC674,16'hC3AC,16'hC0EC,16'hBE34,16'hBB88,16'hB8E4};
'h1e:   dac_data    <= {16'hE3F4,16'hE0E8,16'hDDDC,16'hDAD8,16'hD7DC,16'hD4E4,16'hD1F0,16'hCF04};
'h1f:   dac_data    <= {16'hFCDC,16'hF9B8,16'hF694,16'hF374,16'hF054,16'hED38,16'hEA20,16'hE708};
'h20:   dac_data    <= {16'h15E0,16'h12C8,16'h0FAC,16'h0C8C,16'h096C,16'h0648,16'h0324,16'h0000};
'h21:   dac_data    <= {16'h2E10,16'h2B1C,16'h2824,16'h2528,16'h2224,16'h1F18,16'h1C0C,16'h18F8};
'h22:   dac_data    <= {16'h4478,16'h41CC,16'h3F14,16'h3C54,16'h398C,16'h36B8,16'h33DC,16'h30FC};
'h23:   dac_data    <= {16'h5840,16'h55F4,16'h5398,16'h5130,16'h4EBC,16'h4C3C,16'h49B0,16'h471C};
'h24:   dac_data    <= {16'h68A4,16'h66CC,16'h64E4,16'h62F0,16'h60E8,16'h5ED4,16'h5CB0,16'h5A80};
'h25:   dac_data    <= {16'h7500,16'h73B4,16'h7250,16'h70E0,16'h6F5C,16'h6DC8,16'h6C20,16'h6A6C};
'h26:   dac_data    <= {16'h7CE0,16'h7C28,16'h7B58,16'h7A78,16'h7988,16'h7880,16'h7768,16'h763C};
'h27:   dac_data    <= {16'h7FF4,16'h7FD4,16'h7FA4,16'h7F60,16'h7F04,16'h7E98,16'h7E18,16'h7D88};
'h28:   dac_data    <= {16'h7E18,16'h7E98,16'h7F04,16'h7F60,16'h7FA4,16'h7FD4,16'h7FF4,16'h7FFC};
'h29:   dac_data    <= {16'h7768,16'h7880,16'h7988,16'h7A78,16'h7B58,16'h7C28,16'h7CE0,16'h7D88};
'h2a:   dac_data    <= {16'h6C20,16'h6DC8,16'h6F5C,16'h70E0,16'h7250,16'h73B4,16'h7500,16'h763C};
'h2b:   dac_data    <= {16'h5CB0,16'h5ED4,16'h60E8,16'h62F0,16'h64E4,16'h66CC,16'h68A4,16'h6A6C};
'h2c:   dac_data    <= {16'h49B0,16'h4C3C,16'h4EBC,16'h5130,16'h5398,16'h55F4,16'h5840,16'h5A80};
'h2d:   dac_data    <= {16'h33DC,16'h36B8,16'h398C,16'h3C54,16'h3F14,16'h41CC,16'h4478,16'h471C};
'h2e:   dac_data    <= {16'h1C0C,16'h1F18,16'h2224,16'h2528,16'h2824,16'h2B1C,16'h2E10,16'h30FC};
'h2f:   dac_data    <= {16'h0324,16'h0648,16'h096C,16'h0C8C,16'h0FAC,16'h12C8,16'h15E0,16'h18F8};
'h30:   dac_data    <= {16'hEA20,16'hED38,16'hF054,16'hF374,16'hF694,16'hF9B8,16'hFCDC,16'h0000};
'h31:   dac_data    <= {16'hD1F0,16'hD4E4,16'hD7DC,16'hDAD8,16'hDDDC,16'hE0E8,16'hE3F4,16'hE708};
'h32:   dac_data    <= {16'hBB88,16'hBE34,16'hC0EC,16'hC3AC,16'hC674,16'hC948,16'hCC24,16'hCF04};
'h33:   dac_data    <= {16'hA7C0,16'hAA0C,16'hAC68,16'hAED0,16'hB144,16'hB3C4,16'hB650,16'hB8E4};
'h34:   dac_data    <= {16'h975C,16'h9934,16'h9B1C,16'h9D10,16'h9F18,16'hA12C,16'hA350,16'hA580};
'h35:   dac_data    <= {16'h8B00,16'h8C4C,16'h8DB0,16'h8F20,16'h90A4,16'h9238,16'h93E0,16'h9594};
'h36:   dac_data    <= {16'h8320,16'h83D8,16'h84A8,16'h8588,16'h8678,16'h8780,16'h8898,16'h89C4};
'h37:   dac_data    <= {16'h800C,16'h802C,16'h805C,16'h80A0,16'h80FC,16'h8168,16'h81E8,16'h8278};
'h38:   dac_data    <= {16'h81E8,16'h8168,16'h80FC,16'h80A0,16'h805C,16'h802C,16'h800C,16'h8004};
'h39:   dac_data    <= {16'h8898,16'h8780,16'h8678,16'h8588,16'h84A8,16'h83D8,16'h8320,16'h8278};
'h3a:   dac_data    <= {16'h93E0,16'h9238,16'h90A4,16'h8F20,16'h8DB0,16'h8C4C,16'h8B00,16'h89C4};
'h3b:   dac_data    <= {16'hA350,16'hA12C,16'h9F18,16'h9D10,16'h9B1C,16'h9934,16'h975C,16'h9594};
'h3c:   dac_data    <= {16'hB650,16'hB3C4,16'hB144,16'hAED0,16'hAC68,16'hAA0C,16'hA7C0,16'hA580};
'h3d:   dac_data    <= {16'hCC24,16'hC948,16'hC674,16'hC3AC,16'hC0EC,16'hBE34,16'hBB88,16'hB8E4};
'h3e:   dac_data    <= {16'hE3F4,16'hE0E8,16'hDDDC,16'hDAD8,16'hD7DC,16'hD4E4,16'hD1F0,16'hCF04};
'h3f:   dac_data    <= {16'hFCDC,16'hF9B8,16'hF694,16'hF374,16'hF054,16'hED38,16'hEA20,16'hE708};
'h40:   dac_data    <= {16'h15E0,16'h12C8,16'h0FAC,16'h0C8C,16'h096C,16'h0648,16'h0324,16'h0000};
'h41:   dac_data    <= {16'h2E10,16'h2B1C,16'h2824,16'h2528,16'h2224,16'h1F18,16'h1C0C,16'h18F8};
'h42:   dac_data    <= {16'h4478,16'h41CC,16'h3F14,16'h3C54,16'h398C,16'h36B8,16'h33DC,16'h30FC};
'h43:   dac_data    <= {16'h5840,16'h55F4,16'h5398,16'h5130,16'h4EBC,16'h4C3C,16'h49B0,16'h471C};
'h44:   dac_data    <= {16'h68A4,16'h66CC,16'h64E4,16'h62F0,16'h60E8,16'h5ED4,16'h5CB0,16'h5A80};
'h45:   dac_data    <= {16'h7500,16'h73B4,16'h7250,16'h70E0,16'h6F5C,16'h6DC8,16'h6C20,16'h6A6C};
'h46:   dac_data    <= {16'h7CE0,16'h7C28,16'h7B58,16'h7A78,16'h7988,16'h7880,16'h7768,16'h763C};
'h47:   dac_data    <= {16'h7FF4,16'h7FD4,16'h7FA4,16'h7F60,16'h7F04,16'h7E98,16'h7E18,16'h7D88};
'h48:   dac_data    <= {16'h7E18,16'h7E98,16'h7F04,16'h7F60,16'h7FA4,16'h7FD4,16'h7FF4,16'h7FFC};
'h49:   dac_data    <= {16'h7768,16'h7880,16'h7988,16'h7A78,16'h7B58,16'h7C28,16'h7CE0,16'h7D88};
'h4a:   dac_data    <= {16'h6C20,16'h6DC8,16'h6F5C,16'h70E0,16'h7250,16'h73B4,16'h7500,16'h763C};
'h4b:   dac_data    <= {16'h5CB0,16'h5ED4,16'h60E8,16'h62F0,16'h64E4,16'h66CC,16'h68A4,16'h6A6C};
'h4c:   dac_data    <= {16'h49B0,16'h4C3C,16'h4EBC,16'h5130,16'h5398,16'h55F4,16'h5840,16'h5A80};
'h4d:   dac_data    <= {16'h33DC,16'h36B8,16'h398C,16'h3C54,16'h3F14,16'h41CC,16'h4478,16'h471C};
'h4e:   dac_data    <= {16'h1C0C,16'h1F18,16'h2224,16'h2528,16'h2824,16'h2B1C,16'h2E10,16'h30FC};
'h4f:   dac_data    <= {16'h0324,16'h0648,16'h096C,16'h0C8C,16'h0FAC,16'h12C8,16'h15E0,16'h18F8};
'h50:   dac_data    <= {16'hEA20,16'hED38,16'hF054,16'hF374,16'hF694,16'hF9B8,16'hFCDC,16'h0000};
'h51:   dac_data    <= {16'hD1F0,16'hD4E4,16'hD7DC,16'hDAD8,16'hDDDC,16'hE0E8,16'hE3F4,16'hE708};
'h52:   dac_data    <= {16'hBB88,16'hBE34,16'hC0EC,16'hC3AC,16'hC674,16'hC948,16'hCC24,16'hCF04};
'h53:   dac_data    <= {16'hA7C0,16'hAA0C,16'hAC68,16'hAED0,16'hB144,16'hB3C4,16'hB650,16'hB8E4};
'h54:   dac_data    <= {16'h975C,16'h9934,16'h9B1C,16'h9D10,16'h9F18,16'hA12C,16'hA350,16'hA580};
'h55:   dac_data    <= {16'h8B00,16'h8C4C,16'h8DB0,16'h8F20,16'h90A4,16'h9238,16'h93E0,16'h9594};
'h56:   dac_data    <= {16'h8320,16'h83D8,16'h84A8,16'h8588,16'h8678,16'h8780,16'h8898,16'h89C4};
'h57:   dac_data    <= {16'h800C,16'h802C,16'h805C,16'h80A0,16'h80FC,16'h8168,16'h81E8,16'h8278};
'h58:   dac_data    <= {16'h81E8,16'h8168,16'h80FC,16'h80A0,16'h805C,16'h802C,16'h800C,16'h8004};
'h59:   dac_data    <= {16'h8898,16'h8780,16'h8678,16'h8588,16'h84A8,16'h83D8,16'h8320,16'h8278};
'h5a:   dac_data    <= {16'h93E0,16'h9238,16'h90A4,16'h8F20,16'h8DB0,16'h8C4C,16'h8B00,16'h89C4};
'h5b:   dac_data    <= {16'hA350,16'hA12C,16'h9F18,16'h9D10,16'h9B1C,16'h9934,16'h975C,16'h9594};
'h5c:   dac_data    <= {16'hB650,16'hB3C4,16'hB144,16'hAED0,16'hAC68,16'hAA0C,16'hA7C0,16'hA580};
'h5d:   dac_data    <= {16'hCC24,16'hC948,16'hC674,16'hC3AC,16'hC0EC,16'hBE34,16'hBB88,16'hB8E4};
'h5e:   dac_data    <= {16'hE3F4,16'hE0E8,16'hDDDC,16'hDAD8,16'hD7DC,16'hD4E4,16'hD1F0,16'hCF04};
'h5f:   dac_data    <= {16'hFCDC,16'hF9B8,16'hF694,16'hF374,16'hF054,16'hED38,16'hEA20,16'hE708};
'h60:   dac_data    <= {16'h15E0,16'h12C8,16'h0FAC,16'h0C8C,16'h096C,16'h0648,16'h0324,16'h0000};
'h61:   dac_data    <= {16'h2E10,16'h2B1C,16'h2824,16'h2528,16'h2224,16'h1F18,16'h1C0C,16'h18F8};
'h62:   dac_data    <= {16'h4478,16'h41CC,16'h3F14,16'h3C54,16'h398C,16'h36B8,16'h33DC,16'h30FC};
'h63:   dac_data    <= {16'h5840,16'h55F4,16'h5398,16'h5130,16'h4EBC,16'h4C3C,16'h49B0,16'h471C};
'h64:   dac_data    <= {16'h68A4,16'h66CC,16'h64E4,16'h62F0,16'h60E8,16'h5ED4,16'h5CB0,16'h5A80};
'h65:   dac_data    <= {16'h7500,16'h73B4,16'h7250,16'h70E0,16'h6F5C,16'h6DC8,16'h6C20,16'h6A6C};
'h66:   dac_data    <= {16'h7CE0,16'h7C28,16'h7B58,16'h7A78,16'h7988,16'h7880,16'h7768,16'h763C};
'h67:   dac_data    <= {16'h7FF4,16'h7FD4,16'h7FA4,16'h7F60,16'h7F04,16'h7E98,16'h7E18,16'h7D88};
'h68:   dac_data    <= {16'h7E18,16'h7E98,16'h7F04,16'h7F60,16'h7FA4,16'h7FD4,16'h7FF4,16'h7FFC};
'h69:   dac_data    <= {16'h7768,16'h7880,16'h7988,16'h7A78,16'h7B58,16'h7C28,16'h7CE0,16'h7D88};
'h6a:   dac_data    <= {16'h6C20,16'h6DC8,16'h6F5C,16'h70E0,16'h7250,16'h73B4,16'h7500,16'h763C};
'h6b:   dac_data    <= {16'h5CB0,16'h5ED4,16'h60E8,16'h62F0,16'h64E4,16'h66CC,16'h68A4,16'h6A6C};
'h6c:   dac_data    <= {16'h49B0,16'h4C3C,16'h4EBC,16'h5130,16'h5398,16'h55F4,16'h5840,16'h5A80};
'h6d:   dac_data    <= {16'h33DC,16'h36B8,16'h398C,16'h3C54,16'h3F14,16'h41CC,16'h4478,16'h471C};
'h6e:   dac_data    <= {16'h1C0C,16'h1F18,16'h2224,16'h2528,16'h2824,16'h2B1C,16'h2E10,16'h30FC};
'h6f:   dac_data    <= {16'h0324,16'h0648,16'h096C,16'h0C8C,16'h0FAC,16'h12C8,16'h15E0,16'h18F8};
'h70:   dac_data    <= {16'hEA20,16'hED38,16'hF054,16'hF374,16'hF694,16'hF9B8,16'hFCDC,16'h0000};
'h71:   dac_data    <= {16'hD1F0,16'hD4E4,16'hD7DC,16'hDAD8,16'hDDDC,16'hE0E8,16'hE3F4,16'hE708};
'h72:   dac_data    <= {16'hBB88,16'hBE34,16'hC0EC,16'hC3AC,16'hC674,16'hC948,16'hCC24,16'hCF04};
'h73:   dac_data    <= {16'hA7C0,16'hAA0C,16'hAC68,16'hAED0,16'hB144,16'hB3C4,16'hB650,16'hB8E4};
'h74:   dac_data    <= {16'h975C,16'h9934,16'h9B1C,16'h9D10,16'h9F18,16'hA12C,16'hA350,16'hA580};
'h75:   dac_data    <= {16'h8B00,16'h8C4C,16'h8DB0,16'h8F20,16'h90A4,16'h9238,16'h93E0,16'h9594};
'h76:   dac_data    <= {16'h8320,16'h83D8,16'h84A8,16'h8588,16'h8678,16'h8780,16'h8898,16'h89C4};
'h77:   dac_data    <= {16'h800C,16'h802C,16'h805C,16'h80A0,16'h80FC,16'h8168,16'h81E8,16'h8278};
'h78:   dac_data    <= {16'h81E8,16'h8168,16'h80FC,16'h80A0,16'h805C,16'h802C,16'h800C,16'h8004};
'h79:   dac_data    <= {16'h8898,16'h8780,16'h8678,16'h8588,16'h84A8,16'h83D8,16'h8320,16'h8278};
'h7a:   dac_data    <= {16'h93E0,16'h9238,16'h90A4,16'h8F20,16'h8DB0,16'h8C4C,16'h8B00,16'h89C4};
'h7b:   dac_data    <= {16'hA350,16'hA12C,16'h9F18,16'h9D10,16'h9B1C,16'h9934,16'h975C,16'h9594};
'h7c:   dac_data    <= {16'hB650,16'hB3C4,16'hB144,16'hAED0,16'hAC68,16'hAA0C,16'hA7C0,16'hA580};
'h7d:   dac_data    <= {16'hCC24,16'hC948,16'hC674,16'hC3AC,16'hC0EC,16'hBE34,16'hBB88,16'hB8E4};
'h7e:   dac_data    <= {16'hE3F4,16'hE0E8,16'hDDDC,16'hDAD8,16'hD7DC,16'hD4E4,16'hD1F0,16'hCF04};
'h7f:   dac_data    <= {16'hFCDC,16'hF9B8,16'hF694,16'hF374,16'hF054,16'hED38,16'hEA20,16'hE708};

default:    dac_data    <=  'd0;

        endcase
    end
        else begin
        dac_data  <=  'd0;
        dac_data_valid <=  'd0;
    end
end

endmodule
