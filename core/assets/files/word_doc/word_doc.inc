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
* $Id: word_doc.inc,v 1.5 2004/09/23 22:13:35 amiller Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
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
	* @param string	$file_path	the path of the file we are checking
	*
	* @return boolean
	* @access public
	*/
	function validFile($file_path)
	{
		if (!parent::validFile($file_path)) return false;

		// check that the extension is allowed
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$ext = get_file_type($file_path);
		if (!in_array($ext, $this->allowed_extensions)) {
			$edit_fns = $this->getEditFns();
			trigger_error('Uploaded file is not an MS Word document. Word doc files must be of type doc or dot', E_USER_WARNING);
			return false;
		}
		return true;

	}//end validFile()


	/**
	* Retrieves a list of key data from this asset (used by the indexing service).
	* Subclasses override this to give the content they want to make searchable.
	*
	* @return array				an array of all the keydata for this asset. Keydata is of the form
	* <pre>
	* Array (
	*	Array (
	*		'value'     => ...,
	*		'type'      => ..., (text, date, number, etc.)
	*		'component' => ..., (whatever you want, just as long as this asset
	*							 knows how to deal with it if it needs to)
	*		), ...
	* )
	* </pre>
	* @access public
	*/
	function getKeyData()
	{
		$keydata = parent::getKeyData();

		$file_info = $this->_getExistingFile();
		if (empty($file_info)) return $keydata;
		$file = $file_info['path'];

		$indexer_path = dirname(__FILE__).'/files';
			
		$file = ereg_replace("\"","\\\"",$file);
		$tmp_file = $this->data_path . '/doc_'.time();
		copy($file,$tmp_file);

		$output = '';		
		if (SQ_TOOL_ANTIWORD_ENABLED) {
			// Run a program that converts Word doc to text, set environment variable of bin_path
			$cmd = 'HOME='.$indexer_path.' '.SQ_TOOL_ANTIWORD_PATH.' '.$tmp_file;
			$output =`$cmd`;
		}
		
		unlink($tmp_file);
	
		if(!$output) {
			return $keydata;
		}

		// strip out the tags
		$output = strip_tags($output);
		// strip out the whitespace
		$output = strtolower(trim(preg_replace("/[\s]+/", " ", $output)));

		// index attributes
		$weightings = $this->getWeightings();

		foreach (get_word_counts($output) as $word => $count) {
			$this->buildKeyData($keydata, $word, 'text', 'attr:'.$this->type().':contents', $count * $weightings['contents']['weighting']);
		}

		return $keydata;
	
	}//end getKeyData()


}//end class
?>
