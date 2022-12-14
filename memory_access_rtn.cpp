#include <fstream>
#include <iomanip>
#include <iostream>
#include <string.h>
#include "pin.H"

ofstream outFile;
 
// target probe point with procedure name mangled
// if running without probe, modify flag to TRUE
const string rtn_probe = "_ZTv0_n24_N14text_attention20AttentionTransformerIfE7forwardERNS_6TensorIfEERKS3_"; // Transformer->forward
//const string rtn_probe = "_ZTv0_n24_N14text_attention2T5IfE7forwardERNS_6TensorIfEERKS3_";  //  T5->forward
//const string rtn_probe = "_ZTv0_n24_N14text_attention4GPT2IfE7forwardERNS_6TensorIfEERKS3_"; // GPT2->forward
//const string rtn_probe = "_ZTv0_n24_N14text_attention4BERTIfE7forwardERNS_6TensorIfEERKS3_";  //  BERT->forward
bool rtnstart_flag = FALSE;

// Holds instruction count for a single procedure
typedef struct RtnCount
{
    string _name;
    string _image;
    ADDRINT _address;
    RTN _rtn;
    UINT64 _rtnCount;
    UINT64 _icount;
    UINT64 _sizecountRead;
    UINT64 _sizecountWrite;
    struct RtnCount * _next;
} RTN_COUNT;

// Linked list of instruction counts for each routine
RTN_COUNT * RtnList = 0;

// This function is called before every instruction is executed
VOID docount(UINT64 * counter)
{
    if(rtnstart_flag)
    {
        (*counter)++;
    }
}

// This function is called before every instruction is executed
VOID sizecount(UINT64 * counter, UINT32 memorySize)
{
    if(rtnstart_flag)
    {
        (*counter)+=memorySize;
    }
}
    
const char * StripPath(const char * path)
{
    const char * file = strrchr(path,'/');
    if (file)
        return file+1;
    else
        return path;
}

VOID RtnStartPoint ()
{
    cout << "Start point at Model Fwd" << endl;
    rtnstart_flag = TRUE;
}

// Pin calls this function every time a new rtn is executed
VOID Routine(RTN rtn, VOID *v)
{
    
    // Allocate a counter for this routine
    RTN_COUNT * rc = new RTN_COUNT;

    // The RTN goes away when the image is unloaded, so save it now
    // because we need it in the fini
    rc->_name = RTN_Name(rtn);
    rc->_image = StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str());
    rc->_address = RTN_Address(rtn);
    rc->_icount = 0;
    rc->_rtnCount = 0;
    rc->_sizecountRead = 0;
    rc->_sizecountWrite = 0;

    // Add to list of routines
    rc->_next = RtnList;
    RtnList = rc;
            
    RTN_Open(rtn);
    if (RTN_Name(rtn) == rtn_probe)
    {
        INS_InsertCall(RTN_InsHead(rtn), IPOINT_BEFORE, (AFUNPTR) RtnStartPoint, IARG_END);
    }            
    // Insert a call at the entry point of a routine to increment the call count
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)docount, IARG_PTR, &(rc->_rtnCount), IARG_END);
    
    // For each instruction of the routine
    for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
    {
        // Insert a call to docount to increment the instruction counter for this rtn
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_PTR, &(rc->_icount), IARG_END);
        if (INS_IsMemoryRead(ins))
        {				// For only memory reads???
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)sizecount, IARG_PTR, &(rc->_sizecountRead), IARG_MEMORYREAD_SIZE, IARG_END);
        }
        else if (INS_IsMemoryWrite(ins))
        {				// For only memory writes???
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)sizecount, IARG_PTR, &(rc->_sizecountWrite), IARG_MEMORYWRITE_SIZE, IARG_END);
        }
    }
    RTN_Close(rtn);
}

// This function is called when the application exits
// It prints the name and count for each procedure
VOID Fini(INT32 code, VOID *v)
{
    outFile << "Procedure" << ", "
          << "Image" << ", "
          << "Address" << ", "
          << "Calls" << ", "
          << "Instructions" << ", "
          << "Memory Read" << ", "
          << "Memory Write" << endl;

    for (RTN_COUNT * rc = RtnList; rc; rc = rc->_next)
    {
        if (rc->_icount > 0)
            outFile << rc->_name << ", "
                  << rc->_image << ", "
                  << hex << rc->_address << dec <<", "
                  << rc->_rtnCount << ", "
                  << rc->_icount << ", "
                  << rc->_sizecountRead << ", "
                  << rc->_sizecountWrite << endl;
    }

}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This Pintool counts the number of times a routine is executed" << endl;
    cerr << "and the number of instructions executed in a routine" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // Initialize symbol table code, needed for rtn instrumentation
    PIN_InitSymbols();

    outFile.open("memory_access.csv");

    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    // Register Routine to be called to instrument rtn
    RTN_AddInstrumentFunction(Routine, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
