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
* $Id: word_doc.inc,v 1.18 2006/12/05 23:14:15 emcdonald Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/file/file.inc';
require_once SQ_DATA_PATH.'/private/conf/tools.inc';


/**
* Image
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.18 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Word_Doc extends File
{

	/**
	* All allowed extensions for a word doc (document and template recognised)
	* @var Array(string)
	*/
	var $allowed_extensions = Array('doc', 'dot');


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Word_Doc($assetid=0)
	{
		$this->File($assetid);

	}//end constructor


	/**
	* Checks a file on the file system to see if it can be used as an image
	*
	* @param array	$info	an array of information about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	function validFile($info)
	{
		if (!parent::validFile($info)) return FALSE;

		// check that the extension is allowed
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$ext = get_file_type($info['filename']);
		if (!in_array($ext, $this->allowed_extensions)) {
			trigger_localised_error('CORE0106', E_USER_WARNING);
			return FALSE;
		}
		return TRUE;

	}//end validFile()


	/**
	* Returns the raw content of this word document
	*
	* @return string
	* @access public
	*/
	function getContent()
	{
		$file_info = $this->getExistingFile();
		if (empty($file_info)) return '';
		$file = $file_info['path'];
		$file = ereg_replace('\"', '\\\"', $file);

		$content = '';
		if (SQ_TOOL_ANTIWORD_ENABLED) {
			// Run a program that converts Word doc to text, set environment variable of bin_path
			$cmd = 'ANTIWORDHOME='.SQ_TOOL_ANTIWORD_MAPPING_PATH.' '.SQ_TOOL_ANTIWORD_PATH.' '.$file;
			$content = shell_exec($cmd);

		}

		if (!$content) return '';

		// strip out the whitespace
		$content = strtolower(trim(preg_replace('/[\s]+/', ' ', $content)));

		return $content;

	}//end getContent()


}//end class
?>
