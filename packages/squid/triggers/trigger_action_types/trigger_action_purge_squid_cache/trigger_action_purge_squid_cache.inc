<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: trigger_action_purge_squid_cache.inc,v 1.10 2013/08/08 08:19:44 cupreti Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger Action Purge Squid Cache
*
* A trigger action that clears the Squid Cache of an asset
*
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage squid
*/
class Trigger_Action_Purge_Squid_Cache extends Trigger_Action
{


	/**
	* Constructor
	*
	* Added for consistency. However, this is only ever called statically,
	* so the constructor will never actually be called.
	*
	* @param string	$assetid	The ID of this asset
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Execute this action
	*
	* The settings used by this action are in the form:
	* <pre>
	* Array(
	*		'prefix'			=> string	// the minged prefix string
	*		'use_current_asset'	=> boolean	// whether to include current asset
	* 		'selected_assets'	=> array	// array of the selected assets
	* 		'cascade'			=> string	// whether to cacase to children/dependents
	*		'asset_types'		=> array	// array of asset types
	* 		);
	* </pre>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) return FALSE;

		// For backward compatibility
		if ($settings['cascade'] === TRUE) {
			$settings['cascade'] = 'children';
		} else if ($settings['cascade'] === FALSE) {
			$settings['cascade'] = 'single';
		}

		$todo_assetids = $settings['selected_assets'];
		if (!isset($settings['use_current_asset']) || $settings['use_current_asset']) {
			$todo_assetids[] = $state['asset']->id;
		}
		// tricky: empty string means include all asset types
		$type_code = empty($settings['asset_types']) ? '' : $settings['asset_types'];

		$vars = Array(
					'assetid'		=> $todo_assetids,
					'cascade'		=> $settings['cascade'],
					'type_codes'	=> $type_code,
					'repopulate'	=> FALSE,
				);

		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();

		$status_errors = $hh->freestyleHipo('hipo_job_tool_clear_squid_cache', $vars, SQ_PACKAGES_PATH.'/squid/hipo_jobs');
		if (!empty($status_errors)) {
			return FALSE;
		}

		return $vars;

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		// munge the prefix so that we can use the Cache Manager interfaces to paint/process
		$new_prefix = str_replace('[', '_', $prefix);
		$new_prefix = str_replace(']', '', $new_prefix);

		// level options
		$level_options = Array(
							'single'		=> translate('selected_asset'),
							'dependants'	=> translate('selected_asset_and_dependants'),
							'children'		=> translate('selected_asset_and_children'),
						 );

		// asset types options
		$asset_types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
		foreach ($asset_types as $type_code) {
			$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
			if ($info['allowed_access'] != 'system' && $info['instantiable']) {
				$asset_types_options[$type_code] = $info['name'];
			}
		}
		asort($asset_types_options);
		$asset_types_options = array_reverse($asset_types_options, TRUE);
		$asset_types_options[''] = '-- '.translate('all_asset_types').' --';
		$asset_types_options = array_reverse($asset_types_options, TRUE);

		// initialise settings var
		$settings['use_current_asset'] = isset($settings['use_current_asset']) ? $settings['use_current_asset'] : TRUE;
		$settings['selected_assets'] = isset($settings['selected_assets']) ? $settings['selected_assets'] : Array();
		$settings['cascade'] = isset($settings['cascade']) ? $settings['cascade'] : 'single';
		// For backward compatibility
		if ($settings['cascade'] === TRUE) {
			$settings['cascade'] = 'children';
		} else if ($settings['cascade'] === FALSE) {
			$settings['cascade'] = 'single';
		}
		$settings['asset_types'] = isset($settings['asset_types']) ? $settings['asset_types'] : Array();
		if (empty($settings['asset_types'])) {
			$settings['asset_types'] = Array(0 => '');
		}

		// paint the edit interface
		ob_start();
		?>
		<table border="0">
			<tr>
				<td valign="top"><b><?php echo translate('trigger_clear_cache_current_asset'); ?></b></td>
				<td valign="top" style="padding-bottom: 10px;"><?php
					// current asset
					if ($write_access) {
						echo check_box($prefix.'[use_current_asset]', 1, $settings['use_current_asset']);
						echo translate('trigger_clear_cache_include_current_asset');
					} else {
						echo '&nbsp; &nbsp;';
						echo ($settings['use_current_asset']) ? translate('yes') : translate('no');
					}
				?>
				</td>
			</tr>
			<tr><?php
					// selected assets/choose assets
					if ($write_access) {
						?>
						<td valign="top"><b><?php echo translate('trigger_clear_cache_choose_asset'); ?></b></td>
						<td valign="top" style="padding-bottom: 10px;"><?php
							echo multiple_asset_finder($new_prefix.'_select_assets', $settings['selected_assets']);
					} else {
						?>
						<td valign="top"><b><?php echo translate('trigger_clear_cache_choose_asset_selected'); ?></b></td>
						<td valign="top" style="padding-bottom: 10px;"><?php
							if (empty($settings['selected_assets'])) {
								echo '&nbsp; &nbsp;'.translate('no_asset_selected');
							} else {
								?><ul><?php
								foreach ($settings['selected_assets'] as $selected_asset) {
									?><li><?php
									echo get_asset_tag_line($selected_asset);
									?></li><?php
								}
								?></ul><?php
							}
					}
				?>
				</td>
			</tr>
			<tr>
				<td valign="top"><b><?php echo translate('cascade'); ?></b></td>
				<td valign="top" style="padding-bottom: 10px;"><?php
					// level
					if ($write_access) {
						echo combo_box($new_prefix.'_level', $level_options, FALSE, $settings['cascade'], 0);
					} else {
						echo '&nbsp; &nbsp;'.$level_options[$settings['cascade']];
					}
				?>
				</td>
			</tr>
			<tr><?php
					// selected asset types/asset types
					if ($write_access) {
						?>
						<td valign="top"><b><?php echo translate('asset_types'); ?></b></td>
						<td valign="top" style="padding-bottom: 10px;"><?php echo combo_box($new_prefix.'_asset_types', $asset_types_options, TRUE, $settings['asset_types'], 8); ?></td>
						<?php
					} else {
						?>
						<td valign="top"><b><?php echo translate('trigger_clear_cache_asset_types_selected'); ?></b></td>
						<td valign="top" style="padding-bottom: 10px;"><?php
							if (!empty($settings['asset_types'][0])) {
								?><ul><?php
								foreach ($settings['asset_types'] as $type_code) {
									?><li><?php
									echo get_asset_type_icon($type_code);
									echo $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code, 'name');
									?></li><?php
								}
								?></ul><?php
							} else {
								echo '&nbsp; &nbsp;'.translate('all_asset_types');
							}
						?>
						</td><?php
					}
			?>
			</tr>
			<tr><td colspan="2"><?php echo translate('squid_tool_hostname_note'); ?></td></tr>
		</table>
		<?php
		hidden_field($prefix.'[prefix]', $new_prefix);

		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as
	*								specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		$new_prefix = $request_data['prefix'];
		$settings['prefix'] = $new_prefix;

		// we will combine current asset and selected assets into one todo list
		$settings['use_current_asset'] = array_get_index($request_data, 'use_current_asset', FALSE);
		$settings['selected_assets'] = Array();
		if (isset($_POST[$new_prefix.'_select_assets']) && !empty($_POST[$new_prefix.'_asset_types'])) {
			foreach ($_POST[$new_prefix.'_select_assets'] as $key => $array) {
				// check for fields with no value
				if ($array['assetid'] != 0) {
					$settings['selected_assets'][] = $array['assetid'];
				}
			}
		}

		// level
		$settings['cascade'] = 'single';
		if (isset($_POST[$new_prefix.'_level'])) {
			$settings['cascade'] = $_POST[$new_prefix.'_level'];
		}

		// selected asset types
		if (isset($_POST[$new_prefix.'_asset_types']) && !empty($_POST[$new_prefix.'_asset_types'][0])) {
			$settings['asset_types'] = $_POST[$new_prefix.'_asset_types'];
		} else {
			$settings['asset_types'] = Array();
		}

		return FALSE;

	}//end processInterface()


}//end class

?>
