<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area_edit_fns.inc';

/**
* Design_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Design_Edit_Fns extends Design_Area_Edit_Fns
{

	function Design_Edit_Fns() 
	{
		$this->Design_Area_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = false;
	}

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
		if ($asset->writeAccess()) {
			file_upload($prefix.'_parse_file');
		}

		if (file_exists($asset->data_path.'/parse.txt')) {
		?>
			<a href="<?php echo sq_web_path('data').'/'.$asset->type().'/'.$asset->id; ?>/parse.txt" target="_blank">Current Parse File</a>
		<?php
		}// endif

	}// end paintParseFile()


	/**
	* Processes the URLs posted from the paintURLs fn above
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
		$parse_file = $asset->data_path.'/parse.txt';
		$changes = false;
		if (commit_file_upload($prefix.'_parse_file', $parse_file, true)) {
			// tell the asset to update the customisation at the end of the interface processing
			$asset->_tmp['update_customisations'] = true;
			$changes = $this->parseAndProcessFile($asset);
		}
		return $changes;
	}// end processParseFile()


	/**
	* Parses the parse file and processes the contents
	*
	* @param object Asset				$asset	the asset to which we belong
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
		return (!is_null($contents) && $this->_processContents($asset, $contents));

	}// end parseAndProcessFile()


	/**
	* Parses the string into an array recursively cut up into html and tag pieces
	*
	* @param string	$file_contents		the contents to parse
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
							."<head>\n<".SQ_SYSTEM_SHORT_NAME."_PRINT id_name='__global__' var='html_header' />\n"
							.$temp[1];
		} else {
			$file_contents= "<".SQ_SYSTEM_SHORT_NAME."_PRINT id_name='__global__' var='html_header' />\n"
							.$file_contents;
		}

		// Let's place an print operation after the termination body tag
		// to allow us to print the anything that has come to our attention
		// like image rollovers
		$temp = explode('</body>', $file_contents, 2);
		if (sizeof($temp) > 1) {
			$file_contents= $temp[0]
							."\n<".SQ_SYSTEM_SHORT_NAME."_PRINT id_name='__global__' var='html_footer' />\n</body>"
							.$temp[1];
		} else {
			$file_contents= $file_contents."\n<".SQ_SYSTEM_SHORT_NAME."_PRINT id_name='__global__' var='html_footer' />";
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
		//// FIRST, let's check to see if all the design areas have unique names
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

		//// SECOND, let's remove any design area links that we aren't going to be using any more ////

		$existing_da_links = $asset->getDesignAreaLink();
		$existing_id_names = Array();
		foreach ($existing_da_links as $link) $existing_id_names[$link['value']] = $link['linkid'];

		$removed_id_names = array_diff(array_keys($existing_id_names), array_keys($id_names));
		
		// now remove any unused design area customisations
		foreach($removed_id_names as $id_name) {
			if (!$asset->deleteLink($existing_id_names[$id_name])) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}// end foreach


		//// THIRD, let's validate the design areas that we are going to use ////

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

			// if it doesn't exist, create it
			if (is_null($da)) {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($design_area);
				$da = new $design_area();
				$da_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_3, 'value' => $id_name, 'dependant' => '1');
				$da->setAttrValue('id_name', $id_name);
				if (!$da->create($da_link)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
				$GLOBALS['SQ_SYSTEM']->am->registerAsset($da);
			}// end if

			$design_areas[$id_name] = &$da;

		}// end foreach

		// Now that we have set all the design areas up we can get on with the processing of the contents array
		$remove_indexes = Array();
		foreach($contents as $index => $element) {
			if ($element['_type'] != 'TAG' || $element['operation'] != 'area') continue;

			// if there isn't any print var, default to 'yes'
			if (empty($element['attributes']['print'])) $element['attributes']['print'] = 'yes';

			$edit_fns = $design_areas[$element['attributes']['id_name']]->getEditFns();
			if (!$edit_fns->_processContents($design_areas[$element['attributes']['id_name']], $element['contents'])) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}


			// if they want this design area to be printed then what we need to do is alter the contents
			// array to become a _PRINT operation
			if ($element['attributes']['print'] == 'yes') {
				// Now what we do is morph it so that this element is now a print tag
				$contents[$index] = Array(
											'_type' => 'TAG',
											'attributes'  => Array ('id_name' => $element['attributes']['id_name']),
											'operation' => 'print',
											'self_terminating' => 1
										);
			// otherwise we just remove it as there is nothing for us to do with it anymore
			} else {
				$remove_indexes[] = $index;
			}

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

	}// end _processContents()


	/**
	* Creates the (hopefully ;) single accumulated design file for this design
	*
	* @param object Asset	$asset		the asset to which we belong
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


		$pre_str = '<?php 
		$FILE_PREFIX = $DATA_PATH.\'/'.addslashes($asset->webFileSuffix('')).'\'; 
		$am = &$GLOBALS[\'SQ_SYSTEM\']->am;
		?>';

		$str = '';
		ob_start();
		if ($this->_createDesignFileOutput($asset->attr('contents'), $asset, $asset)) {
			$str = ob_get_contents();
		}
		ob_end_clean();

		if (!$str) return false;


		$str = $pre_str."\n".$str;

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		create_directory($asset->data_path);
		$file = $asset->data_path.'/design_file.php';
		return string_to_file($str, $file);

	}// end createDesignFile()


	/**
	*
	* @param object Asset				$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentFiles(&$asset, &$o, $prefix)
	{
		$files_dir = $asset->data_path.'/files';
		$files = Array();
		$d = @dir($files_dir);
		if ($d !== FALSE) {
			while (false !== ($entry = $d->read())) {
				if (is_dir($d->path.'/'.$entry)) continue;
				if (is_file($d->path.'/'.$entry)) {
					$files[] = $entry;
				}
			}
			$d->close();
			sort($files);
		}
		?>
		<table cellspacing="2" cellpadding="2" border="0">
			<tr>
				<td class="sq-backend-data"><i>File</i></td>
				<td class="sq-backend-data"><i>Last Modified</i></td>
		<?php if ($asset->writeAccess()) { ?>
				<td class="sq-backend-data"><i>Remove ?</i></td>
		<?php }// end if ?>
			</tr>
 		<?php
		foreach($files as $file) {
			$mod_time = filemtime($files_dir.'/'.$file);
		?>
			<tr>
				<td class="sq-backend-data">
					<a href="<?php echo sq_web_path('data').$asset->webFileSuffix($file); ?>" target="_blank"><?php echo $file; ?></a>
				</td>
				<td class="sq-backend-data">
					<?php echo $GLOBALS['SQ_SYSTEM']->datetime($mod_time); ?>
				</td>
			<?php if ($asset->writeAccess()) { ?>
				<td class="sq-backend-data" align="center">
					<input type="checkbox" name="<?php echo $prefix; ?>_remove_files[]" value="<?php echo $file; ?>">
				</td>
			<?php }// end if ?>
			</tr>
		<?php

		}// end for

		if (empty($files)) {
		?>
			<tr>
				<td class="sq-backend-data" colspan="3">No Files Uploaded</td>
			</tr>
		<?php
		}// end if
		?>
		</table>
		<?php

	}// end paintCurrentFiles()

	/**
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processCurrentFiles(&$asset, &$o, $prefix)
	{
		if (empty($_POST[$prefix.'_remove_files'])) return false;
		$changes = false;
		foreach($_POST[$prefix.'_remove_files'] as $file) {
			$full_file = $asset->data_path.'/files/'.$file;
			unlink($full_file);
			$changes = true;
		}
		return $changes;

	}// end processCurrentFiles()

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
		if (!$asset->writeAccess()) return false;
		for($i = 0; $i < 2; $i++) {
			file_upload($prefix.'_assoc_file_'.$i);
			echo "<br>\n";
		}
	}// end paintParseFile()

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

		$files_dir = $asset->data_path.'/files';
		// attempt to create it (if it doesn't already exist)
		if (!create_directory($files_dir)) return false;

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
#						pre_echo($files);
#						pre_echo($remove_path);
#						pre_echo('----------------');

						$result = $tar_ball->extractList($files, $files_dir, $remove_path);
						if (PEAR::isError($result)) {
							trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
							break;
						} else {
							$changes = true;
						}

					}// end foreach

				}// end if

			// just move it to the right place
			} else {
				if (commit_file_upload($prefix.'_assoc_file_'.$i, $files_dir, true)) {
					$changes = true;
				}

			}// end switch

		}// end for

		return $changes;

	}// end processNewFiles()

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
		if (!$asset->writeAccess()) return false;
		check_box($prefix.'_new_customisation');
		text_box($prefix.'_new_customisation_name', $asset->name.' - Customisation', 40);
	}// end paintNewCustomisation()

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
		$dupe = &$asset->duplicate($create_link, $map, false, false);
		if (is_null($dupe)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->am->registerAsset($da);

		// Aquire a lock on the new object
		if (!$dupe->aquireLock()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// Now let's morph the dupe into a design customisation
		if (!$dupe->morph('design_customisation')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// and link it back to ourselves
		if (!$asset->createLink($dupe, SQ_LINK_TYPE_2, 'customisation', null, '1')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		if (!$dupe->setAttrValue('id_name', $new_name)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}
		
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		// Now create the links to our design areas
		$da_links = $asset->getDesignAreaLink();
		foreach($da_links as $link) {
			$da = &$am->getAsset($link['minorid'], $link['type_code'], true);
			if (is_null($da)) continue;
			if (!$dupe->createLink($da, SQ_LINK_TYPE_3, $link['value'], null, '1')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				return false;
			}
		}// end foreach

		// Aquire a lock on the new object
		if (!$dupe->releaseLock()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}// end processNewCustomisation()

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
		$customisation_links = &$asset->getLinks(SQ_LINK_TYPE_2, 'design_customisation', true, 'major', 'customisation');
		if ($customisation_links) {
			$am = &$GLOBALS['SQ_SYSTEM']->am;
			foreach($customisation_links as $link) {
				$customisation = &$am->getAsset($link['minorid'], $link['type_code']);
				if (is_null($customisation)) continue;
				echo '# ', $customisation->id, '&nbsp;&nbsp;<a href="', $customisation->getBackendHref(), '">', $customisation->name, '</a><br>';

			}

		} else {
			?> No Customisations Created <?php
		}
	}// end paintCurrentCustomisations()

}//end class
?>
