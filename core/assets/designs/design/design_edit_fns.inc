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
		$info = get_file_upload_info($prefix.'_parse_file');
		if (!empty($info)) {
			require_once SQ_SQUIZLIB_PATH.'/general/file_system.inc';
			$str = file_to_string($info['tmp_name']);
			$contents = $this->_parseString($str, true);
#			pre_echo("Contents : ".($contents === null));
#			pre_echo($contents);

			if ($contents !== null) {
#				pre_echo("PROCESS CONTENTS");
				if ($this->_processContents($asset, $contents)) {
					pre_echo("CREATE DESIGN FILE");
					$this->createDesignFile($asset, $contents);
				}
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
	function _processContents(&$asset, &$contents) 
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

pre_echo($id_names);

		$design_areas = Array();

		// now let's check to see if any existing design areas exists 
		// and check if they are the same type as the one in the contents
		foreach($id_names as $id_name => $design_area) {

			$da = &$asset->getDesignArea($id_name);

			// if it's a different type, then convert them
			if ($da !== null && $da->type() !== $design_area) {
				$link = &$asset->getDesignAreaLink($id_name, false);
				if (!$asset->deleteLink($link['linkid'])) return false;
				$da = null; // do this so a new version will be created
			}

			// if it doesn't exist, create it
			if ($da === null) {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($design_area);
				$da = new $design_area();
				if ($da->create($id_name)) {
					$GLOBALS['SQ_SYSTEM']->am->registerAsset($da);
					$asset->createLink($da, SQ_LINK_UNITE, $id_name);
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
				// NOw what we do is morph it so that this element is now a print tag
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
		// reverse the order so we don't through the numbers out of whack by removing one from the start
		rsort($remove_indexes, SORT_NUMERIC);
		foreach($remove_indexes as $i) {
			array_splice($contents, $i, 1);
		}

		return true;

		
	}// end _processContents()


	/**
	* Creates the accumulated single (hopefully) design file for this design
	*
	* @param object Asset	$asset		the asset to which we belong
	* @param array			$contents	the contents array we generated with _parseString()
	*
	* @access private
	*/
	function createDesignFile(&$asset, &$contents) 
	{

		$design_area_links = $asset->getDesignAreaLink();
		pre_echo($design_area_links);

		foreach($design_area_links as $link) {
			$da = &$asset->getDesignArea($link['value']);
			if ($da === null) continue;
			$vars = Array();
			foreach($da->var_references as $var_name => $info) {
				$var_da = &$asset->getDesignArea($info['id_name']);
				if ($var_da === null) continue;
				$vars[$var_name] = $var_da->attr($info['var']);
			}// end foreach
			pre_echo('-------------------');
			pre_echo($vars);
			$da->setVarReferenceValues($vars);
		}// end foreach

		$str = '';
		ob_start();
		if ($this->_createDesignFileOutput($asset, $contents, $asset)) {
			$str = ob_get_contents();
		}
		ob_end_clean();

		pre_echo("STR : \n".$str);
		if (!$str) return false;

		require_once SQ_SQUIZLIB_PATH.'/general/file_system.inc';
		$file = $asset->_data_path.'/design_file.php';
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
		echo 'Not Done Yet';
	}// end paintParseFile()

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
		return false;
	}


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
		echo 'Not Done Yet';
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
		return false;
	}

}//end class
?>
