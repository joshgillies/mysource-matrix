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
* $Id: import.inc,v 1.38 2013/03/04 04:52:16 ewang Exp $
*
*/

require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Importing functions, reads an XML file and imports an asset strucutre
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.38 $
* @package MySource_Matrix
* @subpackage lib
*/


/**
* Get an array of import actions from an XML import file
*
* @param string	$import_file	the location of the XML import file
*
* @return array
* @access public
*/
function get_import_actions($import_file)
{
	// parse the import XML file
	if (!is_file($import_file)) {
		trigger_error('Import file "'.$import_file.'" does not exist', E_USER_ERROR);
	}

	// use DOMDocument::load() to see if there is any entity  reference loop
	// which may exhaust us of the memory limit, if not then continue as normal
	$xml = new DOMDocument();
	if (!$xml->load(str_replace(chr(0), '', $import_file), LIBXML_NOENT)) {
		exit();
	} else {
		// get few actions at a time from the XML so that while perforing
		// populate_import_action_array() on it we dont run out of memory
		$actions = Array();
		$import_actions = Array();
		$fh = fopen($import_file, 'r');
		$old_exec_time = ini_get('max_execution_time');
		// set higher execution time to allow matrix
		// to read from the xml that might be quiet big
		set_time_limit (200);
		$xml_header = '';
		while(!feof($fh)) {
			$p	= xml_parser_create();
			xml_parser_set_option($p, XML_OPTION_CASE_FOLDING, 0);
			xml_parser_set_option($p, XML_OPTION_SKIP_WHITE, 1);

			$xml_import_vals = Array();
			$chunk_import_action = Array();
			$index = Array();
			$content = '<actions>';
			$count = 0;
			$num_open_action_tags = 0;
			$num_close_action_tags = 0;

			// make Matrix read atleast 500 lines and the last line read must
			// always be end tag </action> so that the xml constructed is valid
			do {
				if (!($line = fgets($fh, 9999))) break;

				// skip the xml declaration and the starting tag for <actions>
				// but if we are on the </actions> that means we are done with file
				// break out or else there will be a infinite looping in this do...while
				if (trim($line) == '') continue;

				// remove the starting actions and end actions tag
				if (preg_match('/(<actions>)/', trim($line))) {
					$line = preg_replace('/<actions>/', '', $line);
				}
				if (preg_match('/(<\/actions>)/', trim($line))) {
					$line = preg_replace('/<\/actions>/', '', $line);
					$content .= $line;
					break;
				}

				if (preg_match('/(^<\?xml version)/', trim($line))) {
					$xml_header = $line;
					continue;
				}

				// count how many opening action tags in this line
				if (preg_match('/(<action>)/', trim($line), $matches)) {
					$num_open_action_tags += substr_count($line, '<action>');
				}
				// count closing tags
				if (preg_match('/(<\/action>)/', trim($line), $matches)) {
					$num_close_action_tags += substr_count($line, '</action>');
				}

				$content = $content.$line;
				$count++;
			} while ($num_open_action_tags != $num_close_action_tags || $count < 500);
			$content .= '</actions>';

			if ($content != '<actions></actions>') {
				if ($xml_header != '') $content = $xml_header.$content;
				xml_parse_into_struct($p, $content, $xml_import_vals, $index);

				// print an error if one occured
				if ($error_code = xml_get_error_code($p)) {
					echo 'XML Error: '.xml_error_string($error_code).' Line:'.xml_get_current_line_number($p).' Col:'.xml_get_current_column_number($p)."\n";
					exit();
				}
			}
			xml_parser_free($p);

			reset($xml_import_vals);
			populate_import_action_array($chunk_import_action, $xml_import_vals);
			if (!empty($chunk_import_action) && !empty($chunk_import_action['actions'][0])) $actions = array_merge($actions, $chunk_import_action['actions'][0]['action']);
		}

		// restore the original max execution time
		set_time_limit($old_exec_time);
		$import_actions['actions'][0]['action'] = $actions;
	}

	return $import_actions;

}//end get_import_actions()


