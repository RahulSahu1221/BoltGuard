LDmicro0.2
MICRO=Atmel AVR ATmega16 40-PDIP
CYCLE=50000 us at Timer0, YPlcCycleDuty:0, ConfigurationWord(s):0x3CA5223E00000000
CRYSTAL=16000000 Hz
BAUD=9600 Hz, RATE=1000000 Hz, SPEED=100000 Hz
COMPILED=E:\Presentations For JP Companies\Dai-Ichi Detsu\BoltGuard_Station_Controller.hex
COMPILER=IHEX

PULL-UP LIST
    PA: 0xFFFFFFFF 
    PB: 0xFFFFFFFF 
    PC: 0xFFFFFFFF 
    PD: 0xFFFFFFFF 
END

VAR LIST
    2 bytes IFault_Lock 	Now not used !!!
END

IO LIST
    XAny_Fail at 35 0 0
    XBolt1_Pass at 38 0 0
    XBolt2_Pass at 37 0 0
    XBolt3_Pass at 36 0 0
    XReset at 39 0 0
    XStart at 40 0 0
    YEnable_Bolt1 at 1 0 0
    YEnable_Bolt2 at 2 0 0
    YEnable_Bolt3 at 3 0 0
    YFault_Lock at 5 0 0
    YLine_Advance at 4 0 0
END

PROGRAM
RUNG 1
  CONTACTS XStart 0 0
  CONTACTS YFault_Lock 1 0
  COIL YEnable_Bolt1 0 1 0 0
END
RUNG 2
  CONTACTS YEnable_Bolt1 0 0
  CONTACTS XBolt1_Pass 0 0
  PARALLEL
    COIL YEnable_Bolt1 0 0 1 0
    COIL YEnable_Bolt2 0 1 0 0
  END
END
RUNG 3
  CONTACTS YEnable_Bolt2 0 0
  CONTACTS XBolt2_Pass 0 0
  PARALLEL
    COIL YEnable_Bolt2 0 0 1 0
    COIL YEnable_Bolt3 0 1 0 0
  END
END
RUNG 4
  CONTACTS YEnable_Bolt3 0 0
  CONTACTS XBolt3_Pass 0 0
  PARALLEL
    COIL YEnable_Bolt3 0 0 1 0
    COIL YLine_Advance 0 1 0 0
  END
END
RUNG 5
  CONTACTS XAny_Fail 0 0
  PARALLEL
    COIL YFault_Lock 0 1 0 0
    COIL YEnable_Bolt1 0 0 1 0
    COIL YEnable_Bolt2 0 0 1 0
    COIL YEnable_Bolt3 0 0 1 0
    COIL YLine_Advance 0 0 1 0
  END
END
RUNG 6
  CONTACTS XReset 0 0
  CONTACTS YFault_Lock 0 0
  COIL YFault_Lock 0 0 1 0
END
