#!/bin/sh

# Get voices from
# https://yamahablackboxes.com/collection/yamaha-dx7-synthesizer/patches/

mkdir -p sysex/voice

DIR="https://yamahablackboxes.com/patches/dx7/factory"

wget -c "${DIR}"/rom1a.syx -O sysex/voice/000000_rom1a.syx
wget -c "${DIR}"/rom1b.syx -O sysex/voice/000001_rom1b.syx
wget -c "${DIR}"/rom2a.syx -O sysex/voice/000002_rom2a.syx
wget -c "${DIR}"/rom2b.syx -O sysex/voice/000003_rom2b.syx
wget -c "${DIR}"/rom3a.syx -O sysex/voice/000000_rom3a.syx
wget -c "${DIR}"/rom3b.syx -O sysex/voice/000001_rom3b.syx
wget -c "${DIR}"/rom4a.syx -O sysex/voice/000002_rom4a.syx
wget -c "${DIR}"/rom4b.syx -O sysex/voice/000003_rom4b.syx

DIR="https://yamahablackboxes.com/patches/dx7/vrc"

wget -c "${DIR}"/vrc101b.syx -O sysex/voice/000004_vrc101b.syx
wget -c "${DIR}"/vrc102a.syx -O sysex/voice/000005_vrc102a.syx
wget -c "${DIR}"/vrc102b.syx -O sysex/voice/000006_vrc102b.syx
wget -c "${DIR}"/vrc103a.syx -O sysex/voice/000007_vrc103a.syx
wget -c "${DIR}"/vrc103b.syx -O sysex/voice/000008_vrc103b.syx
wget -c "${DIR}"/vrc104a.syx -O sysex/voice/000009_vrc104a.syx
wget -c "${DIR}"/vrc104b.syx -O sysex/voice/000010_vrc104b.syx
wget -c "${DIR}"/vrc105a.syx -O sysex/voice/000011_vrc105a.syx
wget -c "${DIR}"/vrc105b.syx -O sysex/voice/000012_vrc105b.syx
wget -c "${DIR}"/vrc106a.syx -O sysex/voice/000013_vrc106a.syx
wget -c "${DIR}"/vrc106b.syx -O sysex/voice/000014_vrc106b.syx
wget -c "${DIR}"/vrc107a.syx -O sysex/voice/000015_vrc107a.syx
wget -c "${DIR}"/vrc107b.syx -O sysex/voice/000016_vrc107b.syx
wget -c "${DIR}"/vrc108a.syx -O sysex/voice/000017_vrc108a.syx
wget -c "${DIR}"/vrc108b.syx -O sysex/voice/000018_vrc108b.syx
wget -c "${DIR}"/vrc109a.syx -O sysex/voice/000019_vrc109a.syx
wget -c "${DIR}"/vrc109b.syx -O sysex/voice/000020_vrc109b.syx
wget -c "${DIR}"/vrc110a.syx -O sysex/voice/000021_vrc110a.syx
wget -c "${DIR}"/vrc110b.syx -O sysex/voice/000022_vrc110b.syx
wget -c "${DIR}"/vrc111a.syx -O sysex/voice/000023_vrc111a.syx
wget -c "${DIR}"/vrc111b.syx -O sysex/voice/000024_vrc111b.syx
wget -c "${DIR}"/vrc112a.syx -O sysex/voice/000025_vrc112a.syx
wget -c "${DIR}"/vrc112b.syx -O sysex/voice/000026_vrc112b.syx

DIR="http://dxsysex.com/SYSEX_DX7/Misc"

