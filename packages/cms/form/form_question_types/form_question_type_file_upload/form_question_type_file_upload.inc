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
* $Id: form_question_type_file_upload.inc,v 1.1 2004/11/09 04:16:17 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../form_question/form_question.inc';

/**
* Form_Question_Type_File_Upload
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Question_Type_File_Upload extends Form_Question
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	* @param array	$data		the section's entry for this question (contains all question data)
	*
	*/
	function Form_Question_Type_File_Upload($assetid=0, $data=Array())
	{
		$this->Form_Question($assetid, $data);

	}//end Form_Question_Type_File_Upload()

	
	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array();

	}//end _getAllowedLinks()


	/**
	* generates the html for this question to display it in the frontend
	*
	* @param boolean	$content_file	if TRUE, this function will generate content for the content file
	* @param string		$value			current value of the question
	*
	* @access public
	* @return string
	*/
	function getAnswer($content_file=true, $value='')
	{
		$extras = $this->attr('extra');
		$name   = $this->id;

		ob_start();
		file_upload($name, $extras);
		$html = ob_get_contents();
		ob_end_clean();

		// calling text_box will convert the open and close php tags into their html counter-parts
		// so we need to convert them back to proper php tags

		return html_entity_decode($html);

	}//end getAnswer()


	/**
	* process this question
	*
	* @param &object $submission the current submission object
	*
	* @access public
	* @return boolean
	*/
	function processForm()
	{
		$missing = false;
		$form =& $this->getMySection();
		
		$file = get_file_upload_info($this->id);
		
		$summary = $file['name'].', type '.$file['type'].', '.easy_filesize($file['size']);

		// there was an error
		if ($file === false) {
			$form->active_submission->setError('There was an error uploading the file for File Upload question "'.$this->attr('name').'"');
			return false;
		}
		
		// no file uploaded
		if (is_array($file) && empty($file) && $this->attr('required')) {
			$form->active_submission->setError('No file uploaded for required File Upload question "'.$this->attr('name').'"');
			return false;
		}

		if (!parent::processForm()) return false;
		
		$create_loc = $this->attr('create_location');
		$create_loc_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($create_loc);
		
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('file');
		$file_asset = new File();
		
		$file_asset->_tmp['uploading_file'] = true;
		$file_asset->setAttrValue('name', $file['name']);
		$link = Array('asset' => &$create_loc_asset, 'link_type' => SQ_LINK_TYPE_2);
			
		if (!$file_asset->create($link, $file)) return false;

		$lock_status = $GLOBALS['SQ_SYSTEM']->am->acquireLock($file_asset->id, 'attributes');
		$edit_fns = $file_asset->getEditFns();
		if (!$edit_fns->processFileUpload($file_asset, $o, $file_asset->getPrefix(), $file)) {
			if ($lock_status === 1) $GLOBALS['SQ_SYSTEM']->am->releaseLock($file_asset->id, 'attributes');
			$form->active_submission->setError('There was an error uploading the file for File Upload question "'.$this->attr('name').'" - file already exists in path?');
			return false;
		}
		if ($lock_status === 1) $GLOBALS['SQ_SYSTEM']->am->releaseLock($file_asset->id, 'attributes');
						
		$form->active_submission->setAnswer($this->id, $file['name']);
		$form->active_submission->setSummary($this->id,  $this->attr('name'), $summary);

		return true;

	}//end processForm()


	/**
	* returns an array of the types of rules
	*
	* Returned array is 'short name' => 'full name'
	*
	* @access public
	* @return Array(string => string)
	*/
	function getAllowedRules()
	{
		return Array('file_size', 'file_type');

	}//end getAllowedRules()


}//end class
?>
