######################################################################################
#This Makefile is the main entrance for MediaTek CPE SDK maintaining,including:
#firmware building, SDK release, customer SDK release and other test functions for SDK.
#######################################################################################
#######################################################################################
#In 3 cases you need to pay attention to this makefile:
#(1).You have added or modified private source code created by MediaTeK which can not be released to customers directly but can be provided in object format.
#(2).You will be responsbile for new version SDK release, which means you need to update or upgrade this makefile.
#(3).You need to add special target for SDK maintaining or etc.
########################################################################################

#Entrance for firmware building, which will provide default target if no arguments specified.

ifeq ($(strip $(RELEASEBSP) $(CUSTOMERRELEASE)),)
include Project/MakeFile_Main_All
else
include Project/MakeFile_Release_All
endif
