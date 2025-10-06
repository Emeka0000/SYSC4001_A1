/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 *
 */

 #include<interrupts.hpp>

 int main(int argc, char** argv) {
 
     //vectors is a C++ std::vector of strings that contain the address of the ISR
     //delays  is a C++ std::vector of ints that contain the delays of each device
     //the index of these elemens is the device number, starting from 0
     auto [vectors, delays] = parse_args(argc, argv);
     std::ifstream input_file(argv[1]);
 
     std::string trace;      //!< string to store single line of trace file
     std::string execution;  //!< string to accumulate the execution output
 
    /******************ADD YOUR VARIABLES HERE*************************/
    int current_time = 0;
    const int context_save_time = 10;
    const int isr_activity_time = 40;
    const int iret_time = 1;



    /******************************************************************/

    //parse each line of the input trace file
    while(std::getline(input_file, trace)) {
        auto [activity, duration_intr] = parse_trace(trace);

        /******************ADD YOUR SIMULATION CODE HERE*************************/
        if(activity == "CPU"){
            int dur = duration_intr;
            execution += std::to_string(current_time) + ", " + std::to_string(dur) + ", CPU Burst\n";
            current_time += dur;
        }
        else if(activity == "SYSCALL" || activity == "END_IO") {
            int intr_num = duration_intr; // device number / index into vectors/delays

            // 1) Interrupt entry boilerplate: switch to kernel, context save, find vector, load PC
            auto [boilerplate_exec, new_time] = intr_boilerplate(current_time, intr_num, context_save_time, vectors);
            execution += boilerplate_exec;
            current_time = new_time;

            // 2) Execute ISR body. The total device delay is given by delays[intr_num]
            if(intr_num < 0 || intr_num >= static_cast<int>(delays.size())) {
                // defensive check: invalid device number -> log an error entry and skip
                execution += std::to_string(current_time) + ", " + std::to_string(1) + ", invalid device number\n";
            } else {
                int total_delay = delays.at(intr_num);
                int remaining = total_delay;

                if(activity == "SYSCALL") {
                    // SYSCALL ISR activities
                    // First activity: run the ISR (device driver)
                    int chunk1 = (remaining >= isr_activity_time) ? isr_activity_time : remaining;
                    execution += std::to_string(current_time) + ", " + std::to_string(chunk1) + ", SYSCALL: run the ISR (device driver)\n";
                    current_time += chunk1;
                    remaining -= chunk1;

                    // Second activity: transfer data from device to memory (if there's time left)
                    if(remaining > 0) {
                        int chunk2 = (remaining >= isr_activity_time) ? isr_activity_time : remaining;
                        execution += std::to_string(current_time) + ", " + std::to_string(chunk2) + ", transfer data from device to memory\n";
                        current_time += chunk2;
                        remaining -= chunk2;
                    }

                    // Third activity: check for errors (all remaining time)
                    if(remaining > 0) {
                        execution += std::to_string(current_time) + ", " + std::to_string(remaining) + ", check for errors\n";
                        current_time += remaining;
                        remaining = 0;
                    }
                } else { // END_IO
                    // END_IO ISR activities
                    // First activity: run the ISR (device driver)
                    int chunk1 = (remaining >= isr_activity_time) ? isr_activity_time : remaining;
                    execution += std::to_string(current_time) + ", " + std::to_string(chunk1) + ", ENDIO: run the ISR (device driver)\n";
                    current_time += chunk1;
                    remaining -= chunk1;

                    // Second activity: check device status (all remaining time)
                    if(remaining > 0) {
                        execution += std::to_string(current_time) + ", " + std::to_string(remaining) + ", check device status\n";
                        current_time += remaining;
                        remaining = 0;
                    }
                }
            }

            // 3) IRET and return from interrupt
            execution += std::to_string(current_time) + ", " + std::to_string(iret_time) + ", IRET\n";
            current_time += iret_time;
        }
        else {
            // unknown activity: write a warning entry (defensive)
            execution += std::to_string(current_time) + ", " + std::to_string(1) + ", unknown activity: " + activity + "\n";
            current_time += 1;
        }







        /************************************************************************/

    }
 
     input_file.close();
 
     write_output(execution);
 
     return 0;
 }
 