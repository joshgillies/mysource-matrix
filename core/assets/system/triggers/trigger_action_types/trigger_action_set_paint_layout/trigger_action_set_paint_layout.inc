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
* $Id: trigger_action_set_paint_layout.inc,v 1.6 2012/08/30 01:09:14 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';


/**
* Trigger Action Set Paint Layout
*
* A trigger action that sets the paint layout of an asset
*
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Paint_Layout extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:
	* <PRE>
	* Array(
	*		'layout_assetid'			=> int // the paint layout to apply as the asset defined layout
	*		'override_layout_assetid'	=> int // the paint layout to apply as the asset defined override layout
	*		);
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		if (empty($settings['layout_assetid']) && empty($settings['override_layout_assetid'])) {
			// if no settings, fail
			return FALSE;
		}

		// state
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		$am = $GLOBALS['SQ_SYSTEM']->am;

		if (isset($settings['layout_map'])) {
			// parameter map
			$parameter_map_value = array_get_index($settings, 'layout_map', serialize(Array()));
			$attr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
			$layouts['paint_layout::system::frontend'] = $attr_parameter_map->getParameterValue('parameter_map_layout_assetid');
			$layouts['override::paint_layout::system::frontend'] = $attr_parameter_map->getParameterValue('parameter_map_override_layout_assetid');
		}

		// if any parameter map values are present, they take priority
		$layouts['paint_layout::system::frontend'] = empty($layouts['paint_layout::system::frontend']) ? array_get_index($settings, 'layout_assetid', 0) : $layouts['paint_layout::system::frontend'];
		$layouts['override::paint_layout::system::frontend'] = empty($layouts['override::paint_layout::system::frontend']) ? array_get_index($settings, 'override_layout_assetid', 0) : $layouts['override::paint_layout::system::frontend'];

		if (!$state['asset']->setPaintLayouts($layouts)) {
			return FALSE;
		}

		return Array(
				'assetid'			=> $state['asset']->id,
				'layout'			=> isset($settings['layout_assetid']) ? $settings['layout_assetid'] : '',
				'override_layout'	=> isset($settings['override_layout_assetid']) ? $settings['override_layout_assetid'] : '',
			   );

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
		$selected_layout_assetid = empty($settings['layout_assetid']) ? 0 : $settings['layout_assetid'];
		$selected_override_layout_assetid = empty($settings['override_layout_assetid']) ? 0 : $settings['override_layout_assetid'];

		// check if the selected layouts are valid
		if ($selected_layout_assetid && !$GLOBALS['SQ_SYSTEM']->am->assetExists($selected_layout_assetid)) {
			$selected_layout_assetid = 0;
		}
		if ($selected_override_layout_assetid && !$GLOBALS['SQ_SYSTEM']->am->assetExists($selected_override_layout_assetid)) {
			$selected_override_layout_assetid = 0;
		}

		// attribute friendly prefix
		$new_prefix = str_replace(array('[',']'), '_', $prefix);
		hidden_field($prefix.'[new_prefix]', $new_prefix);

		ob_start();
			if (!$write_access) {
				$layout_wa = !empty($selected_layout_assetid) ? get_asset_tag_line($selected_layout_assetid) : '<i>'.translate('None').'</i>';

				$override_layout_wa = !empty($selected_override_layout_assetid) ? get_asset_tag_line($selected_override_layout_assetid) : '<i>'.translate('None').'</i>';

			}

			$layout_buttons = empty($selected_layout_assetid) ? Array('clear') : Array('clear', 'reset');
			$override_layout_buttons = empty($selected_override_layout_assetid) ? Array('clear') : Array('clear', 'reset');

			?>
				<table class="sq-backend-table with-bottom-margin">
					<tr>
						<th><?php echo translate('Set the asset\'s paint layout to:'); ?> </th>

						<td><?php echo $write_access ? asset_finder($prefix.'[layout_assetid]', $selected_layout_assetid, Array('paint_layout_page' => 'D'), 'sq_sidenav', FALSE, 'null', $layout_buttons) : $layout_wa; ?> </td>
					</tr>
					<tr>
						<th><?php echo translate('Set the asset\'s override paint layout to:'); ?></th>

						<td><?php echo $write_access ? asset_finder($prefix.'[override_layout_assetid]', $selected_override_layout_assetid, Array('paint_layout_page' => 'D'), 'sq_sidenav', FALSE, 'null', $override_layout_buttons) : $override_layout_wa; ?> </td>
					</tr>
				</table>
			<?php

		$basic_contents = ob_get_clean();

		// parameter map options
		ob_start();
			$parameter_map_value = array_get_index($settings, 'layout_map', serialize(Array()));
			$attr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
			$attr_parameter_map->setParameter('parameter_map_layout_assetid', 'Paint Layout Asset ID');
			$attr_parameter_map->setParameter('parameter_map_override_layout_assetid', 'Override Paint Layout Asset ID');
			echo $attr_parameter_map->paint($new_prefix.'_parameter_map', !$write_access);
		$parameter_map_contents = ob_get_clean();

		return $basic_contents.$parameter_map_contents;

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action
	*								as specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		if (empty($request_data['layout_assetid']['assetid']) && empty($request_data['override_layout_assetid']['assetid'])) {
			return 'Paint layout not specified';
		}

		$settings['layout_assetid'] = $request_data['layout_assetid']['assetid'];
		$settings['override_layout_assetid'] = $request_data['override_layout_assetid']['assetid'];

		$new_prefix = str_replace(array('[',']'), '_', array_get_index($request_data, 'new_prefix', ''));
		$attr_parameter_map = new Asset_Attribute_Parameter_Map();
		$attr_parameter_map->process($new_prefix.'_parameter_map');
		$settings['layout_map'] = $attr_parameter_map->value;

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs to acquire before executing
	*
	* @param array	$settings	a container for any data the action might want to save
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	public static function getLocks($settings, &$state)
	{
		return Array(
				$state['assetid']	=> Array(
										'links',
										'lookups',
									   ),
			   );

	}//end getLocks()


}//end class

?>
