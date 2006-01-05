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
* $Id: cache_manager_edit_fns.inc,v 1.5 2006/01/05 23:33:21 rong Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_INCLUDE_PATH.'/asset_edit_interface.inc';

/**
* Cache_Manager_Edit_Fns
*
* Edit functions for the Cache manager.
*
* @see Asset_Edit_Fns
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @author  Greg Sherwood <gsherwood@squiz.net>
* @version $Revision: 1.5 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackag __core__
*/
class Cache_Manager_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::Asset_Edit_Fns
	*/
	function Cache_Manager_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


	/**
	* Paints the interface for selecting an asset type to customise
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintChooseTypeCode(&$asset, &$o, $prefix)
	{
		$type_codes = $asset->attr('type_codes');
		$asset_types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();

		foreach ($asset_types as $type_code) {
			$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
			$options[$type_code] = $info['name'];
		}
		asort($options);

		// we dont use the combo_box function here as we are going
		// to colourise the options that have been customised

		if (isset($_REQUEST[$prefix.'_select_type'])) {
			$selected = $_REQUEST[$prefix.'_select_type'];
		} else {
			$selected = (isset($_REQUEST[$prefix.'_type'])) ? $_REQUEST[$prefix.'_type'] : '';
		}

		?>
		<select name="<?php echo $prefix; ?>_select_type" onchange="this.form.submit();">
			<option value="">-- <?php echo translate('select_asset_type'); ?> --</option>
			<?php
			foreach ($options as $type_code => $name) {
				$style = (isset($type_codes[$type_code])) ? ' style="background-color: #C8BDCB";' : '';
				?>
				<option<?php echo $style; ?> value="<?php echo $type_code; ?>"<?php echo ($selected == $type_code) ? ' selected' : ''; ?>>
					<?php echo $name; ?>
				</option>
				<?php
			}
		?></select><?php

		$o->addFormActionGetVar($prefix.'_type', $selected, TRUE);
		submit_button($prefix.'_go', translate('go'));

		if (isset($_REQUEST[$prefix.'_select_type']) && !empty($_REQUEST[$prefix.'_select_type'])) {
			echo '<br/><br/><b>'.translate('currently_customising', $options[$_REQUEST[$prefix.'_select_type']]).'</b>';
		}

		return FALSE;

	}//end paintChooseTypeCode()


	/**
	* Processes the interface for selecting an asset type to customise
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processChooseTypeCode(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processChooseTypeCode()


	/**
	* Paints the interface for customising an individual asset type's cache options
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintCustomiseTypeCodeCaching(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if (isset($_REQUEST[$prefix.'_select_type'])) {
			$selected = $_REQUEST[$prefix.'_select_type'];
		} else {
			$selected = (isset($_REQUEST[$prefix.'_type'])) ? $_REQUEST[$prefix.'_type'] : '';
		}

		if (!empty($selected)) {

			$type_codes = $asset->attr('type_codes');

			// field opened by asset_edit_interface
				$using_default = !isset($type_codes[$selected]['enabled']);
				if ($using_default) {
					echo translate('using_default_value_of',(($asset->attr('enabled')) ? translate('on') : translate('off'))).'<br/>';
				} else {
					$caching = ($type_codes[$selected]['enabled']) ? 1 : 0;
					if ($write_access) {
						combo_box($prefix.'_enabled', Array('1' => translate('on'), '0' => translate('off')), FALSE, $caching);
					} else {
						echo ($type_codes[$selected]['enabled']) ? translate('on') : translate('off');
					}
				}
				if ($write_access) {
					check_box($prefix.'_default_enabled', '1', $using_default);
					echo ' '.translate('use_default');
				}
			$o->closeField();

			$o->openField(translate('default_expiry'), '', translate('valid_cache_length'));
				$using_default = !isset($type_codes[$selected]['expiry']);
				if ($using_default) {
					echo 'Using default value of '.$asset->attr('expiry').' seconds<br/>';
				} else {
					if ($write_access) {
						text_box($prefix.'_expiry', $type_codes[$selected]['expiry'], 5);
					} else {
						echo $type_codes[$selected]['expiry'];
					}
				}
				if ($write_access) {
					check_box($prefix.'_default_expiry', '1', $using_default);
					echo ' '.translate('use_default');
				}
			// field closed by asset_edit_interface

		} else {
			echo translate('customise_asset_selection');
			return FALSE;

		}//end if !empty()

		return $write_access;

	}//end paintCustomiseTypeCodeCaching()


	/**
	* Processes the interface for customising an individual asset type's cache options
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processCustomiseTypeCodeCaching(&$asset, &$o, $prefix)
	{
		$type_codes = $asset->attr('type_codes');

		if (isset($_REQUEST[$prefix.'_type'])) {
			$type = $_REQUEST[$prefix.'_type'];
			$defaults = Array('enabled', 'expiry');
			$save = FALSE;

			foreach ($defaults as $default) {
				$using_default = FALSE;
				if (!isset($_POST[$prefix.'_default_'.$default])) {
					if (!isset($type_codes[$type][$default])) {
						$save = TRUE;
						$type_codes[$type][$default] = $asset->attr($default);
					}
				} else {
					$save = TRUE;
					$using_default = TRUE;
					if (isset($type_codes[$type][$default])) {
						unset($type_codes[$type][$default]);
					}
				}
				if (isset($_POST[$prefix.'_'.$default]) && !$using_default) {
					if (!isset($type_codes[$type][$default]) || $type_codes[$type][$default] != $_POST[$prefix.'_'.$default]) {
						$type_codes[$type][$default] = $_POST[$prefix.'_'.$default];
						$save = TRUE;
					}
				}
			}
			if (isset($type_codes[$type]) && empty($type_codes[$type])) {
				unset($type_codes[$type]);
			}
			return ($save) ? $asset->setAttrValue('type_codes', $type_codes) : FALSE;
		}
		return FALSE;

	}//end processCustomiseTypeCodeCaching()


	/**
	* Paints the interface for clearing the cache files
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintClearCache(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			echo translate('cannot_clear_cache');
			return FALSE;
		}
		asset_finder($prefix.'_clear_cache', '');

		$o->openField(translate('level'));
			$options = Array(
						'single'		=> translate('this_asset'),
						'dependants'	=> translate('this_asset_and_dependants'),
						'children'		=> translate('this_asset_and_children'),
					   );
			combo_box($prefix.'_level', $options, '', FALSE);
		$o->closeField();

		$o->openField(translate('asset_types'));
			$asset_types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
			foreach ($asset_types as $type_code) {
				$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
				if ($info['allowed_access'] != 'system' && $info['instantiable']) {
					$options[$type_code] = $info['name'];
				}
			}

			asort($options);
			$options = array_reverse($options, TRUE);
			$options[''] = '-- '.translate('all_asset_types').' --';
			$options = array_reverse($options, TRUE);
			combo_box($prefix.'_type_codes', $options, TRUE, '', 8);
		$o->closeField();

		return TRUE;

	}//end paintClearCache()


	/**
	* Processes the interface for clearing the cache files, and starts the clear cache hipo
	*
	* @param object	&$asset	the asset of the layout manager
	* @param object	&$o		the backend outputter
	* @param string	$prefix	a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processClearCache(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_clear_cache']) && !empty($_POST[$prefix.'_clear_cache']['assetid'])) {
			$level = (isset($_POST[$prefix.'_level'])) ? $_POST[$prefix.'_level'] : 'single';

			if (isset($_POST[$prefix.'_type_codes']) && !empty($_POST[$prefix.'_type_codes'][0])) {
				$type_codes = $_POST[$prefix.'_type_codes'];
			} else {
				$type_codes = Array();
			}

			$assetid = $_POST[$prefix.'_clear_cache']['assetid'];
			$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();

			$vars = Array(
						'assetid'		=> $assetid,
						'level'			=> $level,
						'type_codes'	=> $type_codes,
					);
			$hh->queueHipo('hipo_job_clear_cache', $vars);

			return TRUE;
		}

	}//end processClearCache()


}//end class

?>