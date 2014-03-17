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
* $Id: content_type_wysiwyg_prefs.inc,v 1.30 2013/02/08 06:11:45 ewang Exp $
*
*/


require_once SQ_LIB_PATH.'/config/prefs.inc';

/**
* Content_Type_WYSIWYG_Prefs
*
* Purpose
*
*    Looks after the creation the WYSIWYG preferences
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.30 $
* @package MySource_Matrix
*/
class Content_Type_WYSIWYG_Prefs extends Prefs
{
	var $pref_vars = Array(
						'SQ_WYSIWYG_STYLE'					=> Array(
																'name'			=> 'wysiwyg_style',
																'description'	=> 'This preference allows you to change the style and size of the WYSIWYG editor',
																'default'		=> Array(
																					'body_type'	=> 'div',
																					'width'		=> '100',
																					'height'	=> '300',
																					'units'		=> Array(
																									'width'		=> '%',
																									'height'	=> 'px',
																								   ),
																				   ),
																'protected'		=> FALSE,
															   ),
						 'SQ_WYSIWYG_APPLY_DESIGN_STYLE'						=> Array (
																'name'			=> 'wysiwyg_apply_design_style',
																'description'	=> 'Apply css styles of design to wysiwyg',
																'default'		=> 0,
																'protected'		=> FALSE,
															   ),
						'SQ_WYSIWYG_PLUGINS'				=> Array(
																'name'			=> 'wysiwyg_plugins',
																'description'	=> 'This preference allows you to select which features of the WYSIWYG editor you would like make available while editing',
																'default'		=> 'all',
																'protected'		=> FALSE,
															   ),
						'SQ_WYSIWYG_DISABLE_IMAGE_RESIZE'	=> Array(
																'name'			=> 'wysiwyg_disable_image_resize',
																'description'	=> 'This preference allows you to select whether to disable the image resizing functionality in WYSIWYG',
																'default'		=> 'no',
																'protected'		=> FALSE,
															   ),
						'SQ_THESAURUS'						=> Array (
																'name'			=> 'thesaurus',
																'description'	=> 'Which thesaurus asset you want to use when editing content, for example to provide keyword expansions',
																'default'		=> 0,
																'protected'		=> FALSE,
															   ),
						'SQ_WYSIWYG_LINK_TYPES'				=> Array(
																'name'			=> 'core_ct_wysiwyg_link_type_templates',
																'description'	=> 'List of link types to be used in the Insert Link popup',
																'default'		=> Array(
																					0	=> Array(
																							'type'		=> 'http://',
																							'template'	=> 'http://%%link%%',
																						   ),
																					1	=> Array(
																							'type'		=> 'https://',
																							'template'	=> 'https://%%link%%',
																						   ),
																					2	=> Array(
																							'type'		=> 'mailto:',
																							'template'	=> 'mailto:%%link%%',
																						   ),
																					3	=> Array(
																							'type'		=> 'ftp://',
																							'template'	=> 'ftp://%%link%%',
																						   ),
																					4	=> Array(
																							'type'		=> 'rtsp://',
																							'template'	=> 'rtsp://%%link%%',
																						   ),
																				   ),
																'protected'		=> FALSE,
															   ),
						'SQ_LIVE_LINK_ONLY'						=> Array (
																'name'			=> 'live_link_only',
																'description'	=> 'To allow links pointing only to live asset',
																'default'		=> 0,
																'protected'		=> FALSE,
															   ),
	    					'SQ_ENABLE_VIPER_EDITOR'						=> Array (
																'name'			=> 'enable_edit+_editor',
																'description'	=> 'Enable Edit+ WYSIWYG editor',
																'default'		=> 1,
																'protected'		=> FALSE,
															   ),
					 );


	/**
	* Constructor
	*
	*/
	function Content_Type_WYSIWYG_Prefs($pref_file='')
	{
		$this->Prefs($pref_file);

	}//end constructor


