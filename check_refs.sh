#!/bin/sh
# A simple grep to check to see that all 'get...' calls to 
# the system object are being called by reference


grep -rsn "[^&]\$GLOBALS\[.\?SQ_RESOLVE.\?\]->get" *
grep -rsn "[^&]\$GLOBALS\[.\?SQ_RESOLVE.\?\]->am->getAsset(" *