/**
* Create an array representation of parsed XML data
*
* @param array	&$store	an array in which to store the final data
* @param array	&$vals	the parsed XML data from xml_parse_into_struct()
*
* @return void
* @access public
*/
function populate_import_action_array(&$store, &$vals)
{
	for (current($vals); NULL !== ($k = key($vals)); next($vals)) {
		$data  = $vals[$k];
		$value = array_get_index($data, 'value', '');
		$tag   = array_get_index($data, 'tag', '');

		switch ($data['type']) {
			case 'complete' : // FOUND A LEAF NODE

				if (isset($store[$tag])) {
					$max = count($store[$tag]);
					if (isset($data['attributes']) && count($data['attributes'])) {
						foreach ($data['attributes'] as $n => $v) {
							$store[$tag][$max][$n] = $v;
						}
					}
					$store[$tag][$max]['_value'] = $value;
				} else {
					if (isset($data['attributes']) && count($data['attributes'])) {
						foreach ($data['attributes'] as $n => $v) {
							$store[$tag][0][$n] = $v;
						}
						$store[$tag][0]['_value'] = $value;
					} else {
						$store[$tag][0] = $value;
					}
				}

			break;

			case 'open' : // OPENING A NEW TAG

				// call next here or it will never be called
				next($vals);

				if (!isset($store[$tag]) || !sizeof($store[$tag])) {
					$store[$tag] = Array();
				}

				$max = count($store[$tag]);
				if (isset($data['attributes']) && count($data['attributes'])) {
					foreach ($data['attributes'] as $n => $v) {
						$store[$tag][$max][$n] = $v;
					}
				}
				populate_import_action_array($store[$tag][$max], $vals);

			break;

			case 'close' : // CLOSING THE OPENED TAG

				// we return from the function here so
				// we will end up one level up (ie. just after
				// the populate_import_action_array fn call in the open case).
				return;

			break;
		}//end switch

	}//end for

}//end populate_import_action_array()


