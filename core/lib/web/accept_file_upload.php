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
* $Id: accept_file_upload.php,v 1.7 2005/04/15 00:37:32 lwright Exp $
*
*/

/**
* Script to accept HTTP uploads and save them to the temp dir
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix
*/

if ((!is_array($_FILES)) || empty($_FILES)) exit();

// get init.inc to do all the hard work figuring out who's logged in etc
require_once dirname(dirname(dirname(__FILE__))).'/include/init.inc';

// kick out the wrong types of people
if (empty($GLOBALS['SQ_SYSTEM']->user) || !is_a($GLOBALS['SQ_SYSTEM']->user, 'backend_user')) {
	trigger_localised_error('SYS0028', E_USER_ERROR);
	exit();
}

// copy all uploaded files, including arrays of files, to the temp dir
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
$ms = &$GLOBALS['SQ_SYSTEM']->getMessagingService();
$ms->openLog();
foreach ($_FILES as $id => $details) {
	if (is_array($details['name'])) {
		foreach ($_FILES[$id]['name'] as $index => $name) {
			if (!empty($name)) {
				$file_name = strtolower(str_replace(' ', '_', $name));
				while (file_exists(SQ_TEMP_PATH.'/'.$file_name)) $file_name = increment_filename($file_name);
				if (move_uploaded_file($_FILES[$id]['tmp_name'][$index], SQ_TEMP_PATH.'/'.$file_name)) {
					print("\nOK ".$file_name."\n");
					$message_body = 'The file '.$_FILES[$id]['name'][$index].' was uploaded to the temp dir';
					if ($file_name != $_FILES[$id]['name'][$index]) $message_body .= ' and renamed to '.$file_name;
					$message = $ms->newMessage(Array(), 'File Uploaded to temp dir', $message_body);
					$ms->logMessage($message);
				} else {
					trigger_localised_error('SYS0009', E_USER_WARNING);
				}
			}
		}
	} else {
		if (!empty($details['name'])) {
			$file_name = strtolower(str_replace(' ', '_', $details['name']));
			while (file_exists(SQ_TEMP_PATH.'/'.$file_name)) $file_name = increment_filename($file_name);
			if (move_uploaded_file($_FILES[$id]['tmp_name'], SQ_TEMP_PATH.'/'.$file_name)) {
				print("\nOK ".$file_name."\n");
				$message_body = 'The file '.$_FILES[$id]['name'].' was uploaded to the temp dir';
				if ($file_name != $_FILES[$id]['name']) $message_body .= ' and renamed to '.$file_name;
				$message = $ms->newMessage(Array(), 'File Uploaded to temp dir', $message_body);
				$ms->logMessage($message);
			} else {
				trigger_localised_error('SYS0009', E_USER_WARNING);
			}
		}
	}
}
$ms->closeLog();

?>