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
* $Id: paint_layout_lookup_value.inc,v 1.18 2012/09/26 00:45:53 akarelia Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/lookup_value/lookup_value.inc';

/**
* Lookup Value
*
* Parent Asset for all different types of lookup value interfaces needed by different parts of the system
*
*
* @author	Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.18 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Paint_Layout_Lookup_Value extends Lookup_Value
{


	/**
	* Return the lookup name to use in the interface
	*
	* @return string
	* @access public
	* @static
	*/
	public static function lookupValueName()
	{
		return translate('Effective Paint Layouts');

	}//end lookupValueName()


	/**
	* Return the prefix that these values use in their name (minus the '::')
	*
	* @return string
	* @access public
	* @static
	*/
	public static function namePrefix()
	{
		return 'paint_layout';

	}//end namePrefix()


	/**
	* Paint the HTML editing interface for this lookup value.
	*
	* @param object	&$owner			the asset whose interface we are painting
	* @param object	&$o				the backend outputter object
	* @param array	$lookups		array of available lookups
	* @param array	$lookup_values	array of the values that we want to save
	*								<pre>
	*									"[url]" => array (
	*										"[name]" => array (
	*											"value" => "[value]",
	*											"inhd" => "[inherited]",
	*										),
	*									),
	*								</pre>
	* @param string	$prefix			unique prefix to use for the form elements
	*
	* @return boolean
	* @access public
	* @static
	*/
	public static function paintInterface(&$owner, &$o, $lookups, $lookup_values, $prefix)
	{
		$wa = ($owner->writeAccess('lookups') && $owner->writeAccess('links'));
		$name_prefix = Paint_Layout_Lookup_Value::namePrefix();

		$o->openSection(translate('Asset Definitions'));


		if (!empty($lookup_values)) {
			$o->openField(translate('Summary'));

			?>
			<table class="sq-backend-table">
				<tr>
					<th class="sq-backend-table-header" style="width:50%;"><?php echo translate('URL'); ?></th>

					<th class="sq-backend-table-header" style="width:50%;"><?php echo 'Effective Paint Layout'; ?></th>
				</tr>
			<?php
			foreach ($lookups as $url) {
				// need to make sure the URL is printed even if it does not have a layout applied
				if (empty($lookup_values[$url][$name_prefix.'::system::frontend'])) {
					$lookup_values[$url][$name_prefix.'::system::frontend'] =  Array('value' => '', 'inhd' => '0');
				}

				foreach ($lookup_values[$url] as $layout_name => $data) {
					// determine the type of layout applied and sort
					$matches = Array();
					preg_match('/^'.$name_prefix.'::(system|user)::(.*)$/', $layout_name, $matches);

					// store so we can use these values for individual URL settings later
					$lookup_values[$url][$layout_name]['type'] = $matches[1];
					$lookup_values[$url][$layout_name]['code'] = $matches[2];

					// don't print user defined paint layouts in the asset definitions table
					if ($matches[1] === 'user') continue;

					$effective_layout = $data['value'];

					$override_suffix = '';
					// get override layout if one exists
					if (isset($lookup_values[$url.'/'])) {
						foreach ($lookup_values[$url.'/'] as $override_layout_name => $override_data) {
							$effective_layout = $override_data['value'];
							$effective_layout_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($effective_layout));
						}
						$suffix = ' - <strong style="font-size:11px"><i>'.translate('Override').'</i></strong>';

					} else {
						$effective_layout_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($effective_layout));
						$suffix = self::_getSuffix($data['inhd'], FALSE);
					}

					?>
					<tr>
						<td class="sq-backend-table-cell"><?php echo $url; ?></td>
						<td class="sq-backend-table-cell"><?php echo !empty($effective_layout_info) ? get_asset_tag_line($effective_layout).$suffix : '<i>'.translate('None').'</i>'; ?></td>

					</tr>
					<?php
				}
			}
			?>
			</table>
			<?php
		$o->closeField();
		}//end if

		$asset_layout = $GLOBALS['SQ_SYSTEM']->am->getLink($owner->id, SQ_LINK_NOTICE, 'paint_layout_page', FALSE, 'paint_layout::system::frontend');
		$o->openField(translate('Paint Layout'));

		if ($wa) {
			asset_finder($prefix.'[asset][paint_layout][value]', empty($asset_layout['minorid']) ? '' : $asset_layout['minorid'], Array('paint_layout_page' => 'D'), 'sq_sidenav', FALSE, 'null', empty($asset_layout) ? Array('clear') : Array('clear', 'reset'));
		} else {
			echo empty($asset_layout) ? '<i>'.translate('None').'</i>' : get_asset_tag_line($asset_layout['minorid']);

		}
		$o->closeField();

		$asset_override = $GLOBALS['SQ_SYSTEM']->am->getLink($owner->id, SQ_LINK_NOTICE, 'paint_layout_page', FALSE, 'override::paint_layout::system::frontend');
		$o->openField(translate('Override Paint Layout'));

		if ($wa) {
			asset_finder($prefix.'[asset][override::paint_layout][value]', empty($asset_override) ? '' : $asset_override['minorid'], Array('paint_layout_page' => 'D'), 'sq_sidenav', FALSE, 'null', empty($asset_override) ? Array('clear') : Array('clear', 'reset'));
		} else {
			echo empty($asset_override) ? '<i>'.translate('None').'</i>' : get_asset_tag_line($asset_override['minorid']);

		}
		$design_info = $owner->getDesignLookups();
		if (empty($design_info) && !empty($lookup_values)) {
			echo '<br/><span style="color:#FF0000">'.translate('<b>NOTE:</b> This asset does not have a design applied to it. You need a Design applied for Paint Layouts to work.').'</span>';

		}

		$o->closeField();

		// close asset definitions section
		$o->closeSection();

		// close the paint layout section
		$o->closeSection();

		// which lookups have user defined layouts
		$lookups_user = Array();

		$o->openSection(translate('URL Specific Paint Layouts'));

		foreach ($lookups as $url) {

			$o->openSection($url);
			$o->openField(translate('Current'));

			?>
			<table class="sq-backend-table">
				<tr>
					<th class="sq-backend-table-header" style="width:50%;"><?php echo translate('Layout Type'); ?></th>
					<th class="sq-backend-table-header" style="width:50%;"><?php echo translate('Paint Layout'); ?></th>
				</tr>
			<?php

			if (!isset($lookup_values[$url])) {
				$lookup_values[$url] = Array(0);
			}

			foreach ($lookup_values[$url] as $layout_name => $data) {
				if ($data['type'] === 'user') {
					// so we know which lookups have user defined layouts for painting the next section
					$lookups_user[] = $url;
					continue;
				}

				$layout_name = empty($layout_name) ? 'paint_layout::system::frontend' : $layout_name;

				$display_name = sprintf(translate('%1$s Defined - %2$s Paint Layout'), empty($data['type']) ? translate('System') : ucwords($data['type']), ucwords(str_replace('_', ' ', $data['code'])));


				$display_override_name = sprintf(translate('%1$s Defined - %2$s Override Paint Layout'), empty($data['type']) ? translate('System') : ucwords($data['type']), ucwords(str_replace('_', ' ', $data['code'])));



				// to prevent overwriting inherited values unless a valid change was made
				hidden_field($prefix.'['.$url.']['.$layout_name.'][value][original]', $data['value']);

				$url_override_layout = 0;
				$url_override_info = Array();
				// get override layout if one exists
				if (isset($lookup_values[$url.'/'])) {
					foreach ($lookup_values[$url.'/'] as $override_layout_name => $override_data) {
						$url_override_layout = $override_data['value'];
						$url_override_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($url_override_layout));
					}
				}

				// do not pre-populate asset finder if the values are the same as the corresponding notice link values
				if (!empty($asset_layout) && isset($asset_layout['minorid']) && ($asset_layout['minorid'] === $data['value'])) {
					$layout_value = '0';
					$url_layout_info = '0';
				} else {
					$layout_value = $data['value'];
					$url_layout_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($data['value']));
				}
				if (!empty($asset_override) && isset($asset_override['minorid']) && ($asset_override['minorid'] === $url_override_layout)) {
					$url_override_layout = '0';
					$url_override_info = '0';
				}
				hidden_field($prefix.'['.$url.'/]['.$layout_name.'][value][original]', $url_override_layout);

				if ($data['inhd']) {
					$normal_buttons = empty($layout_value) ? Array('clear') : Array('reset');
				} else {
					$normal_buttons = empty($layout_value) ? Array('clear') : Array('clear', 'reset');
				}
				$override_buttons = empty($url_override_layout) ? Array('clear') : Array('clear', 'reset');

				?>
					<tr>
						<td class="sq-backend-table-cell"><?php echo $display_name; echo $wa ? self::_getSuffix($data['inhd'], TRUE) : ''; ?></td>
						<?php if ($wa) { ?>
							<td class="sq-backend-table-cell"><?php asset_finder($prefix.'['.$url.']['.$layout_name.'][value]', $layout_value, Array('paint_layout_page' => 'D'), 'sq_sidenav', FALSE, 'null', $normal_buttons);?></td>
						<?php } else { ?>
							<td class="sq-backend-table-cell"><?php echo empty($url_layout_info) ? '<i>'.translate('Not customised').'</i>' : get_asset_tag_line($layout_value).self::_getSuffix($data['inhd'], FALSE); ?></td>

						<?php } ?>
					</tr>
					<tr>
						<td class="sq-backend-table-cell"><?php echo $display_override_name; ?></td>
						<?php if ($wa) { ?>
							<td class="sq-backend-table-cell"><?php asset_finder($prefix.'['.$url.'/]['.$layout_name.'][value]', $url_override_layout, Array('paint_layout_page' => 'D'), 'sq_sidenav', FALSE, 'null', $override_buttons);?></td>
						<?php } else { ?>
							<td class="sq-backend-table-cell"><?php echo empty($url_override_info) ? '<i>'.translate('Not customised').'</i>': get_asset_tag_line($url_override_layout); ?></td>

						<?php } ?>
					</tr>
				<?php

				// remove data not needed anymore for painting
				unset($lookup_values[$url][$layout_name]);
				if (empty($lookup_values[$url])) {
					// populate with empty data for painting the user defined layout section
					$lookup_values[$url][$name_prefix.'::user::'] =  Array();
				}

			}//end foreach

			echo '</table>';

			$o->closeField();

			// end individual URL section
			$o->closeSection();

		}//end foreach

		// close the URL specific definitions
		$o->closeSection();

		$o->openSection(translate('User Defined Paint Layouts'));


		foreach ($lookups as $url) {

			$o->openSection($url);

			$o->openField(translate('Current'));

				if (in_array($url, $lookups_user)) {
					?>
					<table class="sq-backend-table">
						<tr>
							<th class="sq-backend-table-header" style="width:50%;"><?php echo 'Layout Code'; ?></th>
							<th class="sq-backend-table-header" style="width:50%;"><?php echo 'Paint Layout'; ?></th>
						</tr>
					<?php
					foreach ($lookup_values[$url] as $layout_name => $data) {
						$effective_layout_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($data['value']));
						$buttons = Array('reset');
						if (!$data['inhd']) $buttons[] = 'clear';
						?>
							<tr>
							<?php if ($wa) { ?>
								<td class="sq-backend-table-cell"><?php echo $data['inhd'] ? $data['code'] : text_box($prefix.'['.$url.']['.$layout_name.'][name]', $data['code'], 20, 30).self::_getSuffix($data['inhd'], TRUE); ?></td>
							<?php } else { ?>
								<td class="sq-backend-table-cell"><?php echo $data['code']; ?></td>
							<?php } ?>
							<?php if ($wa) { ?>
								<td class="sq-backend-table-cell"><?php echo $data['inhd'] ? get_asset_tag_line($data['value']).self::_getSuffix($data['inhd'], FALSE) : asset_finder($prefix.'['.$url.']['.$layout_name.'][value]', $data['value'], Array('paint_layout_page' => 'D'), 'sq_sidenav', FALSE, 'null', $buttons);?></td>
							<?php } else { ?>
								<td class="sq-backend-table-cell"><?php echo empty($effective_layout_info) ? translate('None') : get_asset_tag_line($data['value']).self::_getSuffix($data['inhd'], FALSE); ?></td>

							<?php } ?>
							</tr>
						<?php

					}//end foreach

					// end user defined layouts for this individual URL
					echo '</table>';
				} else {
					// no user defined layouts yet for this URL
					echo '<i>'.translate('None').'</i>';

				}
			$o->closeField();

			if ($wa) {
				$o->openSection(translate('Create New User Defined Paint Layout'));

				$o->openField(translate('Layout Code'));

					text_box($prefix.'['.$url.']['.$name_prefix.'::user::][name]', '', 20, 30);
				$o->openField(translate('Paint Layout'));

					asset_finder($prefix.'['.$url.']['.$name_prefix.'::user::][value]', 0, Array('paint_layout_page' => 'D'), 'sq_sidenav', FALSE, 'null', Array('clear'));
				$o->closeField();
				$o->closeSection();
			}

			// end individual URL section
			$o->closeSection();

		}//end foreach

		// close the URL specific definitions
		$o->closeSection();

		return $wa;

	}//end paintInterface()


	/**
	* Returns HTML used for indicating that a lookup value is inherited
	*
	* @param boolean	$inhd			whether this lookup value is inherited
	* @param boolean	$parentheses	whether to use parantheses or a hyphen
	*
	* @return string
	* @access private
	* @static
	*/
	private static function _getSuffix($inhd, $parentheses)
	{
		if ($parentheses) {
			return $inhd ? ' <strong style="font-size:11px"><i>('.translate('Inherited').')</i></strong>' : '';

		} else {
			return $inhd ? ' - <strong style="font-size:11px"><i>'.translate('Inherited').'</i></strong>' : '';

		}

	}//end _getSuffix()


	/**
	* Process the editing interface for this lookup value
	*
	* @param object	&$owner			the asset whose interface we are painting
	* @param object	&$o				the backend outputter object
	* @param array	$lookups		array of available lookups
	* @param array	&$lookup_values	array of the values that we want to save
	*								<pre>
	*									"[url]" => array (
	*										"[name]" => array (
	*											"value" => "[value]",
	*											"inhd" => "[inherited]",
	*										),
	*									),
	*								</pre>
	* @param string	$prefix			unique prefix to use for the form elements
	*
	* @return boolean
	* @access public
	* @static
	*/
	public static function processInterface(&$owner, &$o, $lookups, &$lookup_values, $prefix)
	{
		if (!isset($_POST[$prefix]) || !is_array($_POST[$prefix])) {
			return FALSE;
		}

		$lookup_values = Array();
		$name_prefix = Paint_Layout_Lookup_Value::namePrefix();

		foreach ($_POST[$prefix] as $resource => $layouts) {

			foreach ($layouts as $layout_name => $layout_data) {
				if ($layout_name === $name_prefix.'::user::') {
					// creating a new user defined layout
					if (empty($layout_data['name'])) continue;
					if (empty($layout_data['value']['assetid'])) continue;
					$layout_name .= strtolower($layout_data['name']);
					if (isset($lookup_values[$resource][$layout_name])) {
						trigger_error('Layout Name "'.$layout_data['name'].'" already in use for "'.$resource.'"', E_USER_WARNING);
						continue;
					}
				} else if (!is_bool(strstr($layout_name, $name_prefix.'::user::'))) {
					// updating an existing user defined layout
					// this is the new layout code
					$new_layout_name = $name_prefix.'::user::'.strtolower($layout_data['name']);

					// deal with deleted user defined layouts
					if (empty($layout_data['name'])) {
						$lookup_values[$resource][$layout_name]['value'] = '';
						continue;
					}

					// remove the previous one if it has changed
					if ($new_layout_name !== $layout_name) {
						if (isset($_POST[$prefix][$resource][$new_layout_name])) {
							trigger_error('Layout Name "'.$new_layout_name.'" already in use for "'.$resource.'"', E_USER_WARNING);
							continue;
						}
						$lookup_values[$resource][$layout_name]['value'] = '';
					}
					$layout_name = $new_layout_name;
				} else if (!is_bool(strstr($layout_name, $name_prefix.'::system::frontend'))) {
					// url layouts and override layouts fall in here

					if ($layout_data['value']['assetid'] === $layout_data['value']['original']) {
						// don't overwrite inherited system::frontend values unless there is a valid change
						continue;
					} else {
						// empty values come through because the asset finder is not pre-populated with values if an asset based layout is defined
						// if these empty values went through, it would result in updateLookups() being run unnecessarily
						// we need to make sure that values being deleted are not being caught here inadvertently as well
						if (preg_match('/.+\/$/', $resource)) {
							// override layout
							if (isset($_POST[$prefix]['asset']['override::paint_layout']['value']['assetid']) && (empty($layout_data['value']['assetid']))) {
								if ($layout_data['value']['original'] === $_POST[$prefix]['asset']['override::paint_layout']['value']['assetid']) {
									continue;
								}
							}
						} else {
							// normal layout
							if (isset($_POST[$prefix]['asset']['paint_layout']['value']['assetid']) && (empty($layout_data['value']['assetid']))) {
								if ($layout_data['value']['original'] === $_POST[$prefix]['asset']['paint_layout']['value']['assetid']) {
									continue;
								}
							}
						}

					}
				}

				if (empty($layout_data['value']['assetid'])) {
					// set to blank string to delete
					$lookup_values[$resource][$layout_name]['value'] = '';
				} else {
					// check to see if the provided assetid is a paint layout
					$layout = $GLOBALS['SQ_SYSTEM']->am->getAsset($layout_data['value']['assetid']);
					if (!is_null($layout) && $layout instanceof Paint_Layout_Page) {
						$lookup_values[$resource][$layout_name]['value'] = $layout_data['value']['assetid'];
					} else {
						trigger_error('Assetid provided, #'.$layout_data['value']['assetid'].', is not of asset type Paint Layout Page', E_USER_WARNING);
					}
				}

			}//end foreach
		}//end foreach

		return TRUE;

	}//end processInterface()


}//end class

?>
