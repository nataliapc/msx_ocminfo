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

	variable port41
	set port41 0

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

	# SX-E
	set ioports_array(73) 0b01011000
	set ioports_array(0)  0b00011000
	set ioports_array(1)  0b01100000

	# 3.9.1
#	set ioports_array(78) 0b00100111
#	set ioports_array(79) 0b10101011


	variable cmd
	array set cmd {
		0	{ 0 0 0 }
		1	{ 71 0b00010000 0b00000000 }
		2	{ 71 0b00010000 0b00010000 }
		3	{ 66 0b00000001 0b00000000 72 0b00000001 0b00000000 }
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
		39	{ 72 0b00000100 0b00000000 }
		40	{ 72 0b00000100 0b00000100 }
		41	{ 67 0b00000001 0b00000001 }
		42	{ 67 0b00000001 0b00000000 }
		43  { 67 0b00000010 0b00000010 }
		44	{ 67 0b00000010 0b00000000 }
		45  { 67 0b00000100 0b00000100 }
		46	{ 67 0b00000100 0b00000000 }
		47  { 67 0b00001000 0b00001000 }
		48	{ 67 0b00001000 0b00000000 }
		49  { 67 0b00010000 0b00010000 }
		50	{ 67 0b00010000 0b00000000 }
		51  { 67 0b00011000 0b00011000 }
		52	{ 67 0b00011000 0b00000000 }
		53  { 67 0b00100000 0b00100000 }
		54	{ 67 0b00100000 0b00000000 }
		55  { 67 0b01000000 0b01000000 }
		56	{ 67 0b01000000 0b00000000 }
		57  { 67 0b10000000 0b10000000 }
		58	{ 67 0b10000000 0b00000000 }
		59	{ 67 0b11111111 0b11111111 }
		60	{ 67 0b11111111 0b00000000 }
		61	{ 73 0b00000001 0b00000000 } 62	{ 73 0b00000001 0b00000001 }
		63	{ 73 0b00000010 0b00000000 }
		64	{ 73 0b00000010 0b00000010 66 0b00000001 0b00000000 71 0b00000111 0b00000000 72 0b00000001 0b00000000 }
		65	{ 66 0b00000001 0b00000000 72 0b00000001 0b00000001 }
		66	{ 74 0b00000001 0b00000000 } 67	{ 74 0b00000001 0b00000001 }
		71	{  1 0b11110000 0b01000000 }
		72	{  1 0b11110000 0b01010000 }
		73	{  1 0b11110000 0b01100000 }
		74	{  1 0b11110000 0b01110000 }
		75	{  1 0b11110000 0b10000000 }
		76	{  1 0b11110000 0b10010000 }
		77	{  1 0b11110000 0b10100000 }
		78	{  1 0b11110000 0b10110000 }
		79	{  1 0b11110000 0b11000000 }
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
		set watchpoint_smartcmd_w [debug set_watchpoint write_io 0x41 {} { ocm_ioports::trigger_smartcmd_write }]
		set watchpoint_smartcmd_r [debug set_watchpoint read_io 0x41 {} { ocm_ioports::trigger_smartcmd_read }]
		set watchpoint_write44 [debug set_watchpoint write_io 0x44 {} { ocm_ioports::trigger_write44 }]
		set watchpoint_read1 [debug set_watchpoint read_io {0x42 0x4a} {} { ocm_ioports::trigger_read }]
		set watchpoint_read2 [debug set_watchpoint read_io {0x4c 0x4f} {} { ocm_ioports::trigger_read }]
		set watchpoint_write1 [debug set_watchpoint write_io {0x42 0x44} {} { ocm_ioports::trigger_write }]
		set watchpoint_write2 [debug set_watchpoint write_io 0x4d {} { ocm_ioports::trigger_write }]
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
		ocm_info_update
	}

	proc trigger_id_read {} {
		if {$ocm_ioports::ioext_id == 0xd4} {
			set complement [expr {255 - 0xd4}]
			after time 0 "reg a $complement"
		}
	}

	proc trigger_smartcmd_write {} {
		if {![info exists ocm_ioports::cmd($::wp_last_value)]} {
			puts stderr [format "Error: Invalid smart command 0x%02X (%d)" $::wp_last_value $::wp_last_value]
			return
		}
		set cmd $ocm_ioports::cmd($::wp_last_value)
		set ocm_ioports::port41 $::wp_last_value
		foreach {port_num mask value} $cmd {
			set current_value [expr {$ocm_ioports::ioports_array($port_num) & ~$mask}]
			set new_value [expr {$current_value | ($value & $mask)}]
			set ocm_ioports::ioports_array($port_num) $new_value
		}
		ocm_info_update
	}

	proc trigger_smartcmd_read {} {
		set value [expr { 255 - $ocm_ioports::port41}]
		after time 0 "reg a $value"
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

	proc trigger_write {} {
		set in_port [expr $::wp_last_address & 0xFF]
		set in_value [expr {255 - $::wp_last_value}]
		set ocm_ioports::ioports_array($in_port) $in_value
		ocm_info_update
	}

	proc trigger_write44 {} {
		set in_value [expr {255 - $::wp_last_value}]
		set ocm_ioports::port4b_id $in_value
		ocm_info_update
	}

	proc trigger_read4b {} {
		set in_value $ocm_ioports::ioports_array($ocm_ioports::port4b_id)
		after time 0 "reg a $in_value"
	}


	namespace export ocm_ioports_start
	namespace export ocm_ioports_stop


	################################################################## Panel

	variable info_active false
	variable textheight 6
	variable panel_margin 0
	variable sub_panel_width 84
	variable sub_panel_height [expr {$textheight + 1}]
	variable panel_info

	array set flaglabels {
		64 "40h DeviceID"
		66 "42h VirtualDIP"
		67 "43h LockToggles"
		68 "44h LedLights"
		69 "45h AudioVol0"
		70 "46h AudioVol1"
		71 "47h SysInfo0"
		72 "48h SysInfo1"
		73 "49h SysInfo2"
		74 "4Ah SysInfo3"
		0  "4Bh#0 SysInfo4#0"
		1  "4Bh#1 SysInfo4#1"
		76 "4Ch SysInfo5"
		78 "4Eh Version0"
		79 "4Fh Version1"
	}

	proc ocm_info_init {} {
		variable flaglabels
		variable info_active
		variable textheight
		variable panel_margin
		variable sub_panel_width
		variable sub_panel_height
		variable panel_info

		set panel_info [dict create \
			flags [dict create \
				title "  === OCM # I/O Ports ===" width 800 row 0 color 0x000000ff \
				num "" ] \
		]

		set row 1
		foreach port {64 66 67 68 69 70 71 72 73 74 0 1 76 78 79} {
			set label $flaglabels($port)
			set col 0x000000cc
			dict append panel_info \
				$label [dict create \
					title "$label" \
					num $port \
					width $sub_panel_width \
					row $row \
					color $col ]
			set row [expr { $row + 1 }]
		}

		# calc width of software item
		set full_width 0
		dict for {name info} $panel_info {
			if {[dict get $info row] == 1} {
				incr full_width [dict get $info width]
				incr full_width $panel_margin
			}
		}
		incr full_width -$panel_margin
		dict set panel_info flags width $full_width

		# set base element
		osd create rectangle info \
			-x [expr {$panel_margin * 2}] \
			-y [expr {$panel_margin * 2}] \
			-scaled true \
			-alpha 0

		# create subpanels
		dict for {name info} $panel_info {
			set row [dict get $info row]
			create_sub_panel $name \
							[dict get $info title] \
							[dict get $info num ] \
							[dict get $info color] \
							[dict get $info width] \
							[dict get $info row]
		}

		ocm_info_update
	}

	proc create_sub_panel {name title num color width row} {
		variable textheight
		variable panel_margin
		variable sub_panel_height

		set value ""
		if {$num!=""} { set value "N/A" }

		osd create rectangle info.$name \
			-x $panel_margin \
			-y [expr {($sub_panel_height + $panel_margin) * $row}] \
			-h $sub_panel_height \
			-w $width \
			-rgba $color \
			-clip true
		osd create text info.$name.title \
			-x 1 \
			-y 1 \
			-rgba 0xffffffff \
			-text $title \
			-size [expr {int($textheight * 0.8)}]
		osd create text info.$name.value \
			-x [expr {$width - 25} ] \
			-y 1 \
			-rgba 0xffff00ff \
			-text $value \
			-size [expr {int($textheight * 0.8)}]
	}

	proc get_flag {num} {
		return [format %08b $ocm_ioports::ioports_array($num)]
	}

	proc ocm_info_update {} {
		variable info_active
		variable panel_info

		if {$info_active} {
			dict for {name info} $panel_info {
				set num [dict get $info num]
				if {$num!=""} {
					if {$num==64} {
						set value $ocm_ioports::ioext_id
					} else {
						set value [get_flag $num]
					}
					osd configure info.$name.value -text $value
				}
			}
		}
		return $info_active
	}

	proc ocm_toggle_info {} {
		variable info_active

		if {$info_active} {
			set info_active false
			osd destroy info
		} else {
			set info_active true
			ocm_info_init
		}
		return ""
	}


	namespace export ocm_info_init
	namespace export ocm_info_update
	namespace export ocm_toggle_info

} ; #namespace

namespace import ocm_ioports::*


ocm_ioports_start
#ocm_toggle_info