wget -c "${DIR}"/actualbk.syx -O sysex/voice/000027_actualbk.syx
wget -c "${DIR}"/aegix.syx -O sysex/voice/000028_aegix.syx
wget -c "${DIR}"/analog1.syx -O sysex/voice/000029_analog1.syx
wget -c "${DIR}"/analog3.syx -O sysex/voice/000030_analog3.syx
wget -c "${DIR}"/analog4.syx -O sysex/voice/000031_analog4.syx
wget -c "${DIR}"/analog_1.syx -O sysex/voice/000032_analog_1.syx
wget -c "${DIR}"/analog_2.syx -O sysex/voice/000033_analog_2.syx
wget -c "${DIR}"/analog_3.syx -O sysex/voice/000034_analog_3.syx
wget -c "${DIR}"/angelo.syx -O sysex/voice/000035_angelo.syx
wget -c "${DIR}"/atsu_4.syx -O sysex/voice/000036_atsu_4.syx
wget -c "${DIR}"/atsu_5.syx -O sysex/voice/000037_atsu_5.syx
wget -c "${DIR}"/b1.syx -O sysex/voice/000038_b1.syx
wget -c "${DIR}"/b2.syx -O sysex/voice/000039_b2.syx
wget -c "${DIR}"/b3.syx -O sysex/voice/000040_b3.syx
wget -c "${DIR}"/b4.syx -O sysex/voice/000041_b4.syx
wget -c "${DIR}"/bank0000.syx -O sysex/voice/000042_bank0000.syx
wget -c "${DIR}"/bank0006.syx -O sysex/voice/000043_bank0006.syx
wget -c "${DIR}"/bank0008.syx -O sysex/voice/000044_bank0008.syx
wget -c "${DIR}"/bank0009.syx -O sysex/voice/000045_bank0009.syx
wget -c "${DIR}"/bank0023.syx -O sysex/voice/000046_bank0023.syx
wget -c "${DIR}"/bank0024.syx -O sysex/voice/000047_bank0024.syx
wget -c "${DIR}"/bank0042.syx -O sysex/voice/000048_bank0042.syx
wget -c "${DIR}"/bank0043.syx -O sysex/voice/000049_bank0043.syx
wget -c "${DIR}"/bank0044.syx -O sysex/voice/000050_bank0044.syx
wget -c "${DIR}"/bank0045.syx -O sysex/voice/000051_bank0045.syx
wget -c "${DIR}"/bank0046.syx -O sysex/voice/000052_bank0046.syx
wget -c "${DIR}"/bank0047.syx -O sysex/voice/000053_bank0047.syx
wget -c "${DIR}"/bank0048.syx -O sysex/voice/000054_bank0048.syx
wget -c "${DIR}"/bank0049.syx -O sysex/voice/000055_bank0049.syx
wget -c "${DIR}"/bank0050.syx -O sysex/voice/000056_bank0050.syx
wget -c "${DIR}"/bank0054.syx -O sysex/voice/000057_bank0054.syx
wget -c "${DIR}"/bank0056.syx -O sysex/voice/000058_bank0056.syx
wget -c "${DIR}"/bank01.syx -O sysex/voice/000059_bank01.syx
wget -c "${DIR}"/bank05.syx -O sysex/voice/000060_bank05.syx
wget -c "${DIR}"/bank08.syx -O sysex/voice/000061_bank08.syx
wget -c "${DIR}"/bank1.syx -O sysex/voice/000062_bank1.syx
wget -c "${DIR}"/bank11.syx -O sysex/voice/000063_bank11.syx
wget -c "${DIR}"/bank14.syx -O sysex/voice/000064_bank14.syx
wget -c "${DIR}"/bank18.syx -O sysex/voice/000065_bank18.syx
wget -c "${DIR}"/bank2.syx -O sysex/voice/000066_bank2.syx
wget -c "${DIR}"/bank3.syx -O sysex/voice/000067_bank3.syx
wget -c "${DIR}"/bass3.syx -O sysex/voice/000068_bass3.syx
wget -c "${DIR}"/bass_01.syx -O sysex/voice/000069_bass_01.syx
wget -c "${DIR}"/bass_02.syx -O sysex/voice/000070_bass_02.syx
wget -c "${DIR}"/bass_03.syx -O sysex/voice/000071_bass_03.syx
wget -c "${DIR}"/bass_04.syx -O sysex/voice/000072_bass_04.syx
wget -c "${DIR}"/bass_05.syx -O sysex/voice/000073_bass_05.syx
wget -c "${DIR}"/bass_06.syx -O sysex/voice/000074_bass_06.syx
wget -c "${DIR}"/bass_07.syx -O sysex/voice/000075_bass_07.syx
wget -c "${DIR}"/bass_08.syx -O sysex/voice/000076_bass_08.syx
wget -c "${DIR}"/bass_09.syx -O sysex/voice/000077_bass_09.syx
wget -c "${DIR}"/bass_10.syx -O sysex/voice/000078_bass_10.syx
wget -c "${DIR}"/bass_11.syx -O sysex/voice/000079_bass_11.syx
wget -c "${DIR}"/bass_12.syx -O sysex/voice/000080_bass_12.syx
wget -c "${DIR}"/bass_13.syx -O sysex/voice/000081_bass_13.syx
wget -c "${DIR}"/bass_14.syx -O sysex/voice/000082_bass_14.syx
wget -c "${DIR}"/bass_15.syx -O sysex/voice/000083_bass_15.syx
wget -c "${DIR}"/bassea.syx -O sysex/voice/000084_bassea.syx
wget -c "${DIR}"/bassics.syx -O sysex/voice/000085_bassics.syx
wget -c "${DIR}"/bbank1.syx -O sysex/voice/000086_bbank1.syx
wget -c "${DIR}"/bbank2.syx -O sysex/voice/000087_bbank2.syx
wget -c "${DIR}"/bell1.syx -O sysex/voice/000088_bell1.syx
wget -c "${DIR}"/bells.syx -O sysex/voice/000089_bells.syx
wget -c "${DIR}"/bells_01.syx -O sysex/voice/000090_bells_01.syx
wget -c "${DIR}"/bells_02.syx -O sysex/voice/000091_bells_02.syx
wget -c "${DIR}"/bells_03.syx -O sysex/voice/000092_bells_03.syx
wget -c "${DIR}"/belltel.syx -O sysex/voice/000093_belltel.syx
wget -c "${DIR}"/blanks.syx -O sysex/voice/000094_blanks.syx
wget -c "${DIR}"/boruff.syx -O sysex/voice/000095_boruff.syx
wget -c "${DIR}"/brass.syx -O sysex/voice/000096_brass.syx
wget -c "${DIR}"/brass2.syx -O sysex/voice/000097_brass2.syx
wget -c "${DIR}"/brass3.syx -O sysex/voice/000098_brass3.syx
wget -c "${DIR}"/brass_01.syx -O sysex/voice/000099_brass_01.syx
wget -c "${DIR}"/brass_02.syx -O sysex/voice/000100_brass_02.syx
wget -c "${DIR}"/brass_03.syx -O sysex/voice/000101_brass_03.syx
wget -c "${DIR}"/brass_04.syx -O sysex/voice/000102_brass_04.syx
wget -c "${DIR}"/brass_05.syx -O sysex/voice/000103_brass_05.syx
wget -c "${DIR}"/brass_06.syx -O sysex/voice/000104_brass_06.syx
wget -c "${DIR}"/brass_07.syx -O sysex/voice/000105_brass_07.syx
wget -c "${DIR}"/brass_08.syx -O sysex/voice/000106_brass_08.syx
wget -c "${DIR}"/brass_09.syx -O sysex/voice/000107_brass_09.syx
wget -c "${DIR}"/brass_10.syx -O sysex/voice/000108_brass_10.syx
wget -c "${DIR}"/brass_11.syx -O sysex/voice/000109_brass_11.syx
wget -c "${DIR}"/brass_12.syx -O sysex/voice/000110_brass_12.syx
wget -c "${DIR}"/brass_13.syx -O sysex/voice/000111_brass_13.syx
wget -c "${DIR}"/brass_14.syx -O sysex/voice/000112_brass_14.syx
wget -c "${DIR}"/brass_15.syx -O sysex/voice/000113_brass_15.syx
wget -c "${DIR}"/brass_16.syx -O sysex/voice/000114_brass_16.syx
wget -c "${DIR}"/brass_17.syx -O sysex/voice/000115_brass_17.syx
wget -c "${DIR}"/brass_18.syx -O sysex/voice/000116_brass_18.syx
wget -c "${DIR}"/bstx.syx -O sysex/voice/000117_bstx.syx
wget -c "${DIR}"/buck-1.syx -O sysex/voice/000118_buck-1.syx
wget -c "${DIR}"/buck-2.syx -O sysex/voice/000119_buck-2.syx
wget -c "${DIR}"/c-lab_1.syx -O sysex/voice/000120_c-lab_1.syx
wget -c "${DIR}"/cello_01.syx -O sysex/voice/000121_cello_01.syx
wget -c "${DIR}"/cello_02.syx -O sysex/voice/000122_cello_02.syx
wget -c "${DIR}"/church.syx -O sysex/voice/000123_church.syx
wget -c "${DIR}"/ciani.syx -O sysex/voice/000124_ciani.syx
wget -c "${DIR}"/clang1.syx -O sysex/voice/000125_clang1.syx
wget -c "${DIR}"/clarinet.syx -O sysex/voice/000126_clarinet.syx
wget -c "${DIR}"/claude.syx -O sysex/voice/000127_claude.syx
wget -c "${DIR}"/clav1.syx -O sysex/voice/000128_clav1.syx
wget -c "${DIR}"/clav_01.syx -O sysex/voice/000129_clav_01.syx
wget -c "${DIR}"/clav_02.syx -O sysex/voice/000130_clav_02.syx
wget -c "${DIR}"/clav_03.syx -O sysex/voice/000131_clav_03.syx
wget -c "${DIR}"/clav_04.syx -O sysex/voice/000132_clav_04.syx
wget -c "${DIR}"/clav_05.syx -O sysex/voice/000133_clav_05.syx
wget -c "${DIR}"/clav_06.syx -O sysex/voice/000134_clav_06.syx
wget -c "${DIR}"/clavs.syx -O sysex/voice/000135_clavs.syx
wget -c "${DIR}"/combo.syx -O sysex/voice/000136_combo.syx
wget -c "${DIR}"/compuerr.syx -O sysex/voice/000137_compuerr.syx
wget -c "${DIR}"/contemp.syx -O sysex/voice/000138_contemp.syx
wget -c "${DIR}"/country.syx -O sysex/voice/000139_country.syx
wget -c "${DIR}"/czeiszpe.syx -O sysex/voice/000140_czeiszpe.syx
wget -c "${DIR}"/dave1.syx -O sysex/voice/000141_dave1.syx
wget -c "${DIR}"/deckard.syx -O sysex/voice/000142_deckard.syx
wget -c "${DIR}"/dg1.syx -O sysex/voice/000143_dg1.syx
wget -c "${DIR}"/dg2.syx -O sysex/voice/000144_dg2.syx
wget -c "${DIR}"/divers.syx -O sysex/voice/000145_divers.syx
wget -c "${DIR}"/djw001.syx -O sysex/voice/000146_djw001.syx
wget -c "${DIR}"/drm2.syx -O sysex/voice/000147_drm2.syx
wget -c "${DIR}"/drm2a.syx -O sysex/voice/000148_drm2a.syx
wget -c "${DIR}"/drmoct.syx -O sysex/voice/000149_drmoct.syx
wget -c "${DIR}"/drt1.syx -O sysex/voice/000150_drt1.syx
wget -c "${DIR}"/drt2.syx -O sysex/voice/000151_drt2.syx
wget -c "${DIR}"/dx100.syx -O sysex/voice/000152_dx100.syx
wget -c "${DIR}"/dx101.syx -O sysex/voice/000153_dx101.syx
wget -c "${DIR}"/dx103.syx -O sysex/voice/000154_dx103.syx
wget -c "${DIR}"/dx104.syx -O sysex/voice/000155_dx104.syx
wget -c "${DIR}"/dx105.syx -O sysex/voice/000156_dx105.syx
wget -c "${DIR}"/dx106.syx -O sysex/voice/000157_dx106.syx
wget -c "${DIR}"/dx107.syx -O sysex/voice/000158_dx107.syx
wget -c "${DIR}"/dx109.syx -O sysex/voice/000159_dx109.syx
wget -c "${DIR}"/dx110.syx -O sysex/voice/000160_dx110.syx
wget -c "${DIR}"/dx111.syx -O sysex/voice/000161_dx111.syx
wget -c "${DIR}"/dx112.syx -O sysex/voice/000162_dx112.syx
wget -c "${DIR}"/dx113.syx -O sysex/voice/000163_dx113.syx
wget -c "${DIR}"/dx114.syx -O sysex/voice/000164_dx114.syx
wget -c "${DIR}"/dx116.syx -O sysex/voice/000165_dx116.syx
wget -c "${DIR}"/dx117.syx -O sysex/voice/000166_dx117.syx
wget -c "${DIR}"/dx118.syx -O sysex/voice/000167_dx118.syx
wget -c "${DIR}"/dx119.syx -O sysex/voice/000168_dx119.syx
wget -c "${DIR}"/dx120.syx -O sysex/voice/000169_dx120.syx
wget -c "${DIR}"/dx121.syx -O sysex/voice/000170_dx121.syx
wget -c "${DIR}"/dx122.syx -O sysex/voice/000171_dx122.syx
wget -c "${DIR}"/dx123.syx -O sysex/voice/000172_dx123.syx
wget -c "${DIR}"/dx124.syx -O sysex/voice/000173_dx124.syx
wget -c "${DIR}"/dx7bank.syx -O sysex/voice/000174_dx7bank.syx
wget -c "${DIR}"/dx9.syx -O sysex/voice/000175_dx9.syx
wget -c "${DIR}"/dxbasic5.syx -O sysex/voice/000176_dxbasic5.syx
wget -c "${DIR}"/dxdxmi.syx -O sysex/voice/000177_dxdxmi.syx
wget -c "${DIR}"/dxmik2.syx -O sysex/voice/000178_dxmik2.syx
wget -c "${DIR}"/dxoc01.syx -O sysex/voice/000179_dxoc01.syx
wget -c "${DIR}"/dxoc02.syx -O sysex/voice/000180_dxoc02.syx
wget -c "${DIR}"/dxoc03.syx -O sysex/voice/000181_dxoc03.syx
wget -c "${DIR}"/dxoc04.syx -O sysex/voice/000182_dxoc04.syx
wget -c "${DIR}"/dxoc05.syx -O sysex/voice/000183_dxoc05.syx
wget -c "${DIR}"/dxoc06.syx -O sysex/voice/000184_dxoc06.syx
wget -c "${DIR}"/dxoc07.syx -O sysex/voice/000185_dxoc07.syx
wget -c "${DIR}"/dxoc08.syx -O sysex/voice/000186_dxoc08.syx
wget -c "${DIR}"/dxoc09.syx -O sysex/voice/000187_dxoc09.syx
wget -c "${DIR}"/dxoc10.syx -O sysex/voice/000188_dxoc10.syx
wget -c "${DIR}"/dxoc11.syx -O sysex/voice/000189_dxoc11.syx
wget -c "${DIR}"/dxoc12.syx -O sysex/voice/000190_dxoc12.syx
wget -c "${DIR}"/dxoc13.syx -O sysex/voice/000191_dxoc13.syx
wget -c "${DIR}"/dxoc14.syx -O sysex/voice/000192_dxoc14.syx
wget -c "${DIR}"/dxoc15.syx -O sysex/voice/000193_dxoc15.syx
wget -c "${DIR}"/dxoc16.syx -O sysex/voice/000194_dxoc16.syx
wget -c "${DIR}"/dxoc17.syx -O sysex/voice/000195_dxoc17.syx
wget -c "${DIR}"/dxoc18.syx -O sysex/voice/000196_dxoc18.syx
wget -c "${DIR}"/dxoc19.syx -O sysex/voice/000197_dxoc19.syx
wget -c "${DIR}"/dxoc20.syx -O sysex/voice/000198_dxoc20.syx
wget -c "${DIR}"/dxoc21.syx -O sysex/voice/000199_dxoc21.syx
wget -c "${DIR}"/dxoc22.syx -O sysex/voice/000200_dxoc22.syx
wget -c "${DIR}"/dxoc23.syx -O sysex/voice/000201_dxoc23.syx
wget -c "${DIR}"/dxoc24.syx -O sysex/voice/000202_dxoc24.syx
wget -c "${DIR}"/dxoc25.syx -O sysex/voice/000203_dxoc25.syx
wget -c "${DIR}"/dxoc26.syx -O sysex/voice/000204_dxoc26.syx
wget -c "${DIR}"/dxoc27.syx -O sysex/voice/000205_dxoc27.syx
wget -c "${DIR}"/dxoc28.syx -O sysex/voice/000206_dxoc28.syx
wget -c "${DIR}"/dxoc29.syx -O sysex/voice/000207_dxoc29.syx
wget -c "${DIR}"/dxoc30.syx -O sysex/voice/000208_dxoc30.syx
wget -c "${DIR}"/dxorgans.syx -O sysex/voice/000209_dxorgans.syx
wget -c "${DIR}"/dxpluk.syx -O sysex/voice/000210_dxpluk.syx
wget -c "${DIR}"/effect-1.syx -O sysex/voice/000211_effect-1.syx
wget -c "${DIR}"/effect-2.syx -O sysex/voice/000212_effect-2.syx
wget -c "${DIR}"/effects1.syx -O sysex/voice/000213_effects1.syx
wget -c "${DIR}"/effects2.syx -O sysex/voice/000214_effects2.syx
wget -c "${DIR}"/effects3.syx -O sysex/voice/000215_effects3.syx
wget -c "${DIR}"/effects4.syx -O sysex/voice/000216_effects4.syx
wget -c "${DIR}"/effects5.syx -O sysex/voice/000217_effects5.syx
wget -c "${DIR}"/effects6.syx -O sysex/voice/000218_effects6.syx
wget -c "${DIR}"/effects7.syx -O sysex/voice/000219_effects7.syx
wget -c "${DIR}"/effects8.syx -O sysex/voice/000220_effects8.syx
wget -c "${DIR}"/efx.syx -O sysex/voice/000221_efx.syx
wget -c "${DIR}"/esipa.syx -O sysex/voice/000222_esipa.syx
wget -c "${DIR}"/ethnic.syx -O sysex/voice/000223_ethnic.syx
wget -c "${DIR}"/fatstuff.syx -O sysex/voice/000224_fatstuff.syx
wget -c "${DIR}"/flute.syx -O sysex/voice/000225_flute.syx
wget -c "${DIR}"/foo.syx -O sysex/voice/000226_foo.syx
wget -c "${DIR}"/gig-1.syx -O sysex/voice/000227_gig-1.syx
wget -c "${DIR}"/gregeea.syx -O sysex/voice/000228_gregeea.syx
wget -c "${DIR}"/gregeeb.syx -O sysex/voice/000229_gregeeb.syx
wget -c "${DIR}"/gregeec.syx -O sysex/voice/000230_gregeec.syx
wget -c "${DIR}"/gregeed.syx -O sysex/voice/000231_gregeed.syx
wget -c "${DIR}"/gtrs_01.syx -O sysex/voice/000232_gtrs_01.syx
wget -c "${DIR}"/gtrs_a-d.syx -O sysex/voice/000233_gtrs_a-d.syx
wget -c "${DIR}"/gtrs_d.syx -O sysex/voice/000234_gtrs_d.syx
wget -c "${DIR}"/gtrs_el.syx -O sysex/voice/000235_gtrs_el.syx
wget -c "${DIR}"/gtrs_g-j.syx -O sysex/voice/000236_gtrs_g-j.syx
wget -c "${DIR}"/gtrs_j-k.syx -O sysex/voice/000237_gtrs_j-k.syx
wget -c "${DIR}"/gtrs_l-n.syx -O sysex/voice/000238_gtrs_l-n.syx
wget -c "${DIR}"/gtrs_n-r.syx -O sysex/voice/000239_gtrs_n-r.syx
wget -c "${DIR}"/gtrs_s-v.syx -O sysex/voice/000240_gtrs_s-v.syx
wget -c "${DIR}"/gtrsonus.syx -O sysex/voice/000241_gtrsonus.syx
wget -c "${DIR}"/guitar1.syx -O sysex/voice/000242_guitar1.syx
wget -c "${DIR}"/guitar2.syx -O sysex/voice/000243_guitar2.syx
wget -c "${DIR}"/harmonic.syx -O sysex/voice/000244_harmonic.syx
wget -c "${DIR}"/harp_1.syx -O sysex/voice/000245_harp_1.syx
wget -c "${DIR}"/harp_2.syx -O sysex/voice/000246_harp_2.syx
wget -c "${DIR}"/harpsi_1.syx -O sysex/voice/000247_harpsi_1.syx
wget -c "${DIR}"/harpsi_2.syx -O sysex/voice/000248_harpsi_2.syx
wget -c "${DIR}"/italian2.syx -O sysex/voice/000249_italian2.syx
wget -c "${DIR}"/italian3.syx -O sysex/voice/000250_italian3.syx
wget -c "${DIR}"/jb001.syx -O sysex/voice/000251_jb001.syx
wget -c "${DIR}"/jb002.syx -O sysex/voice/000252_jb002.syx
wget -c "${DIR}"/jfd001.syx -O sysex/voice/000253_jfd001.syx
wget -c "${DIR}"/keybds01.syx -O sysex/voice/000254_keybds01.syx
wget -c "${DIR}"/keyboard.syx -O sysex/voice/000255_keyboard.syx
wget -c "${DIR}"/king.syx -O sysex/voice/000256_king.syx
wget -c "${DIR}"/koto.syx -O sysex/voice/000257_koto.syx
wget -c "${DIR}"/kq_sampl.syx -O sysex/voice/000258_kq_sampl.syx
wget -c "${DIR}"/laser.syx -O sysex/voice/000259_laser.syx
wget -c "${DIR}"/magazine.syx -O sysex/voice/000260_magazine.syx
wget -c "${DIR}"/mallets1.syx -O sysex/voice/000261_mallets1.syx
wget -c "${DIR}"/mallets2.syx -O sysex/voice/000262_mallets2.syx
wget -c "${DIR}"/massey.syx -O sysex/voice/000263_massey.syx
wget -c "${DIR}"/metal.syx -O sysex/voice/000264_metal.syx
wget -c "${DIR}"/mf.syx -O sysex/voice/000265_mf.syx
wget -c "${DIR}"/misc_01.syx -O sysex/voice/000266_misc_01.syx
wget -c "${DIR}"/misc_02.syx -O sysex/voice/000267_misc_02.syx
wget -c "${DIR}"/misc_03.syx -O sysex/voice/000268_misc_03.syx
wget -c "${DIR}"/misc_04.syx -O sysex/voice/000269_misc_04.syx
wget -c "${DIR}"/misc_05.syx -O sysex/voice/000270_misc_05.syx
wget -c "${DIR}"/misc_06.syx -O sysex/voice/000271_misc_06.syx
wget -c "${DIR}"/misc_07.syx -O sysex/voice/000272_misc_07.syx
wget -c "${DIR}"/misc_08.syx -O sysex/voice/000273_misc_08.syx
wget -c "${DIR}"/misc_09.syx -O sysex/voice/000274_misc_09.syx
wget -c "${DIR}"/misc_10.syx -O sysex/voice/000275_misc_10.syx
wget -c "${DIR}"/misc_11.syx -O sysex/voice/000276_misc_11.syx
wget -c "${DIR}"/misc_12.syx -O sysex/voice/000277_misc_12.syx
wget -c "${DIR}"/misc_13.syx -O sysex/voice/000278_misc_13.syx
wget -c "${DIR}"/misc_14.syx -O sysex/voice/000279_misc_14.syx
wget -c "${DIR}"/misc_15.syx -O sysex/voice/000280_misc_15.syx
wget -c "${DIR}"/misc_16.syx -O sysex/voice/000281_misc_16.syx
wget -c "${DIR}"/misc_17.syx -O sysex/voice/000282_misc_17.syx
wget -c "${DIR}"/misc_18.syx -O sysex/voice/000283_misc_18.syx
wget -c "${DIR}"/misc_19.syx -O sysex/voice/000284_misc_19.syx
wget -c "${DIR}"/misc_20.syx -O sysex/voice/000285_misc_20.syx
wget -c "${DIR}"/misc_21.syx -O sysex/voice/000286_misc_21.syx
wget -c "${DIR}"/misc_22.syx -O sysex/voice/000287_misc_22.syx
wget -c "${DIR}"/misc_23.syx -O sysex/voice/000288_misc_23.syx
wget -c "${DIR}"/misc_24.syx -O sysex/voice/000289_misc_24.syx
wget -c "${DIR}"/misc_25.syx -O sysex/voice/000290_misc_25.syx
wget -c "${DIR}"/misc_26.syx -O sysex/voice/000291_misc_26.syx
wget -c "${DIR}"/misc_27.syx -O sysex/voice/000292_misc_27.syx
wget -c "${DIR}"/misc_28.syx -O sysex/voice/000293_misc_28.syx
wget -c "${DIR}"/misc_29.syx -O sysex/voice/000294_misc_29.syx
wget -c "${DIR}"/misc_30.syx -O sysex/voice/000295_misc_30.syx
wget -c "${DIR}"/misc_31.syx -O sysex/voice/000296_misc_31.syx
wget -c "${DIR}"/misc_32.syx -O sysex/voice/000297_misc_32.syx
wget -c "${DIR}"/misc_33.syx -O sysex/voice/000298_misc_33.syx
wget -c "${DIR}"/misc_34.syx -O sysex/voice/000299_misc_34.syx
wget -c "${DIR}"/mjfdx7a.syx -O sysex/voice/000300_mjfdx7a.syx
wget -c "${DIR}"/mjfdx7b.syx -O sysex/voice/000301_mjfdx7b.syx
wget -c "${DIR}"/mjfdx7c.syx -O sysex/voice/000302_mjfdx7c.syx
wget -c "${DIR}"/mjfdx7d.syx -O sysex/voice/000303_mjfdx7d.syx
wget -c "${DIR}"/mjfdx7e.syx -O sysex/voice/000304_mjfdx7e.syx
wget -c "${DIR}"/mjfdx7f.syx -O sysex/voice/000305_mjfdx7f.syx
wget -c "${DIR}"/mjfdx7g.syx -O sysex/voice/000306_mjfdx7g.syx
wget -c "${DIR}"/mjfdx7h.syx -O sysex/voice/000307_mjfdx7h.syx
wget -c "${DIR}"/monteleo.syx -O sysex/voice/000308_monteleo.syx
wget -c "${DIR}"/moog.syx -O sysex/voice/000309_moog.syx
wget -c "${DIR}"/mortega.syx -O sysex/voice/000310_mortega.syx
wget -c "${DIR}"/newsnd.syx -O sysex/voice/000311_newsnd.syx
wget -c "${DIR}"/oboe_bas.syx -O sysex/voice/000312_oboe_bas.syx
wget -c "${DIR}"/orch_1.syx -O sysex/voice/000313_orch_1.syx
wget -c "${DIR}"/orchestr.syx -O sysex/voice/000314_orchestr.syx
wget -c "${DIR}"/org_clav.syx -O sysex/voice/000315_org_clav.syx
wget -c "${DIR}"/organ1.syx -O sysex/voice/000316_organ1.syx
wget -c "${DIR}"/organ2.syx -O sysex/voice/000317_organ2.syx
wget -c "${DIR}"/organ3.syx -O sysex/voice/000318_organ3.syx
wget -c "${DIR}"/organ_01.syx -O sysex/voice/000319_organ_01.syx
wget -c "${DIR}"/organ_02.syx -O sysex/voice/000320_organ_02.syx
wget -c "${DIR}"/organ_03.syx -O sysex/voice/000321_organ_03.syx
wget -c "${DIR}"/organ_04.syx -O sysex/voice/000322_organ_04.syx
wget -c "${DIR}"/organ_05.syx -O sysex/voice/000323_organ_05.syx
wget -c "${DIR}"/organ_06.syx -O sysex/voice/000324_organ_06.syx
wget -c "${DIR}"/organ_07.syx -O sysex/voice/000325_organ_07.syx
wget -c "${DIR}"/organ_08.syx -O sysex/voice/000326_organ_08.syx
wget -c "${DIR}"/organ_b3.syx -O sysex/voice/000327_organ_b3.syx
wget -c "${DIR}"/organham.syx -O sysex/voice/000328_organham.syx
wget -c "${DIR}"/orgpipe1.syx -O sysex/voice/000329_orgpipe1.syx
wget -c "${DIR}"/orgpipe2.syx -O sysex/voice/000330_orgpipe2.syx
wget -c "${DIR}"/patches.syx -O sysex/voice/000331_patches.syx
wget -c "${DIR}"/pddx701a.syx -O sysex/voice/000332_pddx701a.syx
wget -c "${DIR}"/pddx701b.syx -O sysex/voice/000333_pddx701b.syx
wget -c "${DIR}"/pddx701c.syx -O sysex/voice/000334_pddx701c.syx
wget -c "${DIR}"/pddx701d.syx -O sysex/voice/000335_pddx701d.syx
wget -c "${DIR}"/pearson.syx -O sysex/voice/000336_pearson.syx
wget -c "${DIR}"/pedanna.syx -O sysex/voice/000337_pedanna.syx
wget -c "${DIR}"/perc1.syx -O sysex/voice/000338_perc1.syx
wget -c "${DIR}"/perc2.syx -O sysex/voice/000339_perc2.syx
wget -c "${DIR}"/perc_a.syx -O sysex/voice/000340_perc_a.syx
wget -c "${DIR}"/perc_b.syx -O sysex/voice/000341_perc_b.syx
wget -c "${DIR}"/percus01.syx -O sysex/voice/000342_percus01.syx
wget -c "${DIR}"/percus02.syx -O sysex/voice/000343_percus02.syx
wget -c "${DIR}"/percus03.syx -O sysex/voice/000344_percus03.syx
wget -c "${DIR}"/percus04.syx -O sysex/voice/000345_percus04.syx
wget -c "${DIR}"/percus05.syx -O sysex/voice/000346_percus05.syx
wget -c "${DIR}"/percus06.syx -O sysex/voice/000347_percus06.syx
wget -c "${DIR}"/percus07.syx -O sysex/voice/000348_percus07.syx
wget -c "${DIR}"/percus08.syx -O sysex/voice/000349_percus08.syx
wget -c "${DIR}"/percus09.syx -O sysex/voice/000350_percus09.syx
wget -c "${DIR}"/percus10.syx -O sysex/voice/000351_percus10.syx
wget -c "${DIR}"/percvibe.syx -O sysex/voice/000352_percvibe.syx
wget -c "${DIR}"/personus.syx -O sysex/voice/000353_personus.syx
wget -c "${DIR}"/piano1.syx -O sysex/voice/000354_piano1.syx
wget -c "${DIR}"/piano_01.syx -O sysex/voice/000355_piano_01.syx
wget -c "${DIR}"/piano_02.syx -O sysex/voice/000356_piano_02.syx
wget -c "${DIR}"/piano_03.syx -O sysex/voice/000357_piano_03.syx
wget -c "${DIR}"/piano_04.syx -O sysex/voice/000358_piano_04.syx
wget -c "${DIR}"/piano_05.syx -O sysex/voice/000359_piano_05.syx
wget -c "${DIR}"/piano_06.syx -O sysex/voice/000360_piano_06.syx
wget -c "${DIR}"/piano_07.syx -O sysex/voice/000361_piano_07.syx
wget -c "${DIR}"/piano_08.syx -O sysex/voice/000362_piano_08.syx
wget -c "${DIR}"/piano_09.syx -O sysex/voice/000363_piano_09.syx
wget -c "${DIR}"/piano_10.syx -O sysex/voice/000364_piano_10.syx
wget -c "${DIR}"/piano_11.syx -O sysex/voice/000365_piano_11.syx
wget -c "${DIR}"/piano_12.syx -O sysex/voice/000366_piano_12.syx
wget -c "${DIR}"/piano_13.syx -O sysex/voice/000367_piano_13.syx
wget -c "${DIR}"/piano_a1.syx -O sysex/voice/000368_piano_a1.syx
wget -c "${DIR}"/piano_e1.syx -O sysex/voice/000369_piano_e1.syx
wget -c "${DIR}"/piano_e2.syx -O sysex/voice/000370_piano_e2.syx
wget -c "${DIR}"/piano_e3.syx -O sysex/voice/000371_piano_e3.syx
wget -c "${DIR}"/piano_e4.syx -O sysex/voice/000372_piano_e4.syx
wget -c "${DIR}"/piano_e5.syx -O sysex/voice/000373_piano_e5.syx
wget -c "${DIR}"/plucked.syx -O sysex/voice/000374_plucked.syx
wget -c "${DIR}"/powerply.syx -O sysex/voice/000375_powerply.syx
wget -c "${DIR}"/pr01.syx -O sysex/voice/000376_pr01.syx
wget -c "${DIR}"/pr02.syx -O sysex/voice/000377_pr02.syx
wget -c "${DIR}"/pr03.syx -O sysex/voice/000378_pr03.syx
wget -c "${DIR}"/pro2.syx -O sysex/voice/000379_pro2.syx
wget -c "${DIR}"/ram_bank.syx -O sysex/voice/000380_ram_bank.syx
wget -c "${DIR}"/ram_cart.syx -O sysex/voice/000381_ram_cart.syx
wget -c "${DIR}"/rambank1.syx -O sysex/voice/000382_rambank1.syx
wget -c "${DIR}"/ramcart.syx -O sysex/voice/000383_ramcart.syx
wget -c "${DIR}"/ramcartp.syx -O sysex/voice/000384_ramcartp.syx
wget -c "${DIR}"/ramnbank.syx -O sysex/voice/000385_ramnbank.syx
wget -c "${DIR}"/ray-1.syx -O sysex/voice/000386_ray-1.syx
wget -c "${DIR}"/ray-10.syx -O sysex/voice/000387_ray-10.syx
wget -c "${DIR}"/ray-11.syx -O sysex/voice/000388_ray-11.syx
wget -c "${DIR}"/ray-12.syx -O sysex/voice/000389_ray-12.syx
wget -c "${DIR}"/ray-13.syx -O sysex/voice/000390_ray-13.syx
wget -c "${DIR}"/ray-14.syx -O sysex/voice/000391_ray-14.syx
wget -c "${DIR}"/ray-15.syx -O sysex/voice/000392_ray-15.syx
wget -c "${DIR}"/ray-16.syx -O sysex/voice/000393_ray-16.syx
wget -c "${DIR}"/ray-17.syx -O sysex/voice/000394_ray-17.syx
wget -c "${DIR}"/ray-18.syx -O sysex/voice/000395_ray-18.syx
wget -c "${DIR}"/ray-19.syx -O sysex/voice/000396_ray-19.syx
wget -c "${DIR}"/ray-2.syx -O sysex/voice/000397_ray-2.syx
wget -c "${DIR}"/ray-20.syx -O sysex/voice/000398_ray-20.syx
wget -c "${DIR}"/ray-21.syx -O sysex/voice/000399_ray-21.syx
wget -c "${DIR}"/ray-3.syx -O sysex/voice/000400_ray-3.syx
wget -c "${DIR}"/ray-4.syx -O sysex/voice/000401_ray-4.syx
wget -c "${DIR}"/ray-5.syx -O sysex/voice/000402_ray-5.syx
wget -c "${DIR}"/ray-6.syx -O sysex/voice/000403_ray-6.syx
wget -c "${DIR}"/ray-7.syx -O sysex/voice/000404_ray-7.syx
wget -c "${DIR}"/ray-8.syx -O sysex/voice/000405_ray-8.syx
wget -c "${DIR}"/ray-9.syx -O sysex/voice/000406_ray-9.syx
wget -c "${DIR}"/rhodes1.syx -O sysex/voice/000407_rhodes1.syx
wget -c "${DIR}"/rhodes2.syx -O sysex/voice/000408_rhodes2.syx
wget -c "${DIR}"/rlegnini.syx -O sysex/voice/000409_rlegnini.syx
wget -c "${DIR}"/rm0001.syx -O sysex/voice/000410_rm0001.syx
wget -c "${DIR}"/rom-1.syx -O sysex/voice/000411_rom-1.syx
wget -c "${DIR}"/rom-2.syx -O sysex/voice/000412_rom-2.syx
wget -c "${DIR}"/rom-3.syx -O sysex/voice/000413_rom-3.syx
wget -c "${DIR}"/rom-4.syx -O sysex/voice/000414_rom-4.syx
wget -c "${DIR}"/sax_01.syx -O sysex/voice/000415_sax_01.syx
wget -c "${DIR}"/sax_02.syx -O sysex/voice/000416_sax_02.syx
wget -c "${DIR}"/sitar.syx -O sysex/voice/000417_sitar.syx
wget -c "${DIR}"/solange.syx -O sysex/voice/000418_solange.syx
wget -c "${DIR}"/solange0.syx -O sysex/voice/000419_solange0.syx
wget -c "${DIR}"/solange1.syx -O sysex/voice/000420_solange1.syx
wget -c "${DIR}"/solange2.syx -O sysex/voice/000421_solange2.syx
wget -c "${DIR}"/solange3.syx -O sysex/voice/000422_solange3.syx
wget -c "${DIR}"/solange4.syx -O sysex/voice/000423_solange4.syx
wget -c "${DIR}"/solange5.syx -O sysex/voice/000424_solange5.syx
wget -c "${DIR}"/solange6.syx -O sysex/voice/000425_solange6.syx
wget -c "${DIR}"/sonus1.syx -O sysex/voice/000426_sonus1.syx
wget -c "${DIR}"/spangler.syx -O sysex/voice/000427_spangler.syx
wget -c "${DIR}"/splits.syx -O sysex/voice/000428_splits.syx
wget -c "${DIR}"/starma.syx -O sysex/voice/000429_starma.syx
wget -c "${DIR}"/steinber.syx -O sysex/voice/000430_steinber.syx
wget -c "${DIR}"/steph1.syx -O sysex/voice/000431_steph1.syx
wget -c "${DIR}"/steph2.syx -O sysex/voice/000432_steph2.syx
wget -c "${DIR}"/steph3.syx -O sysex/voice/000433_steph3.syx
wget -c "${DIR}"/steph4.syx -O sysex/voice/000434_steph4.syx
wget -c "${DIR}"/steve.syx -O sysex/voice/000435_steve.syx
wget -c "${DIR}"/string1.syx -O sysex/voice/000436_string1.syx
wget -c "${DIR}"/string2.syx -O sysex/voice/000437_string2.syx
wget -c "${DIR}"/stringb.syx -O sysex/voice/000438_stringb.syx
wget -c "${DIR}"/strnga-a.syx -O sysex/voice/000439_strnga-a.syx
wget -c "${DIR}"/strnga-b.syx -O sysex/voice/000440_strnga-b.syx
wget -c "${DIR}"/strngb-d.syx -O sysex/voice/000441_strngb-d.syx
wget -c "${DIR}"/strngd-h.syx -O sysex/voice/000442_strngd-h.syx
wget -c "${DIR}"/strngh-k.syx -O sysex/voice/000443_strngh-k.syx
wget -c "${DIR}"/strngk-o.syx -O sysex/voice/000444_strngk-o.syx
wget -c "${DIR}"/strngs01.syx -O sysex/voice/000445_strngs01.syx
wget -c "${DIR}"/strngs02.syx -O sysex/voice/000446_strngs02.syx
wget -c "${DIR}"/strngs03.syx -O sysex/voice/000447_strngs03.syx
wget -c "${DIR}"/strngs04.syx -O sysex/voice/000448_strngs04.syx
wget -c "${DIR}"/strngs05.syx -O sysex/voice/000449_strngs05.syx
wget -c "${DIR}"/strngs06.syx -O sysex/voice/000450_strngs06.syx
wget -c "${DIR}"/strngs07.syx -O sysex/voice/000451_strngs07.syx
wget -c "${DIR}"/strngs08.syx -O sysex/voice/000452_strngs08.syx
wget -c "${DIR}"/strngs09.syx -O sysex/voice/000453_strngs09.syx
wget -c "${DIR}"/strngs10.syx -O sysex/voice/000454_strngs10.syx
wget -c "${DIR}"/strngs12.syx -O sysex/voice/000455_strngs12.syx
wget -c "${DIR}"/strngs13.syx -O sysex/voice/000456_strngs13.syx
wget -c "${DIR}"/strngs14.syx -O sysex/voice/000457_strngs14.syx
wget -c "${DIR}"/strngs15.syx -O sysex/voice/000458_strngs15.syx
wget -c "${DIR}"/strngs16.syx -O sysex/voice/000459_strngs16.syx
wget -c "${DIR}"/strngs17.syx -O sysex/voice/000460_strngs17.syx
wget -c "${DIR}"/strngs18.syx -O sysex/voice/000461_strngs18.syx
wget -c "${DIR}"/strngs19.syx -O sysex/voice/000462_strngs19.syx
wget -c "${DIR}"/strngs20.syx -O sysex/voice/000463_strngs20.syx
wget -c "${DIR}"/sustain1.syx -O sysex/voice/000464_sustain1.syx
wget -c "${DIR}"/synth_01.syx -O sysex/voice/000465_synth_01.syx
wget -c "${DIR}"/synth_02.syx -O sysex/voice/000466_synth_02.syx
wget -c "${DIR}"/synth_03.syx -O sysex/voice/000467_synth_03.syx
wget -c "${DIR}"/synth_04.syx -O sysex/voice/000468_synth_04.syx
wget -c "${DIR}"/synth_05.syx -O sysex/voice/000469_synth_05.syx
wget -c "${DIR}"/synth_06.syx -O sysex/voice/000470_synth_06.syx
wget -c "${DIR}"/synth_07.syx -O sysex/voice/000471_synth_07.syx
wget -c "${DIR}"/synth_08.syx -O sysex/voice/000472_synth_08.syx
wget -c "${DIR}"/synth_09.syx -O sysex/voice/000473_synth_09.syx
wget -c "${DIR}"/synth_10.syx -O sysex/voice/000474_synth_10.syx
wget -c "${DIR}"/synth_11.syx -O sysex/voice/000475_synth_11.syx
wget -c "${DIR}"/synth_12.syx -O sysex/voice/000476_synth_12.syx
wget -c "${DIR}"/synth_13.syx -O sysex/voice/000477_synth_13.syx
wget -c "${DIR}"/synth_14.syx -O sysex/voice/000478_synth_14.syx
wget -c "${DIR}"/synth_15.syx -O sysex/voice/000479_synth_15.syx
wget -c "${DIR}"/synth_16.syx -O sysex/voice/000480_synth_16.syx
wget -c "${DIR}"/synth_17.syx -O sysex/voice/000481_synth_17.syx
wget -c "${DIR}"/synth_18.syx -O sysex/voice/000482_synth_18.syx
wget -c "${DIR}"/synth_19.syx -O sysex/voice/000483_synth_19.syx
wget -c "${DIR}"/synth_20.syx -O sysex/voice/000484_synth_20.syx
wget -c "${DIR}"/synth_21.syx -O sysex/voice/000485_synth_21.syx
wget -c "${DIR}"/synth_22.syx -O sysex/voice/000486_synth_22.syx
wget -c "${DIR}"/synths.syx -O sysex/voice/000487_synths.syx
wget -c "${DIR}"/syx_0628.syx -O sysex/voice/000488_syx_0628.syx
wget -c "${DIR}"/textures.syx -O sysex/voice/000489_textures.syx
wget -c "${DIR}"/tfi5.syx -O sysex/voice/000490_tfi5.syx
wget -c "${DIR}"/tfi6.syx -O sysex/voice/000491_tfi6.syx
wget -c "${DIR}"/tfi7.syx -O sysex/voice/000492_tfi7.syx
wget -c "${DIR}"/tfi8.syx -O sysex/voice/000493_tfi8.syx
wget -c "${DIR}"/tfrack01.syx -O sysex/voice/000494_tfrack01.syx
wget -c "${DIR}"/tfrack02.syx -O sysex/voice/000495_tfrack02.syx
wget -c "${DIR}"/theatre.syx -O sysex/voice/000496_theatre.syx
wget -c "${DIR}"/things.syx -O sysex/voice/000497_things.syx
wget -c "${DIR}"/trumpet.syx -O sysex/voice/000498_trumpet.syx
wget -c "${DIR}"/tx7a.syx -O sysex/voice/000499_tx7a.syx
wget -c "${DIR}"/violin_1.syx -O sysex/voice/000500_violin_1.syx
wget -c "${DIR}"/voices_1.syx -O sysex/voice/000501_voices_1.syx
wget -c "${DIR}"/voices_2.syx -O sysex/voice/000502_voices_2.syx
wget -c "${DIR}"/voices_3.syx -O sysex/voice/000503_voices_3.syx
wget -c "${DIR}"/voices_5.syx -O sysex/voice/000504_voices_5.syx
wget -c "${DIR}"/voices_6.syx -O sysex/voice/000505_voices_6.syx
wget -c "${DIR}"/voices_7.syx -O sysex/voice/000506_voices_7.syx
wget -c "${DIR}"/voices_8.syx -O sysex/voice/000507_voices_8.syx
wget -c "${DIR}"/voices_9.syx -O sysex/voice/000508_voices_9.syx
wget -c "${DIR}"/weird1.syx -O sysex/voice/000509_weird1.syx
wget -c "${DIR}"/weird2.syx -O sysex/voice/000510_weird2.syx
wget -c "${DIR}"/weird3.syx -O sysex/voice/000511_weird3.syx
wget -c "${DIR}"/wheatley.syx -O sysex/voice/000512_wheatley.syx
wget -c "${DIR}"/white.syx -O sysex/voice/000513_white.syx
wget -c "${DIR}"/wodwind1.syx -O sysex/voice/000514_wodwind1.syx
wget -c "${DIR}"/wodwind2.syx -O sysex/voice/000515_wodwind2.syx
wget -c "${DIR}"/wodwind3.syx -O sysex/voice/000516_wodwind3.syx
wget -c "${DIR}"/wodwind4.syx -O sysex/voice/000517_wodwind4.syx
wget -c "${DIR}"/wodwind5.syx -O sysex/voice/000518_wodwind5.syx
wget -c "${DIR}"/wodwind6.syx -O sysex/voice/000519_wodwind6.syx
wget -c "${DIR}"/woodwind.syx -O sysex/voice/000520_woodwind.syx
wget -c "${DIR}"/wurlizer.syx -O sysex/voice/000521_wurlizer.syx
wget -c "${DIR}"/wwind1.syx -O sysex/voice/000522_wwind1.syx
wget -c "${DIR}"/wwind2.syx -O sysex/voice/000523_wwind2.syx
wget -c "${DIR}"/wwind3.syx -O sysex/voice/000524_wwind3.syx
wget -c "${DIR}"/wwind4.syx -O sysex/voice/000525_wwind4.syx
wget -c "${DIR}"/wyatt.syx -O sysex/voice/000526_wyatt.syx
wget -c "${DIR}"/xylos.syx -O sysex/voice/000527_xylos.syx
wget -c "${DIR}"/yam-26.syx -O sysex/voice/000528_yam-26.syx
wget -c "${DIR}"/yamaha.syx -O sysex/voice/000529_yamaha.syx
wget -c "${DIR}"/zone3.syx -O sysex/voice/000530_zone3.syx
