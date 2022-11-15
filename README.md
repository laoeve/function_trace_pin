# function_trace_pin
This Repo deals with Function trace code with Intel PIN TOOL, use for [text_attention](https://github.com/laoeve/text_attention)

0) Makefile finds Intel PIN Tool Setup Dir.

        $wget http://software.intel.com/sites/landingpage/pintool/downloads/pin-3.2-81205-gcc-linux.tar.gz
        $tar -xvf pin-3.2-81205-gcc-linux.tar.gz (at ${PIN_ROOT})

1) configure from Makefile

        $PIN_ROOT = 'Extract PIN Dir.'
        $TOOL_ROOTS := 'Change File Name if needed'

2) modify run.sh
        
        time -p $PIN_ROOT/pin -t $DIR/obj-*/'Change File Name'.so ${@}

3) use run.sh for execute pin code with 

        $ ./run.sh -- ${prog_name} ${options}

   
