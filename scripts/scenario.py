#!/usr/bin/env python3

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'



from distutils import bcppcompiler
import time
import rospy
from std_msgs.msg import String

from ontologenius import *
# from OntologyManipulator import *

_onto = None
_debug = True
_debug_check = True

def getContextOfAgent(agent: str):
    ctx = _onto.individuals.getOn(agent, "agentCurrentContext")
    if(len(ctx)>0):
        if(_debug):
            print(bcolors.OKGREEN+agent+" is in the context of : " + ctx[0]+bcolors.ENDC)
        socialctx = _onto.individuals.getOn(ctx[0],"globalContextIsComposeOfSocialContext")
        if(len(socialctx) != 0):
            roles_dispo_ = _onto.individuals.getOn(socialctx[0],"socialContextHasRole")
            # print(roles_dispo_)
            agent_roles = _onto.individuals.getOn(agent,"agentPlayRole")
            for agent_role in agent_roles:
                for role in roles_dispo_:
                    if(agent_role == role):
                        if(_debug):
                            print(bcolors.OKCYAN+agent+' has the role of a '+agent_role+bcolors.ENDC)
        ctx_meaning = _onto.individuals.getOn(ctx[0],"contextUseContextMeaning")
        if(len(ctx_meaning)!=0):
            for meaning in ctx_meaning:
                # print("meanning : "+meaning)
                objects  =  _onto.individuals.getOn(meaning,"contextMeaningHasObject")
                if(len(objects)!=0):
                    for obj in objects:
                        symbol = _onto.individuals.getOn(meaning,"contextMeaningSymboliseObject")
                        if(_debug):
                            print(bcolors.OKCYAN+"meaning of the object : "+obj+" in this context is : "+symbol[0]+bcolors.ENDC)
        else:
            if(_debug):
                print(bcolors.WARNING+"the pen has not meaning in this context"+bcolors.ENDC)
        return ctx[0]
    else:
        if(_debug):
            print(bcolors.WARNING+"This agent : "+agent+" didn't have context"+bcolors.ENDC)
        return None

    

def stopTheatreAndStartTeach(agent:str):
    if(_debug):
        print(bcolors.HEADER+agent +" stop play theater and start to teach"+bcolors.ENDC)
    _onto.feeder.removeObjectProperty(agent,'agentIsUseInActivity','playTheatre')
    _onto.feeder.addObjectProperty(agent,'agentIsUseInActivity','teach')
    _onto.feeder.waitUpdate()

def stopTeachAndStartTheatre(agent:str):
    if(_debug):
        print(bcolors.HEADER+agent +" stop teach and start play theater"+bcolors.ENDC)
    _onto.feeder.addObjectProperty(agent,'agentIsUseInActivity','playTheatre')
    _onto.feeder.removeObjectProperty(agent,'agentIsUseInActivity','teach')
    _onto.feeder.waitUpdate()
def outOfScene(agent:str):
    if(_debug):
        print(bcolors.HEADER+agent +" go out of scene"+bcolors.ENDC)
    _onto.feeder.removeObjectProperty(agent,'isIn','scene')
    _onto.feeder.waitUpdate()

def onScene(agent:str):
    if(_debug):
        print(bcolors.HEADER+agent +" is on scene"+bcolors.ENDC)
    _onto.feeder.addObjectProperty(agent,'isIn','scene')
    _onto.feeder.waitUpdate()
def stopTheatre(agent):
    if(_debug):
        print(bcolors.HEADER+agent +" stop play theater"+bcolors.ENDC)
    _onto.feeder.removeObjectProperty(agent,'agentIsUseInActivity','playTheatre')
    _onto.feeder.waitUpdate()
def stopTeach(agent):
    if(_debug):
        print(bcolors.HEADER+agent +" stop play theater"+bcolors.ENDC)
    _onto.feeder.removeObjectProperty(agent,'agentIsUseInActivity','teach')
    _onto.feeder.waitUpdate()  
def startTheatre(agent):
    if(_debug):
        print(bcolors.HEADER+agent +" start play theater"+bcolors.ENDC)
    _onto.feeder.addObjectProperty(agent,'agentIsUseInActivity','playTheatre')
    _onto.feeder.waitUpdate()
