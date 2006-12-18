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
* $Id: word_doc.inc,v 1.19 2006/12/18 05:18:02 arailean Exp $
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
* @version $Revision: 1.19 $
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
