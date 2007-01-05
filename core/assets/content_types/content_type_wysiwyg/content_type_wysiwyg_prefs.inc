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
* $Id: content_type_wysiwyg_prefs.inc,v 1.18.2.1 2007/01/05 06:21:12 hnguyen Exp $
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
* @version $Revision: 1.18.2.1 $
* @package MySource_Matrix
*/
class Content_Type_WYSIWYG_Prefs extends Prefs
{
	var $pref_vars = Array(
						'SQ_WYSIWYG_STYLE'					=> Array(
																'name'			=> 'WYSIWYG Style',
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
						'SQ_WYSIWYG_PLUGINS'				=> Array(
																'name'			=> 'WYSIWYG Plugins',
																'description'	=> 'This preference allows you to select which features of the WYSIWYG editor you would like make available while editing',
																'default'		=> 'all',
																'protected'		=> FALSE,
															   ),
						'SQ_WYSIWYG_DISABLE_IMAGE_RESIZE'	=> Array(
																'name'			=> 'WYSIWYG Disable Image Resizing',
																'description'	=> 'This preference allows you to select whether to disable the image resizing functionality in WYSIWYG',
																'default'		=> 'no',
																'protected'		=> FALSE,
															   ),
						'SQ_THESAURUS'						=> Array (
																'name'			=> 'Thesaurus',
																'description'	=> 'Which thesaurus asset you want to use when editing content, for example to provide keyword expansions',
																'default'		=> 0,
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

		if (is_null($pref) || $pref == 'SQ_WYSIWYG_STYLE') {
			if (is_null($pref)) {
				$o->openField(translate('wysiwyg_style'));
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
							$wysiwyg_style['width'] = ereg_replace('[^0-9]', '', $wysiwyg_style['width']);
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
							$wysiwyg_style['height'] = ereg_replace('[^0-9]', '', $wysiwyg_style['height']);
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


		if (is_null($pref) || $pref == 'SQ_WYSIWYG_PLUGINS') {
			if (is_null($pref)) {
				$o->openField(translate('wysiwyg_plugins'));
			}

			$plugins = Array();
			if (!empty($this->pref_vars['SQ_WYSIWYG_PLUGINS']['default'])) {
				$plugins = explode('|', $this->pref_vars['SQ_WYSIWYG_PLUGINS']['default']);
			}
			$all = in_array('all', $plugins);

			if ($have_lock && $is_admin) {
				require_once SQ_FUDGE_PATH.'/general/file_system.inc';
				$all_plugins = list_dirs(SQ_FUDGE_PATH.'/wysiwyg/plugins');
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
				$o->openField(translate('wysiwyg_disable_image_resize'));
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

		}//end if show SQ_WYSIWYG_DISABLE_IMAGE_RESIZE

		if ($GLOBALS['SQ_SYSTEM']->am->installed('thesaurus') && (is_null($pref) || $pref == 'SQ_THESAURUS')) {
			if (is_null($pref)) {
				$o->openField(translate('thesaurus'));
			}

			if ($have_lock && $is_admin) {
				asset_finder('prefs[SQ_THESAURUS]', $this->pref_vars['SQ_THESAURUS']['default'], Array('thesaurus'=>'A'));
				echo '<br /><i>'.translate('thesaurus_used_by_matrix_expand_keywords_plugin').'</i>';
			} else {
				if (!$this->pref_vars['SQ_THESAURUS']['default']) {
					echo '('.translate('none_selected').')';
				} else {
					$name = reset($GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($this->pref_vars['SQ_THESAURUS']['default']), 'thesaurus', TRUE, 'name'));
					echo $name.' (#'.$this->pref_vars['SQ_THESAURUS']['default'].')';
				}
			}
		}//end if show SQ_THESAURUS

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


		// WYSIWYG STYLE
		if (is_null($pref) || $pref == 'SQ_WYSIWYG_STYLE') {
			if (isset($_POST['prefs']['SQ_WYSIWYG_STYLE'])) {
				$this->pref_vars['SQ_WYSIWYG_STYLE']['default'] = $_POST['prefs']['SQ_WYSIWYG_STYLE'];
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
		}

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
			}
		}//end foreach

		return $merged_prefs;

	}//end mergePrefs()


}//end class

?>