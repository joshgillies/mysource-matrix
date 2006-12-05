<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: pdf_file_edit_fns.inc,v 1.12 2006/12/05 23:14:14 emcdonald Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/file/file_edit_fns.inc';

/**
* PDF_File_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class PDF_File_Edit_Fns extends File_Edit_Fns
{


	/**
	* Get a formatted string of extensions (eg 1,2,3 and 4)
	*
	* @param array	$extensions	array of extensions to format
	* @param string	$glue		word used to join last extension to the string (and, or etc)
	*
	* @return string
	* @access public
	*/
	function _getExtensionString($extensions, $glue='and')
	{
		if (empty($extensions) || !is_array($extensions)) {
			return '';
		}

		$count = count($extensions);
		$retval = '';

		for ($i = 0; $i < $count; $i++) {
			$ext = $extensions[$i];
			if ($i != 0 && ($i+1) == $count) {
				// the last in the list
				$retval .= " $glue ";
			} else if ($i != 0) {
				$retval .= ', ';
			}
			$retval .= $ext;
		}

		return $retval;

	}//end _getExtensionString()


	/**
	* Prints the interface for uploading a file
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintFileUpload(&$asset, &$o, $prefix)
	{
		parent::paintFileUpload($asset, $o, $prefix);
		if ($asset->writeAccess('attributes')) {
			echo '<br />';
			$o->note(translate('core_only_upload_files_of_type', $this->_getExtensionString($asset->allowed_extensions)));
		}
		return TRUE;

	}//end paintFileUpload()


}//end class

?>
