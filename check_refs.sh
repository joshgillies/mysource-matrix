#!/bin/sh
# A simple grep to check to see that all 'get...' calls to 
# the system object are being called by reference


grep -rsn "[^&]\$[^(->)]*->getAsset(" *
grep -rsn "[^&]\$[^(->)]*->getSystemAsset(" *
grep -rsn "[^&]\$GLOBALS\[.\?SQ_SYSTEM.\?\]->get" *
grep -rsn "[^&]\$GLOBALS\[.\?SQ_SYSTEM.\?\]->db;" *
grep -rsn "[^&]\$GLOBALS\[.\?SQ_SYSTEM.\?\]->am;" *

