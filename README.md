# function_trace_pin
This Repo deals with Function trace code with Intel PIN TOOL, use for [text_attention](https://github.com/laoeve/text_attention)

0) Makefile finds Intel PIN Tool Setup Dir. D/L from this link

        $wget http://software.intel.com/sites/landingpage/pintool/downloads/pin-3.2-81205-gcc-linux.tar.gz
        $tar -cvf pin-3.2-81205-gcc-linux.tar.gz ${PIN_ROOT}
1) configure from Makefile

        $PIN_ROOT = 'Extract PIN Dir.'
        $TOOL_ROOTS := 'Change File Name'

2) use run.sh for execute pin code with 

        $ ./run.sh -- ${prog_name}

   
