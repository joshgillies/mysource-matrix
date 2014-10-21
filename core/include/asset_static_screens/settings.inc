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
* $Id: settings.inc,v 1.62 2012/09/14 07:13:05 akarelia Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Settings Static Screen Functions
*
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.62 $
* @package MySource_Matrix
*/


/**
* Paints the interface for editing various settings
*
* @param object	&$owner	the asset whose interface we are painting
* @param object	&$o				the outputter class
* @param object	&$ei			the edit interface
* @param array	$keywords	the keywords to replace
*
* @return boolean
* @access public
*/
function paintSettings(&$owner, &$o, &$ei, $keywords=NULL)
{
	$root = $ei->findScreen('static_screen_settings');
	if (is_null($root)) return FALSE;

	$prefix = $owner->getPrefix();

	$created = FALSE;

	if (count($root->children()) == 0) {
		$o->openSection(sprintf(translate('There are no settings for "%s"'), htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET)));

		$o->closeSection();
	}

	foreach ($root->section as $section) {

		// if we are in limbo and this section is not to be printed in limbo - dont print it
		if (is_null($keywords)) {
			if (SQ_IN_LIMBO && ((string) $section['name'] === 'limbo_access' || (int) $section['limbo_access'] === 0)) {
				continue;
			}
		}

		if (!is_null($keywords) && isset($keywords['sections'][strtolower($section->attributes()->name)])) {
			ob_start();
		}

		$o->openSection((is_null($section['display_name'])) ? $section['name'] : $section['display_name']);

		foreach ($section->children() as $field) {
			// should we print the note for the field or not
			$print_note = TRUE;

			switch (strtolower((string) $field['code'])) {
				case 'version_current_version' :
					if ((string) $field['children'][0]['name'] != 'display_name') {
						$display_name = translate('Current Version');

					} else {
						$display_name = (string) $field['children'][0]['content'];
					}
					$o->openField($display_name);
					if (!is_null($keywords) && isset($keywords['fields'][strtolower((string) $field['code'])])) {
						ob_start();
					}
					echo $owner->version;
					if (!is_null($keywords) && isset($keywords['fields'][strtolower((string) $field['code'])])) {
						$ei->_tmp['layout_keyword_replacements'][$keywords['fields'][strtolower((string) $field['code'])]] = ob_get_contents();
						ob_end_flush();
					}
					$o->closeField();
				break;
				case 'version_update_version' :
					// have to have admin access to update the major version
					if ($owner->adminAccess('all')) {
						$created = TRUE;

						if ((string) $field['children'][0]['name'] != 'display_name') {
							$display_name = translate('Update Major Version?');

						} else {
							$display_name = (string) $field['children'][0]['content'];
						}
						$o->openField($display_name);
						if (!is_null($keywords) && isset($keywords['fields'][strtolower((string) $field['code'])])) {
							ob_start();
						}

						check_box($prefix.'_version_update_version');
						label(translate('Increment major version number'), $prefix.'_version_update_version');

						if (!is_null($keywords) && isset($keywords['fields'][strtolower((string) $field['code'])])) {
							$ei->_tmp['layout_keyword_replacements'][$keywords['fields'][strtolower((string) $field['code'])]] = ob_get_contents();
							ob_end_flush();
						}

						if (!is_null($field->note) && (strlen((string)$field->note) > 0)) {
							$o->note((string) $field->note);
						}

						$o->closeField();
					} else {
						$print_note = FALSE;
					}
				break;

				case 'date_created_date' :
				case 'date_published_date' :

					$date_type = (strtolower((string) $field['code']) === 'date_created_date') ? 'created' : 'published';
					require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';
					$parameters = Array(
									'min'			=> '0000-00-00 00:00:00',
									'max'			=> '9999-12-31 12:59:59',
									'allow_circa'	=> '0',
									'show'			=> Array('y', 'm', 'd', 'h', 'i', 's'),
									'null'			=> Array('y', 'm', 'd', 'h', 'i', 's'),
									'style'			=> Array(
														'y'	=> 't',
														'm'	=> 's',
														'd'	=> 's',
														'h'	=> 's',
														'i'	=> 's',
														's'	=> 's',
													   ),
									'print_format'	=> 'l, F jS, Y H:i:s',
								  );

					if ($date_type == 'created') {
						$value  = (!empty($owner->created)) ? ts_iso8601($owner->created) : '---------- --:--:--';
						$userid = $owner->created_userid;
					} else {
						$value  = (!empty($owner->published)) ? ts_iso8601($owner->published) : '---------- --:--:--';
						$userid = $owner->published_userid;
					}

					if ((string) $field['children'][0]['name'] !== 'display_name') {
					    if (strtolower((string) $field['code']) === 'date_created_date') {
					        $display_name = translate('Created Date');
						} else {
					        $display_name = translate('Last Published Date');
						}
					} else {
						$display_name = (string) $field['children'][0]['content'];
					}
					$o->openField($display_name);
					if (!is_null($keywords) && isset($keywords['fields'][strtolower((string) $field['code'])])) {
						ob_start();
					}

					$date_field = new Datetime_Field($prefix.'_date_'.$date_type, $value, $parameters);

					if ($owner->adminAccess('all')) {
						$date_field->printField();

						?>
							<script language="javascript">
								var TIME_UNITS = new Array();
								<?php
								$units = Array(
											'Y'	=> 'y',
											'n'	=> 'm',
											'j'	=> 'd',
											'G'	=> 'h',
											'i'	=> 'i',
											's'	=> 's',
										 );

								foreach ($units as $php_unit => $js_unit) {
									echo 'TIME_UNITS[\''.$js_unit.'\'] = \''.date($php_unit).'\';';
								}
								?>

								function setCurrentTime(prefix) {
									for (u in TIME_UNITS) {
										var id = prefix + 'value_' + u;
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
						normal_button($prefix.'_set_time_'.$date_type, strtolower(translate('Now')), 'setCurrentTime(\''.$prefix.'_date_'.$date_type.'\');');

						echo '&nbsp;<b>'.strtolower(translate('By')).'</b>&nbsp;';

						asset_finder($prefix.'_date_'.$date_type.'_userid', $userid, Array('user' => 'D'));

						if (!is_null($field->note) && (strlen((string)$field->note) > 0)) {
							$o->note((string) $field->note);
						}

					} else {
						$print_note = FALSE;
						if ($date_field->isEmpty() && $date_type == 'published') {
							echo translate('This asset has never been published');

						} else {
							$info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($userid), 'user', FALSE);
							if (empty($info)) {
								$info[$userid] = Array('name' => '&lt;unknown&gt;');
							}

							echo ucfirst($date_type).' '.strtolower(translate('By')).' '.htmlspecialchars($info[$userid]['name'], ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET).' '.strtolower(translate('On')).' '.date('d M Y H:i:s', strtotime($date_field->value));



						}
					}
					if (!is_null($keywords) && isset($keywords['fields'][strtolower((string) $field['code'])])) {
						$ei->_tmp['layout_keyword_replacements'][$keywords['fields'][strtolower((string) $field['code'])]] = ob_get_contents();
						ob_end_flush();
					}
					$o->closeField();

				break;

				case 'force_secure' :
					if ((string) $field['children'][0]['name'] != 'display_name') {
						$display_name = translate('Force Secure');

					} else {
						$display_name = $field['children'][0]['content'];
					}
					$o->openField($display_name);
					if (!is_null($keywords) && isset($keywords['fields'][strtolower((string) $field['code'])])) {
						ob_start();
					}
					$current_langs = $owner->getLanguages();
					// have to have admin access to set the force secure
					$options = Array(
								'0'	=> 'Allow all protocols',
								'1'	=> 'Force secure protocol (HTTPS)',
								'-'	=> 'Force insecure protocol (HTTP)',
							   );
					if ($owner->adminAccess('settings')) {
						$created = TRUE;
						combo_box($prefix.'_force_secure', $options, FALSE, $owner->force_secure);
						//check_box($prefix.'_force_secure', '1', ($owner->force_secure == '1'));
					} else {
						echo $options[$owner->force_secure];
					/* ?>
						<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo ($owner->force_secure) ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
					<?php */
					}
					if (!is_null($keywords) && isset($keywords['fields'][strtolower((string) $field['code'])])) {
						$ei->_tmp['layout_keyword_replacements'][$keywords['fields'][strtolower((string) $field['code'])]] = ob_get_contents();
						ob_end_flush();
					}

					if (!is_null($field->note) && (strlen((string)$field->note) > 0)) {
						$o->note((string) $field->note);
					}

					$o->closeField();
				break;


				case 'intl_languages' :

					if ($field['children'][0]['name'] != 'display_name') {
						$display_name = translate('Languages');

					} else {
						$display_name = $field['children'][0]['content'];
					}

					if (!is_null($keywords) && isset($keywords['fields'][strtolower((string) $field['code'])])) {
						ob_start();
					}
					$o->openSection($display_name);
					require SQ_FUDGE_PATH.'/standards_lists/languages.inc';

					if ($owner->writeAccess('settings')) {
						$created = TRUE;
						$standards_lists_languages = array_reverse($standards_lists_languages);
						$standards_lists_languages[''] = '';
						$standards_lists_languages = array_reverse($standards_lists_languages);
						$o->openField(translate('Add Language?'));

						combo_box($prefix.'_add_intl_language', $standards_lists_languages, FALSE, '');
						$o->closeField();
					}

					$o->openField(translate('Current'));

					$current_langs = $owner->getLanguages();
					if (!empty($current_langs)) {
						sort($current_langs);
						?>
						<table class="sq-backend-table">
							<?php if ($owner->writeAccess('settings')) {
							?>
							<tr>
								<td class="sq-backend-table-header">&nbsp;</td>
								<td class="sq-backend-table-header"><b><?php echo translate('Delete?'); ?></b></td>

							</tr>
							<?php }
							?>
						<?php
						foreach ($current_langs as $lang_code) {
							?>
							<tr>
								<td class="sq-backend-table-cell"><?php echo $standards_lists_languages[$lang_code]; ?></td>
								<?php if ($owner->writeAccess('settings')) {
									$created = TRUE;
								?>
								<td class="sq-backend-table-cell" align="center"><?php check_box($prefix.'_rem_intl_language['.$lang_code.']'); ?></td>
								<?php }
								?>
							</tr>
							<?php
						}
						?></table><?php
					} else {
						echo sprintf(translate('<i>No languages are currently set for "%s"</i>'), htmlspecialchars($owner->name, ENT_COMPAT, SQ_CONF_DEFAULT_CHARACTER_SET));

					}

					if (!is_null($field->note) && (strlen((string)$field->note) > 0)) {
						$o->note((string) $field->note);
					}

					$o->closeField();

					$o->closeSection();
					if (!is_null($keywords) && isset($keywords['fields'][strtolower((string) $field['code'])])) {
						$ei->_tmp['layout_keyword_replacements'][$keywords['fields'][strtolower((string) $field['code'])]] = ob_get_contents();
						ob_end_flush();
					}
				break;

				case 'intl_character_set' :
					if ($field['children'][0]['name'] != 'display_name') {
						$display_name = translate('Character Set');

					} else {
						$display_name = $field['children'][0]['content'];
					}

					if (!is_null($keywords) && isset($keywords['fields'][strtolower((string) $field['code'])])) {
						ob_start();
					}


					if (!is_null($keywords) && isset($keywords['fields'][strtolower((string) $field['code'])])) {
						$ei->_tmp['layout_keyword_replacements'][$keywords['fields'][strtolower((string) $field['code'])]] = ob_get_contents();
						ob_end_flush();
					}
				break;

				case 'morph_new_type' :
					if ($field['children'][0]['name'] != 'display_name') {
						$display_name = translate('New Type');

					} else {
						$display_name = $field['children'][0]['content'];
					}

					$o->openField($display_name);
					if (!is_null($keywords) && isset($keywords['fields'][strtolower((string) $field['code'])])) {
						ob_start();
					}

					if ($owner->adminAccess('all')) {
						$created = TRUE;
						if ($owner->status != SQ_STATUS_UNDER_CONSTRUCTION) {
							// cant morph an asset if it is not under construction
							echo translate('<i>An asset can only be morphed while Under Construction</i>');

						} else {
							$db = MatrixDAL::getDb();
							$am = $GLOBALS['SQ_SYSTEM']->am;
							$user = $am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserid());

							$types = array_merge($am->getTypeAncestors($owner->type()), $am->getTypeDescendants($owner->type()));
							for ($i = 0; $i < count($types); $i++) {
								$types[$i] = '\''.$types[$i].'\'';
							}
							$type_code_cond = 'IN ('.implode(', ', $types).')';

							$user_types = $am->getTypeAncestors($user->type());
							$user_types[] = $user->type();
							for ($i = 0; $i < count($user_types); $i++) {
								$user_types[$i] = '\''.$user_types[$i].'\'';
							}
							$user_type_cond = 'IN ('.implode(', ', $user_types).')';

							$sql = 'SELECT type_code
									FROM sq_ast_typ
									WHERE allowed_access '.$user_type_cond.'
									  AND type_code '.$type_code_cond;

							try {
								$query = MatrixDAL::preparePdoQuery($sql);
								$result = MatrixDAL::executePdoAssoc($query, 0);
							} catch (Exception $e) {
								throw new Exception('Unable to get type codes for these user types: '.$user_types.' due to database error: '.$e->getMessage());
							}

							if (empty($result)) {
								echo translate('<i>No types are available to morph this asset into</i>');

							} else {
								$morph_types = Array('' => '');
								foreach ($result as $type_code) {
									$code_name = str_replace('_', ' ', $type_code);
									$code_name = ucwords($code_name);
									$morph_types[$type_code] = $code_name;
								}
								combo_box($prefix.'_morph_new_type', $morph_types, FALSE, '');
								?>
								<br/><br/>
								<table>
									<tr>
										<td class="sq-backend-data"><p><?php echo translate('To confirm the change of type for this asset, enter the character string you see below into the box supplied, then press the Commit button.'); ?></p></td>

									</tr>
									<tr>
										<td valign="top"><?php security_key(4, 4, 2); ?></td>
									</tr>
								</table>
								<?php
							}
						}//end else
					} else {
						echo translate('<i>You do not have access to morph this asset</i>');

					}

					if (!is_null($keywords) && isset($keywords['fields'][strtolower((string) $field['code'])])) {
						$ei->_tmp['layout_keyword_replacements'][$keywords['fields'][strtolower((string) $field['code'])]] = ob_get_contents();
						ob_end_flush();
					}

					if (!is_null($field->note) && (strlen((string)$field->note) > 0)) {
						$o->note((string) $field->note);
					}

					$o->closeField();
				break;

				//// FRONTEND AND LOGIN DESIGN OPTIONS ////

				case 'designs' :
					if (!is_null($keywords) && isset($keywords['fields'][strtolower((string) $field['code'])])) {
						ob_start();
					}
					$system_designs = Array(
										'design::system::frontend'			=> translate('The Design that is to be used when displaying this asset on the frontend'),

										'design::system::login'				=> translate('The Design that is to be used whenever someone attempts to access this asset without permission'),

										'design::system::password_change'	=> translate('The Design that is to be used whenever the user needs to change their password'),

									  );
					$our_designs = Array(
									'design::system::frontend'			=> 0,
									'design::system::login'				=> 0,
									'design::system::password_change'	=> 0,
								   );
					$override_designs = Array(
											'design::system::frontend'			=> 0,
											'design::system::login'				=> 0,
											'design::system::password_change'	=> 0,
										);

					$our_design_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($owner->id, SQ_LINK_NOTICE, 'design', FALSE);

					foreach ($our_design_links as $link) {
						if (preg_match('/^design::(system|user)::.*$/', $link['value'])) {
							$our_designs[$link['value']] = $link['minorid'];
						} else if (preg_match('/^override::design::(system|user)::.*$/', $link['value'])) {
							$design_name = preg_replace('/^override::(.*)$/', '$1', $link['value']);
							$override_designs[$design_name] = $link['minorid'];
						}// end if
					}// end foreach

					// Get the designs that are associated with our lookups
					$design_lookups = $owner->getDesignLookups();

					// If we have a user design that is only effectively set, then we need to add it to the our_designs
					// array otherwise it doesn't appear below...
					foreach ($design_lookups as $url => $designs) {
						foreach ($designs as $design_name => $designid) {
							if (!isset($our_designs[$design_name])) {
								$our_designs[$design_name] = 0;
							}
						}
					}

					// now add the blank one that they can customise
					$our_designs['design::user::'] = 0;

					foreach ($our_designs as $design_name => $designid) {

						$override_designid = array_get_index($override_designs, $design_name, 0);
						$matches = Array();
						preg_match('/^design::(system|user)::(.*)$/', $design_name, $matches);

						$design_type     = $matches[1];
						$design_code     = $matches[2];
						$new_user_design = ($design_type == 'user' && $design_code == '' && !$designid);
						if ($new_user_design && !$owner->writeAccess('links')) {
							continue;
						}

						if (!$new_user_design) {
							$display_name = sprintf(translate('%1$s Defined %2$s Design'), ucwords($design_type), ucwords(str_replace('_', ' ', $design_code)));

						} else {
							$display_name = translate('Create New User Defined Design');

						}

						$o->openSection($display_name);

						if ($new_user_design) {
							$o->openField(translate('Design Code'));

								text_box($prefix.'_designs['.$design_name.'][name]', '', 20, 30);
							$o->closeField();
						} else {
							$o->openRaw();
								hidden_field($prefix.'_designs['.$design_name.'][name]', $design_name);
							$o->closeRaw();

							if ($design_type == 'user') {
								$o->openField(translate('Design Code'));

									echo $design_code;
								$o->closeField();
							}
						}

						if (!$new_user_design) {
							$effective_designs = Array();
							foreach ($design_lookups as $url => $designs) {
								if (empty($designs[$design_name])) continue;
								$effective_designs[$url] = $designs[$design_name];
							}

							$o->openField(translate('Current'));


							if (!empty($effective_designs)) {
								?>
								<table class="sq-backend-table">
									<tr>
										<th class="sq-backend-table-header"><?php echo translate('URL'); ?></th>

										<th class="sq-backend-table-header"><?php echo translate('Effective Design'); ?></th>

									</tr>
								<?php
								foreach ($effective_designs as $url => $effective_designid) {
									$effective_design_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($effective_designid));
									if (empty($effective_design_info)) continue;
									?>
									<tr>
										<td class="sq-backend-table-cell"><?php echo $url; ?></td>
										<td class="sq-backend-table-cell"><?php echo get_asset_tag_line($effective_designid); ?></td>
									</tr>
									<?php
								}
								?>
								</table>
								<?php
							} else {
								echo translate('There are no effective designs');

							}

							$o->closeField();
						}//end if

						$o->openField(($designid) ? translate('Change?') : translate('New?'));



						if (!$owner->writeAccess('links')) {
							echo '<i>';
							if (!$designid) {
								echo translate('None');

							} else {
								echo get_asset_tag_line($designid);
							}
							echo '</i>';

						} else {
							$created = TRUE;
							$buttons = Array('clear');
							if (!$new_user_design) $buttons[] = 'reset';
							asset_finder($prefix.'_designs['.$design_name.'][designid]', $designid, Array('design' => 'D'), 'sq_sidenav', FALSE, 'null', $buttons);
						}


						if ($design_type == 'system' && isset($system_designs[$design_name])) {
							$o->note($system_designs[$design_name]);
						}

						$o->closeField();

						$o->openField(translate('Override?'));


						if (!$owner->writeAccess('links')) {
							echo '<i>';
							if (!$override_designid) {
								echo translate('None');

							} else {
								echo get_asset_tag_line($override_designid);
							}
							echo '</i>';

						} else {
							$created = TRUE;
							$buttons = Array('clear');
							if (!$new_user_design) $buttons[] = 'reset';
							asset_finder($prefix.'_designs['.$design_name.'][override]', $override_designid, Array('design' => 'D'), 'sq_sidenav', FALSE, 'null', $buttons);
						}


						$o->note(translate('You can override the design this asset would inherit from its parent, with a design that is applied to this asset only. This override design does not affect the inheritance to this asset\'s children of any parent design.'));



						$o->closeField();

						$o->closeSection();
					}//end foreach

					if (!is_null($keywords) && isset($keywords['fields'][strtolower((string) $field['code'])])) {
						$ei->_tmp['layout_keyword_replacements'][$keywords['fields'][strtolower((string) $field['code'])]] = ob_get_contents();
						ob_end_flush();
					}

				break;

			}//end switch

			if (!is_null($keywords) && isset($keywords['fields'][strtolower((string) $field['code'])])) {
				ob_start();
			}
			// before printing the note, we make sure that the note element exists and is not empty
			/*if ($print_note && (!is_null($field->note)) && (strlen((string)$field->note) > 0)) {
				$o->openField('&nbsp;');
				$o->note((string) $field->note);
				$o->closeField();
			}*/
			if (!is_null($keywords) && isset($keywords['fields'][strtolower((string) $field['code'])])) {
						$ei->_tmp['layout_keyword_replacements'][$keywords['fields'][strtolower((string) $field['code'])]] .= ob_get_contents();
						ob_end_flush();
			}

		}//end for

		$o->closeSection();

		if (!is_null($keywords) && isset($keywords['sections'][strtolower((string) $section['name'])])) {
			$ei->_tmp['layout_keyword_replacements'][$keywords['sections'][strtolower((string) $section['name'])]] = ob_get_contents();
			ob_end_flush();
		}

	}//end for


	return $created;

}//end paintSettings()


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
function processSettings(&$owner, &$o, &$ei)
{
	$prefix = $owner->getPrefix();

	// this is an array of assetids that are going to need their lookups updated
	$assets_affected_lookups = Array();

	//// VERSIONING ////
	if ($owner->adminAccess('all')) {
		if (isset($_POST[$prefix.'_version_update_version'])) {
			if (!$owner->incrementVersion('major')) {
				trigger_localised_error('SYS0184', sprintf(translate('Failed updating major version of "%s"'), $owner->name), E_USER_WARNING);
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
							'min'			=> '0000-00-00 00:00:00',
							'max'			=> '9999-12-31 12:59:59',
							'allow_circa'	=> '0',
							'show'			=> Array('y', 'm', 'd', 'h', 'i', 's'),
							'null'			=> Array('y', 'm', 'd', 'h', 'i', 's'),
							'style'			=> Array(
												'y'	=> 't',
												'm'	=> 's',
												'd'	=> 's',
												'h'	=> 's',
												'i'	=> 's',
												's'	=> 's',
											   ),
							'print_format'	=> 'l, F jS, Y H:i:s',
						  );

			$date_field = new Datetime_Field($prefix.'_date_'.$date_type, $value, $parameters);
			$date_field->processField();

			// if the date is only partially complete, then don't set the date
			if (!$date_field->isEmpty() && !preg_match("/^[0-9]{4}-[0-9]{2}-[0-9]{2} ([0-9]{2}\:?){3}$/", $date_field->value)) {
				continue;
			}

			if ($date_type == 'created') {
				// make sure that the date that is specified is not null for the created date
				if ($date_field->isEmpty()) {
					trigger_localised_error('SYS0212', translate('The created date cannot be empty'), E_USER_WARNING);
					continue;
				}
				$current_date   = $owner->created;
				$current_userid = $owner->created_userid;
			} else {
				$current_date   = $owner->published;
				$current_userid = $owner->published_userid;
			}

			if ($date_field->value != $current_date || $current_userid != $userid) {
				$timestamp = (!$date_field->isEmpty()) ? strtotime($date_field->value) : NULL;

				// the timestamp will be -1 if the date exceeds a 32bit integer
				if ($timestamp === -1) continue;

				$owner->setDate($date_type, $timestamp, $userid);
			}
		}//end foreach
	}//end if


	//// FORCE SECURE ////
	if ($owner->adminAccess('settings')) {
		if ($owner->setForceSecure(array_get_index($_POST, $prefix.'_force_secure', 0))) {
			$assets_affected_lookups[] = $owner->id;
		}// end if
	}// end if


	//// LANGUAGES ////
	$current_langs = $owner->getLanguages();
	$set_languages = Array();

	if (!isset($_POST[$prefix.'_rem_intl_language'])) {
		$_POST[$prefix.'_rem_intl_language'] = Array();
	}

	// add a new language
	if (isset($_POST[$prefix.'_add_intl_language']) && !empty($_POST[$prefix.'_add_intl_language'])) {
		$new_language = $_POST[$prefix.'_add_intl_language'];
		if (!in_array($new_language, $current_langs)) {
			$set_languages[] = $new_language;
		}
	}

	// check for delete languages
	foreach ($current_langs as $lang_code) {
		if (!isset($_POST[$prefix.'_rem_intl_language'][$lang_code])) {
			$set_languages[] = $lang_code;
		}
	}

	$owner->setLanguages($set_languages);




	//// MORPHING ////
	if ($owner->adminAccess('all') && $owner->status == SQ_STATUS_UNDER_CONSTRUCTION) {
		if (isset($_POST[$prefix.'_morph_new_type']) && !empty($_POST[$prefix.'_morph_new_type'])) {
			// check that the security key was entered correctly
			if (validate_security_key()) {
				// errors for morphing will get reported in the morph function
				$owner->morph($_POST[$prefix.'_morph_new_type']);
			} else {
				trigger_localised_error('SYS0214', translate('The security key entered was incorrect. Asset type remains unchanged.'), E_USER_WARNING);
			}
		}
	}

	$update_design_cascade  = FALSE;
	///// FRONTEND AND LOGIN DESIGN CHANGES ////
	if (!empty($_POST[$prefix.'_designs']) && $owner->writeAccess('links')) {

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$designs_ok = TRUE;
		$designs_changed = FALSE;

		foreach ($_POST[$prefix.'_designs'] as $design_name => $design_data) {

			$new_designid = $design_data['designid']['assetid'];
			$new_override_designid = $design_data['override']['assetid'];

			if ($design_name == 'design::user::') {
				if (empty($design_data['name'])) continue;
				$old_designid = 0;
				$design_name .= strtolower($design_data['name']);

				if ($GLOBALS['SQ_SYSTEM']->am->getLink($owner->id, SQ_LINK_NOTICE, 'design', FALSE, $design_name)) {
					trigger_localised_error('SYS0134', sprintf(translate('Design Name "%s" already in use'), $design_data['name']), E_USER_WARNING);
					continue;
				}// end if

			} else {
				$old_design_link = $GLOBALS['SQ_SYSTEM']->am->getLink($owner->id, SQ_LINK_NOTICE, 'design', FALSE, $design_name);
				$old_designid    = (empty($old_design_link)) ? 0 : $old_design_link['minorid'];

				$old_override_design_link = $GLOBALS['SQ_SYSTEM']->am->getLink($owner->id, SQ_LINK_NOTICE, 'design', FALSE, 'override::'.$design_name);
				$old_override_designid    = (empty($old_override_design_link)) ? 0 : $old_override_design_link['minorid'];

			}// end if

			// if they are the same then there is nothing to do.
			if ($new_designid != $old_designid) {
				// if there is an existing entry delete it
				if ($old_designid) {
					if (!$owner->deleteLink($old_design_link['linkid'])) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$designs_ok = FALSE;
						break;
					}
				}

				if ($new_designid) {
					$design = $GLOBALS['SQ_SYSTEM']->am->getAsset($new_designid);
					if (is_null($design) || !$design instanceof Design) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$designs_ok = FALSE;
						break;
					}
					if (!$owner->createLink($design, SQ_LINK_NOTICE, $design_name)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$designs_ok = FALSE;
						break;
					}
				}
				$update_design_cascade	= TRUE;
			}

			// same with the overrides
			if ($new_override_designid != $old_override_designid) {
				// if there is an existing entry delete it
				if ($old_override_designid) {
					if (!$owner->deleteLink($old_override_design_link['linkid'])) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$designs_ok = FALSE;
						break;
					}
				}

				if ($new_override_designid) {
					$design = $GLOBALS['SQ_SYSTEM']->am->getAsset($new_override_designid);
					if (is_null($design)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$designs_ok = FALSE;
						break;
					}
					if (!$owner->createLink($design, SQ_LINK_NOTICE, 'override::'.$design_name)) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						$designs_ok = FALSE;
						break;
					}
				}
			}

			$designs_changed = TRUE;

		}//end foreach

		if ($designs_ok) {
			if ($designs_changed) {
				if ($update_design_cascade == TRUE) {
					$assets_affected_lookups[] = $owner->id;
				}//end if
			}//end if
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		}//end if

		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

	}//end if

	if (!$owner->updateLookups()) return FALSE;
	return TRUE;

}//end processSettings()


?>
