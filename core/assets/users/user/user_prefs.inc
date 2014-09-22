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
* $Id: user_prefs.inc,v 1.22 2013/10/24 01:07:31 cupreti Exp $
*
*/


require_once SQ_LIB_PATH.'/config/prefs.inc';
require_once SQ_ATTRIBUTES_PATH.'/duration/duration.inc';
require_once SQ_ATTRIBUTES_PATH.'/int/int.inc';



/**
* User_Prefs
*
* Purpose
*
*    Looks after the session expiry prefences for user groups
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.22 $
* @package MySource_Matrix
*/
class User_Prefs extends Prefs
{
	var $pref_vars = Array (
						'SQ_USER_SESSION_PREFS'		=> Array(
														'name'			=> 'user_prefs_session_settings_header',
														'description'	=> 'This preference allows you to change the session settings for this user group',
														'default'		=> Array(Array('persist' => '0', 'timeout' => '0', 'max_length' => '0')),
														'protected'		=> FALSE,
													   ),
						'SQ_USER_ASSET_MAP_LINEAGE'	=> Array(
														'name'			=> 'user_prefs_asset_map_root_asset_header',
														'description'	=> 'This preference allows you to change the root node that is displated in the Asset Map',
														'default'		=> '',
														'protected'		=> FALSE,
													   ),
						'SQ_USER_SAFE_TYPE3_TRASH'	=> Array(
														'name'			=> 'safe_trash',
														'description'	=> 'This preference allows you to prohibit the deletion of an asset until all notice links are removed',
														'default'		=> FALSE,
														'protected'		=> FALSE,
													   ),
						'SQ_USER_ASSET_MAP_WIDTH'	=> Array(
														'name'			=> 'asset_map_width',
														'description'	=> 'The asset map frame width in pixels.',
														'default'		=> 300,
														'protected'		=> FALSE,
													   ),
						'SQ_USER_COMMIT_BUTTON_POS'	=> Array(
														'name'			=> 'commit_button_position',
														'description'	=> '',
														'default'		=> TRUE,
														'protected'		=> FALSE,
													   ),
						'SQ_USER_LIMBO_HIDE_FRAMES'	=> Array(
														'name'			=> 'user_prefs_limbo_hide_frames',
														'description'	=> 'This preference determines whether or not the default frames are painted in Simple Edit',
														'default'		=> TRUE,
														'protected'		=> FALSE,
													   ),
						'SQ_USER_ATTRIBUTE_IMAGE'		=> Array(
														'name'			=> 'user_prefs_attribute_image',
														'description'	=> 'This preference allows you to specify settings for the attribute images',
														'default'		=> Array('face' => '3', 'size' => '12', 'hpad' => '0', 'vpad' => '0', 'fgcol' => '000000', 'bgcol' => 'FFFFFF'),
														'protected'		=> FALSE,
													   ),
						'SQ_USER_SET_CONTEXT_ON_LOGIN'	=> Array(
															'name'			=> 'user_prefs_set_context_on_login',
															'description'	=> 'Set the selected context when a user in this user group logs in.',
															'default'		=> 'default',
															'protected'		=> FALSE,
														   ),
						'SQ_USER_FILTER_FRONT_END_INPUT'	=> Array(
														'name'			=> 'filter_front_end_user_input',
														'description'	=> 'Filter (strip keywords, strip script tags and escape html) of any input by the user from the front end.',
														'default'		=> TRUE,
														'protected'		=> FALSE,
													   ),
					 );

	var $int_am_frame_width;
	var $int_am_frame_width_default = 300;


	/**
	* Constructor
	*
	*/
	function User_Prefs($pref_file='')
	{
		$this->Prefs($pref_file);

		// attribute for input of SQ_COMMENT_MAX_LENGTH
		$this->int_am_frame_width = new Asset_Attribute_Int();
		$this->int_am_frame_width->setParam('allow_negative', FALSE);
		$this->int_am_frame_width->setParam('allow_empty', FALSE);
		$this->int_am_frame_width->setParam('range_lower', $this->int_am_frame_width_default);

	}//end constructor


