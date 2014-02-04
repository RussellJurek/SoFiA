#! /usr/bin/env python
import re
import sys
import traceback

def eprint():
    print sys.exc_info()[1]
    print traceback.print_tb(sys.exc_info()[2])
#
def is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        return False

def readPipelineOptions(filename="pipeline.options"):
    f=open(filename,'r')
    lines=f.readlines()
    f.close()
    #whiteSpaceRemover = re.compile(r'\s+')
    #lines=[whiteSpaceRemover.sub('', line) for line in lines]
    
    lines=[line.strip(" \t\r\n") for line in lines]
    lines=[line for line in lines if len(line)>0]
    lines2=[]
    for l in lines:
        if len(lines2)>0:
            if lines2[-1][-1]=="\\":
                lines2[-1]=lines2[-1][:-1]+l
                continue
        lines2.append(l)
    lines=lines2
    tasks={}
    for linenr,line in enumerate(lines):
        if line[0]=="#": continue
        try:
            if line.count('=')!=1:
                print "no or too many = present in line %02d: %s"%(linenr+1,line)
                continue
            remainingkeys,value=tuple(line.split("="))
            if len(remainingkeys)<1:
                print "option name missing in      line %02d: %s"%(linenr+1,line)
                continue
            #if len(value)<1:
                #print "value missing in            line %02d: %s"%(linenr+1,line)
                #continue
            subtasks=tasks
            while True:
                keyname=remainingkeys.split(".")[0]
                keyname=keyname.strip(" \t\r\n")
                if len(keyname)<1:
                    print "(sub)key name to short in   line %02d: %s"%(linenr+1,line)
                    break
                
                remainingkeys=str(".").join(remainingkeys.split(".")[1:])
                remainingkeys=remainingkeys.strip(" \t\r\n")
                if not subtasks.has_key(keyname):
                    subtasks[keyname]={}
                subtasks=subtasks[keyname]
                if remainingkeys.count(".")>0:
                    pass
                else:
                    if subtasks.has_key(remainingkeys):
                        print "option already present in   line %02d: %s"%(linenr+1,line)
                        break
                    try:
		        value = value.split('#')[0].strip(" \t\r\n")
		        if len(value)>0 and (is_number(value) or value[0] == '['):
			  tmpval=eval(value,{},{})
		        else:
			  if value in ['True','true','TRUE','Yes','yes','YES']:
			    tmpval = True
			  else:
			    if value in ['False','false','FALSE','No','no','NO']:
			      tmpval = False
			    else: tmpval=eval('\''+value+'\'',{},{})
                    except:
                        print "could not parse option in line %02d: %s"%(linenr+1,line)
                        break
                    subtasks[remainingkeys]=tmpval
                    break
        except:
            #eprint()
            pass
            #print line
    return tasks
#

#tasks=readPipelineOptions()
#print tasks




