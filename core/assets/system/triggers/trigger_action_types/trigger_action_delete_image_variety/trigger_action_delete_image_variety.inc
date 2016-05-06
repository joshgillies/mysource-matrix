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
* $Id: trigger_action_delete_image_variety.inc,v 1.3 2012/10/02 06:19:40 akarelia Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';

/**
* Delete Image Variety Trigger Action
*
* A trigger action that will take into consideration an an Image asset (passed via interface or
* the firing asset) and delete either a variety mentioned or all the variety under the image
*
*
*
* @author  Ash Karelia<akarelia@squiz.com.au>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Delete_Image_Variety extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		// check if we have a image asset to look under
		if ($settings['image_assetid'] == '0' && !$settings['use_current']) return FALSE;

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) return FALSE;

		$old_current_asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
		$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $state['asset']);

		$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));
		$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
		$param_map_value = $atr_parameter_map->getParameterValue('image_var');

		if (is_null($old_current_asset)) {
			$GLOBALS['SQ_SYSTEM']->unsetGlobalDefine('CURRENT_ASSET');
		} else {
			$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $old_current_asset);
		}

		// check if we have one of the variable set to point us tot he variety to delete
		if (empty($param_map_value) && empty($settings['variety_name']) && !$settings['delete_all']) return FALSE;

		// if we are asked to use current asset then over write the asset selected via asset picker
		if ($settings['use_current']) {
			$settings['image_assetid'] = $state['asset']->id;
			$image_to_use = $state['asset'];
		} else {
			//get the asset
			$image_to_use = $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['image_assetid']);
		}

		// make sure the asset exists
		if (!$GLOBALS['SQ_SYSTEM']->am->assetExists($image_to_use->id)) return FALSE;

		// get all the varieties from the image
		$varieties = $image_to_use->attr('varieties');

		// dont have any variety to delete?
		if (empty($varieties)) return FALSE;

		// if deleting all thats it, over writes everythng else
		$variety_deleted = Array();
		if ($settings['delete_all']) {
			foreach ($varieties['data'] as $varid => $data) {
				if (!$image_to_use->deleteVariety($image_to_use->id.':'.$varid)) return FALSE;
				$variety_deleted[] = $image_to_use->id.':'.$varid;
			}// end if
		} else {
			// if we have a name configured to delete but also have a value from parameter
			// map, we go with parameter map value
			if (!empty($param_map_value)) $settings['variety_name'] = $param_map_value;

			$match_found = FALSE;
			foreach ($varieties['data'] as $varid => $data) {
				if ($data['name'] == $settings['variety_name']) {
					$match_found = TRUE;
					if (!$image_to_use->deleteVariety($image_to_use->id.':'.$varid)) return FALSE;
					$variety_deleted[] = $image_to_use->id.':'.$varid;
				}// end if
			}// end foreach
		}// end if/else

		$return_array = Array('majorid' => $image_to_use->id);
		foreach ($variety_deleted as $var) {
			$return_array['variety_deleted'][] = $var;
		}

		return $return_array;



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
		// check settings, set defaults if necessary
		$selected_image	= array_get_index($settings, 'image_assetid', 0);
		$variety_name	= array_get_index($settings, 'variety_name', '');
		$delete_all		= array_get_index($settings, 'delete_all', FALSE);
		$use_current	= array_get_index($settings, 'use_current', FALSE);

		ob_start();
		?>
			<table class="sq-backend-table with-bottom-margin">
				<tr>
					<th><?php echo translate('Image to look under') ?></th>

					<td>
					<?php
						if ($write_access) {
							asset_finder($prefix.'[image_assetid]', $selected_image, Array('image' => 'I'));
						} else {
							if ($selected_image != 0) {
								echo get_asset_tag_line($selected_image);
							} else {
								echo translate('No Image Selected');

							}
						}
					?>
					</td>
				</tr>
				<tr>
					<td colspan="2"><?php echo"<b><i>OR</i></b>"; ?></td>
				</tr>
				<tr>
					<th><?php echo translate('Use Image Asset firing the Trigger')  ?></th>

					<td>
					<?php
						if ($write_access) {
							check_box($prefix.'[use_current]', '1', $use_current, '', ($write_access ? '' : 'disabled="disabled"'));
						} else {
							echo '<img src="'.sq_web_path('lib').'/web/images/'.($use_current ? 'tick' : 'cross').'.png" alt="'.($use_current ? translate('Yes') : translate('No')).'" /> ';


						}
						echo translate('<br/><strong>Note:</strong> If this is checked, it will overwrite the Image asset selected by asset picker.');

					?>
				</tr>
				<tr>
					<th><?php echo translate('Variety name to delete'); ?></th>

					<td>
					<?php
						if ($write_access) {
							text_box($prefix.'[variety_name]', $variety_name, 25);
						} else {
							echo $variety_name;
						}
					?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('Delete all varities'); ?></th>

					<td>
					<?php
						if ($write_access) {
							check_box($prefix.'[delete_all]', '1', $delete_all, '', ($write_access ? '' : 'disabled="disabled"'));
						} else {
							echo '<img src="'.sq_web_path('lib').'/web/images/'.($delete_all ? 'tick' : 'cross').'.png" alt="'.($delete_all ? translate('Yes') : translate('No')).'" /> ';


						}
						echo translate('<br/><strong>Note:</strong> If this is checked, it will overwrite Image variety name to delete as well as any value passed via parameter map.');

					?>
					</td>
				</tr>
			</table>

		<?php
		$new_prefix = str_replace(array('[',']'), '_', $prefix);
		hidden_field($prefix.'[new_prefix]', $new_prefix);

		// print the parameter map
		$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));

		$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);

		$atr_parameter_map->setParameter('image_var', 'Image Variety');
		echo $atr_parameter_map->paint($new_prefix.'_parameter_map', !$write_access);


		$contents = ob_get_clean();
		return $contents;

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as specified
	*								by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		$settings['image_assetid']	= $request_data['image_assetid']['assetid'];
		$settings['variety_name']	= array_get_index($request_data, 'variety_name', '');
		$settings['delete_all']		= array_get_index($request_data, 'delete_all', FALSE);
		$settings['use_current']	= array_get_index($request_data, 'use_current', FALSE);

		// process parameter map
		$new_prefix = str_replace(array('[',']'), '_', array_get_index($request_data, 'new_prefix', ''));
		$atr_parameter_map = new Asset_Attribute_Parameter_Map();
		$atr_parameter_map->process($new_prefix.'_parameter_map');

		$settings['value_map'] = $atr_parameter_map->value;


		return FALSE;

	}//end processInterface()


}//end class

?>
