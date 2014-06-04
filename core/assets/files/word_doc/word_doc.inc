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
* $Id: word_doc.inc,v 1.24 2012/08/30 01:09:07 ewang Exp $
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
* @version $Revision: 1.24 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Word_Doc extends File
{


	/**
	* All allowed extensions for a word doc
	*
	* Document and templates recognised, for both earlier versions of Word, and
	* Office 2007+ OOXML format (with macros both disabled and enabled).
	*
	* @var Array(string)
	*/
	public $allowed_extensions = Array('doc', 'dot', 'docx', 'docm', 'dotx', 'dotm');


	/**
	* Constructor
	*
	* @param int    $assetid    the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

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
		
		$content = '';
		
		// Can't use Antiword on Word 2007+ files...
		$file_ext = substr(basename($file), strrpos($file, '.') + 1);
		if ($file_ext === 'doc' || $file_ext === 'dot') {
            $file = str_replace('"', '\\\"', $file);
    
            if (SQ_TOOL_ANTIWORD_ENABLED) {
                // Run a program that converts Word doc to text, set environment variable of bin_path
                $cmd = 'ANTIWORDHOME='.SQ_TOOL_ANTIWORD_MAPPING_PATH.' '.SQ_TOOL_ANTIWORD_PATH.' '.$file;
                $content = shell_exec($cmd);
    
            }
    
            if (!$content) return '';
    
            // strip out the whitespace
            $content = strtolower(trim(preg_replace('/[\s]+/', ' ', $content)));
        }//end if

		return $content;

	}//end getContent()


}//end class
?>
