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
* @package Resolve::packages::__core__
*/
class Design_Edit_Fns extends Design_Area_Edit_Fns
{

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
		file_upload($prefix.'_parse_file');
	}// end paintParseFile()

	/**
	* Processes the URLs posted from the paintURLs fn above
	*
	* @param object Asset	$asset	the asset to which we belong
	* @param string			$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processParseFile(&$asset, $prefix)
	{
		$parse_file = $asset->data_path.'/parse.txt';

		if (commit_file_upload($prefix.'_parse_file', $parse_file, true)) {
			// tell the asset to update the customisation at the end of the interface processing
			$asset->_tmp['update_customisations'] = true;

			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			$str = file_to_string($parse_file);
			$contents = $this->_parseString($str);

			if (!is_null($contents) && $this->_processContents($asset, $contents)) {
				$asset->_tmp['regenerate_design'] = true;
			}
		}
		return true;
	}// end processParseFile()

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
		// firstly let's check to see if all the design areas have unique names
		foreach($contents as $index => $element) {
			if ($element['_type'] != 'TAG' || $element['operation'] != 'area') continue;

			// if there is a name and it's not the the same type as the current design area
			if (empty($element['attributes']['id_name']) || empty($element['attributes']['design_area'])) {
				trigger_error('All design areas need to have an "id_name" and a "design_area"', E_USER_WARNING);
				return false;
			}//end if

			$element['attributes']['design_area'] = 'design_area_'.strtolower($element['attributes']['design_area']);

			// if there is a name and it's not the the same type as the current design area
			if (isset($id_names[$element['attributes']['id_name']])) {
				trigger_error('All design areas MUST have a unique id_name, "'.$element['attributes']['id_name'].'" is used at least twice', E_USER_WARNING);
				return false;
			}//end if

			$id_names[$element['attributes']['id_name']] = $element['attributes']['design_area'];

		}// end foreach

		//// FIRST, let's remove any design area links that we aren't going to be using any more ////

		$existing_da_links = $asset->getDesignAreaLink();
		$existing_id_names = Array();
		foreach ($existing_da_links as $link) $existing_id_names[$link['value']] = $link['linkid'];

		$removed_id_names = array_diff(array_keys($existing_id_names), array_keys($id_names));
		
		// now remove any unused design area customisations
		foreach($removed_id_names as $id_name) {
			if (!$asset->deleteLink($existing_id_names[$id_name])) return false;
		}// end foreach


		//// SECOND, let's validate the design areas that we are going to use ////

		$design_areas = Array();

		// now let's check to see if any existing design areas exists
		// and check if they are the same type as the one in the contents
		foreach($id_names as $id_name => $design_area) {

			$da = &$asset->getDesignArea($id_name);

			// if it's a different type, then convert them
			if (!is_null($da) && $da->type() !== $design_area) {
				$link = &$asset->getDesignAreaLink($id_name, false);
				if (!$asset->deleteLink($link['linkid'])) return false;
				unset($da); // unset so we don't set the value or all ref's to null
				$da = null; // do this so a new version will be created
			}

			// if it doesn't exist, create it
			if (is_null($da)) {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($design_area);
				$da = new $design_area();
				if ($da->create($id_name)) {
					$GLOBALS['SQ_SYSTEM']->am->registerAsset($da);
					$asset->createLink($da, SQ_LINK_USES, $id_name);
				}// end if
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
			$edit_fns->_processContents($design_areas[$element['attributes']['id_name']], $element['contents']);

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
		return $asset->setAttrValue('contents', $contents);

	}// end _processContents()


	/**
	* Creates the (hopefully ;) single accumulated design file for this design
	*
	* @param object Asset	$asset		the asset to which we belong
	*
	* @access private
	*/
	function createDesignFile(&$asset)
	{

		$design_area_links = $asset->getDesignAreaLink();

		pre_echo('------- Design : '.$asset->attr('id_name').' ------------');

		foreach($design_area_links as $link) {
			$da = &$asset->getDesignArea($link['value']);
			if (is_null($da)) continue;
			$vars = Array();
#			pre_echo('------- '.$da->attr('id_name').' ------------');
#			pre_echo($da->var_references);
			foreach($da->vars['var_references']['value'] as $var_name => $info) {
				$var_da = &$asset->getDesignArea($info['id_name']);
				if (is_null($var_da)) continue;
				$vars[$var_name] = $var_da->attr($info['var']);
			}// end foreach
#			pre_echo('-------------------');
#			pre_echo($vars);
			$da->setVarReferenceValues($vars);
		}// end foreach

		$pre_str = '<?php $FILE_PREFIX = $DATA_PATH.\'/'.addslashes($asset->webFileSuffix('')).'\'; ?>';

		$str = '';
		ob_start();
		if ($this->_createDesignFileOutput($asset, $asset)) {
			$str = ob_get_contents();
		}
		ob_end_clean();

#		pre_echo("STR : \n".$str);
		if (!$str) return false;
		pre_echo("STR EXISTS");


		$str = $pre_str."\n\n".$str;

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		create_directory($asset->data_path);
		$file = $asset->data_path.'/design_file.php';
		pre_echo($file);
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
				<td class="sq-backend-data"><i>Remove ?</i></td>
			</tr>
 		<?php
		foreach($files as $file) {
			$mod_time = filemtime($files_dir.'/'.$file);
		?>
			<tr>
				<td class="sq-backend-data">
					<a href="<?php echo sq_web_path('base').'/__data/'.$asset->webFileSuffix($file); ?>" target="_blank"><?php echo $file; ?></a>
				</td>
				<td class="sq-backend-data">
					<?php echo $GLOBALS['SQ_SYSTEM']->datetime($mod_time); ?>
				</td>
				<td class="sq-backend-data" align="center">
					<input type="checkbox" name="<?php echo $prefix; ?>_remove_files[]" value="<?php echo $file; ?>">
				</td>
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
	* @param object Asset	$asset	the asset to which we belong
	* @param string			$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processCurrentFiles(&$asset, $prefix)
	{
		if (empty($_POST[$prefix.'_remove_files'])) return;
		$asset->_tmp['regenerate_design'] = true;
		foreach($_POST[$prefix.'_remove_files'] as $file) {
			$full_file = $asset->data_path.'/files/'.$file;
			unlink($full_file);
		}
		return true;

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
		for($i = 0; $i < 2; $i++) {
			file_upload($prefix.'_assoc_file_'.$i);
			echo "<br>\n";
		}
	}// end paintParseFile()

	/**
	*
	* @param object Asset	$asset	the asset to which we belong
	* @param string			$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processNewFiles(&$asset, $prefix)
	{
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';

		$files_dir = $asset->data_path.'/files';
		// attempt to create it (if it doesn't already exist)
		if (!create_directory($files_dir)) return false;

		for($i = 0; $i < 2; $i++) {
			$info = get_file_upload_info($prefix.'_assoc_file_'.$i);
			if (empty($info)) continue;

			$asset->_tmp['regenerate_design'] = true;

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
						pre_echo($files);
						pre_echo($remove_path);
						pre_echo('----------------');

						$result = $tar_ball->extractList($files, $files_dir, $remove_path);
						if (PEAR::isError($result)) {
							trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
							return;
						}
					}// end foreach

				}// end if

			// just move it to the right place
			} else {
					commit_file_upload($prefix.'_assoc_file_'.$i, $files_dir, true);

			}// end switch

		}// end for

		return true;

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
		check_box($prefix.'_new_customisation');
		text_box($prefix.'_new_customisation_name', $asset->name.' - Customisation', 40);
	}// end paintNewCustomisation()

	/**
	*
	* @param object Asset	$asset	the asset to which we belong
	* @param string			$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processNewCustomisation(&$asset, $prefix)
	{
		if (empty($_POST[$prefix.'_new_customisation']) || !($new_name = trim($_POST[$prefix.'_new_customisation_name']))) return false;

		// First let's duplicate ourselves, ignoring the directory with the files
		$dupe = &$asset->duplicate(false);
		if (is_null($dupe)) return false;

		// Now let's morph the dupe into a design customisation
		if (!$dupe->morph('design_customisation')) return false;

		// and link it back to ourselves
		$asset->createLink($dupe, SQ_LINK_UNITE, 'customisation');
		$dupe->setAttrValue('id_name', $new_name);

		$am = &$GLOBALS['SQ_SYSTEM']->am;

		// Now create the links to our design areas
		$da_links = $asset->getDesignAreaLink();
		foreach($da_links as $link) {
			$da = &$am->getAsset($link['minorid'], $link['type_code'], true);
			if (is_null($da)) continue;
			$dupe->createLink($da, SQ_LINK_USES, $link['value']);
		}// end foreach

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
		$customisation_links = &$asset->getLinks(SQ_LINK_UNITE, 'design_customisation', true, 'major', 'customisation');
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
