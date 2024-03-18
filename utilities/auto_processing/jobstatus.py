#!/usr/bin/python
# monitor_job.py
from glob import glob
import sys
import time
import commands

def spacepad(content,width=9,right=True):
    stringy = str(content)
    numwidth = len(stringy)
    if right:
        return " "*(width-numwidth)+stringy
    else:
        return stringy+" "*(width-numwidth)

def jobstatus(workdir):
    workdir+="/"
    stagedirs=glob(workdir+"/stage[0-9]")
    nstages=len(stagedirs)
    stagedirs.sort()
    waiting = []
    active = []
    done = []
    failed = []

    for stage in stagedirs:
        waiting.append(len(glob(stage+"/*.job")))
        active.append(len(glob(stage+"/*.lock")))
        done.append(len(glob(stage+"/*.done")))
        failed.append(len(glob(stage+"/*.fail")))

    status="Done"
    if sum(active) > 0:
        status="Active"
    elif sum(waiting) > 0:
        status="Waiting"
    if sum(failed) > 0:
        status+=", but with errors"
    if glob(workdir+"ABORT"):
        status="Aborted"
       
    print time.asctime()
    print "Status = ",status
    print '-'*47
    print spacepad("Stage",6,False),spacepad("Waiting"),spacepad("Active"),\
          spacepad("Done"),spacepad("Failed")
    for stage in range(len(stagedirs)):
        print spacepad(stage+1,6,False),spacepad(waiting[stage]),\
              spacepad(active[stage]), spacepad(done[stage]), \
              spacepad(failed[stage])
    print '-'*47
    print spacepad("Total",6,False),spacepad(sum(waiting)),\
          spacepad(sum(active)),spacepad(sum(done)),spacepad(sum(failed))
    print '-'*47
    
    
    #if sum(done) > 0:
     #   mostrecentdone=commands.getoutput("ls -t1 "+workdir+\
      #                                    "/stage*/*.done | head -n 1")
       # print "Most recent job finished: ",mostrecentdone

    #outsize=commands.getoutput("du -h --max-depth 0 "+workdir+"/output"+\
    #                           " | awk '{print $1}'")
    #print "Total output created: ",outsize
        
    if status == "Aborted":
        print ""
        print "Abort message:"
        print commands.getoutput("cat "+workdir+"/ABORT")
        print ""
        
    
    
if __name__ == "__main__":
    if len(sys.argv) < 2:
        print "Usage: ", sys.argv[0], "<working dir>"
        exit(1)
    jobstatus(sys.argv[1])

