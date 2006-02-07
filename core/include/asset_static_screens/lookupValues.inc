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
* $Id: lookupValues.inc,v 1.10 2006/02/07 02:18:51 skim Exp $
*
*/

require_once 'XML/Tree.php';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Settings Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix
*/


/**
* Paints the interface for editing various settings
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function paintLookupValues(&$owner, &$o, &$ei)
{

	$prefix = $owner->getPrefix();
	$lookups = $owner->getLookups('url');
	$lookup_values = $owner->getLookupValues();

	$am =& $GLOBALS['SQ_SYSTEM']->am;

	$lookup_value_types = $am->getTypeDescendants('lookup_value');
	$lookup_value_details = Array();

	// need to group the values by their value type
	$values = Array();
	foreach ($lookup_value_types as $type_code) {
		$am->includeAsset($type_code);
		eval ('$name_prefix = '.$type_code.'::namePrefix();');
		eval ('$title = '.$type_code.'::lookupValueName();');
		$lookup_value_details[$type_code] = Array(
												'name_prefix'	=> $name_prefix,
												'title'			=> $title,
											);
		$values[$name_prefix] = Array();
	}

	uasort($lookup_value_details, create_function('$a,$b','return strcmp(strtolower($a[\'title\']), strtolower($b[\'title\']));'));

	foreach ($lookup_values as $url => $data) {
		foreach ($data as $value_name => $value_data) {
			if (!preg_match('/^([^:]+)(::|$)/', $value_name, $matches=Array())) {
				continue;
			}
			if (!isset($values[$matches[1]])) continue; // unknown prefix..
			if (!isset($values[$matches[1]][$url])) {
				$values[$matches[1]][$url] = Array();
			}
			$values[$matches[1]][$url][$value_name] = $value_data;
		}
	}
	unset($lookup_values);

	$editing_fields = FALSE;

	for (reset($lookup_value_details); NULL !== ($type_code = key($lookup_value_details)); next($lookup_value_details)) {
		$o->openSection($lookup_value_details[$type_code]['title']);
		$name_prefix = $lookup_value_details[$type_code]['name_prefix'];
		eval('$editing_fields |= '.$type_code.'::paintInterface($owner, $o, $lookups, $values[$name_prefix], $prefix.\'_lookup_values_\'.$name_prefix);');
		$o->closeSection();

	}// end foreach

	return $editing_fields;

}//end paintLookupValues()


/**
* Processes the interface for editing various settings
* returns TRUE if there is a change
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function processLookupValues(&$owner, &$o, &$ei)
{
	$prefix = $owner->getPrefix();
	$lookups = $owner->getLookups('url');
	// get only the lookup values that we have set specifically for ourselves
	$lookup_values = $owner->getLookupValues(FALSE);

	$am =& $GLOBALS['SQ_SYSTEM']->am;

	$lookup_value_types = $am->getTypeDescendants('lookup_value');

	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

	foreach ($lookup_value_types as $type_code) {
		$am->includeAsset($type_code);
		eval('$name_prefix = '.$type_code.'::namePrefix();');
		eval('$success = '.$type_code.'::processInterface($owner, $o, $lookups, $values=Array(), $prefix.\'_lookup_values_\'.$name_prefix);');

		if (!$success) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		foreach ($values as $url => $values_data) {
			foreach ($values_data as $value_name => $data) {
				// no value ? delete...
				if ($data['value'] == '') {
					if (isset($lookup_values[$url][$value_name])) {
						unset($lookup_values[$url][$value_name]);
					}
				} else {
					if (!isset($lookup_values[$url])) {
						$lookup_values[$url] = Array();
					}
					$lookup_values[$url][$value_name] = Array('value' => $data['value']);
				}
			}
		}

	}// end foreach

	if (!$owner->setLookupValues($lookup_values)) {
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return FALSE;
	}
	$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
	$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
	$vars = Array('assetids' => Array($owner->id));
	$hh->queueHipo('hipo_job_update_lookups', $vars);

	return TRUE;

}//end processLookupValues()


?>
