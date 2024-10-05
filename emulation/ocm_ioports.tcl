namespace eval ocm_ioports {

	variable watchpoint_id_write
	variable watchpoint_id_read
	variable watchpoint_smartcmd
	variable watchpoint_write44
	variable watchpoint_read1
	variable watchpoint_read2
	variable watchpoint_read4b

	variable ioext_id
	set ioext_id 0x00

	variable port4b_id
	set port4b_id 0

	variable ioports_array
	# 0x42(66)	[R/Wn] Virtual DIP-SW
	# 0x43(67)	[R/Wn] Lock toggles
	# 0x44(68)	[R/Wn] Lights
	# 0x45(69)	[R/Wn] PSG / Master
	# 0x46(70)	[R/Wn] OPLL / SCC-I
	# 0x47(71)	[R] Custom CPU speed(3) / Turbo MegaSD(1) / tPANA redir (1) / VDP Speed(1) / ...
	# 0x48(72)	[R] tPANA(1) / Keyb Layout(1) / SCRLK (1) / Lights (1) / ...
	# 0x49(73)	[R] Stereo(1) / Bus clock(1) / Machine type(4) / Forced(1) / NTSC/PAL(1)
	# 0x4a(74)	[R] InverseAudio(1) / Pixel 1:1(3) / CenterYJK(1) / LEgacyOutput(1) / ...
	# 0x4b(0)	[R] Scanlines(2) / PSG2(1) / SDRAM(2) / ...
	# 0x4b(1)	[R] Extended megarom reading (1) / Ext.SDRAM(3) / Vertical offset(4)
	# 0x4c(76)	[R] Hard DIP-SW
	# 0x4d(77)	[R/Wn] VRAM slots
	# 0x4e(78)	[R] OCM-PLD version(8)
	# 0x4f(79)	[R] I/O Revision(5) / OCM-PLD sub-version(2) / Default keyb layout(1)
	array set ioports_array {
		66	0b11010110
		67	0b00000000
		68	0b00000000
		69	0b01000111
		70	0b01110111
		71	0b00011111
		72	0b00000010
		73	0b01001100
		74	0b00010000
		 0	0b00010000
		 1	0b01110000
		76	0b11010110
		77  0b00000000
		78	0b00100111
		79	0b11001100
	}

	variable cmd
	array set cmd {
		0	{ 0 0 0 }
		1	{ 71 0b00010000 0b00000000 }
		2	{ 71 0b00010000 0b00010000 }
		3	{ 66 0b00000001 0b00000000 71 0b00000111 0b00000000 72 0b00000001 0b00000000 }
		4	{ 66 0b00000001 0b00000001 71 0b00000111 0b00000001 72 0b00000001 0b00000000 }
		5	{ 66 0b00000001 0b00000001 71 0b00000111 0b00000010 72 0b00000001 0b00000000 }
		6	{ 66 0b00000001 0b00000001 71 0b00000111 0b00000011 72 0b00000001 0b00000000 }
		7	{ 66 0b00000001 0b00000001 71 0b00000111 0b00000100 72 0b00000001 0b00000000 }
		8	{ 66 0b00000001 0b00000001 71 0b00000111 0b00000101 72 0b00000001 0b00000000 }
		9	{ 66 0b00000001 0b00000001 71 0b00000111 0b00000110 72 0b00000001 0b00000000 }
		10	{ 66 0b00000001 0b00000001 71 0b00000111 0b00000111 72 0b00000001 0b00000000 }
		11	{ 71 0b00001000 0b00000000 }
		12	{ 71 0b00001000 0b00001000 }
		13	{ 66 0b00111000 0b00000000 } 14 { 66 0b00111000 0b00001000 } 15 { 66 0b00111000 0b00010000 } 16 { 66 0b00111000 0b00011000 }
		17	{ 66 0b00111000 0b00100000 } 18 { 66 0b00111000 0b00101000 } 19 { 66 0b00111000 0b00110000 } 20 { 66 0b00111000 0b00111000 }
		21	{ 72 0b00000010 0b00000000 }
		22	{ 72 0b00000010 0b00000010 }
		23	{ 66 0b00000110 0b00000000 } 24 { 66 0b00000110 0b00000100 } 25 { 66 0b00000110 0b00000010 } 26 { 66 0b00000110 0b00000110 }
		27	{ 71 0b00100000 0b00000000 }
		28	{ 71 0b00100000 0b00100000 }
		61	{ 73 0b00000001 0b00000000 } 62	{ 73 0b00000001 0b00000001 }
		63	{ 73 0b00000010 0b00000000 }
		64	{ 73 0b00000010 0b00000010 66 0b00000001 0b00000000 71 0b00000111 0b00000000 72 0b00000001 0b00000000 }
		65	{ 66 0b00000001 0b00000000 72 0b00000001 0b00000001 }
		66	{ 74 0b00000001 0b00000000 } 67	{ 74 0b00000001 0b00000001 }
		80	{  0 0b00000011 0b00000000 } 81	{  0 0b00000011 0b00000001 }
		82	{  0 0b00000011 0b00000010 } 83	{  0 0b00000011 0b00000011 }
		84	{  0 0b00000100 0b00000000 } 85	{  0 0b00000100 0b00000100 }
		128	{ 0 0 0 }
		129	{ 74 0b00100000 0b00000000 }
		130	{ 74 0b00100000 0b00100000 }
		135	{ 72 0b00000100 0b00000000 } 136 { 72 0b00000100 0b00000100 }
		176	{ 69 0b01110000 0b00000000 } 177 { 69 0b01110000 0b00010000 } 178 { 69 0b01110000 0b00100000 } 179 { 69 0b01110000 0b00110000 }
		180	{ 69 0b01110000 0b01000000 } 181 { 69 0b01110000 0b01010000 } 182 { 69 0b01110000 0b01100000 } 183 { 69 0b01110000 0b01110000 }
		184	{ 69 0b00000111 0b00000000 } 185 { 69 0b00000111 0b00000001 } 186 { 69 0b00000111 0b00000010 } 187 { 69 0b00000111 0b00000011 }
		188	{ 69 0b00000111 0b00000100 } 189 { 69 0b00000111 0b00000101 } 190 { 69 0b00000111 0b00000110 } 191 { 69 0b00000111 0b00000111 }
		192	{ 70 0b01110000 0b00000000 } 193 { 70 0b01110000 0b00010000 } 194 { 70 0b01110000 0b00100000 } 195 { 70 0b01110000 0b00110000 }
		196	{ 70 0b01110000 0b01000000 } 197 { 70 0b01110000 0b01010000 } 198 { 70 0b01110000 0b01100000 } 199 { 70 0b01110000 0b01110000 }
		200	{ 70 0b00000111 0b00000000 } 201 { 70 0b00000111 0b00000001 } 202 { 70 0b00000111 0b00000010 } 203 { 70 0b00000111 0b00000011 }
		204	{ 70 0b00000111 0b00000100 } 205 { 70 0b00000111 0b00000101 } 206 { 70 0b00000111 0b00000110 } 207 { 70 0b00000111 0b00000111 }
		208	{ 73 0b11000000 0b00000000 }
		209	{ 73 0b11000000 0b01000000 }
		210	{ 73 0b11000000 0b10000000 }
		214	{ 74 0b00010000 0b00000000 }
		215	{ 74 0b00010000 0b00010000 }
	}

	proc ocm_ioports_start {} {
		set watchpoint_id_write [debug set_watchpoint write_io 0x40 {} { ocm_ioports::trigger_id_write }]
		set watchpoint_id_read [debug set_watchpoint read_io 0x40 {} { ocm_ioports::trigger_id_read }]
		set watchpoint_smartcmd [debug set_watchpoint write_io 0x41 {} { ocm_ioports::trigger_smartcmd }]
		set watchpoint_write44 [debug set_watchpoint write_io 0x44 {} { ocm_ioports::trigger_write44 }]
		set watchpoint_read1 [debug set_watchpoint read_io {0x42 0x4a} {} { ocm_ioports::trigger_read }]
		set watchpoint_read2 [debug set_watchpoint read_io {0x4c 0x4f} {} { ocm_ioports::trigger_read }]
		set watchpoint_read4b [debug set_watchpoint read_io 0x4b {} { ocm_ioports::trigger_read4b }]
		return "Enabled OCM I/O Ports emulation"
	}

	proc ocm_ioports_stop {} {
		debug remove_watchpoint $watchpoint_id_write
		debug remove_watchpoint $watchpoint_id_read
		debug remove_watchpoint $watchpoint_smartcmd
		debug remove_watchpoint $watchpoint_write44
		debug remove_watchpoint $watchpoint_read1
		debug remove_watchpoint $watchpoint_read2
		debug remove_watchpoint $watchpoint_read4b
		return "Stopped OCM I/O Ports emulation"
	}

	proc trigger_id_write {} {
		set ocm_ioports::ioext_id $::wp_last_value
	}

	proc trigger_id_read {} {
		if {$ocm_ioports::ioext_id == 0xd4} {
			set complement [expr {255 - 0xd4}]
			after time 0 "reg a $complement"
		}
	}

	proc trigger_smartcmd {} {
		if {![info exists ocm_ioports::cmd($::wp_last_value)]} {
			puts stderr [format "Error: Invalid smart command 0x%02X (%d)" $::wp_last_value $::wp_last_value]
			return
		}
		set cmd $ocm_ioports::cmd($::wp_last_value)
		foreach {port_num mask value} $cmd {
			set current_value [expr {$ocm_ioports::ioports_array($port_num) & ~$mask}]
			set new_value [expr {$current_value | ($value & $mask)}]
			set ocm_ioports::ioports_array($port_num) $new_value
		}
	}

	proc trigger_read {} {
		set in_port [expr $::wp_last_address & 0xFF]
		if {$in_port == 0x44} {
			after time 0 "reg a $ocm_ioports::port4b_id"
		} else {
			set in_value $ocm_ioports::ioports_array($in_port)
			after time 0 "reg a $in_value"
		}
	}

	proc trigger_write44 {} {
		set in_value [expr {255 - $::wp_last_value}]
		set ocm_ioports::port4b_id $in_value
	}

	proc trigger_read4b {} {
		set in_value $ocm_ioports::ioports_array($ocm_ioports::port4b_id)
		after time 0 "reg a $in_value"
	}


	namespace export ocm_ioports_start
	namespace export ocm_ioports_stop

} ; #namespace

namespace import ocm_ioports::*


ocm_ioports_start
