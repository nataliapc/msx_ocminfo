## OCMINFO for MSX++ core (OCM-PLD)


This program aims to be a helpful tool for configuring and maintaining MSX++ (OCM-PLD) FPGA-based systems.\
Currently supported devices: _1ChipMSX_, _Zemmix Neo_, _SX-1_, _SM-X_, _MCP2_, _SX-2_, _SM-X Mini_, _SM-X HB_, _DE0CV_, _SX-E_.

The program is using the [sdcc_msxconio](https://github.com/nataliapc/sdcc_msxconio) library as a git submodule. It is based in the old _'conio'_ library from _Turbo C_.

The core can be found at the [official MSX++ repository](https://github.com/gnogni/ocm-pld-dev) maintained by [@KdL](https://github.com/gnogni).

![F1 panel](.images/f1.jpg) ![F2 panel](.images/f3.jpg)

----


### Usage

Use the function keys (_F1-F5_) or _TAB_ to navigate the different panels, and the cursor keys to select the items. A description/information text will appear at bottom.

To modify an item, you can press _SPACE_, _ENTER_, or _'+'_ to increase its value or _'-'_ to decrease it.

To mute/unmute the program sounds press _'M'_.

Press _'P'_ to get access to the Profiles panel, where you can create user profiles with different settings.

You can also using **OCMINFO** like command line program with parameters:

	Usage: OCMINFO [/n|/L] [/Q] [/?]
	
	Use without parameters to open the configuration panels.
	
	Options:
	  /n    Apply the user profile number 'n'.
	  /L    List the user profiles.
	  /Q    Quiet mode (no verbose).
	  /?    Show this help.

If you want to suggest improvements, feel free to create a github issue.

----

### Thanks

Thanks to [@KdL](https://github.com/gnogni), [@ducasp](https://github.com/ducasp) and [@HRA!](https://github.com/hra1129) for their core developments and support, and to [@cayce-msx](https://github.com/cayce-msx) for his [Wiki](https://github.com/cayce-msx/msxpp-quick-ref/wiki).


