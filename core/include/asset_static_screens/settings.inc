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
* $Id: settings.inc,v 1.10 2004/09/14 00:50:43 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

require_once 'XML/Tree.php';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Settings Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/


/**
* Paints the interface for editing various settings
*
* @param object	Asset					$owner	the asset whose interface we are painting
* @param object	Backend_Outputter		$o		the outputter class
* @param object	Asset_Edit_Interface	&$ei	the edit interface
*
* @access public
* @return boolean
*/
function paintSettings(&$owner, &$o, &$ei)
{
	if(!($file = $ei->_findFile('static_screen_settings'))) return;
	$prefix = $owner->getPrefix();

	$created = false;

	$input = new XML_Tree($file);
	$root  = &$input->getTreeFromFile();
	if (PEAR::isError($root)) {
		trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
		return;
	}

	if (empty($root->children)) {
		$o->OpenSection('There are no settings for "'.$owner->name.'"');
		$o->closeSection();
	}

	for($i = 0; $i < count($root->children); $i++) {
		$section = &$root->children[$i];

		// if we are in limbo and this section is not to be printed in limbo - dont print it
		if (SQ_IN_LIMBO && (!isset($section->attributes['limbo_access']) || (int)$section->attributes['limbo_access'] == 0)) continue;

		$o->openSection($section->attributes['name']);

		if (!($num_fields = count($section->children))) continue;
		for($j = 0; $j < $num_fields; $j++) {
			$field = &$section->children[$j];

			// should we print the note for the field or not
			$print_note = true;

			switch (strtolower($field->attributes['code'])) {
				case 'version_current_version' :
					if ($field->children[0]->name != 'display_name') $display_name = 'Current Version';
					else $display_name = $field->children[0]->content;
					$o->openField($display_name);
					echo $owner->version;
				break;

				case 'version_update_version' :
					// have to have admin access to update the major version
					if ($owner->adminAccess('all')) {
						$created = true;

						if ($field->children[0]->name != 'display_name') $display_name = 'Update Major Version ?';
						else $display_name = $field->children[0]->content;
						$o->openField($display_name);
						check_box($prefix.'_version_update_version');
					} else {
						$print_note = false;
					}
				break;

				case 'date_created_date' :
				case 'date_published_date' :

					$date_type = (strtolower($field->attributes['code']) == 'date_created_date') ? 'created' : 'published';
					require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';

					$parameters = Array(
						'min'                   => '0000-00-00 00:00:00',
						'max'                   => '9999-12-31 12:59:59',
						'allow_circa'           => '0',
						'show'                  => Array('y', 'm', 'd', 'h', 'i', 's'),
						'null'                  => Array('y', 'm', 'd', 'h', 'i', 's'),
						'style'                 => Array(
														'y' => 't',
														'm' => 's',
														'd' => 's',
														'h' => 's',
														'i' => 's',
														's' => 's',
												),
						'print_format'          => 'l, F jS, Y H:i:s',
					);

					if ($date_type == 'created') {
						$value  = (!empty($owner->created)) ? ts_iso8601($owner->created) : '---------- --:--:--';
						$userid = $owner->created_userid;
					} else {
						$value  = (!empty($owner->published)) ? ts_iso8601($owner->published) : '---------- --:--:--';
						$userid = $owner->published_userid;
					}

					if ($field->children[0]->name != 'display_name') $display_name = ucfirst($date_type).' Date';
					else $display_name = $field->children[0]->content;
					$o->openField($display_name);

					$date_field = new Datetime_Field($prefix.'_date_'.$date_type, $value, $parameters);

					if ($owner->adminAccess('all')) {
						$date_field->printField();

						?>
							<script language="javascript">
								var TIME_UNITS = new Array();
								<?php

									$units = Array(
										'Y' => 'y',
										'n' => 'm',
										'j' => 'd',
										'G' => 'h',
										'i' => 'i',
										's' => 's',
									);

									foreach ($units as $php_unit => $js_unit) {
										echo 'TIME_UNITS[\''.$js_unit.'\'] = \''.date($php_unit).'\';';
									}
								?>

								function setCurrentTime(prefix) {
									for (u in TIME_UNITS) {
										var id = prefix + 'value[' + u + ']';
										var unit = document.getElementById(id);

										if (unit.type == 'text') {
											unit.value = TIME_UNITS[u];
										} else {
											for (var i = 0; i < unit.options.length; i++) {
												if (TIME_UNITS[u] == unit.options[i].value) {
													unit.selectedIndex = i;
												}
											}
										}
									}
								}
							</script>
						<?php
						normal_button($prefix.'_set_time_'.$date_type, 'now', 'setCurrentTime(\''.$prefix.'_date_'.$date_type.'\');');
						echo '&nbsp;<b>by</b>&nbsp;';
						asset_finder($prefix.'_date_'.$date_type.'_userid', $userid, Array('user' => 'D'));
						
					} else {
						$print_note = false;
						if ($date_field->isEmpty() && $date_type == 'published') {
							echo ' never published';
						} else {
							$info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($userid), 'user', false);
							if (empty($info)) $info[$userid] = Array('name' => '&lt;unknown&gt;');

							echo ucfirst($date_type).' by '.$info[$userid]['name'].' on '.date('d M Y h:i:s', strtotime($date_field->value));
						}
					}

				break;

				case 'force_secure' :
					if ($field->children[0]->name != 'display_name') $display_name = 'Force Secure';
					else $display_name = $field->children[0]->content;
					$o->openField($display_name);
					$current_langs = $owner->getLanguages();
					// have to have admin access to set the force secure
					if ($owner->adminAccess('settings')) {
						$created = true;
						check_box($prefix.'_force_secure', '1', ($owner->force_secure == '1'));
					} else {
					?>
						<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo ($owner->force_secure) ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
					<?php
					}
				break;


				case 'intl_languages' :
					if ($field->children[0]->name != 'display_name') $display_name = 'Languages';
					else $display_name = $field->children[0]->content;
					$o->openSection($display_name);

					require SQ_FUDGE_PATH.'/standards_lists/languages.inc';

					if ($owner->writeAccess('settings')) {
						$created = true;
						$standards_lists_languages = array_reverse($standards_lists_languages);
						$standards_lists_languages[''] = '';
						$standards_lists_languages = array_reverse($standards_lists_languages);
						$o->openField('Add Language ?');
						combo_box($prefix.'_add_intl_language', $standards_lists_languages, false, '');
					}

					$o->openField('Current');
					$current_langs = $owner->getLanguages();
					if (!empty($current_langs)) {
						sort($current_langs);
						?>
						<table class="sq-backend-table" style="width: 0%;">
							<?php if ($owner->writeAccess('settings')) {
							?>
							<tr>
								<td class="sq-backend-table-header">&nbsp;</td>
								<td class="sq-backend-table-header"><b>Delete ?</b></td>
							</tr>
							<?php } ?>
						<?php
						foreach ($current_langs as $lang_code) {
							?>
							<tr>
								<td class="sq-backend-table-cell"><?php echo $standards_lists_languages[$lang_code]; ?></td>
								<?php if ($owner->writeAccess('settings')) {
									$created = true;
								?>
								<td class="sq-backend-table-cell" align="center"><?php check_box($prefix.'_rem_intl_language['.$lang_code.']'); ?></td>
								<?php } ?>
							</tr>
							<?php
						}
						?></table><?php
					} else {
						echo '<i>No languages are currently set for "'.$owner->name.'"</i>';
					}
					$o->closeSection();
				break;

				case 'intl_character_set' :
					if ($field->children[0]->name != 'display_name') $display_name = 'Character Set';
					else $display_name = $field->children[0]->content;
					$o->openSection($display_name);
					$o->openField('Current');
					require SQ_FUDGE_PATH.'/standards_lists/character_sets.inc';

					if ($owner->writeAccess('settings')) {
						$created = true;
						$standards_lists_character_sets = array_reverse($standards_lists_character_sets);
						$standards_lists_character_sets[''] = '** Use System Default **';
						$standards_lists_character_sets = array_reverse($standards_lists_character_sets);
						combo_box($prefix.'_intl_character_set', $standards_lists_character_sets, false, $owner->charset);
					} else {
						if (isset($standards_lists_character_sets[$owner->charset])) {
							echo $standards_lists_character_sets[$owner->charset];
						} elseif ($owner->charset == '') {
							echo '<i>No character set is currently set for "'.$owner->name.'"</i>';
						} else {
							echo '<i>Unkown character set "'.$owner->charset.'" is currently set for "'.$owner->name.'"</i>';
						}
					}
					$o->closeSection();
				break;

				case 'morph_new_type' :
					if ($field->children[0]->name != 'display_name') $display_name = 'New Type';
					else $display_name = $field->children[0]->content;
					$o->openField($display_name);

					if ($owner->adminAccess('all')) {
						$created = true;
						if ($owner->status != SQ_STATUS_UNDER_CONSTRUCTION) {
							// cant morph an asset if it is not under construction
							echo '<i>An asset can only be morphed while Under Construction</i>';
						} else {
							$db = &$GLOBALS['SQ_SYSTEM']->db;
							$am = &$GLOBALS['SQ_SYSTEM']->am;
							$user = &$am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserid());

							$types = array_merge($am->getTypeAncestors($owner->type()), $am->getTypeDescendants($owner->type()));
							for($i = 0; $i < count($types); $i++) $types[$i] = $db->quote($types[$i]);
							$type_code_cond = 'IN ('.implode(', ', $types).')';

							$user_types = $am->getTypeAncestors($user->type());
							$user_types[] = $user->type();
							for($i = 0; $i < count($user_types); $i++) $user_types[$i] = $db->quote($user_types[$i]);
							$user_type_cond = 'IN ('.implode(', ', $user_types).')';

							$sql = 'SELECT type_code
									FROM '.SQ_TABLE_PREFIX.'asset_type
									WHERE allowed_access '.$user_type_cond.'
									  AND type_code '.$type_code_cond;

							$result = $db->getCol($sql);
							if (DB::isError($result)) {
								trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_ERROR);
								return Array();
							}

							if (empty($result)) {
								echo '<i>No types are available to morph this asset into</i>';
							} else {
								$morph_types = Array('' => '');
								foreach ($result as $type_code) {
									$code_name = str_replace('_', ' ', $type_code);
									$code_name = ucwords($code_name);
									$morph_types[$type_code] = $code_name;
								}
								combo_box($prefix.'_morph_new_type', $morph_types, false, '');
								?>
								<table border="0">
									<tr>
										<td class="sq-backend-data">To change the type of this asset, enter the 15 character string you see below into the box supplied, then press the Commit button</td>
									</tr>
									<tr>
										<td valign="top"><?php security_key(15, 20, 2); ?></td>
									</tr>
								</table>
								<?php
							}
						}
					} else {
						echo '<i>You do not have access to morph this asset</i>';
					}
				break;

				//// FRONTEND AND LOGIN DESIGN OPTIONS ////

				case 'designs' :
					$system_designs = Array(
											'system_design::frontend' => 'The Design that is to be used when displaying this asset on the frontend',
											'system_design::login'    => 'The Design that is to be used whenever someone attempts to access this asset without permission',
										);
					$our_designs = Array(
											'system_design::frontend' => 0,
											'system_design::login'    => 0,
										);

					$our_design_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($owner->id, SQ_LINK_TYPE_3, 'design', false);
					foreach($our_design_links as $link) {
						if (preg_match('/^(system_design|user_design)::.*$/', $link['value'])) {
							$our_designs[$link['value']] = $link['minorid'];
						}// end if
					}// end foreach

					// Get the designs that are associated with our lookups
					$design_lookups = $owner->getDesignLookups();

					// If we have a user design that is only effectively set, then we need to add it to the our_designs
					// array otherwise it doesn't appear below...
					foreach($design_lookups as $url => $designs) {
						foreach($designs as $design_name => $designid) {
							if (!isset($our_designs[$design_name])) $our_designs[$design_name] = 0;
						}
					}

					// now add the blank one that they can customise
					$our_designs['user_design::'] = 0;
					
					foreach($our_designs as $design_name => $designid) {

						$matches = Array();
						preg_match('/^(system|user)_design::(.*)$/', $design_name, $matches);
						$design_type     = $matches[1];
						$design_code     = $matches[2];
						$new_user_design = ($design_type == 'user' && $design_code == '' && !$designid);
						if ($new_user_design && !$owner->writeAccess('links')) continue;

						if (!$new_user_design) {
							$display_name = ucwords($design_type).' Defined - '.ucwords(str_replace('_', ' ', $design_code)).' Design';
						} else {
							$display_name = 'Create New User Defined Design';
						}
						$o->openSection($display_name);

						if ($new_user_design) {
							$o->openField('Design Code');
							text_box($prefix.'_designs['.$design_name.'][name]', '', 20, 30);
						} else {
							hidden_field($prefix.'_designs['.$design_name.'][name]', $design_name);
							if ($design_type == 'user') {
								$o->openField('Design Code');
									echo $design_code;
								$o->closeField();
							}
						}

						if (!$new_user_design) {

							$effective_designs = Array();
							foreach($design_lookups as $url => $designs) {
								if (empty($designs[$design_name])) continue;
								$effective_designs[$url] = $designs[$design_name];
							}

							$o->openField('Current');

							if (!empty($effective_designs)) {
								?>
								<table class="sq-backend-table">
									<tr>
										<td class="sq-backend-table-header">URL</td>
										<td class="sq-backend-table-header">Effective Design</td>
									</tr>
								<?php
								foreach ($effective_designs as $url => $effective_designid) {
									$effective_design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($effective_designid, '', true);
									if (is_null($effective_design)) continue;
									?>
									<tr>
										<td class="sq-backend-table-cell"><?php echo $url; ?></td>
										<td class="sq-backend-table-cell"><?php echo $effective_design->name.' (Id : # '.$effective_design->id.')'; ?></td>
									</tr>
									<?php
								}
								?>
								</table>
								<?php
							} else {
								echo 'There are no effective designs';
							}

							$o->closeField();
						}// endif
						
						$o->openField(($designid) ? 'Change ?' : 'New ?');

						if (!$owner->writeAccess('links')) {
							$created = true;
							echo '<i>';
							if (!$designid) {
								echo 'None';
							} else {
								$design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($designid, '', true);
								echo $design->name.' (Id : #'.$design->id.')';
							}
							echo '</i>';

						} else {
							$buttons = Array('clear');
							if (!$new_user_design) $buttons[] = 'reset';
							asset_finder($prefix.'_designs['.$design_name.'][designid]', $designid, Array('design' => 'D'), 'sq_sidenav', 'null', $buttons);
						}


						if ($design_type == 'system' && isset($system_designs[$design_name])) {
							$o->note($system_designs[$design_name]);
						}

						$o->closeField();
						
						$o->closeSection();

					}// end foreach

				break;
			}

			if ($print_note && isset($field->children[1]) && $field->children[1]->name == 'note' && $field->children[1]->content) {
				$o->note($field->children[1]->content);
			}

		}//end for

		$o->closeSection();

	}//end for

	return $created;

}//end paintSettings()


