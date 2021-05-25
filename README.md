# travelMonitor

The purpose of this project was to familiarize us with creating processes, using system calls fork/exec, process communication through named-pipes, low-level use I/O, signals and bash scripts.

In the framework of this project I implemented a distributed travelMonitor tool that accepts requests from citizens who want to travel to other countries, and checks if they have been appropriately vaccinated, and decides whether a traveler is allowed to enter a country.
More specifically, I implemented the travelMonitor application which creates a number of monitor processes that, together with the application, answers user questions.

![image](https://user-images.githubusercontent.com/75081526/119462842-d6446180-bd49-11eb-81e7-78078ec8f99e.png)

The application can be run as follows (inside the src subdirectory):

    make -s cleanall all run

To get started, the travelMonitor application creates a set of named pipes for
communication with the child processes. The parent process informs each Monitor proccess via named pipe about the
subdirectories to be undertaken by the Monitor. The travelMonitor(parent proccess) distributes evenly (with round-robin
alphabetically) the subdirectories (with the countries located in input_dir) to the Monitor processes.
When the application (parent process) finishes the initialization actions, it waits for a series of
bloom filters (more about that below) from Monitor processes and when it receives all the information, it is ready
to accept input (commands) from the user from the keyboard.

Each Monitor process, for each directory assigned to it, will read all its files and fill a set of data structures
(hash table, skip lists, bloom filters) that it will use to answer questions posed by
parent process. Each Monitor process, after it has finished reading the input files, sends via named
pipe to the parent process a bloom filter for each virus that represents the whole of the vaccinated
citizens of the countries managed by the Monitor process. If while reading files, a Monitor process detects a problem
record (e.g. in syntax or an inconsistent record), it ignores the problematic record.
When the Monitor process finishes reading the files assigned to it and has sent all bloom filters to
parent, it notifies the parent process via named pipe that the Monitor is ready to accept requests.

If a Monitor process receives a SIGINT or SIGQUIT signal then it prints to a file named log_file.pID
the name of the countries it manages, the total number of requests received for entry into the countries 
it manages, and the total number of requests approved and rejected.

If a Monitor process receives a SIGUSR1 signal, it means that 1 or more new files have been put
in one of the subdirectories assigned to it. The monitor process checks the subdirectories to find the new files,
and reads and updates the data structures it holds in memory. After completing the procedure
it sends the updated bloom filters to the parent process.

If a Monitor process terminates abruptly, the parent process forks a new Monitor process
that replaces it.

If the parent process receives SIGINT or SIGQUIT, it first finishes editing the current
command by the user and after responding to the user, sends a SIGKILL signal to the Monitors, and
waits for them to terminate, and eventually prints to a file named log_file.pID the name of all countries
that participated in the application with data, the total number of requests received to enter the countries, 
and the total number of requests approved and rejected.

The user can give the following commands to the application:

    /travelRequest citizenID date countryFrom countryTo virusName
The application first checks the bloom filter sent to it by the Monitor process that manages 
countryFrom. If the bloom filter indicates that the citizenID citizen has not been vaccinated
against virusName prints a corresponding message. Elsewise the
application requests via named pipe from the Monitor process that manages the countryFrom country if
the citizenID has indeed been vaccinated. The application checks if the citizen has been vaccinated 
less than 6 months before the desired date travel date and prints a corresponding message.

    /travelStats virusName date1 date2 [country]
If no country argument is given, the application prints the number of citizens who have requested approval to
travel in the period between [date1 ... date2] and the number of citizens approved and rejected. If given country
argument, the application prints the same information but only for that country. 

    /addVaccinationRecords country
With this request the user has placed in input_dir/country one or more new files to be
edited by a Monitor process. The parent process sends a notification via SIGUSR1 signal to the
Monitor process that manages the country that there are input files to read in
directory. The Monitor process reads whatever new file it finds, updates the data structures it holds in memory and sends
back to the parent process, via named pipe, the updated bloom filters that represent the
group of vaccinated citizens

    /searchVaccinationStatus citizenID
The parent process forwards to all Monitor processes the request through named pipes. The Monitor process
which manages the citizen with a citizenID ID sends through named pipe whatever information
has for the vaccinations that the specific citizen has done/has not done. When the parent receives the
information, he redirects them to stdout.

    /exit
Exits the application. The parent process sends a SIGKILL signal to the Monitors, waiting for them to
terminate, and prints to a file named log_file.pID, the name of all countries that participated in the application with data, the total
the number of applications accepted for entry into the countries, and the total number of applications approved
and rejected. Before it terminates, it obviously releases all of the allocated memory.

There also exists a bash script that creates the input_dir directory that is used as input for the main application. The script can be run as follows:

    ./create_infiles.sh inputFile input_dir numFilesPerDirectory
    
It uses as input the inputFile file created from another bash script "testFile.sh" which can be run as follows:

    ./testFile.sh virusesFile countriesFile numLines duplicatesAllowed 
