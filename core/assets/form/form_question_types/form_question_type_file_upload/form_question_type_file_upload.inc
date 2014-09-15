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
* $Id: form_question_type_file_upload.inc,v 1.49 2013/04/19 06:36:02 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../../form_question/form_question.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';


/**
* Form_Question_Type_File_Upload
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.49 $
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

		if ($this->attr('tabindex')) {
			$extras .= ' tabindex="'.$this->attr('tabindex').'"';
		}

		ob_start();
		$temp_path = array_get_index($this->extra_data, 'temp_filesystem_path', '');
		if (!empty($temp_path)) {
			?><p>Current file: <?php echo basename($temp_path); ?>
			<?php if (!$this->attr('is_required')) {
				echo check_box($name.'_clear', 1, FALSE) ?> Clear?<?php
			}
			?></p><?php
		}
		file_upload($name, $extras);
		$html = ob_get_contents();
		ob_end_clean();

		return $html;

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
	* Look at user-specified and internal rules to see if this question
	* currently has a valid value
	*
	* Returns TRUE if all validation rules pass.
	*
	* We cannot do our validation at the page level, only at the on-submit
	* level. Any validation made at page level may not be true when the form
	* is finally submitted, and the files are finally created in the Create
	* Location(s) required.
	*
	* @param mixed		$answer			Custom value to compare with - if left
	*									NULL (or unspecified), use the current
	*									value of the question
	* @param boolean	$mute_errors	If TRUE, do not add to the "Failed
	*									Rules" log, but simply return FALSE
	*
	* @return boolean
	* @access public
	*/
	function hasValidValue($answer=NULL, $mute_errors=FALSE)
	{

		if (isset($this->_tmp['file_info']) && $this->_tmp['file_info'] === FALSE) {
			// there was an upload error
			$this->failed_rules[] = sprintf(translate('core_form_file_upload_error'), $this->attr('name'));
			return FALSE;
		}

		$ok = TRUE;

		$none_uploaded = !isset($this->_tmp['file_info']) || (is_array($this->_tmp['file_info']) && empty($this->_tmp['file_info']));
		$previously_uploaded = !empty($this->extra_data['temp_filesystem_path']);

		if ($none_uploaded && !$previously_uploaded && $this->attr('is_required') && $this->_isValidationRequired()) {
			// no file uploaded to required field (and none previously uploaded either)
			if (strlen($this->attr('cust_required_error'))) {
				$this->failed_rules[] = $this->attr('cust_required_error');
			} else {
				$this->failed_rules[] = sprintf(translate('core_form_file_upload_none_rule'), $this->attr('name'));
			}
			$ok = FALSE;
		}

		if ($ok) {
			return parent::hasValidValue($answer, $mute_errors);
		}

		return FALSE;

	}//end hasValidValue()


	/**
	* Populate the question based on what exists on the form
	*
	* @return void
	* @access public
	*/
	function populate(Asset $parent=NULL)
	{
		$this->_tmp['file_info'] = get_file_upload_info('q'.$this->id);

		if (isset($this->_tmp['file_info']['name'])) {
			$this->setValue(array_get_index($this->_tmp['file_info'], 'name', translate('core_form_file_upload_none')));
			$this->getSummary();
			//$this->_tmp['summary'] = $this->_tmp['file_info']['name'].', type '.$this->_tmp['file_info']['type'].', '.easy_filesize($this->_tmp['file_info']['size']);
		} else {
			// see if we have an incomplete file upload, if we do, then don't
			// blank out the summary/value
			$existing_file   = FALSE;
			$submission_path = $this->getTempFileUploadPath($parent);
			require_once SQ_FUDGE_PATH.'/standards_lists/mime_types.inc';

			if (is_dir($submission_path)) {
				$dir = new DirectoryIterator($submission_path);
				foreach ($dir as $dir_entry) {
					if ($dir_entry->isFile()) {
						$this->setValue($dir_entry->getFilename());
						//$this->_tmp['summary'] = $dir_entry->getFilename().', type '.$standards_lists_mime_types[get_file_type($dir_entry->getFilename())].', '.easy_filesize($dir_entry->getSize());
						$this->getSummary();
						$existing_file = TRUE;
						break;
					}
				}
			}

			if (!$existing_file) {
				$this->setValue(translate('core_form_file_upload_none'));
			}
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
		return Array('file_size', 'file_type', 'file_virus_check');

	}//end getAllowedRules()


	/**
	* returns a summary about the answers in this question
	*
	* @return string
	* @access public
	*/
	function getSummary()
	{
		//if (!isset($this->_tmp['summary'])) {
			if (!empty($this->extra_data)) {
				$value = reset($this->extra_data);
				$key   = key($this->extra_data);
				$new_summary = '';
				require SQ_FUDGE_PATH.'/standards_lists/mime_types.inc';

				switch($key) {
					case 'temp_filesystem_path' :
						// if using relative path
						if(!empty($value) && strpos($value, '/') !== 0) {
						    $value = SQ_SYSTEM_ROOT . '/'. $value;
						}
						$filepath = $value;
						if (is_file($filepath)) {
							$type = array_get_index($standards_lists_mime_types, get_file_type($filepath), '');
							if (!empty($type)) $type = ', type '.$type;
							$new_summary = basename($value).$type.', '.easy_filesize(filesize($filepath));
						} else {
							$new_summary = translate('core_form_file_upload_none');
						}
					break;

					case 'filesystem_path' :
						if(!empty($value) && strpos($value, '/') !== 0) {
						    $value = SQ_SYSTEM_ROOT . '/'. $value;
						}
						$filepath = $value;
						if (is_file($filepath)) {
							$type = array_get_index($standards_lists_mime_types, get_file_type($filepath), '');
							if (!empty($type)) $type = ', type '.$type;
							$new_summary = basename($value).$type.', '.easy_filesize(filesize($filepath));
						} else {
							$new_summary = translate('core_form_file_upload_none');
						}
					break;

					case 'new_file_assetid' :
					case 'existing_file_assetid' :
						if (!empty($value)){
							if (SQ_IN_BACKEND || SQ_IN_LIMBO) {
								$new_summary = get_asset_tag_line($value);
							} else {
								$file_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($value);
								$file_info = $file_asset->getExistingFile();
								$type = array_get_index($standards_lists_mime_types, get_file_type($file_info['filename']), '');
								if (!empty($type)) $type = ', type '.$type;
								$new_summary = basename($file_info['filename']).$type.', '.easy_filesize($file_info['size']);
							}
						} else {
							$new_summary = translate('core_form_file_upload_none');
						}

					break;
				}

				$this->_tmp['summary'] = $new_summary;
			} else {
				// Check that we are not going to have web path conflicts
				require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
				require_once SQ_LIB_PATH.'/html_form/html_form.inc';
				$valid_names = make_valid_web_paths(Array($this->getValue()));
				$name = array_shift($valid_names);

				$sql = 'SELECT
							a.assetid
						FROM
							'.SQ_TABLE_RUNNING_PREFIX.'ast a
								JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l
								ON a.assetid = l.minorid
						WHERE
							l.majorid = :majorid
							AND a.name = :name';

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'majorid', $this->attr('create_location'));
					MatrixDAL::bindValueToPdo($query, 'name', $name);
					$existing_file = MatrixDAL::executePdoOne($query);
				} catch (Exception $e) {
					throw new Exception('Unable to get summary of answers for: '.$name.' due to database error: '.$e->getMessage());
				}

				if (!empty($existing_file)) {
					$existing_file_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($existing_file);
					$info = $existing_file_asset->getExistingFile();
					$assetid = $this->getUploadedFileId();
					if (trim($assetid) != '') {
						$this->_tmp['summary'] = get_asset_tag_line($assetid);
						return $this->_tmp['summary'];
					}//end if
					$this->_tmp['summary'] = $this->getValue().', '.easy_filesize($info['size']);
				} else {
					$this->_tmp['summary'] = translate('core_form_file_upload_none');
				}
			}//end if Extra Data exists

		//}//end if temp summary exists

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
	function saveValue(Asset $parent)
	{
		$prefix = 'q'.$this->id;
		if (isset($_POST[$prefix.'_clear'])) {
			if (isset($this->extra_data['temp_filesystem_path'])) {
				// Just nuke the directory, because this means there will be no
				// files left. We want to be as clean as possible in the data/
				// path
				$temp_path = $this->extra_data['temp_filesystem_path'];
				if(!empty($temp_path) && strpos($temp_path, '/') !== 0) {
				    $temp_path = SQ_SYSTEM_ROOT . '/' . $temp_path;
				}
				delete_directory(dirname($temp_path));

				$this->setValue('');
				unset($this->_tmp['file_info']);
				unset($this->extra_data['mime_type']);
				unset($this->extra_data['temp_filesystem_path']);
			}
		}

		if (empty($this->_tmp['file_info'])) {
			return TRUE;
		}

		$file_info = $this->_tmp['file_info'];
		$ok = TRUE;

		if ($file_info['error'] === UPLOAD_ERR_OK) {
			$temp_path = $this->getTempFileUploadPath($parent).'/'.$this->getValue();
			$this->createTempFileUpload($parent);
			$this->extra_data = Array(
										'temp_filesystem_path'	=> $temp_path,
										'mime_type'				=> $file_info['type'],
									);

			// We only have to upload this once, so remove file info
			unset($this->_tmp['file_info']);
		} else if ($file_info['error'] !== UPLOAD_ERR_NO_FILE) {
			// If no file uploaded, then that is fine, but otherwise, error
			$ok = FALSE;
		}

		if (!isset($this->extra_data['temp_filesystem_path'])) {
			$this->setValue('');
		}

		return $ok;

	}//end saveValue()


	/**
	* Perform on-submit actions and validation
	*
	* @return boolean
	*/
	public function onSubmitForm(Asset $parent)
	{
		// Do we have a create location?
		$question_create_loc = $this->attr('create_location');
		$ok = TRUE;
		$mime_type = array_get_index($this->extra_data, 'mime_type', '');
		$temp_path = array_get_index($this->extra_data, 'temp_filesystem_path', '');

		if (!empty($question_create_loc) && !empty($temp_path)) {
			// If we are creating a File asset, we need to check whether we need
			// to handle a web path conflict
			require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

			// We are only going to have one valid name, so this is no problem
			$valid_names = make_valid_web_paths(Array($this->getValue()));
			$name = array_shift($valid_names);

			$create_location = $GLOBALS['SQ_SYSTEM']->am->getAsset($question_create_loc);

			// Find an asset underneath the create location that has the same
			// web path as the new would-be asset.
			// Only consider significant links.
			$sql = 'SELECT assetid
						FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_path
						WHERE assetid IN
							(SELECT a.assetid
								FROM '.SQ_TABLE_RUNNING_PREFIX.'ast a
									JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l
									ON a.assetid = l.minorid
								WHERE
									l.majorid = :majorid
									AND '.db_extras_bitand(MatrixDAL::getDbType(), 'l.link_type', ':link_types').' > 0
							)
						AND path = :webpath
					';
			try {
				$query = MatrixDAL::preparePdoQuery($sql);
				MatrixDAL::bindValueToPdo($query, 'majorid',    $question_create_loc);
				MatrixDAL::bindValueToPdo($query, 'link_types', SQ_SC_LINK_WEB_PATHS);
				MatrixDAL::bindValueToPdo($query, 'webpath',    $name);
				$existing_assetid = MatrixDAL::executePdoOne($query);
			} catch (Exception $e) {
				throw new Exception('Unable to check for web path conflicts for filename: '.$name.' due to database error: '.$e->getMessage());
			}

			//check possible orphaned asset
			if(empty($existing_assetid) && ($this->attr('overwrite_rule') == 'O' || $this->attr('overwrite_rule') == 'R')) {
				$orphaned = NULL;
				$parent_urls = $create_location->getLookups();
				for ($j = 0, $num_parent_urls = count($parent_urls); $j < $num_parent_urls; $j++) {
					$new_url = $parent_urls[$j]['url'].'/'.$name;
					// check if another asset is using the new URL
					$orphaned = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL(NULL, $new_url, TRUE, TRUE);
				}
				// if there is an asset using new url and it was not found previously by checking links
				// we will treat it as an orphaned asset
				 if(!empty($orphaned)) {
					// create the link for the orphaned asset
					$GLOBALS['SQ_SYSTEM']->am->createAssetLink($create_location, $orphaned, SQ_LINK_TYPE_2, '');
					$existing_assetid = $orphaned->id;
				}
			}

			$existing_asset = NULL;
			if (!empty($existing_assetid)) {
				switch ($this->attr('overwrite_rule')) {
					case 'D' :
						// D = Raise a Submission Error
						$this->failed_rules[] = sprintf(translate('core_form_file_upload_already_exists'), $this->attr('name'));
						$ok = FALSE;
					break;

					case 'O' :
						// O = Overwrite the file
						$existing_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($existing_assetid);

						if (!($existing_asset instanceof File)) {
							// We will not overwrite an asset that is not a File.
							$this->failed_rules[] = 'Refusing to overwrite a non-File asset at web path '.$name.' - an uploaded file can only overwrite a File asset';
							$ok = FALSE;
						}
					break;

					case 'R' :
						// R = Rename the uploaded file
						// Use webPathsInUse() to help us here.
						$suggested_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($create_location, Array($name), 0, TRUE);
						$name = array_shift($suggested_paths);
					break;
				}
			}

			// We are all good, so create the file
			if ($ok) {

				if ($existing_asset) {
					// update the existing file asset
					$edit_fns = $existing_asset->getEditFns();

					$file_info = Array(
									'tmp_name'			=> strpos($temp_path, '/') === 0  || empty($temp_path) ?  $temp_path : SQ_SYSTEM_ROOT.'/'.$temp_path,
									'name'				=> $name,
									'non_uploaded_file'	=> TRUE,
								 );

					$o = NULL;
					//force to update the file
					$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					if (!$edit_fns->processFileUpload($existing_asset, $o, $existing_asset->getPrefix(), $file_info)) {
						return FALSE;
					}
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

					$this->extra_data = Array(
											'existing_file_assetid'	=> $existing_asset->id,
											'mime_type'				=> $mime_type,
											'temp_filesystem_path' 	=> $temp_path,
										);

				} else {
					$file_ext = get_file_type($this->getValue());

					// Switch based on the file extension
					if (in_array($file_ext, Array('gif', 'jpg', 'jpeg', 'png'))) {
						$file_type = 'Image';

					} else if (preg_match('/^do[ct][xm]?$/', $file_ext)) {
						// DOC, DOT - MS Word (document, template respectively)
						// DOCX, DOCM, DOTX, DOTM - MS Word 2007+ OOXML
						$file_type = 'Word_Doc';

					} else if (preg_match('/^xl[st][xm]?$/', $file_ext)) {
						// XLS, XLT - MS Excel (workbook, template respectively)
						// XLSX, XLSM, XLTX, XLTM - MS Excel 2007+ OOXML
						$file_type = 'Excel_Doc';

					} else if (preg_match('/^p(pt|ps|ot)[xm]?$/', $file_ext)) {
						// PPT, PPS, POT - MS PowerPoint
						// (presentation, slideshow, template respectively)
						// [ext]X, [ext]M - MS PowerPoint 2007+ OOXML
						$file_type = 'PowerPoint_Doc';

					} else if ($file_ext === 'pdf') {
						$file_type = 'PDF_File';

					} else if ($file_ext === 'rtf') {
						$file_type = 'RTF_File';

					} else if ($file_ext === 'txt') {
						$file_type = 'Text_File';

					} else if (substr($mime_type, 0, 5) === 'text/') {
						$file_type = 'Text_File';

					} else {
						$file_type = 'File';

					}//end if on file extension

					$GLOBALS['SQ_SYSTEM']->am->includeAsset($file_type);

					$file_asset = new $file_type();
					$file_asset->_tmp['uploading_file'] = TRUE;

					$link = Array(
								'asset'		=> $GLOBALS['SQ_SYSTEM']->am->getAsset($question_create_loc),
								'link_type'	=> SQ_LINK_TYPE_2,
							);

					$file_info = Array(
									'tmp_name'			=>  strpos($temp_path, '/') === 0  || empty($temp_path) ?  $temp_path : SQ_SYSTEM_ROOT.'/'.$temp_path,
									'name'				=> $name,
									'non_uploaded_file'	=> TRUE,
								 );

					// We need to be able to create a file within the set
					// create location, no matter who we are. To avoid errors
					// regarding write permissions, we'll force this through
					$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					if (!$file_asset->create($link, $file_info)) {
						$this->failed_rules[] = sprintf(translate('core_form_file_upload_error'), $this->attr('name'));
						$ok = FALSE;
					}
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

					unset($file_asset->_tmp['uploading_file']);

					$this->extra_data = Array(
											'new_file_assetid'		=> $file_asset->id,
											'mime_type'				=> $mime_type,
											'temp_filesystem_path'	=> $temp_path,
										);

				}//end if existing file

			}//end if OK

		}//end if create location exists

		return $ok;

	}//end onSubmitForm()


	/**
	* Clean up after the submission is completed
	*
	* Once the form is done with the question, this function should be called
	* to clean up the incomplete submission.
	*
	* @param Asset	$parent	The asser that called for the clean-up
	*
	* @return void
	*/
	public function cleanUp(Asset $parent)
	{
		$filepath = array_get_index($this->extra_data, 'temp_filesystem_path', NULL);
		if (!empty($filepath)) {
			if(strpos($filepath, '/') !== 0) $filepath = SQ_SYSTEM_ROOT.'/'.$filepath;
			delete_directory(dirname($filepath));
		}

	}//end cleanUp()


	/**
	* Create incomplete file upload in the parent's space
	*
	* @param Asset	$parent	The parent asset for this file upload question.
	*						This is most likely a Form, but could be the parent
	*						of a Simple Form.
	*
	* @return void
	*/
	public function createTempFileUpload(Asset $parent)
	{
		$file_info = $this->_tmp['file_info'];

		if ($file_info['error'] === UPLOAD_ERR_OK) {
			$submission_path = $this->getTempFileUploadPath($parent);
			$submission_path = SQ_SYSTEM_ROOT.'/'.$submission_path;
			create_directory($submission_path);

			// Remove existing uploads, to ensure that there is only one file
			// associated with an incomplete attachment
			$dir = new DirectoryIterator($submission_path);
			foreach ($dir as $dir_entry) {
				if ($dir_entry->isFile()) {
					unlink($dir_entry->getPathname());
				}
			}

			$submission_file_name = $submission_path.'/'.$file_info['name'];
			move_uploaded_file($file_info['tmp_name'], $submission_file_name);
			$this->setValue(basename($submission_file_name));
		}

	}//end createTempFileUpload()


	/**
	* Returns the filesystem path
	*
	* @return string
	*/
	public function getTempFileUploadPath(Asset $parent)
	{
		if (method_exists($parent, 'getTempFileUploadPath')) {
			$submission_path = $parent->getTempFileUploadPath();
		} else {
			$submission_path = $this->data_path.'/attachments/'.session_id().'_'.$parent->id;
		}

		$submission_path = str_replace(SQ_SYSTEM_ROOT.'/', '', $submission_path);
		$submission_path .= '/'.str_replace(':', '_', $this->id);

		return $submission_path;
	}


	/**
	* Returns the assetid of the uploaded file
	*
	* @return mixed int|NULL
	* @access public
	*/
	function getUploadedFileId()
	{
		$existing_file = NULL;
		if ($this->value != translate('core_form_file_upload_none') && !empty($this->value)){
			if (!empty($this->extra_data)) {
				// We have a permanent filesystem path, ie. we don't have create location
				if (isset($this->extra_data['filesystem_path'])) {
					return NULL;
				}
				$existing_file = array_get_index($this->extra_data, 'new_file_assetid', NULL);
				if (is_null($existing_file)) {
					$existing_file = array_get_index($this->extra_data, 'existing_file_assetid', NULL);
				}
			}

			// No extra data, so use the old method
			if (is_null($existing_file)) {
				$create_loc = $this->attr('create_location');
				if ($create_loc == 0) return NULL;
				$create_loc_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($create_loc);
				if (is_null($create_loc_asset)) return NULL;

				$db = MatrixDAL::getDb();
				// Check that we are not going to have web path conflicts
				require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
				$valid_names = make_valid_web_paths(Array($this->getValue()));
				$name = array_shift($valid_names);

				$sql = 'SELECT
							a.assetid
						FROM
							'.SQ_TABLE_RUNNING_PREFIX.'ast a
								JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l
								ON a.assetid = l.minorid
						WHERE
							l.majorid = :create_loc
							AND a.name = :name';

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'create_loc', $create_loc);
					MatrixDAL::bindValueToPdo($query, 'name', $name);
					$existing_file = MatrixDAL::executePdoOne($query);
				} catch (Exception $e) {
					throw new Exception('Unable to get the uploaded file assetid due to database error: '.$e->getMessage());
				}
			}
		}

		return $existing_file;

	}//end getUploadedFileId()


}//end class
?>