/**
* Processes the interface for editing various settings
* returns true if there is a change
*
* @param object	Asset					$owner		the asset whose interface we are painting
* @param object	Backend_Outputter		$o			the outputter class
* @param object	Asset_Edit_Interface	&$ei		the edit interface
*
* @access public
* @return boolean
*/
function processSettings(&$owner, &$o, &$ei)
{
	$prefix = $owner->getPrefix();

	// this is an array of assetids that are going to need their lookups updated
	$assets_affected_lookups = Array();

	//// VERSIONING ////
	if ($owner->adminAccess('all')) {
		if (isset($_POST[$prefix.'_version_update_version'])) {
			if (!$owner->incrementVersion('major')) {
				trigger_error('Failed updating major version of "'.$owner->name.'"', E_USER_WARNING);
			}
		}
	}

	//// DATES ////
	if ($owner->adminAccess('all')) {
		require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';

		foreach (Array('created', 'published') as $date_type) {

			// check to see if a user was selected for this particular date, 
			// otherwise use the current logged in user
			if (isset($_POST[$prefix.'_date_'.$date_type.'_userid']['assetid']) && $_POST[$prefix.'_date_'.$date_type.'_userid']['assetid'] != 0) {
				$userid = $_POST[$prefix.'_date_'.$date_type.'_userid']['assetid'];
			} else {
				$userid = $GLOBALS['SQ_SYSTEM']->currentUserid();
			}

			$value = '';
			$parameters = Array(
				'min'                   => '0000-00-00 00:00:00',
				'max'                   => '9999-12-31 12:59:59',
				'allow_circa'           => '0',
				'show'                  => Array('y', 'm', 'd', 'h', 'i', 's'),
				'null'                  => Array('y', 'm', 'd', 'h', 'i', 's'),
				'style'                 => Array(
												'y' => 't',
												'm' => 's',
												'd' => 's',
												'h' => 's',
												'i' => 's',
												's' => 's',
										),
				'print_format'          => 'l, F jS, Y H:i:s',
			);

			$date_field = new Datetime_Field($prefix.'_date_'.$date_type, $value, $parameters);
			$date_field->processField();

			// if the date is only partially complete, then don't set the date
			if (!$date_field->isEmpty() && !preg_match("/^[0-9]{4}-[0-9]{2}-[0-9]{2} ([0-9]{2}\:?){3}$/", $date_field->value)) continue;

			if ($date_type == 'created') {
				// make sure that the date that is specified is not null for the created date
				if ($date_field->isEmpty()) {
					trigger_error('The created date cannot be empty', E_USER_WARNING);
					continue;
				}
				$current_date   = $owner->created;
				$current_userid = $owner->created_userid;
			} else {
				$current_date   = $owner->published;
				$current_userid = $owner->published_userid;
			}

			if ($date_field->value != $current_date || $current_userid != $userid) {
				$timestamp = (!$date_field->isEmpty()) ? strtotime($date_field->value) : null;

				// the timestamp will be -1 if the date exceeds a 32bit integer
				if ($timestamp === -1) continue;

				$owner->setDate($timestamp, $userid, $date_type);
			}
		}//end foreach
	}


	//// FORCE SECURE ////
	if ($owner->adminAccess('settings')) {
		if ($owner->setForceSecure(!empty($_POST[$prefix.'_force_secure']))) {
			$assets_affected_lookups[] = $owner->id;
		}// end if
	}// end if


	//// LANGUAGES ////
	$current_langs = $owner->getLanguages();
	$set_languages = Array();

	if (!isset($_POST[$prefix.'_rem_intl_language'])) $_POST[$prefix.'_rem_intl_language'] = Array();

	// add a new language
	if (isset($_POST[$prefix.'_add_intl_language']) && !empty($_POST[$prefix.'_add_intl_language'])) {
		$new_language = $_POST[$prefix.'_add_intl_language'];
		if (!in_array($new_language, $current_langs)) $set_languages[] = $new_language;
	}

	// check for delete languages
	foreach ($current_langs as $lang_code) {
		if (!isset($_POST[$prefix.'_rem_intl_language'][$lang_code])) $set_languages[] = $lang_code;
	}

	$owner->setLanguages($set_languages);


	//// CHARACTER SET ////
	if (isset($_POST[$prefix.'_intl_character_set'])) {
		$new_charset = $_POST[$prefix.'_intl_character_set'];
		$owner->setCharset($new_charset);
	}


	//// MORPHING ////
	if ($owner->adminAccess('all') && $owner->status == SQ_STATUS_UNDER_CONSTRUCTION) {
		if (isset($_POST[$prefix.'_morph_new_type']) && !empty($_POST[$prefix.'_morph_new_type'])) {
			// check that the security key was entered correctly
			if (validate_security_key()) {
				// errors for morphing will get reported in the morph function
				$owner->morph($_POST[$prefix.'_morph_new_type']);
			} else {
				trigger_error('The security key entered was incorrect. Asset type remains unchanged.', E_USER_WARNING);
			}
		}
	}


	///// FRONTEND AND LOGIN DESIGN CHANGES ////
	if (!empty($_POST[$prefix.'_designs']) && $owner->writeAccess('links')) {

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$designs_ok = true;
		$designs_changed = false;

		foreach ($_POST[$prefix.'_designs'] as $design_name => $design_data) {

			$new_designid = $design_data['designid']['assetid'];

			if ($design_name == 'user_design::') {
				if (empty($design_data['name'])) continue;
				$old_designid = 0;
				$design_name .= strtolower($design_data['name']);

				if ($GLOBALS['SQ_SYSTEM']->am->getLink($owner->id, SQ_LINK_TYPE_3, 'design', false, $design_name)) {
					trigger_error('Design Name "'.$design_data['name'].'" already in use', E_USER_WARNING);
					continue;
				}// end if

			} else {
				$old_design_link = $GLOBALS['SQ_SYSTEM']->am->getLink($owner->id, SQ_LINK_TYPE_3, 'design', false, $design_name);
				$old_designid    = (empty($old_design_link)) ? 0 : $old_design_link['minorid'];

			}// end if

			// if they are the same then there is nothing to do.
			if ($old_designid == $new_designid) continue;

			// if there is an existing entry delete it
			if ($old_designid) {
				if (!$owner->deleteLink($old_design_link['linkid'])) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$designs_ok = false;
					break;
				}
				$assets_affected_lookups[] = $old_designid;
			}

			if ($new_designid) {
				$design = &$GLOBALS['SQ_SYSTEM']->am->getAsset($new_designid);
				if (is_null($design)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$designs_ok = false;
					break;
				}
				if (!$owner->createLink($design, SQ_LINK_TYPE_3, $design_name)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$designs_ok = false;
					break;
				}
			}

			$designs_changed = true;

		}// end foreach

		if ($designs_ok) {
			if ($designs_changed) $assets_affected_lookups[] = $owner->id;
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		}


	}// end if

	if (!empty($assets_affected_lookups)) {
		//// START HIPO PROCESSING ////
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assetids' => array_unique($assets_affected_lookups));
		$hh->queueHipo('hipo_job_update_lookups', $vars);
		////  END HIPO PROCESSING  ////
	}// endif

	return true;

}//end processSettings()


?>
