#!/usr/bin/python

import threading
import time

import dfconvert

exitFlag = 0

class dfconvertThread (threading.Thread):
    def __init__(self, threadID, name, input, abstract):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.input = input
        self.abstract = abstract
    def run(self):
        print "Starting " + self.name
        if exitFlag:
            thread.exit()
        status = dfconvert.get(self.input, self.abstract);
        print "%s: %s status:%s" % (self.name, time.ctime(time.time()), status)
        print "Exiting " + self.name

# Create new threads
thread1 = dfconvertThread(1, "Thread-1", "input.docx", "output1.html")
thread2 = dfconvertThread(2, "Thread-2", "input.docx", "output2.html")
thread3 = dfconvertThread(3, "Thread-3", "input.docx", "output3.html")
thread4 = dfconvertThread(4, "Thread-4", "input.docx", "output4.html")
thread5 = dfconvertThread(5, "Thread-5", "input.docx", "output5.html")

# Start new Threads
thread1.start()
thread2.start()
thread3.start()
thread4.start()
thread5.start()

print "Exiting Main Thread"