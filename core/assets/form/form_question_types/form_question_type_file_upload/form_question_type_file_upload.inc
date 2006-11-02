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
* $Id: form_question_type_file_upload.inc,v 1.6.2.7 2006/11/02 03:10:24 rong Exp $
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
* @version $Revision: 1.6.2.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
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
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array();

	}//end _getAllowedLinks()


	/**
	* generates the html for this question to display it in the frontend
	*
	* @return string
	* @access public
	*/
	function getHtmlField()
	{
		$extras = $this->attr('extra');
		$name   = 'q'.$this->id;

		// prepare a valid value for the field id
		$extras = $extras.' id="'.str_replace(':', '_', $name).'"';

		ob_start();
		file_upload($name, $extras);
		$html = ob_get_contents();
		ob_end_clean();

		return html_entity_decode($html);

	}//end getHtmlField()


	/**
	* Returns the value of value var
	*
	* @return string
	* @access public
	*/
	function getValue()
	{
		if (is_null($this->value)) {
			return '';
		} else {
			return $this->value;
		}

	}//end getValue()


	/**
	* Look at user-specified and internal rules to see if this question currently has a valid value
	*
	* @return boolean
	* @access public
	*/
	function hasValidValue()
	{
		$create_loc = $this->attr('create_location');
		if (empty($create_loc)) {
			// no create location set
			$this->failed_rules[] = translate('core_form_file_upload_no_create_locs', $this->attr('name'));' "'.$this->attr('name').'"';
			return FALSE;
		}

		if ($this->_tmp['file_info'] === FALSE) {
			// there was an upload error
			$this->failed_rules[] = translate('core_form_file_upload_error', $this->attr('name'));
			return FALSE;
		}

		$ok = TRUE;

		if (is_array($this->_tmp['file_info']) && empty($this->_tmp['file_info']) && $this->attr('is_required')) {
			// no file uploaded to required field
			if (strlen($this->attr('cust_required_error'))) {
				$this->failed_rules[] = $this->attr('cust_required_error');
			} else {
				$this->failed_rules[] = translate('core_form_file_upload_none_rule', $this->attr('name'));
			}
			$ok = FALSE;
		}

		if ($this->attr('overwrite_rule') == 'D') {
			// we want to throw an error if a file with this name already exists, so see if one does
			$db =& $GLOBALS['SQ_SYSTEM']->db;

			// Check that we are not going to have web path conflicts
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$valid_names = make_valid_web_paths(Array($this->getValue()));
			$name = array_shift($valid_names);

			$existing_file = $db->getOne('select a.assetid from '.SQ_TABLE_RUNNING_PREFIX.'ast a join '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l on a.assetid = l.minorid where l.majorid = '.$db->quoteSmart($create_loc).' and a.name = '.$db->quoteSmart($name));
			assert_valid_db_result($existing_file);
			if ($existing_file) {
				// file already exists so throw the error
				$ok = FALSE;
				$this->failed_rules[] = translate('core_form_file_upload_already_exists', $this->attr('name'));
			}
		}

		if ($ok) return parent::hasValidValue();

		return FALSE;

	}//end hasValidValue()


	/**
	* Look at user-specified and internal rules to see if this question currently has a valid value
	*
	* @return void
	* @access public
	*/
	function populate()
	{
		$this->_tmp['file_info'] = get_file_upload_info('q'.$this->id);
		$this->setValue(array_get_index($this->_tmp['file_info'], 'name', translate('core_form_file_upload_none')));
		if (isset($this->_tmp['file_info']['name'])) {
			$this->_tmp['summary'] = $this->_tmp['file_info']['name'].', type '.$this->_tmp['file_info']['type'].', '.easy_filesize($this->_tmp['file_info']['size']);
		}

	}//end populate()


	/**
	* returns an array of the types of rules
	*
	* Returned array is 'short name' => 'full name'
	*
	* @return array
	* @access public
	*/
	function getAllowedRules()
	{
		return Array('file_size', 'file_type');

	}//end getAllowedRules()


	/**
	* returns a summary about the answers in this question
	*
	* @return string
	* @access public
	*/
	function getSummary()
	{
		if (!isset($this->_tmp['summary'])) {
			$db =& $GLOBALS['SQ_SYSTEM']->db;

			// Check that we are not going to have web path conflicts
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
			$valid_names = make_valid_web_paths(Array($this->getValue()));
			$name = array_shift($valid_names);

			$existing_file = $db->getOne('select a.assetid from '.SQ_TABLE_RUNNING_PREFIX.'ast a join '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l on a.assetid = l.minorid where l.majorid = '.$db->quoteSmart($this->attr('create_location')).' and a.name = '.$db->quoteSmart($name));
			assert_valid_db_result($existing_file);
			if (!is_null($existing_file)) {
				$existing_file_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($existing_file);
				$info = $existing_file_asset->getExistingFile();
				$this->_tmp['summary'] = $this->getValue().', '.easy_filesize($info['size']);
			} else {
				$this->_tmp['summary'] = translate('core_form_file_upload_none');
			}

		}
		return $this->_tmp['summary'];

	}//end getSummary()


	/**
	* Returns whether questions of this type can be edited once the form has been initially submitted
	*
	* @return boolean
	* @access public
	*/
	function isEditable()
	{
		return FALSE;

	}//end isEditable()


	/**
	* Do any work necessary to make this value "stick", eg create an asset
	*
	* This gets called once we know the whole form is valid and we want to save all the values.
	*
	* @return void
	* @access public
	*/
	function saveValue()
	{
		$create_loc = $this->attr('create_location');
		$create_loc_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($create_loc);
		if (is_null($create_loc_asset) || !isset($this->_tmp['file_info']['name'])) {
			// we shouldn't get to this point if there is no create loc or no file was uploaded
			return;
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('file');

		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// Check that we are not going to have web path conflicts
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_names = make_valid_web_paths(Array($this->getValue()));
		$name = array_shift($valid_names);

		$existing_file = $db->getOne('select a.assetid from '.SQ_TABLE_RUNNING_PREFIX.'ast a join '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l on a.assetid = l.minorid where l.majorid = '.$db->quoteSmart($create_loc).' and a.name = '.$db->quoteSmart($name));
		assert_valid_db_result($existing_file);

		$ok = TRUE;
		if ($existing_file) {
			switch ($this->attr('overwrite_rule')) {
				case 'D':
					// error was already thrown and we shouldn't even be here
					return;
				break;

				case 'O':
					// want overwrite so just leave filename as is
				break;

				case 'R':
					// find a new name to rename the file to
					include_once SQ_FUDGE_PATH.'/general/file_system.inc';
					$filename = $name;
					while ($existing_file) {
						$filename = increment_filename($filename);
						$existing_file = $db->getOne('select a.assetid from '.SQ_TABLE_RUNNING_PREFIX.'ast a join '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l on a.assetid = l.minorid where l.majorid = '.$db->quoteSmart($create_loc).' and a.name = '.$db->quoteSmart($filename));
						assert_valid_db_result($existing_file);
					}
					$this->setValue($filename);
				break;
			}
		}

		if ($existing_file) {

			// update the existing file asset
			$file_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($existing_file);
			$lock_status = $GLOBALS['SQ_SYSTEM']->am->acquireLock($file_asset->id, 'attributes');
			$edit_fns = $file_asset->getEditFns();
			if (!$edit_fns->processFileUpload($file_asset, $o, $file_asset->getPrefix(), $this->_tmp['file_info'])) {
				if ($lock_status === 1) {
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($file_asset->id, 'attributes');
				}
				return FALSE;
			}
			if ($lock_status === 1) {
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($file_asset->id, 'attributes');
			}

		} else {

			// create a new file asset
			$file_asset =& new File();
			$file_asset->_tmp['uploading_file'] = TRUE;

			// set the run level to ensure the file is correctly saved without
			// the hassle of having to deal with permission errors
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
			$link = Array('asset' => &$create_loc_asset, 'link_type' => SQ_LINK_TYPE_2);
			$this->_tmp['file_info']['name'] = $this->getValue();
			if (!$file_asset->create($link, $this->_tmp['file_info'])) {
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				return FALSE;
			}
			// update the summary
			$this->_tmp['summary'] = $this->_tmp['file_info']['name'].', type '.$this->_tmp['file_info']['type'].', '.easy_filesize($this->_tmp['file_info']['size']);
			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		}

		return TRUE;

	}//end saveValue()


	/**
	* Returns the assetid of the uploaded file
	*
	* @return mixed int|NULL
	* @access public
	*/
	function getUploadedFileId()
	{
		$create_loc = $this->attr('create_location');
		$create_loc_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($create_loc);
		if (is_null($create_loc_asset)) return NULL;

		// Check that we are not going to have web path conflicts
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_names = make_valid_web_paths(Array($this->getValue()));
		$name = array_shift($valid_names);

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$existing_file = $db->getOne('select a.assetid from '.SQ_TABLE_RUNNING_PREFIX.'ast a join '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l on a.assetid = l.minorid where l.majorid = '.$db->quoteSmart($create_loc).' and a.name = '.$db->quoteSmart($name));
		assert_valid_db_result($existing_file);

		return $existing_file;

	}//end getUploadedFileId()


}//end class
?>
