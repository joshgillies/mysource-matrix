<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: accept_file_upload.php,v 1.1 2004/11/24 05:01:29 tbarrett Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Script to accept HTTP uploads and save them to the temp dir
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/

if ((!is_array($_FILES)) || empty($_FILES)) exit();

// get init.inc to do all the hard work with access control etc
require_once dirname(dirname(dirname(__FILE__))).'/include/init.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

// copy all uploaded files, including arrays of files, to the temp dir
foreach ($_FILES as $id => $details) {
	if (is_array($details['name'])) {
		foreach ($_FILES[$id]['name'] as $index => $name) {
			if (!empty($name)) {
				$file_name = SQ_TEMP_PATH.'/'.strtolower(str_replace(' ', '_', $name));
				while (file_exists($file_name)) $file_name = increment_filename($file_name);
				if (move_uploaded_file($_FILES[$id]['tmp_name'][$index], $file_name)) { 
					print('OK '.basename($file_name)."\n");
				} else { 
					print('ERROR '.basename($file_name)."\n"); 
				}
			}
		}
	} else {
		if (!empty($details['name'])) {
			$file_name = SQ_TEMP_PATH.'/'.strtolower(str_replace(' ', '_', $details['name']));
			while (file_exists($file_name)) $file_name = increment_filename($file_name);
			if (move_uploaded_file($_FILES[$id]['tmp_name'], $file_name)) { 
				print('OK '.basename($file_name)."\n");
			} else { 
				print('ERROR '.basename($file_name)."\n"); 
			}
		}
	}
}

?>