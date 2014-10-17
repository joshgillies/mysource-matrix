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
						'SQ_LIVE_LINK_ONLY'					=> Array (
																'name'			=> 'live_link_only',
																'description'	=> 'To allow links pointing only to live asset',
																'default'		=> 0,
																'protected'		=> FALSE,
															   ),
    					'SQ_ENABLE_VIPER_EDITOR'			=> Array (
																'name'			=> 'enable_edit+_editor',
																'description'	=> 'Enable Edit+ WYSIWYG editor',
																'default'		=> 1,
																'protected'		=> FALSE,
															   ),
						'SQ_VIPER_PLUGINS'				=> Array(
																'name'			=> 'viper_plugins',
																'description'	=> 'This preference allows you to select which features of the Viper editor you would like make available while editing',
																'default'		=> 'all',
																'protected'		=> FALSE,
															   ),
					 );

	// Store all the viper plugins here. Unlike WYSYWIG we can traverse
	// through file system (on somethin to do on server side) to get the
	// list of the plugins available to us. For further details for this
	// please see RM issue #5323 Add global preferences for Edit+ WYSIWYG
	// plugin options (https://squizmap.squiz.net/matrix/5323)
	var $_viper_plugins = Array();


	/**
	* Constructor
	*
	*/
	function Content_Type_WYSIWYG_Prefs($pref_file='')
	{
		$this->Prefs($pref_file);

		// List of Viper plugins and names. This does not include the core
		// plugins (Keyboard Editor, Toolbar, Inline Toolbar and Cursor
		// Assist) which will be included if any of the others are.
		$this->_viper_plugins = Array(
            'ViperAccessibilityPlugin'	=> translate('Accessibility'),
            'ViperLangToolsPlugin'		=> translate('Choose Language'),
            'ViperCoreStylesPlugin'		=> translate('Core Styles'),
            'ViperFormatPlugin'			=> translate('Formatting'),
            'ViperHistoryPlugin'		=> translate('History'),
            'MatrixImagePlugin'			=> translate('Insert Image'),
            'MatrixLinkPlugin'			=> translate('Insert Link'),
            'MatrixKeywordsPlugin'		=> translate('Keywords <i>(Asset-based & snippets)</i>'),
            'ViperListPlugin'			=> translate('Listing'),
            'MatrixCopyPastePlugin'		=> translate('Matrix Editing Tools <i>(specific to editing Matrix-based URLs)</i>'),
            'ViperSearchReplacePlugin'	=> translate('Search and Replace'),
            'ViperCharMapPlugin'		=> translate('Special Characters'),
            'ViperTableEditorPlugin'	=> translate('Table Editing'),
            'ViperSourceViewPlugin'		=> translate('View Source'),
            'ViperCopyPastePlugin'		=> translate('Editing Tools <i>(Copy, Paste, etc.)</i>'),
          );

        $this->pref_vars['SQ_WYSIWYG_STYLE']['name'] = translate('WYSIWYG Style');

        $this->pref_vars['SQ_WYSIWYG_APPLY_DESIGN_STYLE']['name'] = translate('WYSIWYG CSS Styles');
        $this->pref_vars['SQ_WYSIWYG_APPLY_DESIGN_STYLE']['desc'] = translate('Control whether CSS styles will be applied to the WYSIWYG.').' '.
            translate('The styles are defined in CSS files used as Associated Files for the current asset.').' '.
            translate('The Matrix Apply Design plugin must be enabled for this setting to take effect.');

        $this->pref_vars['SQ_WYSIWYG_PLUGINS']['name'] = translate('WYSIWYG Plugins');

        $this->pref_vars['SQ_WYSIWYG_DISABLE_IMAGE_RESIZE']['name'] = translate('WYSIWYG Image Resize');

        $this->pref_vars['SQ_THESAURUS']['name'] = translate('Thesaurus');
        $this->pref_vars['SQ_THESAURUS']['desc'] = translate('The thesaurus asset used by the Matrix Expand Keywords plugin.');

        $this->pref_vars['SQ_WYSIWYG_LINK_TYPES']['name'] = translate('Protocol Templates');
        $this->pref_vars['SQ_WYSIWYG_LINK_TYPES']['desc'] = translate('This preference allows you to customise the templates used for protocols (or link types) displayed on the Insert Link pop-up.').' '.
            translate('The keyword <b>%%link%%</b> is replaced by the <b>Link</b> field in the Insert Link pop-up - this can appear anywhere in the string, allowing you to make \'pseudo-protocols\' that expand to a link in a certain format.').' '.
            translate('The \'http\', \'https\', \'mailto\' and \'ftp\' protocols are core to the system, and cannot be deleted or modified.');

        $this->pref_vars['SQ_LIVE_LINK_ONLY']['name'] = translate('Live Asset Hyperlink Safety');
        $this->pref_vars['SQ_LIVE_LINK_ONLY']['desc'] = translate('If yes, editors of Live assets will only able permitted to create hyperlinks to other Live assets.');

        $this->pref_vars['SQ_ENABLE_VIPER_EDITOR']['name'] = translate('Admin Mode WYSIWYG Type');
        $this->pref_vars['SQ_ENABLE_VIPER_EDITOR']['desc'] = translate('Select the editor that you wish to use for WYSIWYG Content Containers in Admin Mode.').' '.
            sprintf(
                translate('If the Edit+ editor is selected, the following WYSIWYG preferences will not be applied: %s, %s, %s, %s, %s and %s.'),
                translate('WYSIWYG Style'),
                translate('WYSIWYG CSS Styles'),
                translate('WYSIWYG Plugins'),
                translate('WYSIWYG Image Resize'),
                translate('Thesaurus'),
                translate('Protocol Templates')
            );

        $this->pref_vars['SQ_VIPER_PLUGINS']['name'] = translate('Edit+ Editor Plugins');


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
	    $is_root   = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin  = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
        $have_lock = ($have_lock && $is_admin);

        // Enable Edit+ (Viper) Editor in Admin Mode?
        $options = Array(
                       1 => translate('Use Edit+ editor'),
                       0 => translate('Use Matrix classic editor'),
                   );
        $this->paintComboField($o, $have_lock, $pref, 'SQ_ENABLE_VIPER_EDITOR', $options);

        // Disable WYSIWYG linking from Live to Non-Live assets?
        $options = Array(
                       1 => translate('Live assets can only link to other Live assets'),
                       0 => translate('Live assets can link to all other assets'),
                   );
        $this->paintComboField($o, $have_lock, $pref, 'SQ_LIVE_LINK_ONLY', $options);

        $o->openSection(translate('Edit+ WYSIWYG Editor Preferences'));

		$this->paintField($o, $have_lock, $pref, 'SQ_VIPER_PLUGINS', function($plugins_setting) use ($have_lock) {
			$viper_plugins = Array();
			if (!empty($plugins_setting)) {
				$viper_plugins = explode('|', $plugins_setting);
			}
			$all = in_array('all', $viper_plugins);

			$i = 0;
			?>
			<table class="sq-backend-table">
			<col width="27%" />
			<col width="6%" />
			<col width="27%" />
			<col width="6%" />
			<col width="27%" />
			<col width="6%" />
			<?php
            foreach ($this->_viper_plugins as $index => $plugin_name) {
                if ($i % 3 === 0) {
                    if ($i > 0) {
                        echo '</tr>';
                    }
                    echo '<tr>';
                }

                echo '<th>'.$plugin_name.'</th>';
                echo '<td style="text-align: center">';
                $checked  = ($all || in_array($index, $viper_plugins));
                $alt_text = translate('Disabled');
                if ($checked) {
                    $alt_text = translate('Enabled');
                }

                if ($have_lock) {
                    check_box('prefs[SQ_VIPER_PLUGINS]['.$index.']', 1, $checked);
                } else {

                    echo '<img src="'.sq_web_path('lib').'/web/images/'.($checked ? 'tick' : 'cross').'.gif" title="'.$alt_text.'" alt="'.$alt_text.'" />';
                }
                echo '</td>';
                $i++;
            }
            ?></tr>
            </table>
            <?php
		});//end closure (SQ_VIPER_PLUGINS)

		$o->closeSection();

        $o->openSection(translate('Matrix Classic WYSIWYG Editor Preferences'));

		$this->paintField($o, $have_lock, $pref, 'SQ_WYSIWYG_STYLE', function($wysiwyg_style) use ($have_lock) {
		    $size_units = Array('%' => '%', 'px' => 'px');
            ?>
            <table class="sq-backend-table" style="width: 350px;">
                <tr>
                    <td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Body Type'); ?></td>

                    <td class="sq-backend-table-cell" style="width: 100%;">
                    <?php
                    if ($have_lock) {
                        combo_box('prefs[SQ_WYSIWYG_STYLE][body_type]', Array('DIV' => translate('DIV'), 'iFrame' => translate('iFrame')), FALSE, $wysiwyg_style['body_type']);


                    } else {
                        echo $wysiwyg_style['body_type'];
                    }
                    ?>
                    </td>
                </tr>
                <tr>
                    <td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Width'); ?></td>

                    <td class="sq-backend-table-cell" style="width: 100%;">
                    <?php
                    if (!is_numeric($wysiwyg_style['width'])) {
                        $wysiwyg_style['width'] = preg_replace('/[^0-9]/', '', $wysiwyg_style['width']);
                    }
                    if ($have_lock) {
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
                    <td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Height'); ?></td>

                    <td class="sq-backend-table-cell" style="width: 100%;">
                    <?php
                    if (!is_numeric($wysiwyg_style['height'])) {
                        $wysiwyg_style['height'] = preg_replace('/[^0-9]/', '', $wysiwyg_style['height']);
                    }
                    if ($have_lock) {
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
            <div class="sq-warning-message">
                <?php echo translate('<strong>Note:</strong> "Body Type" only affects Internet Explorer, versions 10 and below. In Mozilla or WebKit browsers, or IE 11 and above, an IFrame is always used.') ?>
            </div>
            <?php
		});//end closure (SQ_WYSIWYG_STYLE)

        // Apply CSS styles to (classic) WYSIWYG?
        $options = Array(
                       1 => translate('Apply CSS styles'),
                       0 => translate('Don\'t apply CSS styles'),
                   );
        $this->paintComboField($o, $have_lock, $pref, 'SQ_WYSIWYG_APPLY_DESIGN_STYLE', $options);

        // Classic WYSIWYG plugins
		$this->paintField($o, $have_lock, $pref, 'SQ_WYSIWYG_PLUGINS', function($plugins_setting) use ($have_lock) {
			$plugins = Array();
			if (!empty($plugins_setting)) {
				$plugins = explode('|', $plugins_setting);
			}
			$all = in_array('all', $plugins);
			sort($plugins);

			$all_plugins = list_dirs(SQ_FUDGE_PATH.'/wysiwyg/plugins');
			if (!empty($all_plugins)) {
                hidden_field('prefs[SQ_WYSIWYG_PLUGINS_PRESENT]', '1');
            }

            $i = 0;

            ?>
			<table class="sq-backend-table">
			<col width="27%" />
			<col width="6%" />
			<col width="27%" />
			<col width="6%" />
			<col width="27%" />
			<col width="6%" />
			<?php
            foreach ($all_plugins as $index => $plugin_name) {
				global $ROOT_PATH;
				$ROOT_PATH = SQ_FUDGE_PATH.'/wysiwyg/';
                $plugin_path = SQ_FUDGE_PATH.'/wysiwyg/plugins/'.$plugin_name;
                if (!is_file($plugin_path.'/'.$plugin_name.'.inc')) {
                    continue;
                }
                include $plugin_path.'/'.$plugin_name.'.inc';
                $check_result = call_user_func(Array($plugin_name, 'check_usable'));

                if ($i % 3 === 0) {
                    if ($i > 0) {
                        echo '</tr>';
                    }
                    echo '<tr>';
                }

                echo '<th>';
                if ($have_lock) {
                    label('&nbsp;'.ucwords(str_replace('_', ' ', $plugin_name)), 'prefs[SQ_WYSIWYG_PLUGINS]['.$plugin_name.']');
                } else {
                    echo ucwords(str_replace('_', ' ', $plugin_name));
                }
                if ($check_result !== TRUE) {
                    echo ' <i>('.$check_result.')</i>';
                }
                echo '</th>';
                echo '<td style="text-align: center">';

                $checked  = ($all || in_array($plugin_name, $plugins));
                $alt_text = translate('Disabled');
                if ($checked) {
                    $alt_text = translate('Enabled');
                }

                $extra = ($check_result === TRUE) ? '' : 'disabled="disabled"';

                if ($have_lock) {
                    check_box('prefs[SQ_WYSIWYG_PLUGINS]['.$plugin_name.']', '1', $checked,'',$extra);
                } else {
                    echo '<img src="'.sq_web_path('lib').'/web/images/'.($checked ? 'tick' : 'cross').'.gif" title="'.$alt_text.'" alt="'.$alt_text.'" />';
                }
                echo '</td>';
                $i++;
            }
            ?></tr>
            </table>
            <?php


/*

			if ($have_lock) {
				require_once SQ_FUDGE_PATH.'/general/file_system.inc';
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
					label('&nbsp;'.ucwords(str_replace('_', ' ', $plugin_name)), 'prefs[SQ_WYSIWYG_PLUGINS]['.$plugin_name.']');
					if ($check_result !== TRUE) {
						echo ' <i>('.$check_result.')</i>';
					}
					echo '<br/>';
				}
			} else {
				if ($all) {
					echo translate('All plugins are selected');

				} else {
					if (empty($plugins)) {
						echo translate('No plugins are selected');

					} else {
						echo '<ul>';
						foreach ($plugins as $plugin_name) {
							echo '<li class="sq-backend-data">'.ucwords(str_replace('_', ' ', $plugin_name)).'</li>';
						}
						echo '</ul>';
					}
				}
			}*/
		});//end closure (SQ_WYSIWYG_PLUGINS)

        // Disable image resizing in (classic) WYSIWYG?
        $options = Array(
                       'yes' => translate('Disable image resizing'),
                       'no'  => translate('Allow image resizing'),
                   );
        $this->paintComboField($o, $have_lock, $pref, 'SQ_WYSIWYG_DISABLE_IMAGE_RESIZE', $options);

        // Thesaurus selection.
        $this->paintAssetMapField($o, $have_lock, $pref, 'SQ_THESAURUS', FALSE, Array('thesaurus' => 'A'), translate('No Thesaurus asset selected'));

        // Protocol templates.
		$this->paintField($o, $have_lock, $pref, 'SQ_WYSIWYG_LINK_TYPES', function($pref_value) use ($have_lock) {
			if ($have_lock) {

				?><table class="sq-backend-table" style="width:auto">
				<tr>
					<th>Link Type</th>
					<th>Template</th>
					<th><?php echo translate('Delete?') ?></th>

				</tr><?php
				foreach ($pref_value as $key => $item) {
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
				sq_wysiwyg_link_types_nextIndex = <?php echo (int)count($pref_value); ?>;
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
				foreach ($pref_value as $item) {
					?><tr><?php
					?><td><?php echo $item['type']     ?></td><?php
					?><td><?php echo $item['template'] ?></td><?php
					?></tr><?php
				}
				?></table><?php
			}

		});// end closure (SQ_WYSIWYG_LINK_TYPES)

		$o->closeSection();

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

		// VIPER PLUGINS
		if (is_null($pref) || $pref == 'SQ_VIPER_PLUGINS') {
				$viper_plugins = Array();

				if (isset($_POST['prefs']['SQ_VIPER_PLUGINS'])) {
					$viper_plugins = $_POST['prefs']['SQ_VIPER_PLUGINS'];
				}
				$plugin_list = '';
				$all = TRUE;

				foreach ($this->_viper_plugins as $index => $plugin_name) {
					if (!isset($viper_plugins[$index]) || !$viper_plugins[$index]) {
						$all = FALSE;
						continue;
					}
					$plugin_list .= $index.'|';
				}

				if ($all) {
					$plugin_list = 'all';
				} else {
					$plugin_list = trim($plugin_list, '|');
				}

				$this->pref_vars['SQ_VIPER_PLUGINS']['default'] = $plugin_list;
		}//end if


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
				$this->pref_vars['SQ_WYSIWYG_DISABLE_IMAGE_RESIZE']['default'] = $_POST['prefs']['SQ_WYSIWYG_DISABLE_IMAGE_RESIZE'];
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
