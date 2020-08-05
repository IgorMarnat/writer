//
//  main.cpp
//  writer
//
//  Created by Igor Marnat on 2/8/17.
//  Copyright © 2017 Igor Marnat. All rights reserved.
//

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

//enum TLogLevel {log_error, log_warning, log_info, log_debug};
enum TLogLevel {log_debug, log_info, log_warning, log_error};

class Log
{
public:
    Log();
    virtual ~Log();
    std::ostringstream& Get(TLogLevel level = log_info);
public:
    static TLogLevel& ReportingLevel();
protected:
    std::ostringstream os;
private:
    Log(const Log&);
    Log& operator =(const Log&);
private:
    TLogLevel messageLevel;
    static TLogLevel repLevel;
};

std::ostringstream& Log::Get(TLogLevel level)
{
    os << " " << std::to_string(level) << ": ";
    os << std::string(level > log_debug ? 0 : level - log_debug, '\t');
    messageLevel = level;
    return os;
}
Log::~Log()
{
    os << "ml: " << messageLevel << " rl: " << Log::ReportingLevel() << " ";
    if (messageLevel >= Log::ReportingLevel())
    {
        os << std::endl;
        fprintf(stderr, "%s", os.str().c_str());
        fflush(stderr);
    }
}

Log::Log () {
    
}

TLogLevel& Log::ReportingLevel() {
    return Log::repLevel;
}

TLogLevel Log::repLevel;

int fd = -1;
int run = 1;

void termHandler (int signum) {
    std::cout << "in termhandler, signal " << signum << std::endl;
    if (fd != -1) {
        close (fd);
    }
    run = 0;
    Log().Get(log_debug) << "TermHandler, exiting on signal " << signum << "... " << std::endl;
}

#define BUFSIZE 100

int main(int argc, const char * argv[]) {
    char c;
    const char* opts = "n:t:";
    char buf [BUFSIZE];
    int nread;
    bool monitor = false;
    int nbytes = BUFSIZE;
    int timeout = 5;
    
    
    Log::ReportingLevel() = log_debug;
    Log().Get(log_info) << "Started" << std::endl;
    
    while ((c = getopt (argc, (char *const *)argv, opts)) != -1) {
        switch (c) {
            case 'n':
                nbytes = atoi (optarg);
                
                Log().Get (log_debug) << "Nbytes: " << std::to_string (nbytes).c_str();
                
                if (nbytes > BUFSIZE) {
                    Log().Get (log_error) << "ERROR: Nbytes " << std::to_string (nbytes) << "is bigger than buffer size " << std::to_string (BUFSIZE) << "exiting";
                    exit (1);
                }
                break;
            case 't':
                // time in sec
                timeout = atoi (optarg);
                break;
            case '?':
                std::cout << "Unknown option, please use one of " << opts << std::endl;
                exit (1);
                break;
        }
    }
    
    Log().Get(log_debug) << "Parsed options" << std::endl;
    
    if (optind < argc) {
        Log().Get(log_debug) << "Goint to write to the file " << argv [optind] << std::endl;
        fd = open (argv [optind], O_APPEND | O_CREAT);
        if (fd == -1) {
            Log().Get(log_error) <<  "Can't open " << argv[optind] << "exiting ... ";
            exit (1);
        }
    }
    // no file name provided, use stdout
    else {
        Log().Get(log_debug) << "No file name provided, using STDOUT (fd:" << STDOUT_FILENO << ")" << std::endl;
        fd = STDOUT_FILENO;
        
    }
    
    //signal (SIGTERM, termHandler);
    struct sigaction sigIntHandler;
    
    sigIntHandler.sa_handler = termHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    
    sigaction(SIGINT, &sigIntHandler, NULL);
    
    Log().Get(log_debug) << "Going to write the data to fd, press CTRL-C to stop ... " << fd << std::endl;
    srand (time (NULL));

    while (run) {
        if (fd >= 0) {
            for (int i = 0; i < BUFSIZE; i ++)
                buf [i] = ' ' + (rand () % 94);
            
            Log().Get(log_debug) << "Writing " << nbytes << std::endl;
            write (fd, buf, nbytes);
            sleep (timeout);
        }
        else {
            Log().Get(log_debug) << "Before error message " << std::endl;
            Log().Get(log_error) << "No file name provided, bye!" << std::endl;
            Log().Get(log_debug) << "After error message " << std::endl;
        }
        
    }
    Log().Get(log_debug) << "Exiting " << std::endl;
    return 0;
}

