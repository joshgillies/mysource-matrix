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
* $Id: form_question_type_file_upload.inc,v 1.13 2005/06/15 04:03:15 lwright Exp $
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
* @version $Revision: 1.13 $
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
	* @access public
	* @return string
	*/
	function getHtmlField()
	{
		$extras = $this->attr('extra');
		$name   = $this->id;

		ob_start();
		file_upload($name, $extras);
		$html = ob_get_contents();
		ob_end_clean();

		return html_entity_decode($html);

	}//end getHtmlField()


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
	* @access public
	* @return boolean
	*/
	function hasValidValue()
	{
		$create_loc = $this->attr('create_location');
		if (empty($create_loc)) {
			// no create location set
			$this->failed_rules[] = translate('cms_form_file_upload_no_create_locs', $this->attr('name'));' "'.$this->attr('name').'"';
			return false;
		}

		if ($this->_tmp['file_info'] === false) {
			// there was an upload error
			$this->failed_rules[] = translate('cms_form_file_upload_error', $this->attr('name'));
			return false;
		}

		$ok = true;

		if (is_array($this->_tmp['file_info']) && empty($this->_tmp['file_info']) && $this->attr('is_required')) {
			// no file uploaded to required field
			$this->failed_rules[] = translate('cms_form_file_upload_none_rule', $this->attr('name'));
			$ok = false;
		}

		if ($this->attr('overwrite_rule') == 'D') {
			// we want to throw an error if a file with this name already exists, so see if one does
			$db =& $GLOBALS['SQ_SYSTEM']->db;
			$existing_file = $db->getOne('select a.assetid from '.SQ_TABLE_RUNNING_PREFIX.'ast a join '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l on a.assetid = l.minorid where l.majorid = '.$db->quoteSmart($create_loc).' and a.name = '.$db->quoteSmart($this->getValue()));
			assert_valid_db_result($existing_file);
			if ($existing_file) {
				// file already exists so throw the error
				$ok = false;
				$this->failed_rules[] = translate('cms_form_file_upload_already_exists', $this->attr('name'));
			}
		}

		if ($ok) {
			return parent::hasValidValue();
		}
		return false;

	}//end hasValidValue()


	/**
	* Look at user-specified and internal rules to see if this question currently has a valid value
	*
	* @access public
	* @return void
	*/
	function populate()
	{
		$this->_tmp['file_info'] = get_file_upload_info($this->id);
		$this->setValue(array_get_index($this->_tmp['file_info'], 'name', translate('cms_form_file_upload_none')));
		if (isset($this->_tmp['file_info']['name'])) {
			$this->_tmp['summary'] = $this->_tmp['file_info']['name'].', type '.$this->_tmp['file_info']['type'].', '.easy_filesize($this->_tmp['file_info']['size']);
		}

	}//end populate()


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
		if (!isset($this->_tmp['summary'])) {
			$db =& $GLOBALS['SQ_SYSTEM']->db;
			$existing_file = $db->getOne('select a.assetid from '.SQ_TABLE_RUNNING_PREFIX.'ast a join '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l on a.assetid = l.minorid where l.majorid = '.$db->quoteSmart($this->attr('create_location')).' and a.name = '.$db->quoteSmart($this->getValue()));
			assert_valid_db_result($existing_file);
			if (!is_null($existing_file)) {
				$existing_file_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($existing_file);
				$info = $existing_file_asset->getExistingFile();
				$this->_tmp['summary'] = $this->getValue().', '.easy_filesize($info['size']);
			} else {
				$this->_tmp['summary'] = translate('cms_form_file_upload_none');
			}
		}
		return $this->_tmp['summary'];

	}//end getSummary()


	/**
	* Returns whether questions of this type can be edited once the form has been initially submitted
	*
	* @access public
	* @return boolean
	*/
	function isEditable()
	{
		return false;

	}//end isEditable()


	/**
	* Do any work necessary to make this value "stick", eg create an asset
	*
	* This gets called once we know the whole form is valid and we want to save all the values.
	*
	* @access public
	* @return void
	*/
	function saveValue()
	{
		$create_loc = $this->attr('create_location');
		$create_loc_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($create_loc);
		if (is_null($create_loc_asset)) {
			// we shouldn't get to this point if there is no create loc
			return;
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('file');

		$db =& $GLOBALS['SQ_SYSTEM']->db;
		$existing_file = $db->getOne('select a.assetid from '.SQ_TABLE_RUNNING_PREFIX.'ast a join '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l on a.assetid = l.minorid where l.majorid = '.$db->quoteSmart($create_loc).' and a.name = '.$db->quoteSmart($this->getValue()));
		assert_valid_db_result($existing_file);

		$ok = true;
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
					while ($existing_file) {
						$file['name'] = increment_filename($file['name']);
						$existing_file = $db->getOne('select a.assetid from '.SQ_TABLE_RUNNING_PREFIX.'ast a join '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l on a.assetid = l.minorid where l.majorid = '.$db->quoteSmart($create_loc).' and a.name = '.$db->quoteSmart($file['name']));
						assert_valid_db_result($existing_file);
					}
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
				return false;
			}
			if ($lock_status === 1) {
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($file_asset->id, 'attributes');
			}

		} else {

			// create a new file asset
			$file_asset = new File();
			$file_asset->_tmp['uploading_file'] = true;

			// set the run level to ensure the file is correctly saved without
			// the hassle of having to deal with permission errors
			$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);

			$file_asset->setAttrValue('name', $this->getValue());
			$link = Array('asset' => &$create_loc_asset, 'link_type' => SQ_LINK_TYPE_2);
			if (!$file_asset->create($link, $this->_tmp['file_info'])) {
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				return false;
			}

			$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		}

		return true;

	}//end saveValue()


}//end class
?>
