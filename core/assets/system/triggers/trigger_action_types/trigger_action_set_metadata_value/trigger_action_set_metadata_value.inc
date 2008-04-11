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
* $Id: trigger_action_set_metadata_value.inc,v 1.27 2008/04/11 03:24:48 ncallahan Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';

/**
* Trigger_Action_Set_Metadata_Value
*
* A trigger action that applies or denies a metadata value
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.27 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Metadata_Value extends Trigger_Action
{


	/**
	* Execute this action
	*
	* Returns an array of data about what it did, or false on error
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       'fieldid'		=> int    // metadata_field id
	*		'field_name'	=> string // name of the field
	*       'value'			=> string // the value to set
	*       );
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
		$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		// check required settings
		if (empty($settings['fieldid'])) return FALSE;
		// null value indicates default
		if (empty($settings['value']) && !is_null($settings['value'])) {
			return FALSE;
		}


		$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));
		$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
		$param_map_value = $atr_parameter_map->getParameterValue('metadata_value');

		// settings contains the 'fallback' static value.
		if (!empty($param_map_value)) {
			$value = $param_map_value;
		} else {
			$value = $settings['value'];
		}

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		// verify the fieldid
		$success = $mm->isMetadataFieldAssetid($settings['fieldid']);
		if (!$success) return FALSE;

		// get the asset
		$asset = $state['asset'];

		// get the current metadata values
		$metadata = $mm->getMetadata($asset->id);

		// get the field name
		$field_name = '';
		if (isset($metadata[$settings['fieldid']])) {
			$field_name = $metadata[$settings['fieldid']][0]['name'];
		}

		// Sanitise the value for equal signs (=)
		// As per Bug #2951 HTML Submitted becomes malformed due to a simple equals sign
		$value = str_replace('=', '\=', $value);

		// set the new value
		$metadata[$settings['fieldid']] = Array(
											0	=> Array (
													'name'	=> $field_name,
													'value'	=> $value,
												   ),
										  );
		$success = $mm->setMetadata($asset->id, $metadata);

		// Regenerate Metadata File
		if ($success) $mm->regenerateMetadata($asset->id);

		// all done very smoothly like an ice cream
		if ($success) {
			return Array(
					'fieldid'		=> $settings['fieldid'],
					'field_name'	=> $field_name,
					'value'			=> $value,
				   );
		} else {
			return FALSE;
		}

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
		$output					= '';
		$metadata_fieldid		= array_get_index($settings, 'fieldid', 0);
		$metadata_field_value	= array_get_index($settings, 'value', NULL);
		$metadata_field         = NULL;

		if ($metadata_fieldid) {
		    $metadata_field = $GLOBALS['SQ_SYSTEM']->am->getAsset($metadata_fieldid);
		}
		// asset finder for Metadata Field
		ob_start();
			if ($write_access) {
				echo 'Choose Metadata Field&nbsp;&nbsp;&nbsp;';
				asset_finder($prefix.'[fieldid]', $metadata_fieldid, Array('metadata_field' => 'D'));
			} else {
			    if ($metadata_fieldid  && $metadata_field) {
				    echo get_asset_tag_line($metadata_fieldid);
				} else if ($metadata_fieldid  && !$metadata_field) {
				    echo '<span class="sq-backend-warning">Unknown asset (Id: #'.$metadata_fieldid.')</span>';
			    } else {
				    echo '<b>No metadata field specified</b>';
				}
			}
			$output_part1 = ob_get_contents();
		ob_end_clean();
		$output .= $output_part1;

		// Input field for the value
		ob_start();
			if ($write_access && $metadata_field) {
				echo '<br>Set the value for this field<br>';
				// set the field name
				$edit_fns = $metadata_field->getEditFns();
				$edit_fns->paintValueInterface($metadata_field, $GLOBALS['SQ_SYSTEM']->backend->out, $metadata_field_value, TRUE, TRUE, FALSE);
				hidden_field($prefix.'[form_printed]', 1);

				// attribute friendly prefix
				$new_prefix = ereg_replace('\[', '_', ereg_replace('\]', '_', $prefix));
				hidden_field($prefix.'[new_prefix]', $new_prefix);

				$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));
				$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);

				$atr_parameter_map->setParameter('metadata_value', 'Metadata Field Value');
				echo $atr_parameter_map->paint($new_prefix.'_parameter_map');

			}
			$output_part2 = ob_get_contents();
		ob_end_clean();
		$output .= $output_part2;

		return $output;

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
		// process fieldid
		$fieldid_data = array_get_index($request_data, 'fieldid', 0);
		$form_printed = array_get_index($request_data, 'form_printed', 0);

		$fieldid = $fieldid_data['assetid'];

		if ($fieldid) {
			$settings['fieldid'] = $fieldid;

			$metadata_field = $GLOBALS['SQ_SYSTEM']->am->getAsset($fieldid);

			// Can we edit the destination field? If we can't then what's the point;
			// we can't even change it through a trigger
			if (!$metadata_field->attr('editable')) {
				trigger_localised_error('CORE0246', E_USER_WARNING, $metadata_field->name, $metadata_field->id);
				return FALSE;
			}

			$edit_fns = $metadata_field->getEditFns();

			$result = '';
			if ($form_printed) {
				$field_cascade_value=FALSE;
				$edit_fns->processInlineValueInterface($metadata_field, $result, $field_cascade_value);
			}

			$settings['field_name']	= $metadata_field->name;
			$settings['value']		= $result;

			$new_prefix = ereg_replace('\[', '_', ereg_replace('\]', '_', array_get_index($request_data, 'new_prefix', '')));
			if ($new_prefix) {
				$atr_parameter_map = new Asset_Attribute_Parameter_Map();
				$atr_parameter_map->process($new_prefix.'_parameter_map');
				$settings['value_map'] = $atr_parameter_map->value;
			}
		}

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs acquire before executing
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	public static function getLocks($settings, &$state)
	{
		return Array($state['assetid'] => Array('all'));

	}//end getLocks()


}//end class

?>
