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
* $Id: text_file_edit_fns.inc,v 1.3 2006/01/18 00:59:32 lwright Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/file/file_edit_fns.inc';

/**
* Text_File_Edit_Fns
*
* @author  Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Text_File_Edit_Fns extends File_Edit_Fns
{


	/**
	* Paint the interface for editing the parse file
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintEditFile(&$asset, &$o, $prefix)
	{
		if (SQ_ROLLBACK_VIEW) {
			// get an older version of the parse file
			$fv =& $GLOBALS['SQ_SYSTEM']->getFileVersioning();
			$rep_file = $asset->data_path_suffix.'/'.$asset->name;
			$then = iso8601_ts($_SESSION['sq_rollback_view']['rollback_time']);
			$info = @$fv->_checkOutCheck($rep_file, NULL, $then);
			$parse_file = $info['source_file'];
			ob_start();
			$success = $fv->output($rep_file, NULL, $then);
			$str = ob_get_clean();
			if (!$success) return FALSE;
		} else {
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			$parse_file = $asset->data_path.'/'.$asset->name;
			$str = file_to_string($parse_file);
		}

		if ($asset->writeAccess('attributes')) {
			text_area($prefix.'_new_file', $str, 0, 0, 0, 'style="font-family: monospace; white-space: pre; width: 99%; height: 480px"');
			return TRUE;
		} else {
			echo '<pre>', htmlspecialchars($str), '</pre>';
		}
		return FALSE;

	}//end paintEditFile()


	/**
	* Processes the interface for the editing the parse file
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processEditFile(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if (isset($_POST[$prefix.'_new_file'])) {

			// make them unix format ;)
			$str = str_replace("\r\n", "\n", $_POST[$prefix.'_new_file']);

			require_once SQ_FUDGE_PATH.'/general/file_system.inc';

			// it's easier if we just use the processFileUpload() (future proof's this a little bit)
			// so create a temporary file that will be moved to the proper file
			$file_info = Array(
							'name'				=> $asset->name,
							'tmp_name'			=> $asset->data_path.'/'.$asset->name.'.new',
							'non_uploaded_file'	=> TRUE,
						 );

			 if (!string_to_file($str, $file_info['tmp_name'])) {
				 return FALSE;
			 }

			$ret_val = $this->processFileUpload($asset, $o, $prefix, $file_info);
			unlink($file_info['tmp_name']);
			return $ret_val;

		}//end if

		return FALSE;

	}//end processEditFile()


}//end class
?>
