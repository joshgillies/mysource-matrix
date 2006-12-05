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
* $Id: word_doc_edit_fns.inc,v 1.8 2006/12/05 23:14:15 emcdonald Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/file/file_edit_fns.inc';

/**
* Image_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Word_Doc_Edit_Fns extends File_Edit_Fns
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
			?><br/><span style="font-size: 10px">You may only upload files of type <?php echo $this->_getExtensionString($asset->allowed_extensions)?></span><?php
		}
		return TRUE;

	}//end paintFileUpload()


	/**
	* Processes the interface for uploading a file
	*
	* @param object	&$asset	the file asset
	* @param object	&$o		the Backend Outputter object
	* @param string	$prefix	prefix for form elements
	* @param array	$info	info about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	function processFileUpload(&$asset, &$o, $prefix, $info=Array())
	{
		if (!parent::processFileUpload($asset, $o, $prefix, $info)) {
			return FALSE;
		}
		return TRUE;

	}//end processFileUpload()


}//end class
?>
