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
* $Id: import.inc,v 1.4.2.1 2006/11/06 02:58:06 skim Exp $
*
*/

require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Importing functions, reads an XML file and imports an asset strucutre
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.4.2.1 $
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
		echo 'XML Error: '.xml_error_string($error_code)."\n";
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
	for (current($vals); null !== ($k = key($vals)); next($vals)) {
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
* @param array	$action		the array of action data
* @param array	&$outputs	an array in which to store import action outputs
*
* @return boolean
* @access public
*/
function execute_import_action($action, &$outputs)
{
	$settings = Array();
	$state = Array();

	// gather settings required for the action
	foreach ($action as $key => $value) {
		// skip action metadata
		if ($key == 'action_id') continue;
		if ($key == 'action_type') continue;

		$value = get_import_action_input_value($action, $key, $outputs);

		if ($key == 'asset') {
			// this is a special case where the value populates
			// the state array for the trigger action so it knows what
			// asset to perform the action on (if any)
			$state['assetid'] = $value;
		} else {
			$settings[$key] = $value;
		}
	}

	$action_class = 'trigger_action_'.$action['action_type'][0];
	$GLOBALS['SQ_SYSTEM']->am->includeAsset($action_class);
	eval('$output = '.$action_class.'::execute($settings, $state);');
	if ($output === false) return false;

	$outputs[$action['action_id'][0]] = $output;
	return true;

}//end execute_import_action()


/**
* Gets the import value for an action from the XML import file
*
* Also handles references to previous action outputs.
* The function will die with E_USER_ERROR if it encounters any errors.
*
* @param array	$action		the array of action data
* @param string	$attribute	the name of the input attribute to get
* @param array	$outputs	the output values from previous import actions
*
* @return string
* @access public
*/
function get_import_action_input_value($action, $attribute, $outputs=Array())
{
	// first make sure the attribute exists for this action
	if (!isset($action[$attribute])) {
		trigger_error('Failed getting '.$attribute.' for '.$action['action_type'].' action "'.$action['action_id'].'" - attribute is not defined for action', E_USER_ERROR);
	} else {
		$value = $action[$attribute][0];
	}

	// if the value is an array, just return it
	if (is_array($value)) return $value;

	$matches = Array();
	preg_match_all('|\[\[output://([^\]]+)\]\]|', $value, $matches);
	foreach ($matches[0] as $key => $match) {
		list($action_id, $output) = explode('.', $matches[1][$key], 2);
		if (!isset($outputs[$action_id])) {
			trigger_error('Failed getting '.$attribute.' for '.$action['action_type'].' action "'.$action['action_id'].'" - attribute references action "'.$action_id.'" that has not yet been run', E_USER_ERROR);
		}
		if (!isset($outputs[$action_id][$output])) {
			trigger_error('Failed getting '.$attribute.' for '.$action['action_type'].' action "'.$action['action_id'].'" - attribute references action output "'.$output.'" that was not genereted by action "'.$action_id.'"', E_USER_ERROR);
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
	preg_match_all('|\[\[conf://([^\]]+)\]\](.*)|', $value, $matches);
	foreach ($matches[1] as $key => $match) {
		eval('$conf_value = SQ_CONF_'.strtoupper($match).';');
		if ($match == 'system_root_urls') {
			$tmp = explode("\n", $conf_value);
			$conf_value = $tmp[0].$matches[2][0];
		}
		$value = $conf_value;
	}

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