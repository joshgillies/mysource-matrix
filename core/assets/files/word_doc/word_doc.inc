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
* $Id: word_doc.inc,v 1.17 2006/04/04 04:06:07 skim Exp $
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
* @version $Revision: 1.17 $
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
