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
* $Id: pdf_file.inc,v 1.24 2006/07/21 06:39:47 skim Exp $
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
class PDF_File extends File
{

	/**
	* All allowed extensions for an image
	* @var Array(string)
	*/
	var $allowed_extensions = Array('pdf');


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function PDF_File($assetid=0)
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
			trigger_localised_error('CORE0105', E_USER_WARNING);
			return FALSE;
		}
		return TRUE;

	}//end validFile()


	/**
	* Returns the raw content of this pdf file
	*
	* @return string
	* @access public
	*/
	function getContent()
	{
		$file_info = $this->getExistingFile();
		if (empty($file_info)) return '';
		$file = $file_info['path'];

		$indexer_path = dirname(__FILE__).'/files';

		$content = '';

		if (SQ_TOOL_PDFTOHTML_ENABLED) {
			$pwd = $this->attr('password');
			$pwd = (empty($pwd)) ? '' : $pwd = ' -opw '.$pwd.' ';

			// Run a program that converts a PDF to HTML, set environment variable of bin_path
			$cmd = SQ_TOOL_PDFTOHTML_PATH." -i -nomerge -noframes -stdout $pwd $file";
			$cmd = escapeshellcmd($cmd);
			$content = shell_exec($cmd);
		}

		$file = ereg_replace('\"','\\\"',$file);

		if (!$content) return '';
		// strip out the tags
		$content = trim(strip_tags($content));

		// in some cases file path ends up as the first string in the file
		// this is the way pdftohtml treats files without the title, it puts filename into html title

		if (($this->status & SQ_SC_STATUS_SAFE_EDITING) && (is_a($GLOBALS['SQ_SYSTEM']->user, 'public_user'))) {
			// strip file versioning suffix from the filename, e.g. 'file.pdf,ffv5' becomes 'file.pdf'
			$file = preg_replace('/,ffv\d+$/', '', $file);
		}
		// if file extension is .pdf, the .pdf part is omitted by pdftohtml from the first string (strange)
		// here we prepare the file name for that case
		if (substr($file, -4) == '.pdf') {
			$file = substr($file, 0, strlen($file)-4);
		}

		// if we suspect that the first string is the file path, remove it
		// NOTE: this will only work on *nix type filesystems. This is
		if ($content{0} == '/') {
			$content = substr($content, strlen($file));
		}

		// strip out the unnecessary whitespace and html entities like &amp;
		$search = Array('/[\s]+/', '/\&[^;\s]+;/');
		$replace = Array(' ', '');

		$content = strtolower(trim(preg_replace($search, $replace, $content)));

		return $content;

	}//end getContent()


}//end class
?>
