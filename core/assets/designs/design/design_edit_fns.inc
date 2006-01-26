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
* $Id: design_edit_fns.inc,v 1.102 2006/01/26 22:34:03 lwright Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.102 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Edit_Fns extends Design_Area_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Design_Edit_Fns()
	{
		$this->Design_Area_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paint the interface for the parse file uploading
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintParseFile(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('parsing')) {
			file_upload($prefix.'_parse_file');
			return TRUE;
		}
		return FALSE;

	}//end paintParseFile()


	/**
	* Processes the interface for the parse file uploading
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processParseFile(&$asset, &$o, $prefix)
	{

		// make sure a file was uploaded before processing it
		$upload_info = get_file_upload_info($prefix.'_parse_file');
		if (empty($upload_info)) return FALSE;

		$parse_file  = $asset->data_path.'/parse.txt';
		$changes     = FALSE;
		$new_version = FALSE;

		$fv =& $GLOBALS['SQ_SYSTEM']->getFileVersioning();

		// delete existing parse file if it exists
		if (is_file($parse_file)) {
			$new_version = TRUE;
			if (!unlink($parse_file)) {
				trigger_localised_error('CORE0164', E_USER_WARNING);
				return FALSE;
			}
		}

		// copy over the uploaded file
		if (commit_file_upload($prefix.'_parse_file', $parse_file, TRUE)) {
			// tell the asset to update the customisation at the end of the interface processing
			$asset->_tmp['update_customisations'] = TRUE;
			$asset->_tmp['generate_design'] = TRUE;
			$changes = $this->parseAndProcessFile($asset);

			// if we are overwriting our current parse file, we need to add a new version to the repository
			if ($new_version) {
				$file_status = $fv->upToDate($parse_file);
				if (FUDGE_FV_MODIFIED & $file_status) {
					if (!$fv->commit($parse_file, '')) {
						trigger_localised_error('CORE0160', E_USER_WARNING);
					}
				}
			} else {
				// attempt to add the parse file to the repository
				if (!$fv->add($asset->data_path_suffix, $parse_file, '')) {
					trigger_localised_error('CORE1057', E_USER_WARNING);
				}
			}
		}//end if file uploaded

		// make sure we have the latest version of our file
		if (!$fv->checkOut($asset->data_path_suffix.'/parse.txt', $asset->data_path)) {
			trigger_localised_error('CORE0158', E_USER_WARNING);
			return FALSE;
		}

		return $changes;

	}//end processParseFile()


	/**
	* Paint the interface for the reparsing the current parse file
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintReparseFile(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$parse_file = $asset->data_path.'/parse.txt';
			if (!is_file($parse_file)) {
				echo translate('core_no_existing_parsefile');
			} else {
				check_box($prefix.'_reparse');
				echo translate('core_manually_reparse');
			}
			return TRUE;
		}
		return FALSE;

	}//end paintReparseFile()


	/**
	* Processes the interface for the reparsing the current parse file
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processReparseFile(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_reparse'])) {
			if ($this->parseAndProcessFile($asset)) {
				$asset->_tmp['update_customisations'] = TRUE;
				$asset->_tmp['generate_design'] = TRUE;
				return TRUE;
			}
		}
		return FALSE;

	}//end processReparseFile()


	/**
	* Paint the interface for editing the parse file
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintEditParseFile(&$asset, &$o, $prefix)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		if (SQ_ROLLBACK_VIEW) {
			// get an older version of the parse file
			$fv =& $GLOBALS['SQ_SYSTEM']->getFileVersioning();
			$rep_file = $asset->data_path_suffix.'/parse.txt';
			$then = iso8601_ts($_SESSION['sq_rollback_view']['rollback_time']);
			$info = @$fv->_checkOutCheck($rep_file, null, $then);
			$parse_file = $info['source_file'];
		} else {
			$parse_file = $asset->data_path.'/parse.txt';
		}
		if (!is_file($parse_file)) {
			echo translate('core_require_upload_parsefile');
			return TRUE;
		}

		$str = file_to_string($parse_file);

		if ($asset->writeAccess('parsing')) {
			text_area($prefix.'_new_parse_file', str_replace("\t", '    ', $str), 0, 0, 0, 'style="font-family: monospace; white-space: pre; width: 99%; height: 480px"');
			return TRUE;
		} else {
			echo '<pre>', htmlspecialchars($str), '</pre>';
		}
		return FALSE;

	}//end paintEditParseFile()


	/**
	* Processes the interface for the editing the parse file
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processEditParseFile(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_new_parse_file'])) {

			$parse_file  = $asset->data_path.'/parse.txt';
			$new_file    = $_POST[$prefix.'_new_parse_file'];

			if (!is_file($parse_file)) return FALSE;

			$fv =& $GLOBALS['SQ_SYSTEM']->getFileVersioning();

			// delete existing parse file
			if (!unlink($parse_file)) {
				trigger_localised_error('CORE0164', E_USER_WARNING);
				return FALSE;
			}

			// copy over the uploaded file
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			if (string_to_file($new_file, $parse_file)) {
				// tell the asset to update the customisation at the end of the interface processing
				$asset->_tmp['update_customisations'] = TRUE;
				$asset->_tmp['generate_design'] = TRUE;
				$changes = $this->parseAndProcessFile($asset);

				// add a new version to the repository
				$file_status = $fv->upToDate($parse_file);
				if (FUDGE_FV_MODIFIED & $file_status) {
					if (!$fv->commit($parse_file, '')) {
						trigger_localised_error('CORE0160', E_USER_WARNING);
					}
				} else {
					$changes = TRUE;
				}

			} else {
				trigger_localised_error('CORE0167', E_USER_WARNING);
			}

			// make sure we have the latest version of our file
			if (!$fv->checkOut($asset->data_path_suffix.'/parse.txt', $asset->data_path)) {
				trigger_localised_error('CORE0158', E_USER_WARNING);
				return FALSE;
			}

			if ($changes) {
				return $this->parseAndProcessFile($asset);
			}
		}//end if new parse file submitted

		return FALSE;

	}//end processEditParseFile()


	/**
	* Paint New Files
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintNewFiles(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('links')) return FALSE;
		for ($i = 0; $i < 2; $i++) {
			file_upload($prefix.'_assoc_file_'.$i);
			echo "<br />\n";
		}

	}//end paintNewFiles()


	/**
	* Process New Files
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processNewFiles(&$asset, &$o, $prefix)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$changes = FALSE;

		for ($i = 0; $i < 2; $i++) {
			$info = get_file_upload_info($prefix.'_assoc_file_'.$i);
			if (empty($info)) continue;

			// if this is a tar archive, extract it
			if (preg_match('/\\.tar\\.gz$/', $info['name'])
				|| preg_match('/\\.tgz$/', $info['name'])
				|| preg_match('/\\.tar$/', $info['name']) ) {

				require_once 'Archive/Tar.php';
				$tar_ball =& new Archive_Tar($info['tmp_name']);
				if (($contents = $tar_ball->listContent()) != 0) {

					// Basically we want to make sure that all the files in the
					// tar ball end up in a single directory, so we have to mess about with the extracting a bit
					$extract_list = Array();
					foreach ($contents as $entry) {
						if ($entry['typeflag'] != '0' && $entry['typeflag'] != '') {
							continue;
						}

						$k = dirname($entry['filename']);
						if (!isset($extract_list[$k])) {
							$extract_list[$k] = Array();
						}
						$extract_list[$k][] = $entry['filename'];

					}//end foreach

					foreach ($extract_list as $remove_path => $files) {
						// extract the files from the tar archive to a temporary directory
						$files_dir = $asset->data_path.'/.temp_files';
						if (!create_directory($files_dir)) return FALSE;
						if (!clear_directory($files_dir)) return FALSE;

						$result = $tar_ball->extractList($files, $files_dir, $remove_path);
						if (PEAR::isError($result)) {
							trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
							break;
						} else {
							$GLOBALS['SQ_SYSTEM']->am->includeAsset('file');
							$design_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'value' => '', 'sort_order' => 1, 'is_dependant' => 1);

							foreach ($files as $filename) {
								$filename = preg_replace('|^'.$remove_path.'/|', '', $filename);
								$temp_path = $files_dir.'/'.$filename;
								$temp_info = Array('name' => $filename, 'tmp_name' => $temp_path);
								$temp_info['non_uploaded_file'] = TRUE;
								if ($this->_processUploadedFile($asset, $temp_info)) {
									$changes = TRUE;
								}
							}
						}

						delete_directory($files_dir);

					}//end foreach

				}//end if

			// just create the file asset
			} else {

				if ($this->_processUploadedFile($asset, $info)) {
					$changes = TRUE;
				}

			}//end if tar archive

		}//end for

		return $changes;

	}//end processNewFiles()


	/**
	* Creates a new file asset, populates it, and links it to $asset
	*
	* @param object	&$asset	the asset to link the file to
	* @param array	$info	info about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	function _processUploadedFile(&$asset, $info=Array())
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$existing_ids = Array();
		$existing = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'file', FALSE);
		foreach ($existing as $link) {
			$existing_ids[$link['minorid']] = $link['linkid'];
		}
		$existing_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($existing_ids));

		$existing_fileid = 0;
		foreach ($existing_info as $asset_id => $asset_info) {
			// if the name is the same, delete the link
			if ($asset_info['name'] == $info['name']) {
				$existing_fileid = $asset_id;
				break;
			}
		}

		if (!$existing_fileid) {
			// Check the filetype of the file being uploaded and create the
			// appropriate asset type
			switch (get_file_type($info['name'])) {
				case 'css' :
						$new_asset_type = 'text_file';
				break;
				case 'gif' :
				case 'jpg' :
				case 'jpeg' :
				case 'png' :
						$new_asset_type = 'image';
				break;
				default :
						$new_asset_type = 'file';
				break;
			}//end switch

			$GLOBALS['SQ_SYSTEM']->am->includeAsset($new_asset_type);
			$new_file =& new $new_asset_type();
			$new_file->_tmp['uploading_file'] = TRUE;
			$new_file->setAttrValue('name', $info['name']);
			$design_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'value' => '', 'sort_order' => 1, 'is_dependant' => 1);

			return $new_file->create($design_link, $info);

		} else {

			// we already have a design file with the same name, so just upload over the top of it
			$new_file =& $GLOBALS['SQ_SYSTEM']->am->getAsset($existing_fileid);

			$lock_status = $GLOBALS['SQ_SYSTEM']->am->acquireLock($new_file->id, 'attributes');
			$edit_fns = $new_file->getEditFns();
			$success = $edit_fns->processFileUpload($new_file, $o, $new_file->getPrefix(), $info);

			if ($lock_status === 1) {
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($new_file->id, 'attributes');
			}

			return $success;
		}

	}//end _processUploadedFile()


	/**
	* Paint New Customisation
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintNewCustomisation(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('all')) {
			$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($asset->type(), 'name');
			echo translate('core_design_must_acquire_locks_new_customisation', strtolower($type_name));
			return FALSE;
		}

		check_box($prefix.'_new_customisation');
		text_box($prefix.'_new_customisation_name', $asset->name.' - Customisation', 40);
		return TRUE;

	}//end paintNewCustomisation()


	/**
	* Process New Customisation
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processNewCustomisation(&$asset, &$o, $prefix)
	{
		if (empty($_POST[$prefix.'_new_customisation']) || !($new_name = trim($_POST[$prefix.'_new_customisation_name']))) {
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// First let's duplicate ourselves, ignoring dependants and the directory with the files
		$map = Array();
		$create_link = Array();

		$clone =& $GLOBALS['SQ_SYSTEM']->am->cloneAsset($asset, $create_link, $map, Array('attributes', 'permissions'), FALSE);
		if (is_null($clone)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// Acquire a lock on the new object
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($clone->id, 'all')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// Now let's morph the dupe into a design customisation
		if (!$clone->morph('design_customisation')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// and link it back to ourselves
		if (!$asset->createLink($clone, SQ_LINK_TYPE_2, 'customisation', null, '1')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}
		if (!($clone->setAttrValue('id_name', $new_name) && $clone->saveAttributes())) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$am =& $GLOBALS['SQ_SYSTEM']->am;

		// Now create the links to our design areas
		$da_links = $asset->getDesignAreaLink();
		foreach ($da_links as $link) {
			$da =& $am->getAsset($link['minorid'], $link['minor_type_code'], TRUE);
			if (is_null($da)) continue;
			if (!$clone->createLink($da, SQ_LINK_TYPE_3, $link['value'], null, '1')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}//end foreach

		// set an initial web path
		$initial_path = strtolower($new_name);
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($initial_path));
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($asset, $valid_paths, $clone->id, TRUE);
		if (!$clone->saveWebPaths($good_paths)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		// link up all the files from this design to the customisation
		$file_link_ids = Array();
		$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'file', FALSE);
		foreach ($file_links as $link) {
			$minor =& $am->getAsset($link['minorid'], $link['minor_type_code']);
			if (!$clone->createLink($minor, SQ_LINK_TYPE_3, '', null, 1)) {
				trigger_localised_error('CORE0162', E_USER_WARNING);
				$am->forgetAsset($minor);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}
		$am->forgetAsset($minor);

		// link up all the css files from this design to the customisation
		$css_link_ids = Array();
		$css_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'design_css', FALSE);
		foreach ($css_links as $link) {
			$minor =& $am->getAsset($link['minorid'], $link['minor_type_code']);
			if (!$clone->createLink($minor, SQ_LINK_TYPE_3, '', null, 1)) {
				trigger_localised_error('CORE0161', E_USER_WARNING);
				$am->forgetAsset($minor);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}
		$am->forgetAsset($minor);

		// release the lock on the new object
		if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($clone->id, 'all')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end processNewCustomisation()


	/**
	* Paint current customisation
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentCustomisations(&$asset, &$o, $prefix)
	{
		$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'design_customisation', TRUE, 'major', 'customisation');
		if ($customisation_links) {
			$am =& $GLOBALS['SQ_SYSTEM']->am;
			foreach ($customisation_links as $link) {
				$customisation =& $am->getAsset($link['minorid'], $link['minor_type_code']);
				if (is_null($customisation)) continue;
				echo get_asset_tag_line($customisation->id, 'details').'<br/>';
			}

		} else {
			echo translate('core_design_no_customisations_created');
		}

	}//end paintCurrentCustomisations()


	/**
	* Parses the parse file and processes the contents
	*
	* @param object	&$asset	the asset to which we belong
	*
	* @return boolean
	* @access public
	*/
	function parseAndProcessFile(&$asset)
	{
		$parse_file = $asset->data_path.'/parse.txt';
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$str = file_to_string($parse_file);
		$contents = $this->_parseString($asset, $str);
		$res = (!is_null($contents) && $this->_processContents($asset, $contents) && $asset->saveAttributes());
		return $res;

	}//end parseAndProcessFile()


	/**
	* Parses the string into an array recursively cut up into html and tag pieces
	*
	* @param object	&$asset			the asset to which we belong
	* @param string	$file_contents	the contents to parse
	*
	* @return array
	* @access private
	*/
	function _parseString(&$asset, $file_contents)
	{
		// convert the head and body tags to lowercase
		$file_contents = preg_replace(	Array('/<head/i', '/<\\/head/i', '/<body/i', '/<\\/body/i'),
										Array('<head', '</head', '<body', '</body'),
										$file_contents);


		// Let's place an print operation after the head tag
		// to allow us to print the default header stuff like
		// style sheets, JS files and meta tag stuff
		$temp = explode('<head>', $file_contents, 2);
		if (sizeof($temp) > 1) {
			$file_contents= $temp[0]
							."<head>\n<".$this->tag_name."_PRINT id_name='__global__' var='html_header' />\n"
							.$temp[1];
		}

		// Let's place an print operation after the termination body tag
		// to allow us to print the anything that has come to our attention
		// like image rollovers
		$temp = explode('</body>', $file_contents, 2);
		if (sizeof($temp) > 1) {
			$file_contents= $temp[0]
							."\n<".$this->tag_name."_PRINT id_name='__global__' var='html_footer' />\n</body>"
							.$temp[1];
		}

		return parent::_parseString($file_contents);

	}//end _parseString()


	/**
	* Loops through the contents array and creates the necessary objects
	*
	* @param object	&$asset		the asset to which we belong
	* @param array	$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	function _processContents(&$asset, $contents)
	{
		$id_names = Array();
		// let's check to see if all the design areas have unique names
		foreach ($contents as $index => $element) {
			if ($element['_type'] != 'TAG' || $element['operation'] != 'area') {
				continue;
			}

			// if there is a name and it's not the the same type as the current design area
			if (empty($element['attributes']['id_name']) || empty($element['attributes']['design_area'])) {
				trigger_localised_error('CORE0143', E_USER_WARNING);
				return FALSE;
			}//end if

			// validate id name
			if (!preg_match('/^[a-z][a-z0-9_]*$/i', $element['attributes']['id_name']) || $element['attributes']['id_name'] != preg_replace('/__+/', '', $element['attributes']['id_name'])) {
				trigger_localised_error('CORE0181', E_USER_WARNING, $element['attributes']['id_name']);
				return FALSE;
			}

			$element['attributes']['design_area'] = 'design_area_'.strtolower($element['attributes']['design_area']);

			// if there is a name and it's not the the same type as the current design area
			if (isset($id_names[$element['attributes']['id_name']])) {
				trigger_localised_error('CORE0142', E_USER_WARNING, $element['attributes']['id_name']);
				return FALSE;
			}//end if

			$id_names[$element['attributes']['id_name']] = $element['attributes']['design_area'];

		}//end foreach

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		if (isset($GLOBALS['SQ_PROCESSED_DESIGN_AREAS'])) {
			// let's remove any design area links that we aren't going to be using any more
			$existing_da_links = $asset->getDesignAreaLink();
			$existing_id_names = Array();
			foreach ($existing_da_links as $link) {
				$existing_id_names[$link['value']] = $link['linkid'];
			}

			$removed_id_names = array_diff(array_keys($existing_id_names), array_keys($GLOBALS['SQ_PROCESSED_DESIGN_AREAS']));

			// now remove any unused design area customisations
			foreach ($removed_id_names as $id_name) {
				if (!$asset->deleteLink($existing_id_names[$id_name])) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
			}//end foreach
		} else {
			$GLOBALS['SQ_PROCESSED_DESIGN_AREAS'] = Array();
		}

		// validate the design areas that we are going to use

		$design_areas = Array();

		// now let's check to see if any existing design areas exists
		// and check if they are the same type as the one in the contents
		foreach ($id_names as $id_name => $design_area) {

			$da =& $asset->getDesignArea($id_name);

			// if it's a different type, then convert them
			if (!is_null($da) && $da->type() !== $design_area) {
				$link = $asset->getDesignAreaLink($id_name, FALSE);
				if (!$asset->deleteLink($link['linkid'])) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
				unset($da); // unset so we don't set the value or all ref's to null
				$da = null; // do this so a new version will be created
			}

			// if it doesn't exist, create it, and acquire the lock
			if (is_null($da)) {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($design_area);
				$da =& new $design_area();
				$da_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_3, 'value' => $id_name, 'is_dependant' => '1');
				$da->setAttrValue('id_name', $id_name);
				if (!$da->create($da_link)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($da->id, 'parsing')) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
			}

			$design_areas[$id_name] =& $da;

		}//end foreach


		// Now that we have set all the design areas up we can get on with the processing of the contents array
		foreach ($contents as $index => $element) {
			if ($element['_type'] != 'TAG' || $element['operation'] != 'area') {
				continue;
			}

			$edit_fns = $design_areas[$element['attributes']['id_name']]->getEditFns();
			if (!$edit_fns->_processContents($design_areas[$element['attributes']['id_name']], $element['contents'])) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			$print = !in_array(array_get_index($element['attributes'], 'print', 'yes'), Array('no', 'FALSE', '0'));
			$design_areas[$element['attributes']['id_name']]->setAttrValue('print', $print);
			$cache = in_array(array_get_index($element['attributes'], 'cache', FALSE), Array('yes', 'TRUE', '1'));
			$design_areas[$element['attributes']['id_name']]->setAttrValue('cache', $cache);

			if (!$design_areas[$element['attributes']['id_name']]->saveAttributes()) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			// if they want this design area to be printed then what we need to do is alter the contents
			// array to become a _PRINT operation

			// Now what we do is morph it so that this element is now a print tag
/*
			$contents[$index] = Array(
									'_type'				=> 'TAG',
									'attributes'		=> Array ('id_name' => $element['attributes']['id_name']),
									'operation'			=> 'print',
									'self_terminating'	=> 1,
									'cache'				=> (isset($element['attributes']['cache']) && in_array($element['attributes']['cache'], Array('yes', '1', 'TRUE'))) ? 1 : 0,
								);
*/
		}//end foreach

		// now just set the contents and we are away...
		if ($asset->setAttrValue('contents', $contents)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return TRUE;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

	}//end _processContents()


}//end class

?>
