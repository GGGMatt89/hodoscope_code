void GhDaqManager::reBuilder()
{
  
  std::stringstream source; // Name of the current file to be written on disk
  std::fstream fsource; // File to be written on disk
  
  GhDetectorDataVector * vector;
  
  unsigned long nbTriggers = 0, totalNbTriggers = 0; // Current trigger nb, total trigger nb
  unsigned offset = 0; // Offset incremented for naming each new file
  this->eventnb = 1; // Event written counter for a run 
  
  bool newfiled = false; // Has the first file been created yet?
  bool datafileopened = false; // Has the first file been opened yet?
  
  long bytes_written = 0; // Bytes written in file
  
  // Header/Footer buffers
  uint8_t fileheader[GH_FILE_HEADER_SIZE], filefooter[GH_FILE_FOOTER_SIZE];
  uint8_t eventheader[GH_FILE_EVENT_HEADER_SIZE];
  
  bool shutdown = false, stopping = false, started = false; // Is shutting down, stopping or starting
  
  // Memory mapping for buffering current event to be read from shared memory
  // 	also used to send data to event display via eventserver so contains as well space for main header
  int ghfd;
  uint8_t * ghmap, * ghmap_ptr;
  // Prepare a file large enough to hold one event data and a run header
  std::stringstream smap;
  smap << GH_MAP_FILEPATH << "/" << ".ghmap.dat";
  ghfd = open(smap.str().c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  lseek(ghfd, GH_MAP_FILESIZE+1, SEEK_SET);
  write(ghfd, "", 1);
  lseek(ghfd, 0, SEEK_SET);
  
  // Now the file is ready to be mmapped.
  ghmap = (uint8_t *) mmap(0, GH_MAP_FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, ghfd, 0);
  close(ghfd);
  if (ghmap == MAP_FAILED) {
    //close(this->ghfd);
    die("Error mmapping the ghfile");
  }
  //
  
  // Main loop while run is running
  while(true) {
    
    // Thread management
    pthread_mutex_lock(&this->mut_running);
    if(!this->isRunning) {
      if(started) { stopping = true; LOG->add(DEBUG, "GhDaqManager::reBuilder stopping...");}
      else {
        LOG->add(DEBUG, "GhDaqManager::reBuilder : wait...");
        //if(this->threadToWait) barrier_wait(&this->bar_running); 
	if(this->threadToWait) {
	  pthread_mutex_unlock(&this->mut_running);
	  barrier_wait(&this->bar_running);
	  pthread_mutex_lock(&this->mut_running);
        }
        pthread_cond_wait(&this->cond_running, &this->mut_running);
        LOG->add(DEBUG, "GhDaqManager::reBuilder : ...wakeup");
        started = true;
	// Initialise ghmap with zeros between each run
	memset(ghmap,0,GH_MAP_FILESIZE);
	
      }
    }
    if(this->isShuttingDown) { shutdown = true; }
    pthread_mutex_unlock(&this->mut_running);
    //

    // Is a new file has to be opened ?
    if(!shutdown && (!newfiled || (nbTriggers>0 && (nbTriggers%this->eventfile == 0)))) {
      
      if(DEBUG_PRINTF) printf("NEW FILE %u: %lu triggers / %ld bytes\n", offset, nbTriggers, bytes_written);
      
      // Building new file name
      source.str("");      
      source << this->runb << "-" << offset << ".dat";
      
      this->cfile = source.str();
      
      source.str("");
      source << this->dataPath << "/" << this->cfile;
      
      // FILE RUN HEADER is being written
      
      newfiled = true;

      // FILE HEADER
      //	1-2: F0F0, 3-6: RUN NB, 7-8: TRIGGERS in file
      packi16((unsigned char *)fileheader, GH_FILE_STARTING_WORD);
      packi32(fileheader+2, this->runb);
      packi32(fileheader+8, this->eventfile);

      memcpy(ghmap, fileheader, GH_FILE_HEADER_SIZE);
      bytes_written = GH_FILE_HEADER_SIZE;
      //

      // Init variables relatives to current file
      nbTriggers = 0;
      // 
      offset++;
    }
    
    pthread_mutex_lock(&this->mut_writer);
    if(!shutdown && !stopping && this->ghTriggerStore.empty()) {
      pthread_cond_wait(&this->cond_writer, &this->mut_writer);
    }
    if(this->ghTriggerStore.empty()) {
      usleep(5000);
      pthread_mutex_unlock(&this->mut_writer);
    }
    else {
      vector = this->ghTriggerStore.back();
      pthread_mutex_unlock(&this->mut_writer);
    
      // Loop on the event list
      if(!vector->empty()) {
  
        ghmap_ptr = ghmap + GH_FILE_HEADER_SIZE; // Moving mmap pointer
  
        // Get back trigger data
  
        // EVENT HEADER build header and write it to mmap
        //   1-2:ABCD, 3-6: EVENTNB, 7-10: TRIGGER VALUE, 11-12 TRIGGER NB
        unsigned trignum = vector->size();
        unsigned char * eventheader_ptr = (unsigned char *)eventheader;
        packi16(eventheader_ptr, GH_EVT_DELIMITING_WORD);
        packi32(eventheader_ptr+2, this->eventnb);
        packi32(eventheader_ptr+6, vector->num_trigger);
        packi16(eventheader_ptr+10, trignum);
        //
        memcpy(ghmap_ptr, eventheader, GH_FILE_EVENT_HEADER_SIZE);
        ghmap_ptr += GH_FILE_EVENT_HEADER_SIZE;
        bytes_written+=GH_FILE_EVENT_HEADER_SIZE;
        //
        this->eventnb++; // event counter
        //

        // Event data mmapwrite
        uint32_t datasize = vector->mmapwrite(ghmap_ptr);

        // Event EventServer according to sampling rate
	/*if(totalNbTriggers%GH_EVENTSERVER_SAMPLING_RATE == 0 && this->eventServer->IsRunning()) 
	  this->eventServer->Send((char*) ghmap, GH_FILE_HEADER_SIZE+GH_FILE_EVENT_HEADER_SIZE+datasize);*/
        // ... //
          
        // Create the file (and write header) only when data ok 
        if(!datafileopened) {
          fsource.open (source.str(), std::fstream::out | std::fstream::app | std::fstream::binary);
          fsource.write((char*) fileheader, GH_FILE_HEADER_SIZE);
          datafileopened = true;
        } 
  
        // mmap is written to file
        fsource.write((char*) (ghmap+GH_FILE_HEADER_SIZE), GH_FILE_EVENT_HEADER_SIZE+datasize);
        bytes_written+=datasize;
        fsource.flush();	  
  
        // Event counter
        nbTriggers++;
	totalNbTriggers++;
  
        // Free Event list
        vector->cleanall();

      }
    
      pthread_mutex_lock(&this->mut_writer);
      this->ghTriggerStore.pop_back();
      pthread_mutex_unlock(&this->mut_writer);
    
      delete vector;
      vector = NULL;
    
      // FILE FOOTER
      //   1-2:F1F1, 3-4: EVENTNB in file, 5-8: BYTES written
      // write the file footer in case file is full or acquisition is stopped

      if(shutdown || stopping || nbTriggers==this->eventfile)
      {
        // footer only if there is data
        if(nbTriggers) {
	  packi16(filefooter, GH_FILE_ENDING_WORD);
          packi32(filefooter+2, nbTriggers);
	  bytes_written += GH_FILE_FOOTER_SIZE;
	  packi32(filefooter+6, bytes_written);
          fsource.write((char*)filefooter,GH_FILE_FOOTER_SIZE);
	    
          char msg[100];

          snprintf(msg, 100, "Manager : closing file %d-%d.dat - %ld events / %ld ko", this->runb, offset-1, nbTriggers, bytes_written/1000);
          LOG->add(INFO, msg);  
          fsource.close();
	  datafileopened = false;
        }
        // if not, delete the empty new file before stopping [Should not happen anymore]
        else {
	  fsource.close();
	  datafileopened = false;
	  //::unlink(source.str().c_str());
        }
      }
      this->cnbevt = nbTriggers;
    
    }
    // Status handling on stop or shutdown
    if(shutdown) {
      if(!this->ghTriggerStore.empty()) LOG->add(WARNING, "GhDaqManager::reBuilder : data remain unbuilt...");
      break;
    }
    else if(stopping) { LOG->add(DEBUG, "GhDaqManager::reBuilder : ...stopped"); this->cfile = ""; stopping = false; started = false; newfiled = false; nbTriggers = 0; totalNbTriggers = 0; this->eventnb = 1; offset = 0; bytes_written = 0;}
   
  }
  
  // Freeing mmap and buffers
  if (munmap(ghmap, GH_MAP_FILESIZE) == -1) {
    die("Error un-mmapping the file");
  }
  
  LOG->add(DEBUG, "GhDaqManager::reBuilder : stop for shutdown...");
  //pthread_exit(NULL);
}
