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
* $Id: layouts.inc,v 1.6 2012/08/30 01:09:20 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Layouts Static Screen Functions
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix
*/


/**
* Paints the interface for adding a layout
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	&$ei	the edit interface
* @param array	$keywords	the list of keywords
*
* @return boolean
* @access public
*/
function paintLayouts(&$owner, &$o, &$ei, $keywords=NULL)
{
	$root = $ei->findScreen('static_screen_layouts');
	$prefix = $owner->getPrefix();
	$write_access = $owner->writeAccess('attributes');

	$lookups = $owner->getLookups('url');
	$lookup_values = $owner->getLookupValues();

	$return_lookup = FALSE;

	$current_value = Array();
	$current_value = _formatLayoutCode($lookup_values);

	foreach ($root->section as $section) {
		$o->openSection((is_null($section['display_name'])) ? $section['name'] : $section['display_name']);

		// if we are in limbo and this section is not to be printed in limbo - dont print it
		if (is_null($keywords)) {
			if (SQ_IN_LIMBO && ((string) $section['name'] === 'limbo_access' || (int) $section['limbo_access'] === 0)) {
				continue;
			}
		}

		if (!is_null($keywords) && isset($keywords['sections'][strtolower($section->attributes()->name)])) {
			ob_start();
		}

		foreach ($section->children() as $field) {
			// should we print the note for the field or not
			$print_note = TRUE;

			$child = strtolower((string) $field['code']);
			switch ($child) {
				// Show the Add Layout
				case 'add_layout' :
					if ($owner->adminAccess('settings')) {
						$return_lookup = TRUE;

						// Allow changes here
						$o->openField('');
						$types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
						asset_type_chooser($prefix.'_add_layouts', TRUE, Array());

						$o->closeField();
					} else {
						$return_lookup = TRUE;
					}
				break;

				// Show the edit layout
				case 'edit_layouts' :
					if (!is_null($current_value)) {
						foreach ($current_value as $type_code => $values) {
							// should we print the note for the field or not
							$print_note = TRUE;

							$uc_type_code = ucwords(str_replace('_', ' ', $type_code));
							$o->openSection($uc_type_code);

							// Show the fields
							$return_lookup = TRUE;

							// Show the extra controls like delete this layout
							if ($write_access) {
								$o->openField('');
								check_box($prefix.'_remove['.$type_code.']');
								label(sprintf(translate('remove_customisations_for'), $uc_type_code), $prefix.'_remove['.$type_code.']');
								$o->closeField();
							}

							// Show the defined screens
							$defined_screens = 0;
							foreach ($values as $screen_key => $assetid) {
								if ($screen_key != 'noscreen') {
									$o->openField('');
										?>
										<table class="sq-backend-table">
											<tr>
												<td class="sq-backend-table-header" width="33%"><?php echo translate('screen_name'); ?></td>
												<td class="sq-backend-table-header" width="33%"><?php echo translate('layout'); ?></td>
												<?php if ($write_access) {
													?>
													<td align="center" class="sq-backend-table-header" width="33%"><?php echo translate('remove'); ?></td>
												<?php }
												?>
											</tr>
											<tr>
												<td class="sq-backend-table-cell">
													<?php echo ucfirst($screen_key) ?>
												</td>
												<td class="sq-backend-table-cell">
													<?php
														if ($write_access) {
															asset_finder($prefix.'_find['.$type_code.'@'.$screen_key.']', $assetid, Array('layout' => 'I'));
														} else {
															if (empty($assetid)) {
																echo '<em>'.translate('none').'</em>';
															} else {
																echo get_asset_tag_line($assetid);
															}
														}
													?>
												</td>
												<?php if ($write_access) {
													?>
													<td align="center" class="sq-backend-table-cell">
														<?php check_box($prefix.'_remove_screen['.$type_code.'@'.$screen_key.']'); ?>
													</td>
												<?php }
												?>
											</tr>
										</table>
									<?php
									$defined_screens++;
									$o->closeField();
								}//end if
							}//end foreach
							// No screens so show message
							if (!($defined_screens > 0)) {
								if (!$owner->adminAccess('settings')) {
									$o->openField('&nbsp;');
									echo 'No layouts defined for any of '.$uc_type_code."'s screens";
									$o->closeField();
								}
							}
							// Give the option to add more screens
							if ($write_access) {
								$o->openField('');
									if ($write_access) {
										?>
										<br />
										<?php
										echo translate('add_layout');
										echo ': ';

										$ei = new Asset_Edit_Interface($type_code);

										$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
										$as = new $type_code();
										$ef = $as->getEditFns();

										$screens = Array(''); // first option is blank
										foreach (array_keys($ef->static_screens) as $screen) {
											$screens[] = $screen;
										}
										foreach (array_keys($ei->getScreens()) as $screen) {
											$screens[] = $screen;
										}

										// remove any screens that have already been defined
										$screens = array_unique(array_diff($screens, array_keys($values)));

										$uc_screens = Array();
										foreach ($screens as $screen) {
											$uc_screens[$screen] = ucfirst($screen);
										}
										// print a box to add more screens
										combo_box($prefix.'_screens['.$type_code.']', $uc_screens, FALSE, Array());
									}

								$o->closeField();
							}//end if
							$o->closeSection();

						}//end foreach
					}//end if
				break;
			}//end switch
		}//end foreach

		if (!is_null($keywords) && isset($keywords['fields'][strtolower((string) $field['code'])])) {
			ob_start();
		}
		// before printing the note, we make sure that the note element exists and is not empty
		if ($print_note && (!is_null($field->note)) && (strlen((string)$field->note) > 0)) {
			$o->openField('&nbsp;');
			$o->note((string) $field->note);
			$o->closeField();
		}
		$o->closeSection();
	}//end foreach

	return $return_lookup;

}//end paintLayouts()


