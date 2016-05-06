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
* $Id: xsl_file_edit_fns.inc,v 1.3 2012/08/30 01:09:07 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/file/file_edit_fns.inc';

/**
* XSL_File_Edit_Fns
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Xsl_File_Edit_Fns extends File_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint the interface for editing the parse file
	*
	* @param object				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintEditFile($asset, Backend_Outputter $o, $prefix)
	{
		if (SQ_ROLLBACK_VIEW) {
			// get an older version of the parse file
			$fv = $GLOBALS['SQ_SYSTEM']->getFileVersioning();
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

		// enable the Ace editor to edit backend code (if available)
		if($o->isViperAceAvailable()) {
			$o->enableViperAceEditor();
			$read_only = '';
			if(!$asset->writeAccess('attributes')) $read_only = 'sq-viper-ace-editor-textarea-readonly';
			text_area($prefix.'_new_file', $str, 0, 0, 0, 'style="font-family: \'Courier New\', Courier, monospace; white-space: pre-wrap; width: 99%; height: 480px" class="sq-viper-ace-editor-textarea '.$read_only.' sq-viper-ace-editor-textarea-mode-xml sq-viper-ace-editor-textarea-large"');
			return TRUE;
		}
		
		if ($asset->writeAccess('attributes')) {
			text_area($prefix.'_new_file', $str, 0, 0, 0, 'style="font-family: \'Courier New\', Courier, monospace; white-space: pre-wrap; width: 99%; height: 480px"');
			return TRUE;
		} else {
			echo '<pre>', htmlspecialchars($str), '</pre>';
		}
		return FALSE;

	}//end paintEditFile()


	/**
	* Processes the interface for the editing the parse file
	*
	* @param object				$asset	the asset whose interface we are painting
	* @param Backend_Outputter	$o		the Backend Outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processEditFile($asset, Backend_Outputter $o, $prefix)
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
