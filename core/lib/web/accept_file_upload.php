<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: accept_file_upload.php,v 1.12 2012/08/30 01:09:21 ewang Exp $
*
*/

/**
* Script to accept HTTP uploads and save them to the temp dir
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix
*/

if ((!is_array($_FILES)) || empty($_FILES)) exit();

// get init.inc to do all the hard work figuring out who's logged in etc
require_once dirname(dirname(dirname(__FILE__))).'/include/init.inc';

// kick out the wrong types of people
if (empty($GLOBALS['SQ_SYSTEM']->user) || !$GLOBALS['SQ_SYSTEM']->user->canAccessBackend()) {
	trigger_localised_error('SYS0028', translate('You must be logged in as a backend user to upload files'), E_USER_ERROR)
	exit();
}

// copy all uploaded files, including arrays of files, to the temp dir
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
$ms->openLog();
foreach ($_FILES as $id => $details) {
	if (is_array($details['name'])) {
		foreach ($_FILES[$id]['name'] as $index => $name) {
			if (!empty($name)) {
				$file_name = strtolower(str_replace(' ', '_', $name));
				while (file_exists(SQ_TEMP_PATH.'/'.$file_name)) $file_name = increment_filename($file_name);
				if (move_uploaded_file($_FILES[$id]['tmp_name'][$index], SQ_TEMP_PATH.'/'.$file_name)) {
					print("\nOK ".$file_name."\n");
				} else {
					trigger_localised_error('SYS0009', translate('Error uploading file %s'), E_USER_WARNING)
				}
			}
		}
	} else {
		if (!empty($details['name'])) {
			$file_name = strtolower(str_replace(' ', '_', $details['name']));
			while (file_exists(SQ_TEMP_PATH.'/'.$file_name)) $file_name = increment_filename($file_name);
			if (move_uploaded_file($_FILES[$id]['tmp_name'], SQ_TEMP_PATH.'/'.$file_name)) {
				print("\nOK ".$file_name."\n");
			} else {
				trigger_localised_error('SYS0009', translate('Error uploading file %s'), E_USER_WARNING)
			}
		}
	}
}
$ms->closeLog();

?>