	/**
	* Paints the backend interface to edit session preferences
	*
	* @param object		&$o			reference to the backend outputter
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to paint
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$o, $have_lock, $pref=NULL)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		if (is_null($pref) || $pref == 'SQ_USER_SESSION_PREFS') {
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_USER_SESSION_PREFS']['name']));
			}
			$session_settings = $this->pref_vars['SQ_USER_SESSION_PREFS']['default'];
			?>
			<?php
			foreach ($session_settings as $id => $extra_setting) {
			?>
			<div class="sq-backend-table sq-backend-table-cell">
				<strong><?php echo ($id == '0') ? translate('Default').' '.translate('Rule') : sprintf(translate('Rule %s'), $id); ?>:</strong><br />



				<?php echo translate('End the user\'s session'); ?>...

				<ul id="user-prefs">
					<li>
						<?php
						if ($have_lock && $is_admin) {
							check_box('prefs[SQ_USER_SESSION_PREFS]['.$id.'][persist]', '0', ($extra_setting['persist'] == 0), '', 'id="prefs_SQ_USER_SESSION_PREFS_'.$id.'_persist"');
							label(translate('When the user closes the browser'), 'prefs_SQ_USER_SESSION_PREFS_'.$id.'_persist');

						} else {
							echo '<img src="'.sq_web_path('lib').'/web/images/'.(empty($extra_setting['persist']) ? 'tick' : 'cross').'.gif" />';
							echo translate('When the user closes the browser');

						}
						?>
					</li>
					<li id="timeout<?php echo $id; ?>">
						<?php
						$dur = new Asset_Attribute_Duration(0, $extra_setting['timeout']);
						if ($have_lock && $is_admin) {
							check_box('prefs[SQ_USER_SESSION_PREFS]['.$id.'][timeout_enabled]', '1', ($extra_setting['timeout'] != 0), 'toggleInputs(this.parentNode)', 'id="prefs_SQ_USER_SESSION_PREFS_'.$id.'_timeout_enabled"');
							label(translate('When the time since their last Matrix-Activity* is longer than'), 'prefs_SQ_USER_SESSION_PREFS_'.$id.'_timeout_enabled');

							echo ' ';
						} else {
							echo '<img src="'.sq_web_path('lib').'/web/images/'.(!empty($extra_setting['timeout']) ? 'tick' : 'cross').'.gif" />';
							echo translate('When the time since their last Matrix-Activity* is longer than');

						}
						$dur->paint('prefs_dur_timeout_'.$id, !$have_lock);
						?>
					</li>
					<li id="max_length<?php echo $id; ?>">
						<?php
						$dur = new Asset_Attribute_Duration(0, $extra_setting['max_length']);
						if ($have_lock && $is_admin) {
							check_box('prefs[SQ_USER_SESSION_PREFS]['.$id.'][max_length_enabled]', '1', ($extra_setting['max_length'] != 0), 'toggleInputs(this.parentNode)', 'id="prefs_SQ_USER_SESSION_PREFS_'.$id.'_max_length_enabled"');
							label(translate('When the time since their last login is longer than'), 'prefs_SQ_USER_SESSION_PREFS_'.$id.'_max_length_enabled');

							echo ' ';
						} else {
							echo '<img src="'.sq_web_path('lib').'/web/images/'.(!empty($extra_setting['max_length']) ? 'tick' : 'cross').'.gif" />';
							echo translate('When the time since their last login is longer than');

						}
						$dur->paint('prefs_dur_max_length_'.$id, !$have_lock); ?>
					</li>
					<li>
						<?php
						// Do some checks first
						if (!isset($extra_setting['ip_from'])) {
							$extra_setting['ip_from'] = '';
						}//end if
						if (!isset($extra_setting['ip_to'])) {
							$extra_setting['ip_to'] = '';
						}//end if

						// Paint the interface
						if ($have_lock && $is_admin) {
							$js = "onChange=\"Javascript: if (this.checked == false) { document.getElementById('prefs[SQ_USER_SESSION_PREFS][".$id."][ip_from]').value = ''; } \"";
							check_box('prefs[SQ_USER_SESSION_PREFS]['.$id.'][limit]', '0', (!empty($extra_setting['ip_from'])), '', 'id="prefs_SQ_USER_SESSION_PREFS_'.$id.'_limit" '.$js);
							label(translate('Limit this rule to an IP Address or Range'), 'prefs_SQ_USER_SESSION_PREFS_'.$id.'_limit');

							echo ' ';
							text_box('prefs[SQ_USER_SESSION_PREFS]['.$id.'][ip_from]', $extra_setting['ip_from']);
							echo ' to ';
							text_box('prefs[SQ_USER_SESSION_PREFS]['.$id.'][ip_to]', $extra_setting['ip_to']);
						} else {
							echo '<img src="'.sq_web_path('lib').'/web/images/'.((!empty($extra_setting['ip_from'])) ? 'tick' : 'cross').'.gif" />';
							echo translate('Limit this rule to an IP Address or Range');

							echo ' ';
							echo $extra_setting['ip_from'];
							if (!empty($extra_setting['ip_to'])) {
								echo ' to ';
								echo $extra_setting['ip_to'];
							}//end if
						}//end if
						?>
					</li>
					<li>
						<table>
						<?php

						$user_types = array_get_index($extra_setting, 'user_types', '');
						if ($have_lock && $is_admin) {
							$js = "onChange=\"Javascript: if (this.checked == true) { document.getElementById('prefs[SQ_USER_SESSION_PREFS][".$id."][user_types]').disabled = false; } else { document.getElementById('prefs[SQ_USER_SESSION_PREFS][".$id."][user_types]').disabled = true;  }\"";
							check_box('prefs[SQ_USER_SESSION_PREFS]['.$id.'][limit_user_types]', '0', (!empty($user_types)), '', 'id="prefs_SQ_USER_SESSION_PREFS_'.$id.'_limit_user_types" '.$js);
							label(translate('Limit this rule to the User types** (comma seperated)'), 'prefs_SQ_USER_SESSION_PREFS_'.$id.'_limit_user_types');

							text_box('prefs[SQ_USER_SESSION_PREFS]['.$id.'][user_types]', $user_types, 70, '', FALSE, 'id="prefs[SQ_USER_SESSION_PREFS]['.$id.'][user_types]"');
							echo '<script type="text/javascript">if (document.getElementById("prefs[SQ_USER_SESSION_PREFS]['.$id.'][user_types]").value == \'\') { document.getElementById("prefs[SQ_USER_SESSION_PREFS]['.$id.'][user_types]").disabled = true; }</script>';
						} else {
							echo '<img src="'.sq_web_path('lib').'/web/images/'.((!empty($user_types)) ? 'tick' : 'cross').'.gif" />';
							echo translate('Limit this rule to the User types** (comma seperated)');

							echo ' ';
							echo $user_types;
						}

						?>
						</table>
					</li>
					<?php
					// Don't delete the first rule, but allow every other
					if ($id != '0' && $have_lock && $is_admin) {
					?>
					<li>
					<?php
						$js = "onChange=\"Javascript: if (this.checked == true) { if(!confirm(js_translate('Are you sure you want to delete this rule?'))) { this.checked=false; } } \"";

						check_box('prefs_delete_rule_number_'.$id, '1', FALSE, 'prefs_sq_user_prefs_delete_'.$id, $js);
						label(translate('Delete Rule'), 'prefs_delete_rule_number_'.$id);

					?>
					</li>
					<?php
					}//end if
					?>
				</ul>
			</div>
			<?php
			}//end foreach
			$o->note('* '.translate('Matrix-Activity refers to either accessing a page on the frontend, or having the backend interface open.  If none of the boxes above are ticked then the session expiry time will be equal to the longest session expiry preference configured in the system, to a maximum of a year and a minimum of a week.'));

			$available_user_types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('user', TRUE);
			$o->note('** Available user types in this system are: '.implode(', ', $available_user_types));
			?>
			<br />
			<?php

			if ($have_lock && $is_admin) {
				check_box('prefs_make_new_rule', '1', FALSE, 'prefs_sq_user_prefs_new');
				label(translate('Add a new session expiry rule'), 'prefs_make_new_rule');

			}//end if

			if ($have_lock && $is_admin) {
				?>
				<script type="text/javascript">
					function toggleInputs(elt) {
						var inputs = elt.getElementsByTagName('INPUT');
						for (var i=0; i < inputs.length; i++) {
							if (inputs[i].type.toLowerCase() == 'text') {
								inputs[i].disabled = !inputs[i].disabled;
							}
						}
					}
					<?php
					// disable fields initially if appropriate
					foreach (Array('timeout', 'max_length') as $field) {
						foreach ($session_settings as $id => $session_setting) {
							if (empty($session_setting[$field])) {
								?>
								toggleInputs(document.getElementById('<?php echo $field.$id; ?>'));
								<?php
							}//end if
						}//end foreach
					}//end foreach
					?>
				</script>
				<?php
			}
			if (is_null($pref)) $o->closeField();
		}//end if


		if (is_null($pref) || $pref == 'SQ_USER_ASSET_MAP_LINEAGE') {
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_USER_ASSET_MAP_LINEAGE']['name']));
			}
			?>
			<div class="sq-backend-table sq-backend-table-cell">
			<?php

			if ($have_lock && $is_admin) {

				if (isset($this->pref_vars['SQ_USER_ASSET_MAP_LINEAGE']['default'])) {
					$asset_map_settings = $this->pref_vars['SQ_USER_ASSET_MAP_LINEAGE']['default'];
				} else {
					$asset_map_settings = '';
				}
				asset_finder('prefs[SQ_USER_ASSET_MAP_LINEAGE]', $asset_map_settings);
			} else {
				if (!empty($this->pref_vars['SQ_USER_ASSET_MAP_LINEAGE']['default'])) {
					echo get_asset_tag_line($this->pref_vars['SQ_USER_ASSET_MAP_LINEAGE']['default']);
				} else {
					echo translate('Root Folder');

				}
			}
			?>
			<?php
			$o->note(translate('The asset that will be displayed as the Asset Map\'s root node'));

			?>
			</div>
			<?php

			if (is_null($pref)) $o->closeField();
		}//end if

		if (is_null($pref) || $pref == 'SQ_USER_SAFE_TYPE3_TRASH') {
		if (is_null($pref)) $o->openField(translate($this->pref_vars['SQ_USER_SAFE_TYPE3_TRASH']['name']));
			?>
			<div class="sq-backend-table sq-backend-table-cell">
			<?php
				$safe_trash = $this->pref_vars['SQ_USER_SAFE_TYPE3_TRASH']['default'];
				if ($have_lock && $is_admin) {
					check_box('prefs[SQ_USER_SAFE_TYPE3_TRASH]', 1, ($safe_trash), '', 'id="prefs_SQ_USER_SAFE_TYPE3_TRASH"');
					label(translate('Yes'), 'prefs_SQ_USER_SAFE_TYPE3_TRASH');

					$o->note('Check this box if you want to prohibit the deletion of an asset until all notice links are removed.');
				} else {
					echo '<img src="'.sq_web_path('lib').'/web/images/'.($safe_trash ? 'tick' : 'cross').'.gif" />';
					echo('If this setting is enabled, the system will prohibit the deletion of an asset until all notice links are removed.');
				}
			?>
			</div>
			<?php

			if (is_null($pref)) $o->closeField();
		}//end if


		if (is_null($pref) || $pref == 'SQ_USER_ASSET_MAP_WIDTH') {
			if (is_null($pref)) $o->openField(translate($this->pref_vars['SQ_USER_ASSET_MAP_WIDTH']['name']));
			?>
			<div class="sq-backend-table sq-backend-table-cell">
			<?php

			if ($have_lock && $is_admin) {

				if (isset($this->pref_vars['SQ_USER_ASSET_MAP_WIDTH']['default'])) {
					$asset_map_settings = $this->pref_vars['SQ_USER_ASSET_MAP_WIDTH']['default'];
				} else {
					$asset_map_settings = '';
				}
				$this->int_am_frame_width->value = $asset_map_settings;
				$this->int_am_frame_width->paint($pref.'_am_frame_width', !$have_lock);
			} else {
				if (!empty($this->pref_vars['SQ_USER_ASSET_MAP_WIDTH']['default'])) {
					echo $this->pref_vars['SQ_USER_ASSET_MAP_WIDTH']['default'];
				}
			}
			?>
			<?php
			$o->note($this->pref_vars['SQ_USER_ASSET_MAP_WIDTH']['description']);
			?>
			</div>
			<?php

			if (is_null($pref)) $o->closeField();
		}//end if

		if (is_null($pref) || $pref == 'SQ_USER_COMMIT_BUTTON_POS') {
			if (is_null($pref)) $o->openField(translate($this->pref_vars['SQ_USER_COMMIT_BUTTON_POS']['name']));
			?>
			<div class="sq-backend-table sq-backend-table-cell">
			<?php
				$options = Array(
								FALSE	=> 'Bottom of page',
								TRUE	=> 'Anchored to bottom of frame',
							   );

				$position = $this->pref_vars['SQ_USER_COMMIT_BUTTON_POS']['default'];
				if ($have_lock && $is_admin) {
					combo_box('prefs[SQ_USER_COMMIT_BUTTON_POS]', $options, FALSE, $position, 0, 'id="prefs_SQ_USER_COMMIT_BUTTON_POS"');
					$o->note('If set to "Anchored to bottom of frame", the Commit button will appear at the bottom of the frame and not scroll with the rest of the page. Note that this setting does not affect Simple Edit mode.');
				} else {
					echo $options[$position];
					$o->note('If set to "Anchored to bottom of frame", the Commit button will appear at the bottom of the frame and not scroll with the rest of the page. Note that this setting does not affect Simple Edit mode');
				}
			?>
			</div>
			<?php

			if (is_null($pref)) $o->closeField();
		}//end if

		if (is_null($pref) || $pref == 'SQ_USER_LIMBO_HIDE_FRAMES') {
			if (is_null($pref)) $o->openField(translate($this->pref_vars['SQ_USER_LIMBO_HIDE_FRAMES']['name']));
			?>
			<div class="sq-backend-table sq-backend-table-cell">
			<?php
				$options = Array(
								FALSE	=> 'Do not hide frames',
								TRUE	=> 'Hide frames',
								);
				$hide_frames = $this->pref_vars['SQ_USER_LIMBO_HIDE_FRAMES']['default'];
				if ($have_lock && $is_admin) {
					combo_box('prefs[SQ_USER_LIMBO_HIDE_FRAMES]', $options, FALSE, $hide_frames, 0, 'id="prefs_SQ_USER_LIMBO_HIDE_FRAMES"');
					$o->note('If set to "Do not hide frames", Matrix will paint the default Simple Edit Interface which includes the header bar and Asset Map resizing frame.');
				} else {
					echo $options[$hide_frames];
					$o->note('If set to "Do not hide frames", Matrix will paint the default Simple Edit Interface which includes the header bar and Asset Map resizing frame.');
				}
			?>
			</div>
			<?php
			if (is_null($pref)) $o->closeField();
		}//end if

		if (is_null($pref) || $pref == 'SQ_USER_ATTRIBUTE_IMAGE') {
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_USER_ATTRIBUTE_IMAGE']['name']));
			}
			$attribute_settings = $this->pref_vars['SQ_USER_ATTRIBUTE_IMAGE']['default'];
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Font'); ?></td>

					<td class="sq-backend-table-cell" style="width: 100%;">
						<?php
						require_once(SQ_FUDGE_PATH.'/general/file_system.inc');
						$font_files = list_files(SQ_LIB_PATH.'/fonts');
						$fonts_installed = Array(
											1	=> translate('Built in font 1'),

											2	=> translate('Built in font 2'),

											3	=> translate('Built in font 3'),

											4	=> translate('Built in font 4'),

											5	=> translate('Built in font 5'),

										   );
						foreach ($font_files as $font_file) {
							$result = preg_match('/(.*)\.ttf$/i', $font_file, $matches);
							if (isset($matches[1]) && !empty($matches[1])) {
								$fonts_installed[$matches[1]] = ucwords($matches[1]);
							}//end if
						}//end foreach
						if ($have_lock && $is_admin) {
							combo_box('prefs[SQ_USER_ATTRIBUTE_IMAGE][face]', $fonts_installed, FALSE, $attribute_settings['face'], 1, 'id="prefs_SQ_USER_ATTRIBUTE_IMAGE_face"');
						} else {
							if (isset($fonts_installed[$attribute_settings['face']])) {
								echo $fonts_installed[$attribute_settings['face']];
							} else {
								echo translate('No valid font');

							}//end if
						}//end if
						?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Size'); ?></td>

					<td class="sq-backend-table-cell" style="width: 100%;">
						<?php
						$default_fonts = Array( 1,2,3,4,5 );
						if ($have_lock && $is_admin && !in_array($attribute_settings['face'], $default_fonts)) {
							text_box('prefs[SQ_USER_ATTRIBUTE_IMAGE][size]', $attribute_settings['size'], 'id="prefs_SQ_USER_ATTRIBUTE_IMAGE_size"');
						} else {
							if (in_array($attribute_settings['face'], $default_fonts)) {
								echo translate('Size is fixed for Built in font');

							} else {
								echo $attribute_settings['size'];
							}//end if
						}
						?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Horizontal padding'); ?></td>

					<td class="sq-backend-table-cell" style="width: 100%;">
						<?php
						if ($have_lock && $is_admin) {
							text_box('prefs[SQ_USER_ATTRIBUTE_IMAGE][hpad]', $attribute_settings['hpad'], 'id="prefs_SQ_USER_ATTRIBUTE_IMAGE_hpad"');
						} else {
							echo $attribute_settings['hpad'];
						}
						?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Vertical padding'); ?></td>

					<td class="sq-backend-table-cell" style="width: 100%;">
						<?php
						if ($have_lock && $is_admin) {
							text_box('prefs[SQ_USER_ATTRIBUTE_IMAGE][vpad]', $attribute_settings['vpad'], 'id="prefs_SQ_USER_ATTRIBUTE_IMAGE_vpad"');
						} else {
							echo $attribute_settings['vpad'];
						}
						?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Foreground Colour'); ?></td>

					<td class="sq-backend-table-cell" style="width: 100%;">
						<?php
						require_once(SQ_ATTRIBUTES_PATH.'/colour/colour.inc');
						$fg = new Asset_Attribute_Colour(0, $attribute_settings['fgcol']);
						$fg->paint('prefs_image_fgcol_', !($have_lock && $is_admin));
						?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Background Colour'); ?></td>

					<td class="sq-backend-table-cell" style="width: 100%;">
						<?php
						require_once(SQ_ATTRIBUTES_PATH.'/colour/colour.inc');
						$bg = new Asset_Attribute_Colour(0, $attribute_settings['bgcol']);
						$bg->paint('prefs_image_bgcol_', !($have_lock && $is_admin));
						?>
					</td>
				</tr>
			</table>
			<?php
			if (is_null($pref)) $o->closeField();
		}//end if


		if (is_null($pref) || $pref == 'SQ_USER_SET_CONTEXT_ON_LOGIN') {
			if (is_null($pref)) $o->openField(translate($this->pref_vars['SQ_USER_SET_CONTEXT_ON_LOGIN']['name']));
			$contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
			$options = Array(
						'default'	=> 'Do not change context on login',
						'sep'		=> '-----------',
					   );
			foreach ($contexts as $contextid => $context_data) {
				$options[$contextid] = $context_data['name'];
			}
			?>
			<div class="sq-backend-table sq-backend-table-cell">
			<?php

			if ($have_lock && $is_admin) {
				if (isset($this->pref_vars['SQ_USER_SET_CONTEXT_ON_LOGIN']['default'])) {
					$set_context_on_login = $this->pref_vars['SQ_USER_SET_CONTEXT_ON_LOGIN']['default'];
					if (array_key_exists($set_context_on_login, $contexts) === FALSE) {
						$set_context_on_login = 'default';
					} else {
						$set_context_on_login = (int)$set_context_on_login;
					}
				} else {
					$set_context_on_login = 'default';
				}

				$default = $set_context_on_login;

				combo_box('prefs[SQ_USER_SET_CONTEXT_ON_LOGIN]', $options, FALSE, Array($default));
			} else {
				if (!empty($this->pref_vars['SQ_USER_SET_CONTEXT_ON_LOGIN']['default'])) {
					echo $options[$this->pref_vars['SQ_USER_SET_CONTEXT_ON_LOGIN']['default']];
				}
			}
			?>
			<?php
			$o->note($this->pref_vars['SQ_USER_SET_CONTEXT_ON_LOGIN']['description']);
			?>
			</div>
			<?php

			if (is_null($pref)) $o->closeField();
		}//end if

		if (is_null($pref) || $pref == 'SQ_USER_FILTER_FRONT_END_INPUT') {
			if (is_null($pref)) $o->openField(translate($this->pref_vars['SQ_USER_FILTER_FRONT_END_INPUT']['name']));
			?>
			<div class="sq-backend-table sq-backend-table-cell">
			<?php
				$options = Array(
								FALSE	=> 'Do not filter input',
								TRUE	=> 'Filter input',
								);
				$filter_input = $this->pref_vars['SQ_USER_FILTER_FRONT_END_INPUT']['default'];
				if ($have_lock && $is_admin) {
					combo_box('prefs[SQ_USER_FILTER_FRONT_END_INPUT]', $options, FALSE, $filter_input, 0, 'id="prefs_SQ_USER_FILTER_FRONT_END_INPUT"');
				} else {
					echo $options[$filter_input];
				}
				$o->note('If set to "Filter Input", Matrix will '.$this->pref_vars['SQ_USER_FILTER_FRONT_END_INPUT']['description']);
			?>
			</div>
			<?php
			if (is_null($pref)) $o->closeField();
		}//end if
		return TRUE;

	}//end paintBackend()


	/**
	* Processes the backend interface to edit session preferences
	*
	* @param object		&$o			reference to the backend outputter
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	*
	* @return boolean
	* @access public
	*/
	function processBackend(&$o, $have_lock, $pref=NULL)
	{
		if (!($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin())) {
			return FALSE;
		}

		// Default
		$make_new	= Array(
						'ip_from'		=> '',
						'ip_to'			=> '',
						'persist'		=> 0,
						'timeout'		=> 0,
						'max_length'	=> 0,
						'user_types'	=> '',
					  );
		$new_prefs	= Array( $make_new );
		if (is_null($pref) || $pref == 'SQ_USER_SESSION_PREFS') {
			foreach ($_POST['prefs']['SQ_USER_SESSION_PREFS'] as $id => $submitted_pref) {
				$new_prefs[$id]['ip_from']		= (isset($submitted_pref['ip_from'])) ? $submitted_pref['ip_from'] : '';
				$new_prefs[$id]['ip_to']		= (isset($submitted_pref['ip_to'])) ? $submitted_pref['ip_to'] : '';
				$new_prefs[$id]['persist']		= !isset($submitted_pref['persist']);
				$new_prefs[$id]['user_types']	= (isset($submitted_pref['user_types'])) ? $submitted_pref['user_types'] : '';

				// Remove empty spaces from the comma seperated user type list
				$new_prefs[$id]['user_types'] = strtolower(str_replace(' ', '', $new_prefs[$id]['user_types']));

				if (isset($submitted_pref['timeout_enabled'])) {
					$timeout_dur = new Asset_Attribute_Duration();
					$timeout_dur->process('prefs_dur_timeout_'.$id);
					$new_prefs[$id]['timeout'] = $timeout_dur->value;
				} else {
					$new_prefs[$id]['timeout'] = '0';
				}//end if
				if (isset($submitted_pref['max_length_enabled'])) {
					$max_length_dur = new Asset_Attribute_Duration();
					$max_length_dur->process('prefs_dur_max_length_'.$id);
					$new_prefs[$id]['max_length'] = $max_length_dur->value;
				} else {
					$new_prefs[$id]['max_length'] = '0';
				}//end if

				if ($id != '0') {
					if (isset($_POST['prefs_delete_rule_number_'.$id]) && $_POST['prefs_delete_rule_number_'.$id]) {
						unset($new_prefs[$id]);
					}//end if
				}//end if
			}//end foreach

			// Was a new rules selected?
			if (isset($_POST['prefs_make_new_rule']) && $_POST['prefs_make_new_rule']) {
				$new_prefs[] = $make_new;
			}//end if

			$this->pref_vars['SQ_USER_SESSION_PREFS']['default'] = $new_prefs;
		}//end if

		if (is_null($pref) || $pref == 'SQ_USER_ASSET_MAP_LINEAGE') {
			// update lineage pref
			if (isset($_POST['prefs']['SQ_USER_ASSET_MAP_LINEAGE'])) {
				$this->pref_vars['SQ_USER_ASSET_MAP_LINEAGE']['default'] = $_POST['prefs']['SQ_USER_ASSET_MAP_LINEAGE']['assetid'];
			}
		}

		// Go through the valid entries and look for the most valid
		$gc_min = 0;
		foreach ($new_prefs as $id => $new_pref) {
			$temp = 0;
			if (isset($new_pref['ip_from'])) {
				if (!empty($new_pref['ip_from'])) {
					continue;
				}//end if
			}//end if

			// No IP Restriction found, continue
			$temp = max($new_pref['timeout'], $new_pref['max_length']);

			// Look for the largest general result
			if (!empty($temp) && $temp > $gc_min) {
				$gc_min = $temp;
			}//end if
		}//end foreach

		if ($gc_min > SQ_CONF_SESSION_GC_MAXLIFETIME) {
			// update the main conf
			include_once(SQ_INCLUDE_PATH.'/system_config.inc');
			$system_conf = new System_Config();
			if (!$system_conf->canAcquireLock()) {
				trigger_localised_error('CORE0233', translate('This operation requires the System Configuration lock to be acquired, but another user has that lock.  Please try again later.'), E_USER_NOTICE)
				return FALSE;
			}

			$system_conf->acquireLock();
			$system_conf->save(Array('SQ_CONF_SESSION_GC_MAXLIFETIME' => $gc_min + 86400), FALSE, FALSE);
			$system_conf->releaseLock();
		}

		if (is_null($pref) || $pref == 'SQ_USER_ASSET_MAP_MODERN') {
			// Modern Asset Map pref (default to Java asset map).
			if (isset($_POST['prefs']['SQ_USER_ASSET_MAP_MODERN'])) {
				$this->pref_vars['SQ_USER_ASSET_MAP_MODERN']['default'] = $_POST['prefs']['SQ_USER_ASSET_MAP_MODERN'];
			} else {
				$this->pref_vars['SQ_USER_ASSET_MAP_MODERN']['default'] = FALSE;
			}
		}

		if (is_null($pref) || $pref == 'SQ_USER_SAFE_TYPE3_TRASH') {
			// safe trash pref
			if (isset($_POST['prefs']['SQ_USER_SAFE_TYPE3_TRASH'])) {
				$this->pref_vars['SQ_USER_SAFE_TYPE3_TRASH']['default'] = $_POST['prefs']['SQ_USER_SAFE_TYPE3_TRASH'];
			} else {
				$this->pref_vars['SQ_USER_SAFE_TYPE3_TRASH']['default'] = FALSE;
			}
		}

		if (is_null($pref) || $pref == 'SQ_USER_COMMIT_BUTTON_POS') {
			// commit button position pref
			if (isset($_POST['prefs']['SQ_USER_COMMIT_BUTTON_POS'])) {
				$this->pref_vars['SQ_USER_COMMIT_BUTTON_POS']['default'] = $_POST['prefs']['SQ_USER_COMMIT_BUTTON_POS'];
			} else {
				$this->pref_vars['SQ_USER_COMMIT_BUTTON_POS']['default'] = FALSE;
			}
		}


		if (is_null($pref) || $pref == 'SQ_USER_ASSET_MAP_WIDTH') {
			@$this->int_am_frame_width->process($pref.'_am_frame_width');
			if (!empty($this->int_am_frame_width->value)) {
				$this->pref_vars['SQ_USER_ASSET_MAP_WIDTH']['default'] = $this->int_am_frame_width->value;
			} else {
				trigger_localised_error('CORE0270', translate('An invalid "Asset Map Width" value has been supplied. Default has been set'), E_USER_NOTICE)
				$this->pref_vars['SQ_USER_ASSET_MAP_WIDTH']['default'] = $this->int_am_frame_width_default;
			}
		}

		if (is_null($pref) || $pref == 'SQ_USER_SET_CONTEXT_ON_LOGIN') {
			// Set user context upon login pref
			if (isset($_POST['prefs']['SQ_USER_SET_CONTEXT_ON_LOGIN']) && (strpos($_POST['prefs']['SQ_USER_SET_CONTEXT_ON_LOGIN'], 'sep') === FALSE)) {
				$this->pref_vars['SQ_USER_SET_CONTEXT_ON_LOGIN']['default'] = $_POST['prefs']['SQ_USER_SET_CONTEXT_ON_LOGIN'];
			} else {
				$this->pref_vars['SQ_USER_SET_CONTEXT_ON_LOGIN']['default'] = 'default';
			}
		}

		if (is_null($pref) || $pref == 'SQ_USER_LIMBO_HIDE_FRAMES') {
			// hide frames in limbo  pref
			if (isset($_POST['prefs']['SQ_USER_LIMBO_HIDE_FRAMES'])) {
				$this->pref_vars['SQ_USER_LIMBO_HIDE_FRAMES']['default'] = $_POST['prefs']['SQ_USER_LIMBO_HIDE_FRAMES'];
			} else {
				$this->pref_vars['SQ_USER_LIMBO_HIDE_FRAMES']['default'] = FALSE;
			}
		}

		if (is_null($pref) || $pref == 'SQ_USER_FILTER_FRONT_END_INPUT') {
			// filter front end input
			if (isset($_POST['prefs']['SQ_USER_FILTER_FRONT_END_INPUT'])) {
				$this->pref_vars['SQ_USER_FILTER_FRONT_END_INPUT']['default'] = $_POST['prefs']['SQ_USER_FILTER_FRONT_END_INPUT'];
			} else {
				$this->pref_vars['SQ_USER_FILTER_FRONT_END_INPUT']['default'] = FALSE;
			}
		}

		if (is_null($pref) || $pref == 'SQ_USER_ATTRIBUTE_IMAGE') {
			$new_prefs = Array(
							'face'	=> 'arial',
							'size'	=> '12',
							'hpad'	=> 0,
							'vpad'	=> 0,
							'fgcol'	=> '000000',
							'bgcol'	=> 'FFFFFF',
						 );
			if (isset($_POST['prefs']['SQ_USER_ATTRIBUTE_IMAGE']['face'])) {
				$new_prefs['face'] = $_POST['prefs']['SQ_USER_ATTRIBUTE_IMAGE']['face'];
			}
			if (isset($_POST['prefs']['SQ_USER_ATTRIBUTE_IMAGE']['size'])) {
				$new_prefs['size'] = $_POST['prefs']['SQ_USER_ATTRIBUTE_IMAGE']['size'];
			}
			if (isset($_POST['prefs']['SQ_USER_ATTRIBUTE_IMAGE']['hpad'])) {
				$new_prefs['hpad'] = $_POST['prefs']['SQ_USER_ATTRIBUTE_IMAGE']['hpad'];
			}
			if (isset($_POST['prefs']['SQ_USER_ATTRIBUTE_IMAGE']['vpad'])) {
				$new_prefs['vpad'] = $_POST['prefs']['SQ_USER_ATTRIBUTE_IMAGE']['vpad'];
			}
			require_once(SQ_ATTRIBUTES_PATH.'/colour/colour.inc');
			$fg = new Asset_Attribute_Colour();
			$fg->process('prefs_image_fgcol_');
			$new_prefs['fgcol'] = $fg->value;
			$bg = new Asset_Attribute_Colour();
			$bg->process('prefs_image_bgcol_');
			$new_prefs['bgcol'] = $bg->value;
			$this->pref_vars['SQ_USER_ATTRIBUTE_IMAGE']['default'] = $new_prefs;
		}//end if


		return TRUE;

	}//end processBackend()


