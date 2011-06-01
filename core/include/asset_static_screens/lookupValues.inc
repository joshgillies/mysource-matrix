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
* $Id: lookupValues.inc,v 1.16 2008/07/02 05:27:03 bshkara Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Settings Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix
*/


/**
* Paints the interface for editing various settings
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	&$ei	the edit interface
* @param array	$keywords	the list of keywords
*
* @return boolean
* @access public
*/
function paintLookupValues(&$owner, &$o, &$ei, $keywords=NULL)
{

	$prefix = $owner->getPrefix();
	$lookups = $owner->getLookups('url');
	$lookup_values = $owner->getLookupValues(TRUE);

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
			$matches = Array();
			if (!preg_match('/^([^:]+)(::|$)/', $value_name, $matches)) {
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

	$layouts_changed = FALSE;
	$layouts_ok = TRUE;

	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
	$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
	foreach ($lookup_value_types as $type_code) {
		$am->includeAsset($type_code);
		eval('$name_prefix = '.$type_code.'::namePrefix();');
		$values = Array();
		eval('$success = '.$type_code.'::processInterface($owner, $o, $lookups, $values, $prefix.\'_lookup_values_\'.$name_prefix);');

		if (!$success) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		foreach ($values as $resource => $values_data) {
			foreach ($values_data as $value_name => $data) {
				if ($resource === 'asset') {
					// asset layout definition
					$layout_name = $value_name.'::system::frontend';
					$existing_layout_link = $GLOBALS['SQ_SYSTEM']->am->getLink($owner->id, SQ_LINK_NOTICE, 'paint_layout_page', TRUE, $layout_name);
					$create_new = $delete_existing = FALSE;
					if (empty($data['value'])) {
						$delete_existing = TRUE;
					} else if (empty($existing_layout_link)) {
						// a layout notice link does not currently exist so create it
						$create_new = TRUE;
					} else {
						// notice link currently exists, only create a new notice link if necessary
						if ($existing_layout_link['minorid'] !== $data['value']) {
							$delete_existing = TRUE;
							$create_new = TRUE;
						}
					}

					if ($delete_existing) {
						if ($existing_layout_link) {
							if (!$owner->deleteLink($existing_layout_link['linkid'])) {
								$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
								$layouts_ok = FALSE;
								break;
							} else {
								$layouts_changed = TRUE;
							}

							// also need to delete any other URLs which use the same value
							// deleting an asset based value means all other URLs which use the same value are also deleted
							if (!$new_values = $owner->deleteLookupValue($layout_name, $existing_layout_link['minorid'])) {
								$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
								$layouts_ok = FALSE;
								break;
							} else {
								$lookup_values = $new_values;
								unset($new_values);
							}
						}
					}

					if ($create_new) {
						// create new notice link
						$layout = $GLOBALS['SQ_SYSTEM']->am->getAsset($data['value']);
						if (is_null($layout)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$layouts_ok = FALSE;
							break;
						}
						if (!$owner->createLink($layout, SQ_LINK_NOTICE, $layout_name)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$layouts_ok = FALSE;
							break;
						} else {
							$layouts_changed = TRUE;
						}
					}

					continue;
				}//end if

				// delete if no value
				if (empty($data['value'])) {
					if (isset($lookup_values[$resource][$value_name])) {
						unset($lookup_values[$resource][$value_name]);
					}
					// if value is deleted, run updateLookups() so the asset based definition can take effect if there is one defined
					$layouts_changed = TRUE;
				} else {
					if (!isset($lookup_values[$resource])) {
						$lookup_values[$resource] = Array();
					}
					$lookup_values[$resource][$value_name] = Array('value' => $data['value']);
				}
			}
		}

	}// end foreach

	if (!$owner->setLookupValues($lookup_values)) {
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return FALSE;
	}
	if ($layouts_ok) {
		if ($layouts_changed) {
			if (!$owner->updateLookups()) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}
	}

	$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
	$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	return TRUE;

}//end processLookupValues()


?>
