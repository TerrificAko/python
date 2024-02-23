module dac_2m(
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
'h00:   dac_data    <= {16'h2B1C,16'h2528,16'h1F18,16'h18F8,16'h12C8,16'h0C8C,16'h0648,16'h0000};
'h01:   dac_data    <= {16'h55F4,16'h5130,16'h4C3C,16'h471C,16'h41CC,16'h3C54,16'h36B8,16'h30FC};
'h02:   dac_data    <= {16'h73B4,16'h70E0,16'h6DC8,16'h6A6C,16'h66CC,16'h62F0,16'h5ED4,16'h5A80};
'h03:   dac_data    <= {16'h7FD4,16'h7F60,16'h7E98,16'h7D88,16'h7C28,16'h7A78,16'h7880,16'h763C};
'h04:   dac_data    <= {16'h7880,16'h7A78,16'h7C28,16'h7D88,16'h7E98,16'h7F60,16'h7FD4,16'h7FFC};
'h05:   dac_data    <= {16'h5ED4,16'h62F0,16'h66CC,16'h6A6C,16'h6DC8,16'h70E0,16'h73B4,16'h763C};
'h06:   dac_data    <= {16'h36B8,16'h3C54,16'h41CC,16'h471C,16'h4C3C,16'h5130,16'h55F4,16'h5A80};
'h07:   dac_data    <= {16'h0648,16'h0C8C,16'h12C8,16'h18F8,16'h1F18,16'h2528,16'h2B1C,16'h30FC};
'h08:   dac_data    <= {16'hD4E4,16'hDAD8,16'hE0E8,16'hE708,16'hED38,16'hF374,16'hF9B8,16'h0000};
'h09:   dac_data    <= {16'hAA0C,16'hAED0,16'hB3C4,16'hB8E4,16'hBE34,16'hC3AC,16'hC948,16'hCF04};
'h0a:   dac_data    <= {16'h8C4C,16'h8F20,16'h9238,16'h9594,16'h9934,16'h9D10,16'hA12C,16'hA580};
'h0b:   dac_data    <= {16'h802C,16'h80A0,16'h8168,16'h8278,16'h83D8,16'h8588,16'h8780,16'h89C4};
'h0c:   dac_data    <= {16'h8780,16'h8588,16'h83D8,16'h8278,16'h8168,16'h80A0,16'h802C,16'h8004};
'h0d:   dac_data    <= {16'hA12C,16'h9D10,16'h9934,16'h9594,16'h9238,16'h8F20,16'h8C4C,16'h89C4};
'h0e:   dac_data    <= {16'hC948,16'hC3AC,16'hBE34,16'hB8E4,16'hB3C4,16'hAED0,16'hAA0C,16'hA580};
'h0f:   dac_data    <= {16'hF9B8,16'hF374,16'hED38,16'hE708,16'hE0E8,16'hDAD8,16'hD4E4,16'hCF04};
'h10:   dac_data    <= {16'h2B1C,16'h2528,16'h1F18,16'h18F8,16'h12C8,16'h0C8C,16'h0648,16'h0000};
'h11:   dac_data    <= {16'h55F4,16'h5130,16'h4C3C,16'h471C,16'h41CC,16'h3C54,16'h36B8,16'h30FC};
'h12:   dac_data    <= {16'h73B4,16'h70E0,16'h6DC8,16'h6A6C,16'h66CC,16'h62F0,16'h5ED4,16'h5A80};
'h13:   dac_data    <= {16'h7FD4,16'h7F60,16'h7E98,16'h7D88,16'h7C28,16'h7A78,16'h7880,16'h763C};
'h14:   dac_data    <= {16'h7880,16'h7A78,16'h7C28,16'h7D88,16'h7E98,16'h7F60,16'h7FD4,16'h7FFC};
'h15:   dac_data    <= {16'h5ED4,16'h62F0,16'h66CC,16'h6A6C,16'h6DC8,16'h70E0,16'h73B4,16'h763C};
'h16:   dac_data    <= {16'h36B8,16'h3C54,16'h41CC,16'h471C,16'h4C3C,16'h5130,16'h55F4,16'h5A80};
'h17:   dac_data    <= {16'h0648,16'h0C8C,16'h12C8,16'h18F8,16'h1F18,16'h2528,16'h2B1C,16'h30FC};
'h18:   dac_data    <= {16'hD4E4,16'hDAD8,16'hE0E8,16'hE708,16'hED38,16'hF374,16'hF9B8,16'h0000};
'h19:   dac_data    <= {16'hAA0C,16'hAED0,16'hB3C4,16'hB8E4,16'hBE34,16'hC3AC,16'hC948,16'hCF04};
'h1a:   dac_data    <= {16'h8C4C,16'h8F20,16'h9238,16'h9594,16'h9934,16'h9D10,16'hA12C,16'hA580};
'h1b:   dac_data    <= {16'h802C,16'h80A0,16'h8168,16'h8278,16'h83D8,16'h8588,16'h8780,16'h89C4};
'h1c:   dac_data    <= {16'h8780,16'h8588,16'h83D8,16'h8278,16'h8168,16'h80A0,16'h802C,16'h8004};
'h1d:   dac_data    <= {16'hA12C,16'h9D10,16'h9934,16'h9594,16'h9238,16'h8F20,16'h8C4C,16'h89C4};
'h1e:   dac_data    <= {16'hC948,16'hC3AC,16'hBE34,16'hB8E4,16'hB3C4,16'hAED0,16'hAA0C,16'hA580};
'h1f:   dac_data    <= {16'hF9B8,16'hF374,16'hED38,16'hE708,16'hE0E8,16'hDAD8,16'hD4E4,16'hCF04};
'h20:   dac_data    <= {16'h2B1C,16'h2528,16'h1F18,16'h18F8,16'h12C8,16'h0C8C,16'h0648,16'h0000};
'h21:   dac_data    <= {16'h55F4,16'h5130,16'h4C3C,16'h471C,16'h41CC,16'h3C54,16'h36B8,16'h30FC};
'h22:   dac_data    <= {16'h73B4,16'h70E0,16'h6DC8,16'h6A6C,16'h66CC,16'h62F0,16'h5ED4,16'h5A80};
'h23:   dac_data    <= {16'h7FD4,16'h7F60,16'h7E98,16'h7D88,16'h7C28,16'h7A78,16'h7880,16'h763C};
'h24:   dac_data    <= {16'h7880,16'h7A78,16'h7C28,16'h7D88,16'h7E98,16'h7F60,16'h7FD4,16'h7FFC};
'h25:   dac_data    <= {16'h5ED4,16'h62F0,16'h66CC,16'h6A6C,16'h6DC8,16'h70E0,16'h73B4,16'h763C};
'h26:   dac_data    <= {16'h36B8,16'h3C54,16'h41CC,16'h471C,16'h4C3C,16'h5130,16'h55F4,16'h5A80};
'h27:   dac_data    <= {16'h0648,16'h0C8C,16'h12C8,16'h18F8,16'h1F18,16'h2528,16'h2B1C,16'h30FC};
'h28:   dac_data    <= {16'hD4E4,16'hDAD8,16'hE0E8,16'hE708,16'hED38,16'hF374,16'hF9B8,16'h0000};
'h29:   dac_data    <= {16'hAA0C,16'hAED0,16'hB3C4,16'hB8E4,16'hBE34,16'hC3AC,16'hC948,16'hCF04};
'h2a:   dac_data    <= {16'h8C4C,16'h8F20,16'h9238,16'h9594,16'h9934,16'h9D10,16'hA12C,16'hA580};
'h2b:   dac_data    <= {16'h802C,16'h80A0,16'h8168,16'h8278,16'h83D8,16'h8588,16'h8780,16'h89C4};
'h2c:   dac_data    <= {16'h8780,16'h8588,16'h83D8,16'h8278,16'h8168,16'h80A0,16'h802C,16'h8004};
'h2d:   dac_data    <= {16'hA12C,16'h9D10,16'h9934,16'h9594,16'h9238,16'h8F20,16'h8C4C,16'h89C4};
'h2e:   dac_data    <= {16'hC948,16'hC3AC,16'hBE34,16'hB8E4,16'hB3C4,16'hAED0,16'hAA0C,16'hA580};
'h2f:   dac_data    <= {16'hF9B8,16'hF374,16'hED38,16'hE708,16'hE0E8,16'hDAD8,16'hD4E4,16'hCF04};
'h30:   dac_data    <= {16'h2B1C,16'h2528,16'h1F18,16'h18F8,16'h12C8,16'h0C8C,16'h0648,16'h0000};
'h31:   dac_data    <= {16'h55F4,16'h5130,16'h4C3C,16'h471C,16'h41CC,16'h3C54,16'h36B8,16'h30FC};
'h32:   dac_data    <= {16'h73B4,16'h70E0,16'h6DC8,16'h6A6C,16'h66CC,16'h62F0,16'h5ED4,16'h5A80};
'h33:   dac_data    <= {16'h7FD4,16'h7F60,16'h7E98,16'h7D88,16'h7C28,16'h7A78,16'h7880,16'h763C};
'h34:   dac_data    <= {16'h7880,16'h7A78,16'h7C28,16'h7D88,16'h7E98,16'h7F60,16'h7FD4,16'h7FFC};
'h35:   dac_data    <= {16'h5ED4,16'h62F0,16'h66CC,16'h6A6C,16'h6DC8,16'h70E0,16'h73B4,16'h763C};
'h36:   dac_data    <= {16'h36B8,16'h3C54,16'h41CC,16'h471C,16'h4C3C,16'h5130,16'h55F4,16'h5A80};
'h37:   dac_data    <= {16'h0648,16'h0C8C,16'h12C8,16'h18F8,16'h1F18,16'h2528,16'h2B1C,16'h30FC};
'h38:   dac_data    <= {16'hD4E4,16'hDAD8,16'hE0E8,16'hE708,16'hED38,16'hF374,16'hF9B8,16'h0000};
'h39:   dac_data    <= {16'hAA0C,16'hAED0,16'hB3C4,16'hB8E4,16'hBE34,16'hC3AC,16'hC948,16'hCF04};
'h3a:   dac_data    <= {16'h8C4C,16'h8F20,16'h9238,16'h9594,16'h9934,16'h9D10,16'hA12C,16'hA580};
'h3b:   dac_data    <= {16'h802C,16'h80A0,16'h8168,16'h8278,16'h83D8,16'h8588,16'h8780,16'h89C4};
'h3c:   dac_data    <= {16'h8780,16'h8588,16'h83D8,16'h8278,16'h8168,16'h80A0,16'h802C,16'h8004};
'h3d:   dac_data    <= {16'hA12C,16'h9D10,16'h9934,16'h9594,16'h9238,16'h8F20,16'h8C4C,16'h89C4};
'h3e:   dac_data    <= {16'hC948,16'hC3AC,16'hBE34,16'hB8E4,16'hB3C4,16'hAED0,16'hAA0C,16'hA580};
'h3f:   dac_data    <= {16'hF9B8,16'hF374,16'hED38,16'hE708,16'hE0E8,16'hDAD8,16'hD4E4,16'hCF04};
'h40:   dac_data    <= {16'h2B1C,16'h2528,16'h1F18,16'h18F8,16'h12C8,16'h0C8C,16'h0648,16'h0000};
'h41:   dac_data    <= {16'h55F4,16'h5130,16'h4C3C,16'h471C,16'h41CC,16'h3C54,16'h36B8,16'h30FC};
'h42:   dac_data    <= {16'h73B4,16'h70E0,16'h6DC8,16'h6A6C,16'h66CC,16'h62F0,16'h5ED4,16'h5A80};
'h43:   dac_data    <= {16'h7FD4,16'h7F60,16'h7E98,16'h7D88,16'h7C28,16'h7A78,16'h7880,16'h763C};
'h44:   dac_data    <= {16'h7880,16'h7A78,16'h7C28,16'h7D88,16'h7E98,16'h7F60,16'h7FD4,16'h7FFC};
'h45:   dac_data    <= {16'h5ED4,16'h62F0,16'h66CC,16'h6A6C,16'h6DC8,16'h70E0,16'h73B4,16'h763C};
'h46:   dac_data    <= {16'h36B8,16'h3C54,16'h41CC,16'h471C,16'h4C3C,16'h5130,16'h55F4,16'h5A80};
'h47:   dac_data    <= {16'h0648,16'h0C8C,16'h12C8,16'h18F8,16'h1F18,16'h2528,16'h2B1C,16'h30FC};
'h48:   dac_data    <= {16'hD4E4,16'hDAD8,16'hE0E8,16'hE708,16'hED38,16'hF374,16'hF9B8,16'h0000};
'h49:   dac_data    <= {16'hAA0C,16'hAED0,16'hB3C4,16'hB8E4,16'hBE34,16'hC3AC,16'hC948,16'hCF04};
'h4a:   dac_data    <= {16'h8C4C,16'h8F20,16'h9238,16'h9594,16'h9934,16'h9D10,16'hA12C,16'hA580};
'h4b:   dac_data    <= {16'h802C,16'h80A0,16'h8168,16'h8278,16'h83D8,16'h8588,16'h8780,16'h89C4};
'h4c:   dac_data    <= {16'h8780,16'h8588,16'h83D8,16'h8278,16'h8168,16'h80A0,16'h802C,16'h8004};
'h4d:   dac_data    <= {16'hA12C,16'h9D10,16'h9934,16'h9594,16'h9238,16'h8F20,16'h8C4C,16'h89C4};
'h4e:   dac_data    <= {16'hC948,16'hC3AC,16'hBE34,16'hB8E4,16'hB3C4,16'hAED0,16'hAA0C,16'hA580};
'h4f:   dac_data    <= {16'hF9B8,16'hF374,16'hED38,16'hE708,16'hE0E8,16'hDAD8,16'hD4E4,16'hCF04};
'h50:   dac_data    <= {16'h2B1C,16'h2528,16'h1F18,16'h18F8,16'h12C8,16'h0C8C,16'h0648,16'h0000};
'h51:   dac_data    <= {16'h55F4,16'h5130,16'h4C3C,16'h471C,16'h41CC,16'h3C54,16'h36B8,16'h30FC};
'h52:   dac_data    <= {16'h73B4,16'h70E0,16'h6DC8,16'h6A6C,16'h66CC,16'h62F0,16'h5ED4,16'h5A80};
'h53:   dac_data    <= {16'h7FD4,16'h7F60,16'h7E98,16'h7D88,16'h7C28,16'h7A78,16'h7880,16'h763C};
'h54:   dac_data    <= {16'h7880,16'h7A78,16'h7C28,16'h7D88,16'h7E98,16'h7F60,16'h7FD4,16'h7FFC};
'h55:   dac_data    <= {16'h5ED4,16'h62F0,16'h66CC,16'h6A6C,16'h6DC8,16'h70E0,16'h73B4,16'h763C};
'h56:   dac_data    <= {16'h36B8,16'h3C54,16'h41CC,16'h471C,16'h4C3C,16'h5130,16'h55F4,16'h5A80};
'h57:   dac_data    <= {16'h0648,16'h0C8C,16'h12C8,16'h18F8,16'h1F18,16'h2528,16'h2B1C,16'h30FC};
'h58:   dac_data    <= {16'hD4E4,16'hDAD8,16'hE0E8,16'hE708,16'hED38,16'hF374,16'hF9B8,16'h0000};
'h59:   dac_data    <= {16'hAA0C,16'hAED0,16'hB3C4,16'hB8E4,16'hBE34,16'hC3AC,16'hC948,16'hCF04};
'h5a:   dac_data    <= {16'h8C4C,16'h8F20,16'h9238,16'h9594,16'h9934,16'h9D10,16'hA12C,16'hA580};
'h5b:   dac_data    <= {16'h802C,16'h80A0,16'h8168,16'h8278,16'h83D8,16'h8588,16'h8780,16'h89C4};
'h5c:   dac_data    <= {16'h8780,16'h8588,16'h83D8,16'h8278,16'h8168,16'h80A0,16'h802C,16'h8004};
'h5d:   dac_data    <= {16'hA12C,16'h9D10,16'h9934,16'h9594,16'h9238,16'h8F20,16'h8C4C,16'h89C4};
'h5e:   dac_data    <= {16'hC948,16'hC3AC,16'hBE34,16'hB8E4,16'hB3C4,16'hAED0,16'hAA0C,16'hA580};
'h5f:   dac_data    <= {16'hF9B8,16'hF374,16'hED38,16'hE708,16'hE0E8,16'hDAD8,16'hD4E4,16'hCF04};
'h60:   dac_data    <= {16'h2B1C,16'h2528,16'h1F18,16'h18F8,16'h12C8,16'h0C8C,16'h0648,16'h0000};
'h61:   dac_data    <= {16'h55F4,16'h5130,16'h4C3C,16'h471C,16'h41CC,16'h3C54,16'h36B8,16'h30FC};
'h62:   dac_data    <= {16'h73B4,16'h70E0,16'h6DC8,16'h6A6C,16'h66CC,16'h62F0,16'h5ED4,16'h5A80};
'h63:   dac_data    <= {16'h7FD4,16'h7F60,16'h7E98,16'h7D88,16'h7C28,16'h7A78,16'h7880,16'h763C};
'h64:   dac_data    <= {16'h7880,16'h7A78,16'h7C28,16'h7D88,16'h7E98,16'h7F60,16'h7FD4,16'h7FFC};
'h65:   dac_data    <= {16'h5ED4,16'h62F0,16'h66CC,16'h6A6C,16'h6DC8,16'h70E0,16'h73B4,16'h763C};
'h66:   dac_data    <= {16'h36B8,16'h3C54,16'h41CC,16'h471C,16'h4C3C,16'h5130,16'h55F4,16'h5A80};
'h67:   dac_data    <= {16'h0648,16'h0C8C,16'h12C8,16'h18F8,16'h1F18,16'h2528,16'h2B1C,16'h30FC};
'h68:   dac_data    <= {16'hD4E4,16'hDAD8,16'hE0E8,16'hE708,16'hED38,16'hF374,16'hF9B8,16'h0000};
'h69:   dac_data    <= {16'hAA0C,16'hAED0,16'hB3C4,16'hB8E4,16'hBE34,16'hC3AC,16'hC948,16'hCF04};
'h6a:   dac_data    <= {16'h8C4C,16'h8F20,16'h9238,16'h9594,16'h9934,16'h9D10,16'hA12C,16'hA580};
'h6b:   dac_data    <= {16'h802C,16'h80A0,16'h8168,16'h8278,16'h83D8,16'h8588,16'h8780,16'h89C4};
'h6c:   dac_data    <= {16'h8780,16'h8588,16'h83D8,16'h8278,16'h8168,16'h80A0,16'h802C,16'h8004};
'h6d:   dac_data    <= {16'hA12C,16'h9D10,16'h9934,16'h9594,16'h9238,16'h8F20,16'h8C4C,16'h89C4};
'h6e:   dac_data    <= {16'hC948,16'hC3AC,16'hBE34,16'hB8E4,16'hB3C4,16'hAED0,16'hAA0C,16'hA580};
'h6f:   dac_data    <= {16'hF9B8,16'hF374,16'hED38,16'hE708,16'hE0E8,16'hDAD8,16'hD4E4,16'hCF04};
'h70:   dac_data    <= {16'h2B1C,16'h2528,16'h1F18,16'h18F8,16'h12C8,16'h0C8C,16'h0648,16'h0000};
'h71:   dac_data    <= {16'h55F4,16'h5130,16'h4C3C,16'h471C,16'h41CC,16'h3C54,16'h36B8,16'h30FC};
'h72:   dac_data    <= {16'h73B4,16'h70E0,16'h6DC8,16'h6A6C,16'h66CC,16'h62F0,16'h5ED4,16'h5A80};
'h73:   dac_data    <= {16'h7FD4,16'h7F60,16'h7E98,16'h7D88,16'h7C28,16'h7A78,16'h7880,16'h763C};
'h74:   dac_data    <= {16'h7880,16'h7A78,16'h7C28,16'h7D88,16'h7E98,16'h7F60,16'h7FD4,16'h7FFC};
'h75:   dac_data    <= {16'h5ED4,16'h62F0,16'h66CC,16'h6A6C,16'h6DC8,16'h70E0,16'h73B4,16'h763C};
'h76:   dac_data    <= {16'h36B8,16'h3C54,16'h41CC,16'h471C,16'h4C3C,16'h5130,16'h55F4,16'h5A80};
'h77:   dac_data    <= {16'h0648,16'h0C8C,16'h12C8,16'h18F8,16'h1F18,16'h2528,16'h2B1C,16'h30FC};
'h78:   dac_data    <= {16'hD4E4,16'hDAD8,16'hE0E8,16'hE708,16'hED38,16'hF374,16'hF9B8,16'h0000};
'h79:   dac_data    <= {16'hAA0C,16'hAED0,16'hB3C4,16'hB8E4,16'hBE34,16'hC3AC,16'hC948,16'hCF04};
'h7a:   dac_data    <= {16'h8C4C,16'h8F20,16'h9238,16'h9594,16'h9934,16'h9D10,16'hA12C,16'hA580};
'h7b:   dac_data    <= {16'h802C,16'h80A0,16'h8168,16'h8278,16'h83D8,16'h8588,16'h8780,16'h89C4};
'h7c:   dac_data    <= {16'h8780,16'h8588,16'h83D8,16'h8278,16'h8168,16'h80A0,16'h802C,16'h8004};
'h7d:   dac_data    <= {16'hA12C,16'h9D10,16'h9934,16'h9594,16'h9238,16'h8F20,16'h8C4C,16'h89C4};
'h7e:   dac_data    <= {16'hC948,16'hC3AC,16'hBE34,16'hB8E4,16'hB3C4,16'hAED0,16'hAA0C,16'hA580};
'h7f:   dac_data    <= {16'hF9B8,16'hF374,16'hED38,16'hE708,16'hE0E8,16'hDAD8,16'hD4E4,16'hCF04};

default:    dac_data    <=  'd0;

        endcase
    end
        else begin
        dac_data  <=  'd0;
        dac_data_valid <=  'd0;
    end
end

endmodule
