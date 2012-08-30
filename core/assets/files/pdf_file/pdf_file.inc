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
* $Id: pdf_file.inc,v 1.34 2012/08/30 01:09:07 ewang Exp $
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
* @version $Revision: 1.34 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class PDF_File extends File
{


	/**
	* All allowed extensions for an image
	* @var Array(string)
	*/
	public $allowed_extensions = Array('pdf');


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
			// Run the command
			exec($cmd, $content, $return);

			// check to see if the command was executed correctly
			// return value is 0, all good or else log a warning
			if ($return != '0') {
				trigger_error('Error occured while getting contents for pdf File Asset #'.$this->id, E_USER_WARNING);
				return '';
			}
		}

		$file = str_replace('"','\\\"',$file);
		if (empty($content)) return '';

		// the content we are spat out by exec() is in array form
		// convert it to string so that we can process it further
		$content = implode(PHP_EOL, $content);

		// strip out the tags
		$content = trim(strip_tags($content));
		if (empty($content)) return '';

		// in some cases file path ends up as the first string in the file
		// this is the way pdftohtml treats files without the title, it puts filename into html title
		if (($this->status & SQ_SC_STATUS_SAFE_EDITING) && ($GLOBALS['SQ_SYSTEM']->user instanceof Public_User)) {
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
		if (isset($content[0]) && $content[0] == '/') {
			$content = substr($content, strlen($file));
		}

		// strip out the unnecessary whitespace and html entities like &amp;
		$search = Array('/[\s,]+/', '/\&[^;\s]+;/');
		$replace = Array(' ', '');

		$content = strtolower(trim(preg_replace($search, $replace, $content)));

		return $content;

	}//end getContent()


}//end class
?>