	/**
	* Paints the backend interface to edit preferences
	*
	* @param object		&$o			reference to the backend outputter object
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

		// enable viper editor?
		if (is_null($pref) || $pref == 'SQ_ENABLE_VIPER_EDITOR') {		
						
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_ENABLE_VIPER_EDITOR']['name']));
			}
						
			$pref_var =& $this->pref_vars['SQ_ENABLE_VIPER_EDITOR']['default'];

			if ($have_lock && $is_admin) {
				$options = Array(1 => 'Yes', 0 => 'No');
				combo_box('prefs[SQ_ENABLE_VIPER_EDITOR]', $options, FALSE, $pref_var);
				echo $o->note(translate('enable_edit+_editor_note'));
			} else {
				echo ($pref_var) ? translate('yes') : translate('no');
			}
						
			if (is_null($pref)) $o->closeField();
			
		}//end if show SQ_ENABLE_VIPER_EDITOR
		
		
		if (is_null($pref) || $pref == 'SQ_WYSIWYG_STYLE') {
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_WYSIWYG_STYLE']['name']));
			}

				$wysiwyg_style = $this->pref_vars['SQ_WYSIWYG_STYLE']['default'];
				$size_units = Array('%' => '%', 'px' => 'px');
				?>
				<table class="sq-backend-table" style="width: 350px;">
					<tr>
						<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('body_type'); ?></td>
						<td class="sq-backend-table-cell" style="width: 100%;">
						<?php
						if ($have_lock && $is_admin) {
							combo_box('prefs[SQ_WYSIWYG_STYLE][body_type]', Array('div' => translate('div'), 'iframe' => translate('iframe')), FALSE, $wysiwyg_style['body_type']);
						} else {
							echo $wysiwyg_style['body_type'];
						}
						?>
						<br /><i><?php echo translate('mozilla_always_uses_iframe'); ?></i>
						</td>
					</tr>
					<tr>
						<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('width'); ?></td>
						<td class="sq-backend-table-cell" style="width: 100%;">
						<?php
						if (!is_numeric($wysiwyg_style['width'])) {
							$wysiwyg_style['width'] = preg_replace('/[^0-9]/', '', $wysiwyg_style['width']);
						}
						if ($have_lock && $is_admin) {
							int_text_box('prefs[SQ_WYSIWYG_STYLE][width]', $wysiwyg_style['width'], FALSE, 5);
							echo '&nbsp;';
							$default_unit = '%';
							if (isset($wysiwyg_style['units']['width'])) {
								$default_unit = $wysiwyg_style['units']['width'];
							}
							combo_box('prefs[SQ_WYSIWYG_STYLE][units][width]', $size_units, FALSE, $default_unit);
						} else {
							if (isset($wysiwyg_style['width'])) {
								echo $wysiwyg_style['width'];
							}

							if (isset($wysiwyg_style['units']['width'])) {
								echo $wysiwyg_style['units']['width'];
							}
						}
						?>
						</td>
					</tr>
					<tr>
						<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('height'); ?></td>
						<td class="sq-backend-table-cell" style="width: 100%;">
						<?php
						if (!is_numeric($wysiwyg_style['height'])) {
							$wysiwyg_style['height'] = preg_replace('/[^0-9]/', '', $wysiwyg_style['height']);
						}
						if ($have_lock && $is_admin) {
							int_text_box('prefs[SQ_WYSIWYG_STYLE][height]', $wysiwyg_style['height'], FALSE, 5);
							echo '&nbsp;';
							$default_unit = 'px';
							if (isset($wysiwyg_style['units']['height'])) {
								$default_unit = $wysiwyg_style['units']['height'];
							}
							combo_box('prefs[SQ_WYSIWYG_STYLE][units][height]', $size_units, FALSE, $default_unit);
						} else {
							if (isset($wysiwyg_style['height'])) {
								echo $wysiwyg_style['height'];
							}
							if (isset($wysiwyg_style['units']['height'])) {
								echo $wysiwyg_style['units']['height'];
							}
						}
						?>
						</td>
					</tr>
				</table>
				<?php

			if (is_null($pref)) $o->closeField();

		}//end if show SQ_WYSIWYG_STYLE

		if (is_null($pref) || $pref == 'SQ_WYSIWYG_APPLY_DESIGN_STYLE') {			
						
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_WYSIWYG_APPLY_DESIGN_STYLE']['name']));
			}
						
			$pref_var =& $this->pref_vars['SQ_WYSIWYG_APPLY_DESIGN_STYLE']['default'];
			if ($have_lock && $is_admin) {
				$options = Array(1 => 'Yes', 0 => 'No');
				combo_box('prefs[SQ_WYSIWYG_APPLY_DESIGN_STYLE]', $options, FALSE, $pref_var);
				echo $o->note(translate('wysiwyg_apply_design_style_note'));
			} else {
				echo ($pref_var) ? translate('yes') : translate('no');
			}
						
			if (is_null($pref)) $o->closeField();
			
		}//end if show SQ_WYSIWYG_APPLY_DESIGN_STYLE
		

		if (is_null($pref) || $pref == 'SQ_WYSIWYG_PLUGINS') {
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_WYSIWYG_PLUGINS']['name']));
			}

			$plugins = Array();
			if (!empty($this->pref_vars['SQ_WYSIWYG_PLUGINS']['default'])) {
				$plugins = explode('|', $this->pref_vars['SQ_WYSIWYG_PLUGINS']['default']);
			}
			$all = in_array('all', $plugins);
			sort($plugins);

			if ($have_lock && $is_admin) {
				require_once SQ_FUDGE_PATH.'/general/file_system.inc';
				$all_plugins = list_dirs(SQ_FUDGE_PATH.'/wysiwyg/plugins');
				sort($all_plugins);
				if (!empty($all_plugins)) {
					hidden_field('prefs[SQ_WYSIWYG_PLUGINS_PRESENT]', '1');
				}
				global $ROOT_PATH;
				$ROOT_PATH = SQ_FUDGE_PATH.'/wysiwyg/';
				foreach ($all_plugins as $plugin_name) {
					$plugin_path = SQ_FUDGE_PATH.'/wysiwyg/plugins/'.$plugin_name;
					if (!is_file($plugin_path.'/'.$plugin_name.'.inc')) {
						continue;
					}
					include $plugin_path.'/'.$plugin_name.'.inc';
					$check_result = call_user_func(Array($plugin_name, 'check_usable'));
					$extra = ($check_result === TRUE) ? '' : 'disabled="disabled"';
					check_box('prefs[SQ_WYSIWYG_PLUGINS]['.$plugin_name.']', '1', ($all || in_array($plugin_name, $plugins)),'',$extra);
					echo '&nbsp;'.ucwords(str_replace('_', ' ', $plugin_name));
					if ($check_result !== TRUE) {
						echo ' <i>('.$check_result.')</i>';					}
					echo '<br/>';
				}
			} else {
				if ($all) {
					echo translate('all_plugins_selected');
				} else {
					if (empty($plugins)) {
						echo translate('no_plugins_selected');
					} else {
						echo '<ul>';
						foreach ($plugins as $plugin_name) {
							echo '<li class="sq-backend-data">'.ucwords(str_replace('_', ' ', $plugin_name)).'</li>';
						}
						echo '</ul>';
					}
				}
			}

			if (is_null($pref)) $o->closeField();

		}//end if show SQ_WYSIWYG_PLUGINS

		if (is_null($pref) || $pref == 'SQ_WYSIWYG_DISABLE_IMAGE_RESIZE') {
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_WYSIWYG_DISABLE_IMAGE_RESIZE']['name']));
			}
				$wysiwyg_disable_image_resize = $this->pref_vars['SQ_WYSIWYG_DISABLE_IMAGE_RESIZE']['default'];
				?>
				<table class="sq-backend-table" style="width: 100px;">
					<tr>
						<td class="sq-backend-table-header" nowrap="nowrap">
						<?php
							echo translate('disable');
						?>
						</td>
						<td class="sq-backend-table-cell" style="width: 100%;">
						<?php
						if ($have_lock && $is_admin) {
							combo_box('prefs[SQ_WYSIWYG_DISABLE_IMAGE_RESIZE][default]', Array('yes' => 'Yes', 'no' => 'No'), FALSE, $wysiwyg_disable_image_resize);
						} else {
							echo ucfirst($wysiwyg_disable_image_resize);
						}
						?>
						</td>
					</tr>
				</table>
				<?php
			if (is_null($pref)) $o->closeField();

		}//end if show SQ_WYSIWYG_DISABLE_IMAGE_RESIZE

		if ($GLOBALS['SQ_SYSTEM']->am->installed('thesaurus') && (is_null($pref) || $pref == 'SQ_THESAURUS')) {
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_THESAURUS']['name']));
			}

			if ($have_lock && $is_admin) {
				asset_finder('prefs[SQ_THESAURUS]', $this->pref_vars['SQ_THESAURUS']['default'], Array('thesaurus'=>'A'));
				$o->note(translate('thesaurus_used_by_matrix_expand_keywords_plugin'));
			} else {
				if (!$this->pref_vars['SQ_THESAURUS']['default']) {
					echo '('.translate('none_selected').')';
				} else {
					$name = reset($GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($this->pref_vars['SQ_THESAURUS']['default']), 'thesaurus', TRUE, 'name'));
					echo $name.' (#'.$this->pref_vars['SQ_THESAURUS']['default'].')';
				}
			}
			if (is_null($pref)) $o->closeField();
		}//end if show SQ_THESAURUS

		if (is_null($pref) || $pref == 'SQ_WYSIWYG_LINK_TYPES') {
			$pref_var =& $this->pref_vars['SQ_WYSIWYG_LINK_TYPES'];

			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_WYSIWYG_LINK_TYPES']['name']));
			}

			if ($have_lock && $is_admin) {

				?><table class="sq-backend-table" style="width:auto">
				<tr>
					<th>Link Type</th>
					<th>Template</th>
					<th><?php echo translate('delete_question') ?></th>
				</tr><?php
				foreach ($pref_var['default'] as $key => $item) {
					// The first four items (http, https, mailto, ftp) cannot be altered
					if ($key >= 4) {
						?><tr><?php
						?><td><?php text_box('prefs[SQ_WYSIWYG_LINK_TYPES][default]['.$key.'][type]',     $item['type'],     10) ?></td><?php
						?><td><?php text_box('prefs[SQ_WYSIWYG_LINK_TYPES][default]['.$key.'][template]', $item['template'], 50) ?></td><?php
						?><td><?php check_box('prefs[SQ_WYSIWYG_LINK_TYPES][default]['.$key.'][delete]', 1, FALSE, 'document.getElementById("prefs[SQ_WYSIWYG_LINK_TYPES][default]['.$key.'][type]").disabled = document.getElementById("prefs[SQ_WYSIWYG_LINK_TYPES][default]['.$key.'][template]").disabled = this.checked') ?></td><?php
						?></tr><?php
					} else {
						?><tr><?php
						?><td><?php hidden_field('prefs[SQ_WYSIWYG_LINK_TYPES][default]['.$key.'][type]',     $item['type']);     echo $item['type']     ?></td><?php
						?><td><?php hidden_field('prefs[SQ_WYSIWYG_LINK_TYPES][default]['.$key.'][template]', $item['template']); echo $item['template'] ?></td><?php
						?><td></td><?php
						?></tr><?php
					}
				}
				?>
				<tr>
				<td colspan="3"><a href="#" id="prefs[SQ_WYSIWYG_LINK_TYPES][add]" onclick="sq_wysiwyg_link_types_addAnother(this.parentNode.parentNode); return false;">Add another...</a></td>
				</tr>
				</table>
				<script type="text/javascript">
				sq_wysiwyg_link_types_nextIndex = <?php echo (int)count($pref_var['default']); ?>;
				function sq_wysiwyg_link_types_addAnother(node) {
					var parent = node.parentNode;

					// create a new row
					tr = document.createElement('tr');

					// build the cells
					td1 = document.createElement('td');
					td1.innerHTML = '<input type="text" name="prefs[SQ_WYSIWYG_LINK_TYPES][default][' + sq_wysiwyg_link_types_nextIndex + '][type]" value="" size="10" />';
					td2 = document.createElement('td');
					td2.innerHTML = '<input type="text" name="prefs[SQ_WYSIWYG_LINK_TYPES][default][' + sq_wysiwyg_link_types_nextIndex + '][template]" value=""  size="50" />';
					td3 = document.createElement('td');
					td3.innerHTML = '<em>new</em>';

					// Now put them all together
					tr.appendChild(td1);
					tr.appendChild(td2);
					tr.appendChild(td3);
					parent.insertBefore(tr, node);

					// Go to next index
					sq_wysiwyg_link_types_nextIndex++;
				}
				</script>
				<?php

			} else {

				?><table class="sq-backend-table" style="width:auto">
				<tr>
					<th>Link Type</th>
					<th>Template</th>
				</tr><?php
				foreach ($pref_var['default'] as $item) {
					?><tr><?php
					?><td><?php echo $item['type']     ?></td><?php
					?><td><?php echo $item['template'] ?></td><?php
					?></tr><?php
				}
				?></table><?php
			}

			$o->note(translate('core_ct_wysiwyg_link_type_templates_note'));
			if (is_null($pref)) $o->closeField();
		}// end if show SQ_WYSIWYG_LINK_TYPES
		
		if (is_null($pref) || $pref == 'SQ_LIVE_LINK_ONLY') {			
						
			if (is_null($pref)) {
				$o->openField(translate($this->pref_vars['SQ_LIVE_LINK_ONLY']['name']));
			}
						
			$pref_var =& $this->pref_vars['SQ_LIVE_LINK_ONLY']['default'];
			if ($have_lock && $is_admin) {
				$options = Array(1 => 'Yes', 0 => 'No');
				combo_box('prefs[SQ_LIVE_LINK_ONLY]', $options, FALSE, $pref_var);
				echo $o->note(translate('live_link_only_note'));
			} else {
				echo ($pref_var) ? translate('yes') : translate('no');
			}
						
			if (is_null($pref)) $o->closeField();
			
		}//end if show SQ_LIVE_LINK_ONLY	

		return TRUE;

	}//end paintBackend()


	/**
	* Processes the backend interface to edit preferences
	*
	* @param object		&$o			reference to the backend outputter object
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	*
	* @return boolean
	* @access public
	*/
	function processBackend(&$o, $have_lock, $pref=NULL)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$is_admin) return FALSE;

		// enable viper
		if (is_null($pref) || $pref == 'SQ_ENABLE_VIPER_EDITOR') {
			if (isset($_POST['prefs']['SQ_ENABLE_VIPER_EDITOR'])) {
				$this->pref_vars['SQ_ENABLE_VIPER_EDITOR']['default'] = $_POST['prefs']['SQ_ENABLE_VIPER_EDITOR'];
			}
		}		

		// WYSIWYG STYLE
		if (is_null($pref) || $pref == 'SQ_WYSIWYG_STYLE') {
			if (isset($_POST['prefs']['SQ_WYSIWYG_STYLE'])) {
				$this->pref_vars['SQ_WYSIWYG_STYLE']['default'] = $_POST['prefs']['SQ_WYSIWYG_STYLE'];
			}
		}

		// WYSIWYG APPLY DESIGN STYLE
		if (is_null($pref) || $pref == 'SQ_WYSIWYG_APPLY_DESIGN_STYLE') {
			if (isset($_POST['prefs']['SQ_WYSIWYG_APPLY_DESIGN_STYLE'])) {
				$this->pref_vars['SQ_WYSIWYG_APPLY_DESIGN_STYLE']['default'] = $_POST['prefs']['SQ_WYSIWYG_APPLY_DESIGN_STYLE'];
			}
		}
		

		// WYSIWYG PLUGINS
		if (is_null($pref) || $pref == 'SQ_WYSIWYG_PLUGINS') {
			if (isset($_POST['prefs']['SQ_WYSIWYG_PLUGINS_PRESENT'])) {
				$wysiwyg_plugins = Array();
				if (isset($_POST['prefs']['SQ_WYSIWYG_PLUGINS'])) {
					$wysiwyg_plugins = $_POST['prefs']['SQ_WYSIWYG_PLUGINS'];
				}
				$plugin_list = '';
				$all = TRUE;

				require_once SQ_FUDGE_PATH.'/general/file_system.inc';
				$all_plugins = list_dirs(SQ_FUDGE_PATH.'/wysiwyg/plugins');

				foreach ($all_plugins as $plugin_name) {
					$plugin_path = SQ_FUDGE_PATH.'/wysiwyg/plugins/'.$plugin_name;
					if (!is_file($plugin_path.'/'.$plugin_name.'.inc')) {
						continue;
					}

					if (!isset($wysiwyg_plugins[$plugin_name]) || !$wysiwyg_plugins[$plugin_name]) {
						$all = FALSE;
						continue;
					}
					$plugin_list .= $plugin_name.'|';
				}

				if ($all) {
					$plugin_list = 'all';
				} else {
					$plugin_list = trim($plugin_list, '|');
				}
				$this->pref_vars['SQ_WYSIWYG_PLUGINS']['default'] = $plugin_list;
			}
		}//end if

		// WYSIWYG DISABLE IMAGE RESIZE
		if (is_null($pref) || $pref == 'SQ_WYSIWYG_DISABLE_IMAGE_RESIZE') {
			if (isset($_POST['prefs']['SQ_WYSIWYG_DISABLE_IMAGE_RESIZE'])) {
				$this->pref_vars['SQ_WYSIWYG_DISABLE_IMAGE_RESIZE']['default'] = $_POST['prefs']['SQ_WYSIWYG_DISABLE_IMAGE_RESIZE']['default'];
			}
		}

		// THESAURUS
		if (is_null($pref) || $pref == 'SQ_THESAURUS') {
			if (isset($_POST['prefs']['SQ_THESAURUS'])) {
				$this->pref_vars['SQ_THESAURUS']['default'] = $_POST['prefs']['SQ_THESAURUS']['assetid'];
			}
		}

		// WYSIWYG LINK TYPE TEMPLATES
		if (is_null($pref) || $pref == 'SQ_WYSIWYG_LINK_TYPES') {
			if (isset($_POST['prefs']['SQ_WYSIWYG_LINK_TYPES'])) {
				// Get what we have been given
				$this->pref_vars['SQ_WYSIWYG_LINK_TYPES']['default'] = $_POST['prefs']['SQ_WYSIWYG_LINK_TYPES']['default'];

				foreach ($this->pref_vars['SQ_WYSIWYG_LINK_TYPES']['default'] as $key => $value) {
					// Get rid of indexes we don't need, as a cleansing measure
					$allowed_sub_keys = Array('type', 'template', 'delete');
					foreach ($value as $sub_key) {
						if (!in_array($sub_key, $allowed_sub_keys)) {
							unset($this->pref_vars['SQ_WYSIWYG_LINK_TYPES']['default'][$key][$sub_key]);
						}
					}

					// If we are set to delete a row, then delete the lot of it
					if (array_get_index($value, 'delete', FALSE)) {
						unset($this->pref_vars['SQ_WYSIWYG_LINK_TYPES']['default'][$key]);
					}
				}
				// Re-order so that we start from zero again
				$this->pref_vars['SQ_WYSIWYG_LINK_TYPES']['default'] = array_values($this->pref_vars['SQ_WYSIWYG_LINK_TYPES']['default']);
			}
		}
		
		// LIVE LINK ONLY
		if (is_null($pref) || $pref == 'SQ_LIVE_LINK_ONLY') {
			if (isset($_POST['prefs']['SQ_LIVE_LINK_ONLY'])) {
				$this->pref_vars['SQ_LIVE_LINK_ONLY']['default'] = $_POST['prefs']['SQ_LIVE_LINK_ONLY'];
			}
		}
		

		return TRUE;

	}//end processBackend()


	/**
	* Merges two preference arrays for a content type
	*
	* @param array	$prefs1	array of preferences to merge
	* @param array	$prefs2	array of preferences to merge
	*
	* @return array
	* @access public
	*/
	function mergePrefs($prefs1, $prefs2)
	{
		$merged_prefs = Array();
		$pref_vars = Array('SQ_WYSIWYG_STYLE', 'SQ_WYSIWYG_PLUGINS', 'SQ_THESAURUS');

		foreach ($pref_vars as $var_name) {
			if (isset($prefs1[$var_name]) && !isset($prefs2[$var_name])) {
				// this is only set in prefs1
				$merged_prefs[$var_name] = $prefs1[$var_name];
			} else if (isset($prefs2[$var_name]) && !isset($prefs1[$var_name])) {
				// this is only set in prefs2
				$merged_prefs[$var_name] = $prefs2[$var_name];
			} else if (isset($prefs1[$var_name]) && isset($prefs2[$var_name])) {
				// this is set in both prefs
				switch ($var_name) {
					case 'SQ_WYSIWYG_STYLE' :
						// can't merge the style, so just use the first one
						$merged_prefs[$var_name] = $prefs1[$var_name];
					break;

					case 'SQ_WYSIWYG_PLUGINS' :
						$plug1 = explode('|', $prefs1['SQ_WYSIWYG_PLUGINS']['default']);
						if (is_array($prefs2['SQ_WYSIWYG_PLUGINS']) && isset($prefs2['SQ_WYSIWYG_PLUGINS']['default'])) {
							$plug2 = explode('|', $prefs2['SQ_WYSIWYG_PLUGINS']['default']);
						} else {
							$plug2 = explode('|', $prefs2['SQ_WYSIWYG_PLUGINS']);
						}
						$merged_prefs['SQ_WYSIWYG_PLUGINS']['default'] = implode('|', array_unique(array_merge($plug1, $plug2)));
					break;

					case 'SQ_THESAURUS' :
						// use the first thesaurus
						$merged_prefs['SQ_THESAURUS']['default'] = $prefs1[$var_name]['default'];
					break;

				}

				$merged_prefs[$var_name]['name']		= $prefs1[$var_name]['name'];
				$merged_prefs[$var_name]['description']	= $prefs1[$var_name]['description'];

				if (isset($prefs2[$var_name]['protected'])) {
					$merged_prefs[$var_name]['protected'] = ($prefs1[$var_name]['protected'] || $prefs2[$var_name]['protected']);
				} else {
					$merged_prefs[$var_name]['protected'] = $prefs1[$var_name]['protected'];
				}
			}//end if
		}//end foreach

		return $merged_prefs;

	}//end mergePrefs()


}//end class

?>
