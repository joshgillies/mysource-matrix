#!/bin/sh
#/**
#* Copyright (c) 2003 - Squiz Pty Ltd
#*
#* $Id: check_refs.sh,v 1.7 2003/09/26 05:26:24 brobertson Exp $
#* $Name: not supported by cvs2svn $
#*/

# A simple grep to check to see that all 'get...' calls to 
# the system object are being called by reference


grep -rsn "[^&]\$[^(->)]*->getAsset(" *
grep -rsn "[^&]\$[^(->)]*->getSystemAsset(" *
grep -rsn "[^&]\$GLOBALS\[.\?SQ_SYSTEM.\?\]->get" *
grep -rsn "[^&]\$GLOBALS\[.\?SQ_SYSTEM.\?\]->db;" *
grep -rsn "[^&]\$GLOBALS\[.\?SQ_SYSTEM.\?\]->am;" *