def checkCtx(agent,ctx_wanted):
    ctx = getContextOfAgent(agent)
    res = ctx==ctx_wanted
    if(_debug_check):
        if(res):
            print(bcolors.OKGREEN+"Valid"+bcolors.ENDC)
        else:
            print(bcolors.FAIL+"Fail"+bcolors.ENDC)
    return res
def checkNoCtx(agent,list_ctx):
    global _debug_check 
    result = False
    _debug_check_temp = _debug_check
    _debug_check = False
    for ctx in list_ctx:
        result = result or checkCtx(agent,ctx)
    _debug_check = _debug_check_temp
    if(_debug_check):
        if(not result):
            print(bcolors.OKGREEN+"Valid"+bcolors.ENDC)
        else:
            print(bcolors.FAIL+"Fail"+bcolors.ENDC)
    return result





if __name__ == "__main__":

    rospy.init_node('contextScenario')

    _onto = OntologyManipulator()
    print(_onto.reasoners.list())
    _onto.reasoners.deactivate("ReasonerContext")
    _onto.close()
    _onto.feeder.waitConnected()
    time.sleep(1)
    print(bcolors.HEADER+"------------- INIT state without Reasoner Context --------------"+bcolors.ENDC)
    list_agent_ = _onto.individuals.getType("Agent")
    for l in list_agent_:
        print('agent : '+l)
        # print(_onto.individuals.getRelatedWith(l))
        getContextOfAgent(l)
    print("-----------------------------------")
    print()    
    print()
    print()
    _onto.reasoners.activate("ReasonerContext")
    print(bcolors.HEADER+"------------- INIT state with Reasoner Context --------------"+bcolors.ENDC)
    print("[Goal] : check creation of context on pr2")
    list_agent_ = _onto.individuals.getType("Agent")
    for l in list_agent_:
        print('agent : '+l)
        # print(_onto.individuals.getRelatedWith(l))
        getContextOfAgent(l)
    print("-----------------------------------")
    print()    
    print()
    print()

    print("[Goal] : check switch context of bob on activity swap")
    stopTheatreAndStartTeach('bob')
    # ctx = getContextOfAgent('bob')
    checkCtx("bob","classroomGlobalContext")

    # print(_onto.individuals.getRelatedWith("bob"))
    print("-----------------------------------")
    print("[Goal] : check keep context of bob on location change")
    outOfScene('bob')
    checkCtx("bob","classroomGlobalContext")
    print("-----------------------------------")
    print("[Goal] : check keep context of bob on more precise location but action don't match")
    onScene('bob')
    checkCtx("bob","classroomGlobalContext")
    print("-----------------------------------")
    print("[Goal] : check switch of context on activty swap on not empty context")
    # print(_onto.individuals.getRelatedWith("bob"))
    stopTeachAndStartTheatre("bob")
    checkCtx("bob","theatreGlobalContext")
    print("-----------------------------------")
    print("[Goal] : check keep context without action")
    stopTheatre("bob")
    checkCtx("bob","theatreGlobalContext")
    print("-----------------------------------")
    print("[Goal] : check swap context on location change")
    outOfScene('bob')
    checkCtx("bob","classroomGlobalContext")
    print("-----------------------------------")
    print("[Goal] : check finished of empty context")
    outOfScene('alice')
    stopTheatre('alice')
    outOfScene('bob')
    getContextOfAgent('bob')
    getContextOfAgent('alice')
    print("-----------------------------------")
    print("[Goal] : check not link with finish context")
    onScene('bob')
    # getContextOfAgent("bob")
    checkCtx('bob',"classroomGlobalContext")
    print("-----------------------------------")
    # input()
    print("[Goal] : create new ctx with activity")
    startTheatre('bob')
    # getContextOfAgent("bob")
    if(_debug):
        getContextOfAgent("bob")
    checkNoCtx('bob',["theatreGlobalContext","classroomGlobalContext"])

    print("-----------------------------------")


    rospy.spin()
