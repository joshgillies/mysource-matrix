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
* $Id: design_edit_fns.inc,v 1.69 2004/06/18 03:40:45 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
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
		$this->static_screens['details']['force_unlock'] = false;

	}//end constructor


	/**
	* Paint the interface for the parse file uploading
	*
	* @param object Asset				$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintParseFile(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('parsing')) {
			file_upload($prefix.'_parse_file');
			return true;
		}
		return false;

	}//end paintParseFile()


	/**
	* Processes the interface for the parse file uploading
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processParseFile(&$asset, &$o, $prefix)
	{
		// make sure a file was uploaded before processing it
		$upload_info = get_file_upload_info($prefix.'_parse_file');
		if (empty($upload_info)) return false;
		
		$parse_file  = $asset->data_path.'/parse.txt';
		$changes     = false;
		$new_version = false;

		$fv = &$GLOBALS['SQ_SYSTEM']->getFileVersioning();

		// delete existing parse file if it exists
		if (is_file($parse_file)) {
			$new_version = true;
			if (!unlink($parse_file)) {
				trigger_error('Failed deleting existing parse file', E_USER_WARNING);
				return false;
			}
		}

		// copy over the uploaded file
		if (commit_file_upload($prefix.'_parse_file', $parse_file, true)) {
			// tell the asset to update the customisation at the end of the interface processing
			$asset->_tmp['update_customisations'] = true;
			$changes = $this->parseAndProcessFile($asset);

			// if we are overwriting our current parse file, we need to add a new version to the repository
			if ($new_version) {
				$file_status = $fv->upToDate($parse_file);
				if (FUDGE_FV_MODIFIED & $file_status) {
					if (!$fv->commit($parse_file, '', false)) {
						trigger_error('Failed committing file version', E_USER_WARNING);
					}
				}
			} else {
				// attempt to add the parse file to the repository
				if (!$fv->add($asset->data_path_suffix, $parse_file, '', false)) {
					trigger_error('Failed adding parse file version', E_USER_WARNING);
				}
			}
		}//end if file uploaded

		// make sure we have the latest version of our file
		if (!$fv->checkOut($asset->data_path_suffix.'/parse.txt', $asset->data_path)) {
			trigger_error('Failed checking out latest version of parse file', E_USER_WARNING);
			return false;
		}

		return $changes;

	}//end processParseFile()


	/**
	* Paint the interface for the reparsing the current parse file
	*
	* @param object Asset				$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintReparseFile(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$parse_file = $asset->data_path.'/parse.txt';
			if (!is_file($parse_file)) {
				echo 'Not available - no existing parse file available';
			} else {
				check_box($prefix.'_reparse');
				echo ' Yes, manually reparse the existing file';
			}
			return true;
		}
		return false;

	}//end paintReparseFile()


	/**
	* Processes the interface for the reparsing the current parse file
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processReparseFile(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_reparse'])) {
			if($this->parseAndProcessFile($asset)) {
				$asset->_tmp['update_customisations'] = true;
				return true;
			}
		}
		return false;

	}//end processReparseFile()


	/**
	* Paint the interface for editing the parse file
	*
	* @param object Asset				$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintEditParseFile(&$asset, &$o, $prefix)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$parse_file = $asset->data_path.'/parse.txt';
		if (!is_file($parse_file)) {
			echo 'You need to upload a parse file before you can edit it';
			return true;
		}

		$str = file_to_string($parse_file);

		if ($asset->writeAccess('parsing')) {
			text_area($prefix.'_new_parse_file', str_replace("\t", "    ", $str), 100, 30, 0, 'style="font-family: monospace; white-space: pre;"');
			return true;
		} else {
			echo '<pre>', htmlspecialchars($str), '</pre>';
		}
		return false;

	}//end paintEditParseFile()


	/**
	* Processes the interface for the editing the parse file
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processEditParseFile(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_new_parse_file'])) {

			$parse_file  = $asset->data_path.'/parse.txt';
			$new_file    = $_POST[$prefix.'_new_parse_file'];

			if (!is_file($parse_file)) return false;

			$fv = &$GLOBALS['SQ_SYSTEM']->getFileVersioning();

			// delete existing parse file
			if (!unlink($parse_file)) {
				trigger_error('Failed deleting existing parse file', E_USER_WARNING);
				return false;
			}

			// copy over the uploaded file
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			if (string_to_file($new_file, $parse_file)) {
				// tell the asset to update the customisation at the end of the interface processing
				$asset->_tmp['update_customisations'] = true;
				$changes = $this->parseAndProcessFile($asset);

				// add a new version to the repository
				$file_status = $fv->upToDate($parse_file);
				if (FUDGE_FV_MODIFIED & $file_status) {
					if (!$fv->commit($parse_file, '', false)) {
						trigger_error('Failed committing file version', E_USER_WARNING);
					}
				} else {
					$changes = true;
				}
				
			} else {
				trigger_error('Failed writing parse file, trying to recover previous version from repository', E_USER_WARNING);
			}

			// make sure we have the latest version of our file
			if (!$fv->checkOut($asset->data_path_suffix.'/parse.txt', $asset->data_path)) {
				trigger_error('Failed checking out latest version of parse file', E_USER_WARNING);
				return false;
			}
			
			if ($changes) return $this->parseAndProcessFile($asset);
		}

		return false;

	}//end processEditParseFile()


	/**
	* Parses the parse file and processes the contents
	*
	* @param object Asset	$asset	the asset to which we belong
	*
	* @return boolean
	* @access public
	*/
	function parseAndProcessFile(&$asset)
	{
		$parse_file = $asset->data_path.'/parse.txt';
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$str = file_to_string($parse_file);
		$contents = $this->_parseString($str);
		return (!is_null($contents) && $this->_processContents($asset, $contents) && $asset->saveAttributes());

	}//end parseAndProcessFile()


	/**
	* Parses the string into an array recursively cut up into html and tag pieces
	*
	* @param string	$file_contents	the contents to parse
	*
	* @return Array
	* @access private
	*/
	function _parseString($file_contents)
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
		} else {
			$file_contents= "<".$this->tag_name."_PRINT id_name='__global__' var='html_header' />\n"
							.$file_contents;
		}

		// Let's place an print operation after the termination body tag
		// to allow us to print the anything that has come to our attention
		// like image rollovers
		$temp = explode('</body>', $file_contents, 2);
		if (sizeof($temp) > 1) {
			$file_contents= $temp[0]
							."\n<".$this->tag_name."_PRINT id_name='__global__' var='html_footer' />\n</body>"
							.$temp[1];
		} else {
			$file_contents= $file_contents."\n<".$this->tag_name."_PRINT id_name='__global__' var='html_footer' />";
		}

		return parent::_parseString($file_contents);

	}//end _parseString()


	/**
	* Loops through the contents array and creates the necessary objects
	*
	* @param object Asset	$asset		the asset to which we belong
	* @param array			$contents	the contents array we generated with _parseString()
	*
	* @return boolean
	* @access private
	*/
	function _processContents(&$asset, $contents)
	{
		$id_names = Array();
		// let's check to see if all the design areas have unique names
		foreach($contents as $index => $element) {
			if ($element['_type'] != 'TAG' || $element['operation'] != 'area') continue;

			// if there is a name and it's not the the same type as the current design area
			if (empty($element['attributes']['id_name']) || empty($element['attributes']['design_area'])) {
				trigger_error('All design areas need to have an "id_name" and a "design_area"', E_USER_WARNING);
				return false;
			}//end if

			// validate id name
			if (!preg_match('/^[a-z][a-z0-9_]*$/i', $element['attributes']['id_name']) || $element['attributes']['id_name'] != preg_replace('/__+/', '', $element['attributes']['id_name'])) {
				trigger_error('The id_name "'.$element['attributes']['id_name'].'" contains illegal characters, the id_name must be an alpha character, optionally followed by any number of alphanumeric characters or underscores.<br>NOTE: you cannot have more that one consecutive underscore', E_USER_WARNING);
				return false;
			}

			$element['attributes']['design_area'] = 'design_area_'.strtolower($element['attributes']['design_area']);

			// if there is a name and it's not the the same type as the current design area
			if (isset($id_names[$element['attributes']['id_name']])) {
				trigger_error('All design areas MUST have a unique id_name, "'.$element['attributes']['id_name'].'" is used at least twice', E_USER_WARNING);
				return false;
			}//end if

			$id_names[$element['attributes']['id_name']] = $element['attributes']['design_area'];

		}// end foreach
		
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		
		if (isset($GLOBALS['SQ_PROCESSED_DESIGN_AREAS'])) {
			// let's remove any design area links that we aren't going to be using any more
			$existing_da_links = $asset->getDesignAreaLink();
			$existing_id_names = Array();
			foreach ($existing_da_links as $link) $existing_id_names[$link['value']] = $link['linkid'];

			$removed_id_names = array_diff(array_keys($existing_id_names), array_keys($GLOBALS['SQ_PROCESSED_DESIGN_AREAS']));

			// now remove any unused design area customisations
			foreach($removed_id_names as $id_name) {
				if (!$asset->deleteLink($existing_id_names[$id_name])) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}// end foreach
		} else {
			$GLOBALS['SQ_PROCESSED_DESIGN_AREAS'] = Array();
		}


		// validate the design areas that we are going to use

		$design_areas = Array();

		// now let's check to see if any existing design areas exists
		// and check if they are the same type as the one in the contents
		foreach($id_names as $id_name => $design_area) {

			$da = &$asset->getDesignArea($id_name);

			// if it's a different type, then convert them
			if (!is_null($da) && $da->type() !== $design_area) {
				$link = &$asset->getDesignAreaLink($id_name, false);
				if (!$asset->deleteLink($link['linkid'])) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
				unset($da); // unset so we don't set the value or all ref's to null
				$da = null; // do this so a new version will be created
			}

			// if it doesn't exist, create it, and acquire the lock
			if (is_null($da)) {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($design_area);
				$da = new $design_area();
				$da_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_3, 'value' => $id_name, 'dependant' => '1');
				$da->setAttrValue('id_name', $id_name);
				if (!$da->create($da_link)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($da->id, 'parsing')) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
			}

			$design_areas[$id_name] = &$da;

		}// end foreach

		// Now that we have set all the design areas up we can get on with the processing of the contents array
		$remove_indexes = Array();
		foreach($contents as $index => $element) {
			if ($element['_type'] != 'TAG' || $element['operation'] != 'area') continue;

			// if there isn't any print var, default to 'yes'
			if (!isset($element['attributes']['print'])) $element['attributes']['print'] = 'yes';

			$edit_fns = $design_areas[$element['attributes']['id_name']]->getEditFns();
			if (!$edit_fns->_processContents($design_areas[$element['attributes']['id_name']], $element['contents'])) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
			if (!$design_areas[$element['attributes']['id_name']]->saveAttributes()) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}


			// if they want this design area to be printed then what we need to do is alter the contents
			// array to become a _PRINT operation
			switch($element['attributes']['print']) {
				case 'yes'  : 
				case 'true' : 
				case '1'    : 
					// Now what we do is morph it so that this element is now a print tag
					$contents[$index] = Array(
												'_type' => 'TAG',
												'attributes'  => Array ('id_name' => $element['attributes']['id_name']),
												'operation' => 'print',
												'self_terminating' => 1
											);
					break;
				// otherwise we just remove it as there is nothing for us to do with it anymore
				default : 
					$remove_indexes[] = $index;
			}// end switch

		}//end foreach

		// Now remove the elems
		// reverse the order so we don't throw the numbers out of whack by removing one from the start
		rsort($remove_indexes, SORT_NUMERIC);
		foreach($remove_indexes as $i) {
			array_splice($contents, $i, 1);
		}

		// now just set the contents and we are away...
		if ($asset->setAttrValue('contents', $contents)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

	}//end _processContents()


	/**
	* Creates the (hopefully ;) single accumulated design file for this design
	*
	* @param object Asset	$asset	the asset to which we belong
	*
	* @return boolean
	* @access private
	*/
	function createDesignFile(&$asset)
	{
		$design_area_links = $asset->getDesignAreaLink();

		// Firstly we need to set all the var_reference variables so that when the design areas are
		// printed they can work correctly
		//
		// This is a two stage process, one to set any normal vars and two to set any vars that want DA
		// output

		$var_ref_values = Array();
		$da_refs = Array();
		foreach($design_area_links as $link) {
			$da = &$asset->getDesignArea($link['value']);
			if (is_null($da)) continue;

			$da_var_refs = $da->getVarReferences();
			for(reset($da_var_refs); null !== ($id_name = key($da_var_refs)); next($da_var_refs)) {
				$var_da = &$asset->getDesignArea($id_name);
				if (is_null($var_da)) continue;
				if (!isset($var_ref_values[$id_name])) $var_ref_values[$id_name] = Array();
				foreach($da_var_refs[$id_name] as $var) {
					// does this var want the whole design area ?
					if ($var === '') {
						$da_refs[] = $id_name;
					// else just a normal var (make sure we don't have it already)
					} elseif (!isset($var_ref_values[$id_name][$var])) {
						$var_ref_values[$id_name][$var] = $var_da->attr($var);

					}// end if

				}// end foreach

			}// end foreach

		}// end foreach

		// OK, now that we have all the var reference values we can just set them for all the DA's
		foreach($design_area_links as $link) {
			$da = &$asset->getDesignArea($link['value']);
			if (is_null($da)) continue;
			$da->setVarReferenceValues($var_ref_values);
		}// end foreach

		// Now that all the all the DA's have their vars set, we can do the DA output ones
		$da_ref_values = Array();
		foreach($da_refs as $id_name) {
			$da = &$asset->getDesignArea($id_name);
			if (is_null($da)) continue;
			ob_start();
			$da->printArea($asset);
			// NOTE: format like this because we are calling setVarReferenceValues()
			$da_ref_values[$id_name] = Array('' => ob_get_contents());
			ob_end_clean();
		}// end foreach

		// OK, now that we have all the da reference values we can just set them for all the DA's
		foreach($design_area_links as $link) {
			$da = &$asset->getDesignArea($link['value']);
			if (is_null($da)) continue;
			$da->setVarReferenceValues($da_ref_values);
		}// end foreach

		// we are checking to see if there is a global $ASSET_LINEAGE here
		// because we need other assets to be able to modify the lineage, for
		// instance 404 pages.

		$pre_str = '<?php
		global $ASSET_LINEAGE;
		$am = &$GLOBALS[\'SQ_SYSTEM\']->am;
		if (!isset($ASSET_LINEAGE) || empty($ASSET_LINEAGE)) $ASSET_LINEAGE = @$am->getLineageFromURL(null, null);
		define(\'SQ_LIMBO_EDITING\', (SQ_IN_LIMBO && (!isset($_REQUEST[\'ignore_frames\']) || !$_REQUEST[\'ignore_frames\'])));
		?>';

		$str = '';
		ob_start();
		if ($this->_createDesignFileOutput($asset->attr('contents'), $asset, $asset)) {
			// get the contents, removing any close then open tags that happen right next to each other
			// (more for code neatness than anything :)
			$str = str_replace('?'.'><'.'?php', '', ob_get_contents());
		}
		ob_end_clean();
		if (!$str) return false;
		$str = $pre_str."\n".$str;

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		create_directory($asset->data_path);
		$file = $asset->data_path.'/design_file.php';
		$new_file = !file_exists($file);
		if (!string_to_file($str, $file)) return false;


		//// FILE VERSIONING ////

		$fv = &$GLOBALS['SQ_SYSTEM']->getFileVersioning();

		if ($new_file) {
			// attempt to add the file to the repository
			if (!$fv->add($asset->data_path_suffix, $file, '', false)) {
				trigger_error('Failed adding design file version', E_USER_WARNING);
				return false;
			}
		} else {
			// updating existing file
			$file_status = $fv->upToDate($file);
			if (FUDGE_FV_MODIFIED & $file_status) {
				if (!$fv->commit($file, '', false)) {
					trigger_error('Failed committing design file version', E_USER_WARNING);
					return false;
				}
			}
		}

		// make sure we have the latest version of our file
		if (!$fv->checkOut($asset->data_path_suffix.'/design_file.php', $asset->data_path)) {
			trigger_error('Failed checking out latest version of design file', E_USER_WARNING);
			return false;
		}

		return true;

	}//end createDesignFile()


	/**
	*
	* @param object Asset				$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintNewFiles(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('links')) return false;
		for($i = 0; $i < 2; $i++) {
			file_upload($prefix.'_assoc_file_'.$i);
			echo "<br>\n";
		}

	}//end paintParseFile()


	/**
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processNewFiles(&$asset, &$o, $prefix)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$changes = false;

		for($i = 0; $i < 2; $i++) {
			$info = get_file_upload_info($prefix.'_assoc_file_'.$i);
			if (empty($info)) continue;

			// if this is a tar archive, extract it
			if (preg_match('/\\.tar\\.gz$/', $info['name'])
				|| preg_match('/\\.tgz$/', $info['name'])
				|| preg_match('/\\.tar$/', $info['name']) ) {

				require_once 'Archive/Tar.php';
				$tar_ball = new Archive_Tar($info['tmp_name']);
				if (($contents = $tar_ball->listContent()) != 0) {

					// Basically we want to make sure that all the files in the
					// tar ball end up in a single directory, so we have to fuck about with the extracting a bit
					$extract_list = Array();
					foreach($contents as $entry) {
						if ($entry['typeflag'] != '0' && $entry['typeflag'] != '') continue;

						$k = dirname($entry['filename']);
						if (!isset($extract_list[$k])) $extract_list[$k] = Array();
						$extract_list[$k][] = $entry['filename'];

					}// end foreach

					foreach($extract_list as $remove_path => $files) {
						// extract the files from the tar archive to a temporary directory
						$files_dir = $asset->data_path.'/.temp_files';
						if (!create_directory($files_dir)) return false;
						if (!clear_directory($files_dir)) return false;

						$result = $tar_ball->extractList($files, $files_dir, $remove_path);
						if (PEAR::isError($result)) {
							trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
							break;
						} else {
							$GLOBALS['SQ_SYSTEM']->am->includeAsset('file');
							$design_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'value' => '', 'sort_order' => 1, 'dependant' => 1);

							foreach ($files as $filename) {
								$filename = preg_replace('|^'.$remove_path.'/|', '', $filename);
								$temp_path = $files_dir.'/'.$filename;
								$temp_info = Array('name' => $filename, 'tmp_name' => $temp_path);
								$temp_info['non_uploaded_file'] = true;
								if ($this->_processUploadedFile($asset, $temp_info)) $changes = true;
							}
						}

						delete_directory($files_dir);

					}// end foreach

				}// end if

			// just create the file asset
			} else {

				if ($this->_processUploadedFile($asset, $info)) $changes = true;

			}// end if tar archive

		}// end for

		return $changes;

	}//end processNewFiles()


	/**
	* Creates a new file asset, populates it, and links it to $asset
	* @param object Asset	$asset	the asset to link the file to
	* @param array			$info	info about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	function _processUploadedFile(&$asset, $info=Array())
	{
		$existing_ids = Array();
		$existing = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'file', false);
		foreach ($existing as $link) $existing_ids[$link['minorid']] = $link['linkid'];
		$existing_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($existing_ids));
		
		$existing_fileid = 0;
		foreach ($existing_info as $existing_info) {
			// if the name is the same, delete the link
			if ($existing_info['name'] == $info['name']) {
				$existing_fileid = $existing_info['assetid'];
				break;
			}
		}

		if (!$existing_fileid) {

			$GLOBALS['SQ_SYSTEM']->am->includeAsset('file');
			$new_file =& new File();
			$new_file->_tmp['uploading_file'] = true;
			$new_file->setAttrValue('name', $info['name']);
			$design_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'value' => '', 'sort_order' => 1, 'dependant' => 1);

			if (!$new_file->create($design_link, $info)) return false;

		} else {

			// we already have a design file with the same name, so just upload over the top of it
			$new_file = &$GLOBALS['SQ_SYSTEM']->am->getAsset($existing_fileid);
		}

		$lock_status = $GLOBALS['SQ_SYSTEM']->am->acquireLock($new_file->id, 'attributes');
		$edit_fns = $new_file->getEditFns();
		if ($edit_fns->processFileUpload($new_file, $o, $new_file->getPrefix(), $info)) {
			if ($lock_status === 1) $GLOBALS['SQ_SYSTEM']->am->releaseLock($new_file->id, 'attributes');
			return true;
		}
		if ($lock_status === 1) $GLOBALS['SQ_SYSTEM']->am->releaseLock($new_file->id, 'attributes');

		return false;

	}//end _processUploadedFile()


	/**
	*
	* @param object Asset				$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintNewCustomisation(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('links')) return false;
		check_box($prefix.'_new_customisation');
		text_box($prefix.'_new_customisation_name', $asset->name.' - Customisation', 40);
		return true;

	}//end paintNewCustomisation()


	/**
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processNewCustomisation(&$asset, &$o, $prefix)
	{
		if (empty($_POST[$prefix.'_new_customisation']) || !($new_name = trim($_POST[$prefix.'_new_customisation_name']))) {
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// First let's duplicate ourselves, ignoring dependants and the directory with the files
		$map = Array();
		$create_link = Array();

		$clone = &$GLOBALS['SQ_SYSTEM']->am->cloneAsset($asset, $create_link, $map, Array('attributes', 'permissions'), false);
		if (is_null($clone)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->am->rememberAsset($da);

		// Acquire a lock on the new object
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($clone->id, 'all')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// Now let's morph the dupe into a design customisation
		if (!$clone->morph('design_customisation')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// and link it back to ourselves
		if (!$asset->createLink($clone, SQ_LINK_TYPE_2, 'customisation', null, '1')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		if (!($clone->setAttrValue('id_name', $new_name) && $clone->saveAttributes())) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$am = &$GLOBALS['SQ_SYSTEM']->am;

		// Now create the links to our design areas
		$da_links = $asset->getDesignAreaLink();
		foreach($da_links as $link) {
			$da = &$am->getAsset($link['minorid'], $link['minor_type_code'], true);
			if (is_null($da)) continue;
			if (!$clone->createLink($da, SQ_LINK_TYPE_3, $link['value'], null, '1')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}// end foreach

		// set an initial web path
		$initial_path = strtolower($new_name);
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		$valid_paths = make_valid_web_paths(Array($initial_path));
		$good_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($asset, $valid_paths, $clone->id, true);
		if (!$clone->saveWebPaths($good_paths)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// link up all the files from this design to the customisation
		$file_link_ids = Array();
		$file_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2 | SQ_LINK_TYPE_3, 'file', false);
		foreach ($file_links as $link) {
			$minor = &$am->getAsset($link['minorid'], $link['minor_type_code']);
			if (!$clone->createLink($minor, SQ_LINK_TYPE_3, '', null, 1)) {
				trigger_error('Failed creating file link to customisation', E_USER_WARNING);
				$am->forgetAsset($minor);
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}
		$am->forgetAsset($minor);

		// release the lock on the new object
		if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($clone->id, 'all')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end processNewCustomisation()


	/**
	*
	* @param object Asset				$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentCustomisations(&$asset, &$o, $prefix)
	{
		$customisation_links = &$GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'design_customisation', true, 'major', 'customisation');
		if ($customisation_links) {
			$am = &$GLOBALS['SQ_SYSTEM']->am;
			foreach($customisation_links as $link) {
				$customisation = &$am->getAsset($link['minorid'], $link['minor_type_code']);
				if (is_null($customisation)) continue;
				echo '# ', $customisation->id, '&nbsp;&nbsp;<a href="', $customisation->getBackendHref(), '">', $customisation->name, '</a><br>';

			}

		} else {
			?> No Customisations Created <?php
		}

	}//end paintCurrentCustomisations()


}//end class

?>