/**
* Processes the interface for adding a layout
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o		the outputter class
* @param object	&$ei	the edit interface
*
* @return boolean
* @access public
*/
function processLayouts(&$owner, &$o, &$ei)
{
	$prefix = $owner->getPrefix();
	$write_access = $owner->writeAccess('lookups');

	// We don't have write access so exit
	if (!$write_access) return FALSE;

	// Grab the info we need
	$layouts = $owner->getLookupValues();
	$urls = Array();

	if (!empty($layouts)) {
		$urls = array_keys($layouts);
	} else {
		$all_urls = $owner->getURLs();

		// Grab the address innards of the URL and store this only
		foreach ($all_urls as $url) {
			$urls[] = $url['url'];
		}
	}

	// If url is not warn user and stop
	if (empty($urls)) {
		trigger_localised_error('SYS0330', E_USER_WARNING);
	}

	// Process Adding a new layout of type code xx
	if (isset($_POST[$prefix.'_add_layouts']) && !empty($_POST[$prefix.'_add_layouts'])) {
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// Format the current layouts to a design we like
		$layout_schemas = Array();
		foreach ($layouts as $layout_designs) {
			$layout_schemas[] = key($layout_designs);
		}

		// Remember last postings so we don't too many of the same
		$prev_post = Array();

		foreach ($_POST[$prefix.'_add_layouts'] as $type_code) {
			// Check they are not trying to add multiple entries of the same page
			if (in_array($type_code, $prev_post)) {
				continue;
			} else {
				$prev_post[] = $type_code;
			}

			// layouts::type_code::screen where is not defined screen will default to noscreen with asset of 0
			if (!empty($type_code)) {
				$layout_schema = 'layout::'.$type_code.'::noscreen';
				foreach ($urls as $url) {
					$layouts[$url][$layout_schema] = Array();
					$layouts[$url][$layout_schema]['value'] = 0;
				}
			}
		}//end foreach

		if (!$owner->setLookupValues($layouts)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
	}//end if

	// Process Editing layout screens
	// Add the asset to the screen
	if (isset($_POST[$prefix.'_find']) && !empty($_POST[$prefix.'_find'])) {
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		foreach ($_POST[$prefix.'_find'] as $key => $value) {
			$details = explode('@', $key);

			// Make sure an asset was selected
			if (!isset($value['assetid']) || empty($value['assetid'])) {
				$assetid = 0;
			} else {
				$assetid = $value['assetid'];
			}

			// Format the values for lookup values
			$lookup = 'layout::'.$details[0].'::'.$details[1];

			// If an asset is already set for here delete it so we can re add it later
			foreach ($urls as $url) {
				if (in_array($lookup, $layouts[$url])) {
					unset($layouts[$url][$lookup]);
				}

				// Make out changes
				if (!isset($layouts[$url][$lookup])) {
					$layouts[$url][$lookup] = Array();
				}
				$layouts[$url][$lookup]['value'] = $assetid;
			}
		}//end foreach

		if (!$owner->setLookupValues($layouts)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
	}//end if

	// Add the screen to the type code
	if (isset($_POST[$prefix.'_screens']) && !empty($_POST[$prefix.'_screens'])) {
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		foreach ($_POST[$prefix.'_screens'] as $type_code => $screen) {
			// Make sure something was selected
			if (empty($screen)) continue;

			// Format the values for lookup values
			$lookup = 'layout::'.$type_code.'::'.$screen;

			foreach ($urls as $url) {
				// Check for duplication
				if (in_array($lookup, $layouts[$url])) continue;

				$layouts[$url][$lookup] = Array();
				$layouts[$url][$lookup]['value'] = 0;
				if (!empty($screen)) {
					// Delete the no screen entry if setting a screen
					unset($layouts[$url]['layout::'.$type_code.'::noscreen']);
				}
			}
		}

		if (!$owner->setLookupValues($layouts)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
	}//end if

	// Remove a screen from a type code
	if (isset($_POST[$prefix.'_remove_screen']) && !empty($_POST[$prefix.'_remove_screen'])) {
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		foreach ($_POST[$prefix.'_remove_screen'] as $key => $value) {
			$details = explode('@', $key);

			foreach ($urls as $url) {
				if ($value) {
					unset($layouts[$url]['layout::'.$details[0].'::'.$details[1]]);
				}

				// Check for another entry, if none found we need to add a noscreen entry
				// Otherwise we lose our type code
				$another_entry = 0;
				foreach ($layouts[$url] as $looking_key => $looking_value) {
					$search_string = '/^layout::'.$details[0].'::/i';
					if (preg_match($search_string, $looking_key)) {
						$another_entry++;
					}
				}
				if (!$another_entry) {
					$layouts[$url]['layout::'.$details[0].'::noscreen'] = Array();
					$layouts[$url]['layout::'.$details[0].'::noscreen']['value'] = 0;
				}
			}
		}

		if (!$owner->setLookupValues($layouts)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
	}//end if

	// Remove a type code from layouts
	if (isset($_POST[$prefix.'_remove']) && !empty($_POST[$prefix.'_remove'])) {
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		foreach ($_POST[$prefix.'_remove'] as $type_code => $value) {
			if ($value) {
				foreach ($urls as $url) {
					foreach ($layouts[$url] as $lookup_key => $lookup_value) {
						$search_string = '/^layout::'.$type_code.'::/i';
						if (preg_match($search_string, $lookup_key)) {
							unset($layouts[$url][$lookup_key]);
						}
					}
				}
			}
		}

		if (!$owner->setLookupValues($layouts)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
	}//end if

	return TRUE;

}//end processLayouts()


/**
* Returns the lookup array into something we can work with
*
* @param string	$lookup_values	the lookup array we need to format
*
* @return array
* @access public
*/
function _formatLayoutCode($lookup_values)
{
	$return_value = Array();

	// Return nothing if empty
	if (empty($lookup_values)) return $return_value;

	foreach ($lookup_values as $item) {
		// Only grab the layouts only
		foreach ($item as $lookup => $value) {
			if (preg_match('/^layout::/i', $lookup)) {
				$layout_format = explode('::', $lookup);
				if (!isset($return_value[$layout_format[1]])) {
					$return_value[$layout_format[1]] = Array();
				}
				$return_value[$layout_format[1]][$layout_format[2]] = $value['value'];
			}
		}
	}

	return $return_value;

}//end _formatLayoutCode()


?>