/**
* Execute an import action given the action data
*
* Also populates the output array for storage of import action outputs
*
* @param array	$action			the array of action data
* @param array	&$outputs		an array in which to store import action outputs
* @param array	$input_values	an array of known good input values to use
*
* @return boolean
* @access public
*/
function execute_import_action($action, &$outputs, $input_values=array())
{
	$settings = Array();
	$state = Array();

	$action_class = sanitise('trigger_action_'.$action['action_type'][0]);
	$GLOBALS['SQ_SYSTEM']->am->includeAsset($action_class);

	// we call execute as a static method, but instantiate for method_exists
	$class_instance = new $action_class;


	// design parse file needs to be replaced with new asset id
	// let's run get_import_action_input_value on it
	if ($action_class == 'trigger_action_set_design_parse_file' && isset($action['file_path'][0]) && is_file($action['file_path'][0])){
		$parse_file_content = file_get_contents($action['file_path'][0]);
		$temp_array['parse_file'] = Array($parse_file_content);
		$parse_file_content = get_import_action_input_value($temp_array, 'parse_file', $outputs, $input_values);
		$temp_file = tempnam(SQ_TEMP_PATH.'/import_assets', "import_from_xml_design_parse");
		if(!empty($temp_file)) {
			file_put_contents($temp_file, $parse_file_content);
			$action['file_path'][0] = $temp_file;
		}
	}

	// gather settings required for the action
	foreach ($action as $key => $value) {
		// skip action metadata
		if ($key == 'action_id') continue;
		if ($key == 'action_type') continue;

		$value = get_import_action_input_value($action, $key, $outputs, $input_values);

   		if (strtolower(SQ_CONF_DEFAULT_CHARACTER_SET) != 'utf-8') {
			$pattern = '/((&#[0-9]*;))/';
			preg_match_all($pattern, $value, $matches);

			$matches = $matches[0];
			$replacement = Array();

			foreach ($matches as $match) {
				$str = str_replace('&#', '', $match);
				$str= intval(str_replace(';', '', $str));
				if ($str > 126 && $str < 256) {
					$new_value = chr($str);
				} else {
					$new_value = $match;
				}
				$replacement[] = $new_value;
			}
			$value = str_replace($matches, $replacement, $value);
			$value = preg_replace('|<br/>|', chr(13), $value);
		}

		if ($key == 'asset') {
			// this is a special case where the value populates
			// the state array for the trigger action so it knows what
			// asset to perform the action on (if any)
			$state['assetid'] = $value;
			continue;
		}

		$function_name = sanitise('addSetting'.ucfirst($key));

		if (method_exists($class_instance, $function_name)) {
			$settings = call_user_func(Array($action_class, $function_name), $value, $settings);
			continue;
		}

		$settings[$key] = $value;

	}



	$output = call_user_func_array(Array($action_class, 'execute'), Array($settings, &$state));

	// remove the temp design parse file
	if(isset($temp_file) && !empty($temp_file)) {
		unlink($temp_file);
	}

	if ($output === FALSE) return FALSE;

	$outputs[$action['action_id'][0]] = $output;



	if ($action_class == 'trigger_action_set_attribute_value' && isset($output['attribute']) && $output['attribute'] == 'varieties'){
		if (isset($output['assetid'])) {
			$image = $GLOBALS['SQ_SYSTEM']->am->getAsset($output['assetid']);
			if ($image && $image->type() == 'image'){
				$edit_fns = $image->getEditFns();
				$edit_fns->_recreateVarietyImages($image);
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($image);
			}
		}
	}

	if ($action_class == 'trigger_action_create_asset' && isset($output['assetid'])) {
		$asset_imported = $GLOBALS['SQ_SYSTEM']->am->getAsset($output['assetid']);
		
		// cascading permission, schemas etc from parent asset
		if(isset($output['parentid']) && is_numeric($output['parentid'])) {
		    $parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($output['parentid']);
		    $GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		    $parent->_tmp[__CLASS__.'_in_create_cascading'] = TRUE;
		    $parent->cloneComponents($asset_imported, Array('permissions', 'metadata_schemas', 'workflow', 'content_tags', 'roles'));
		    unset($parent->_tmp[__CLASS__.'_in_create_cascading']);
		    $GLOBALS['SQ_SYSTEM']->restoreRunLevel();
		    $GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent);
		}

		// .. and if it was update the design areas under it
		if ($asset_imported && $asset_imported->type() == 'design_customisation') {
			$parent_design = $asset_imported->getParentDesign();
			if(!empty($parent_design))
			    $asset_imported->updateFromParent($parent_design);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent_design);

		} else if ($asset_imported && strpos($asset_imported->type(), 'design_area') !== FALSE) {
			// okie we just imported a design_area, we know its parent is design_customisation because
			// a regular design's design_areas are created out of our parse file
			// in this case we need to find the design_customisation first
			// and then update it w.r.t its parent Design
			$design_customisation = $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['parentid'], 'design_customisation', TRUE);
			if ($design_customisation) {
				$parent_design = $design_customisation->getParentDesign();

				$custom_da_links = $design_customisation->getDesignAreaLink();
				$data = Array();

				// get all the design_area on our customisations, and remove the ones that are
				// already 'customised' by out customisation
				foreach ($custom_da_links as $custom_da_link) {
					$parent_da = $parent_design->getDesignAreaLink($custom_da_link['value']);
					if (empty($parent_da)) continue;

					$data[$custom_da_link['value']] = Array($custom_da_link['minorid'], $custom_da_link['minor_type_code']);
				}
				$design_customisation_edit_fns = $design_customisation->getEditFns();

				if ($data && !$design_customisation_edit_fns->_updateDesignAreaLink($design_customisation, $data)) {
					$design_customisation->updateFromParent($parent_design);
				}

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($design_customisation);
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent_design);
			}

		}// end if/else

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset_imported);

	}//end if (trigger_action_create_asset)

	return TRUE;

}//end execute_import_action()


