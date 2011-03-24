<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: import.inc,v 1.22 2011/03/24 23:46:22 akarelia Exp $
*
*/

require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Importing functions, reads an XML file and imports an asset strucutre
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.22 $
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

	$xml_file = file_to_string($import_file);
	$p = xml_parser_create();
	xml_parser_set_option($p, XML_OPTION_CASE_FOLDING, 0);
	xml_parser_set_option($p, XML_OPTION_SKIP_WHITE, 1);

	$index = Array();
	$xml_import_vals = Array();
	xml_parse_into_struct($p, $xml_file, $xml_import_vals, $index);

	// print an error if one occured
	if ($error_code = xml_get_error_code($p)) {
		echo 'XML Error: '.xml_error_string($error_code).' Line:'.xml_get_current_line_number($p).' Col:'.xml_get_current_column_number($p)."\n";
		exit();
	}

	xml_parser_free($p);

	$import_actions = Array();
	reset($xml_import_vals);
	populate_import_action_array($import_actions, $xml_import_vals);

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

	$action_class = 'trigger_action_'.$action['action_type'][0];
	$GLOBALS['SQ_SYSTEM']->am->includeAsset($action_class);

	// we call execute as a static method, but instantiate for method_exists
	$class_instance = new $action_class;

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
				if ($str > 126) {
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

		$function_name = 'addSetting'.ucfirst($key);

		if (method_exists($class_instance, $function_name)) {
			eval('$settings = '.$action_class.'::'.$function_name.'($value, $settings);');
			continue;
		}

		$settings[$key] = $value;

	}

	eval('$output = '.$action_class.'::execute($settings, $state);');
	if ($output === FALSE) return FALSE;

	$outputs[$action['action_id'][0]] = $output;
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

	if (isset($input_values[(string)$value]) === true) return $input_values[$value];

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
			eval('$conf_value = SQ_CONF_'.strtoupper($match).';');
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
				if (isset($db_conf[$matches[1][0]][$matches[2][0]])) {
					// DSN section name is correct
					$value = $db_conf[$matches[1][0]][$matches[2][0]];
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


?>
