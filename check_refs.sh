#!/bin/sh
#/**
#* +--------------------------------------------------------------------+
#* | Squiz.net Open Source Licence                                      |
#* +--------------------------------------------------------------------+
#* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
#* +--------------------------------------------------------------------+
#* | This source file may be used subject to, and only in accordance    |
#* | with, the Squiz Open Source Licence Agreement found at             |
#* | http://www.squiz.net/licence.                                      |
#* | Make sure you have read and accept the terms of that licence,      |
#* | including its limitations of liability and disclaimers, before     |
#* | using this software in any way. Your use of this software is       |
#* | deemed to constitute agreement to be bound by that licence. If you |
#* | modify, adapt or enhance this software, you agree to assign your   |
#* | intellectual property rights in the modification, adaptation and   |
#* | enhancement to Squiz Pty Ltd for use and distribution under that   |
#* | licence.                                                           |
#* +--------------------------------------------------------------------+
#*
#* $Id: check_refs.sh,v 1.9.10.1 2005/07/27 13:09:10 brobertson Exp $
#*
#*/

# A simple grep to check to see that all 'get...' calls to 
# the system object are being called by reference


grep -rsn "[^&]\$[^(->)]*->getAsset(" *
grep -rsn "[^&]\$[^(->)]*->getSystemAsset(" *
grep -rsn "[^&]\$GLOBALS\[.\?SQ_SYSTEM.\?\]->get" *
grep -rsn "[^&]\$GLOBALS\[.\?SQ_SYSTEM.\?\]->db;" *
grep -rsn "[^&]\$GLOBALS\[.\?SQ_SYSTEM.\?\]->am;" *