/**
* Gets the import value for an action from the XML import file
*
* Also handles references to previous action outputs.
* The function will die with E_USER_ERROR if it encounters any errors.
*
* @param array	$action			the array of action data
* @param string	$attribute		the name of the input attribute to get
* @param array	$outputs		the output values from previous import actions
* @param array	$input_values	an array of known good input values to use
*
* @return string
* @access public
*/
function get_import_action_input_value($action, $attribute, $outputs=Array(), $input_values=array())
{
	// first make sure the attribute exists for this action
	if (!isset($action[$attribute])) {
		trigger_error('Failed getting '.$attribute.' for '.$action['action_type'][0].' action "'.$action['action_id'][0].'" - attribute is not defined for action', E_USER_ERROR);
	} else {
		$value = $action[$attribute][0];
	}

	if (!is_array($value)) {
		if (isset($input_values[(string)$value]) === true) {
			return $input_values[$value];
		}
	}

	// loop until there are no more changes
	do {
		$current_value = $value;

		// if the value is an array, just return it
		if (is_array($value)) return $value;
		$matches = Array();
		preg_match_all('|\[\[output://([^\]]+)\]\]|', $value, $matches);
		foreach ($matches[0] as $key => $match) {
			list($action_id, $output) = explode('.', $matches[1][$key], 2);
			if (!isset($outputs[$action_id])) {
				trigger_error('Failed getting '.$attribute.' for '.$action['action_type'][0].' action "'.$action['action_id'][0].'" - attribute references action "'.$action_id.'" that has not yet been run', E_USER_ERROR);
			}
			if (!isset($outputs[$action_id][$output])) {
				trigger_error('Failed getting '.$attribute.' for '.$action['action_type'][0].' action "'.$action['action_id'][0].'" - attribute references action output "'.$output.'" that was not generated by action "'.$action_id.'"', E_USER_ERROR);
			}
			$value = str_replace($match, $outputs[$action_id][$output], $value);
		}

		$matches = Array();
		preg_match_all('|\[\[system://([^\]]+)\]\]|', $value, $matches);
		foreach ($matches[0] as $key => $match) {
			$system_asset = substr($match, 11, -2);
			$assetid = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid($system_asset);
			if (!$assetid) {
				trigger_error('Failed getting system asset "'.$system_asset.'"', E_USER_ERROR);
			}
			$value = str_replace($match, $assetid, $value);
		}

		$matches = Array();
		preg_match_all('|\[\[conf://([^\]]+)\]\]|', $value, $matches);
		$orig = (isset($matches[0][0]) ? $matches[0][0] : '');
		foreach ($matches[1] as $key => $match) {
			$conf_value = constant('SQ_CONF_'.strtoupper($match));
			if ($match == 'system_root_urls') {
				$tmp = explode("\n", $conf_value);
				$conf_value = $tmp[0];
			}
			$value = str_replace($orig, $conf_value, $value);
		}

		$matches = Array();
		preg_match_all('|\[\[dal://([^\]_]+)_([^\]]+)\]\]|', $value, $matches);
		if (!empty($matches[1]) && !empty($matches[2])) {
			include SQ_DATA_PATH.'/private/conf/db.inc';
			if (isset($db_conf[$matches[1][0]])) {
				// DB name is correct
				$dsn = $db_conf[$matches[1][0]];
				// 'db' connection can be configured with multiple DSNs. But we only get the first DSN in the list for now.
				if (($matches[1][0] == 'db') && !isset($dsn['DSN']) && isset($dsn[0])) {
					$dsn = $dsn[0];
				}
				if (isset($dsn[$matches[2][0]])) {
					// DSN section name is correct
					$value = $dsn[$matches[2][0]];
				}
			}
		}

		// construct-specific calls - will not work on a regular import
		$matches = Array();
		preg_match_all('|\[\[construct://([^\]]+)\]\]|', $value, $matches);
		for ($ii = 0; $ii < count($matches[0]); $ii++) {
			$construct_value = '';
			switch ($matches[1][$ii]) {
				case 'unit_test_path':
					$construct_value = (defined('TC_CONF_UNIT_TEST_PATH') ? TC_CONF_UNIT_TEST_PATH : '');
				break;
				case 'construct_path':
					$construct_value = (defined('TC_CONF_CONSTRUCT_PATH') ? TC_CONF_CONSTRUCT_PATH : '');
				break;
				case 'simpletest_path':
					$construct_value = (defined('TC_CONF_SIMPLETEST_PATH') ? TC_CONF_SIMPLETEST_PATH : '');
				break;
				case 'matrix_url':
					$construct_value = (defined('TC_CONF_MATRIX_URL') ? strip_url(TC_CONF_MATRIX_URL, TRUE): '');
				break;
				case 'time':
					$construct_value = time();
				break;
			}
			$value = str_replace($matches[0][$ii], $construct_value, $value);
		}

	} while ($current_value != $value);

	return $value;

}//end get_import_action_input_value()


/**
* Print the output of an import action as XML
*
* @param string	$action_id	the unique ID of the import action
* @param array	$outputs	the output values from the import action
* @param string	$indent		whitespace indentation to be printed before the tags
*
* @return void
* @access public
*/
function print_import_action_output_as_xml($action_id, $outputs, $indent='')
{
	echo "$indent<output>\n";
	echo "$indent\t<action_id>$action_id</action_id>\n";
	foreach ($outputs as $key => $value) {
		echo "$indent\t<$key>$value</$key>\n";
	}
	echo "$indent</output>\n";

}//end print_import_action_output_as_xml()


/**
* Sanitise the input string for class name or function name
*
* @param string	$string  user input string
*
* @return string
* @access public
*/
function sanitise($string)
{
    return (preg_replace('/[^0-9_a-zA-Z]+/', '', $string));
}

?>
