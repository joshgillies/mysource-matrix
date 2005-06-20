<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: form_question_type_file_upload.inc,v 1.4.2.6 2005/06/20 02:17:53 lwright Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question/form_question.inc';

/**
* Form_Question_Type_File_Upload
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.4.2.6 $
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

	}//end constructor


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


	function getValue()
	{
		if (!$this->active_submission) {
			return null;
		}

		if (isset($this->active_submission[$this->id])) {
			return $this->active_submission[$this->id];
		} else {
			return '';
		}
	}//end getValue()


	/**
	* process this question
	*
	* @param &object $submission the current submission object
	*
	* @access public
	* @return boolean
	*/
	function evaluate()
	{
		$missing = false;
		$form =& $this->getMySection();

		$file = get_file_upload_info($this->id);

		// there was an error
		if ($file === false) {
			$form->failed_rules[] = 'There was an error uploading the file for File Upload question "'.$this->attr('name').'"';
			return false;
		}

		// no file uploaded
		if (is_array($file) && empty($file)) {
			if ($this->attr('required')) {
				$this->failed_rules[] = 'No file uploaded for required File Upload question "'.$this->attr('name').'"';
				return false;
			} else {
				$this->_tmp['summary'] = 'No file uploaded';
				return true;
			}
		}

		// no create location set!!
		$create_loc = $this->attr('create_location');
		if (empty($create_loc)) {
			$this->failed_rules[] = 'No create location has been set for File Upload question "'.$this->attr('name').'"';
			return false;
		}

		$this->active_submission[$this->id] = $file['name'];
		if (!parent::evaluate()) {
			return false;
		}

		$create_loc_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($create_loc);

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('file');

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$existing_file = $db->getOne('select a.assetid from '.SQ_TABLE_RUNNING_PREFIX.'ast a join '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l on a.assetid = l.minorid where l.majorid = '.$db->quoteSmart($create_loc).' and a.name = '.$db->quoteSmart($file['name']));
		assert_valid_db_result($existing_file);

		$ok = true;
		if ($existing_file) {
			switch ($this->attr('overwrite_rule')) {
				case 'D':
					$this->failed_rules[] = 'There was an error uploading the file for File Upload question "'.$this->attr('name').'" - file already exists in path';
					$ok = false;
				break;

				case 'O':				// overwrite - just leave as is
				break;

				case 'R':
					include_once SQ_FUDGE_PATH.'/general/file_system.inc';
					while ($existing_file) {
						$file['name'] = increment_filename($file['name']);
						$existing_file = $db->getOne('select a.assetid from '.SQ_TABLE_RUNNING_PREFIX.'ast a join '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l on a.assetid = l.minorid where l.majorid = '.$db->quoteSmart($create_loc).' and a.name = '.$db->quoteSmart($file['name']));
						assert_valid_db_result($existing_file);
					}
				break;
			}
		}

		if (!$ok) {
			return false;
		}

		if ($existing_file) {
			$file_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($existing_file);

			$lock_status = $GLOBALS['SQ_SYSTEM']->am->acquireLock($file_asset->id, 'attributes');
			$edit_fns = $file_asset->getEditFns();

			if (!$edit_fns->processFileUpload($file_asset, $o, $file_asset->getPrefix(), $file)) {
				if ($lock_status === 1) $GLOBALS['SQ_SYSTEM']->am->releaseLock($file_asset->id, 'attributes');
				return false;
			}
			if ($lock_status === 1) $GLOBALS['SQ_SYSTEM']->am->releaseLock($file_asset->id, 'attributes');

		} else {

			$file_asset = new File();

			$file_asset->_tmp['uploading_file'] = true;
			$link = Array('asset' => &$create_loc_asset, 'link_type' => SQ_LINK_TYPE_2);

			// set SQ_INSTALL flag to ensure the file is correctly saved without
			// the hassle of having to deal with permission errors
			$GLOBALS['SQ_INSTALL'] = true;

			$file_asset->setAttrValue('name', $file['name']);
			if (!$file_asset->create($link, $file)) {
				$GLOBALS['SQ_INSTALL'] = false;
				return false;
			}

			// File::create() will create the asset but won't attach the uploaded
			// file - we need to do this manually, and here we go
			$edit_fns = $file_asset->getEditFns();
			if (!$edit_fns->processFileUpload($file_asset, $o, $file_asset->getPrefix(), $file)) {
				return false;
			}

			$GLOBALS['SQ_INSTALL'] = false;

		}

		$this->_tmp['summary'] = $file['name'].', type '.$file['type'].', '.easy_filesize($file['size']);

		return true;

	}//end evaluate()


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


	function getSummary()
	{
		return array_get_index($this->_tmp, 'summary', 'No file uploaded');

	}//end getSummary()


}//end class
?>