	/**
	* Merges two group-defined preference arrays for a content type
	*
	* Note that this function is only called if there is a conflict between two groups' preferences.
	* If there is a conflict between a group's prefs and the global prefs, the group automatically wins and
	* this function is not called.
	* Array returned is of the format (prefs_var => prefs_details)
	*
	* @param array	$prefs1	array of preferences to merge
	* @param array	$prefs2	array of preferences to merge
	*
	* @return array
	* @access public
	*/
	function mergePrefs($prefs1, $prefs2)
	{
		// For the preferences that cannot be meraged (i.e. not defined here), get the preferences from first settings data
		$res = $prefs2;
		if (isset($prefs1['SQ_USER_SESSION_PREFS']['default']) && isset($prefs2['SQ_USER_SESSION_PREFS']['default'])) {
			// Follow the princple that the most-open option (the one with the check disabled,
			// or with the longest timeout) wins.
			$session_settings_1 = $prefs1['SQ_USER_SESSION_PREFS']['default'];
			if (is_array($prefs2['SQ_USER_SESSION_PREFS']) && isset($prefs2['SQ_USER_SESSION_PREFS']['default'])) {
				$session_settings_2 = $prefs2['SQ_USER_SESSION_PREFS']['default'];
			} else {
				$session_settings_2 = $prefs2['SQ_USER_SESSION_PREFS'];
			}
			$merged['persist'] = max($session_settings_1['persist'], $session_settings_2['persist']);
			foreach (Array('timeout', 'max_length') as $pref_comp) {
				if (0 == min($session_settings_1[$pref_comp], $session_settings_2[$pref_comp])) {
					$merged[$pref_comp] = 0;
				} else {
					$merged[$pref_comp] = max($session_settings_1[$pref_comp], $session_settings_2[$pref_comp]);
				}
			}

			$res['SQ_USER_SESSION_PREFS']['default'] = $merged;
		} else if (isset($prefs1['SQ_USER_ASSET_MAP_LINEAGE']['default']) && isset($prefs2['SQ_USER_ASSET_MAP_LINEAGE'])) {

			// asset map settings merge get the common
			$asset_map_settings1 = $prefs1['SQ_USER_ASSET_MAP_LINEAGE']['default'];
			if (is_array($prefs2['SQ_USER_ASSET_MAP_LINEAGE']) && isset($prefs2['SQ_USER_ASSET_MAP_LINEAGE']['default'])) {
				$asset_map_settings2 = $prefs2['SQ_USER_ASSET_MAP_LINEAGE']['default'];
			} else {
				$asset_map_settings2 = $prefs2['SQ_USER_ASSET_MAP_LINEAGE'];
			}

			if (empty($asset_map_settings1) && !empty($asset_map_settings2)) {
				$asset_map_settings1 = $asset_map_settings2;
				$asset_map_settings2 = '';
			}

			if (is_array($res['SQ_USER_ASSET_MAP_LINEAGE'])) {
				$res['SQ_USER_ASSET_MAP_LINEAGE']['default'] = $asset_map_settings1;
			} else {
				$res['SQ_USER_ASSET_MAP_LINEAGE'] = Array('default' => $asset_map_settings1);
			}

			if (!empty($asset_map_settings2)) {
				$res['SQ_USER_ASSET_MAP_LINEAGE']['default'] .= ','.$asset_map_settings2;
			}

		}

		if (isset($prefs1['SQ_USER_SAFE_TYPE3_TRASH']['default']) && isset($prefs2['SQ_USER_SAFE_TYPE3_TRASH'])) {

			// for safe trash option, if user belongs to multiple user groupS, option Yes > No
			// note that param 2 is value only
			$safe_trash1 = $prefs1['SQ_USER_SAFE_TYPE3_TRASH']['default'];
			$safe_trash2 = $prefs2['SQ_USER_SAFE_TYPE3_TRASH']['default'];
			if (is_array($res['SQ_USER_SAFE_TYPE3_TRASH'])) {
				$res['SQ_USER_SAFE_TYPE3_TRASH']['default'] = ($safe_trash1 || $safe_trash2) ? TRUE : FALSE;
			} else {
				$res['SQ_USER_SAFE_TYPE3_TRASH'] = Array('default' => ($safe_trash1 || $safe_trash2) ? TRUE : FALSE);

			}

		}

		return $res;

	}//end mergePrefs()


}//end class

?>
