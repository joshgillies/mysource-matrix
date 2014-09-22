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
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger_Action_Set_Metadata_Field_Date
*
* Sets the a metadata field date with the curernt date time plus a variable offset
*
* @author  Geoffroy Noel <gnoel@squiz.co.uk>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Metadata_Field_Date extends Trigger_Action
{

	/**
	* Execute this action
	* Set the metadata field date with the current date/time + offset
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
		$mm =& $GLOBALS['SQ_SYSTEM']->getMetadataManager();

		// check required settings
		if (empty($settings['fieldid'])) return FALSE;


		// If no context ID is set at ALL, then presume it's an old trigger, and
		// that we use the current context instead.
		if (array_key_exists('contextid', $settings) === FALSE) {
			$settings['contextid'] = NULL;
		}

		if ($settings['contextid'] === NULL) {
			$settings['contextid'] = $GLOBALS['SQ_SYSTEM']->getContextId();
		}

		$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
		if (array_key_exists($settings['contextid'], $all_contexts) === FALSE) {
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->changeContext($settings['contextid']);

		// Format: 2008-02-01 00:00:00
		$value = date('Y-m-d H:i:s', strtotime('+'.$settings['offset_months'].' month '.$settings['offset_days'].' day '.$settings['offset_hours'].' hour '.$settings['offset_mins'].' min'));

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				$GLOBALS['SQ_SYSTEM']->restoreContext();
				return FALSE;
			} else {
				$state['asset'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		// verify the fieldid
		$success = $mm->isMetadataFieldAssetid($settings['fieldid']);
		if (!$success) {
			$GLOBALS['SQ_SYSTEM']->restoreContext();
			return FALSE;
		}

		// get the asset
		$asset =& $state['asset'];

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
											0	=>	Array(
														'name'	=> $field_name,
														'value'	=> $value,
													)
										  );

		$success = $mm->setMetadata($asset->id, $metadata);

		// Regenerate Metadata File
		if ($success) {
			$field = $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['fieldid']);
			// If this date field is contextable, regenerate the metadata file in the current context only;
			// otherwise, regenerate all metadata files
			if ($field->attr('is_contextable')) {
				$mm->generateContentFile($asset->id);
			} else {
				$all_contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
				foreach ($all_contexts as $contextid => $context_data) {
					$mm->generateContentFile($asset->id, FALSE, $contextid);
				}
			}
		}

		$GLOBALS['SQ_SYSTEM']->restoreContext();

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
		$metadata_fieldid		= array_get_index($settings, 'fieldid', 0);
		$metadata_field_value	= array_get_index($settings, 'value', NULL);

		// asset finder for Metadata Field
		ob_start();
			echo '<div class="sq-break-line-inputs">';
			if ($write_access) {
				echo translate('Choose metadata field').': ';

				asset_finder($prefix.'[fieldid]', $metadata_fieldid, Array('metadata_field' => 'D'));
			} else {
				echo translate('Metadata field').': ';

				if (!empty($metadata_fieldid)) {
					echo get_asset_tag_line($metadata_fieldid);
				} else {
					echo '<em>'.translate('No metadata field selected').'</em>';

				}
			}
			echo '</div>';

			echo '<div class="sq-break-line-inputs">';
			$disabled = ($write_access ? '' : 'disabled=disabled');
			ob_start();
			text_box($prefix.'[offset_months]', array_get_index($settings, 'offset_months', 0), 5, 2, FALSE, $disabled);
			echo '|';
			text_box($prefix.'[offset_days]', array_get_index($settings, 'offset_days', 0), 5, 2, FALSE, $disabled);
			echo '|';
			text_box($prefix.'[offset_hours]', array_get_index($settings, 'offset_hours', 0), 5, 2, FALSE, $disabled);
			echo '|';
			text_box($prefix.'[offset_mins]', array_get_index($settings, 'offset_mins', 0), 5, 2, FALSE, $disabled);
			list($text_box_months, $text_box_days, $text_box_mins, $text_box_hours) = explode('|', ob_get_clean());
			echo sprintf(translate('Offset: %1$s month(s) %2$s days(s) %3$s hour(s) %4$s min(s)'), $text_box_months, $text_box_days, $text_box_mins, $text_box_hours);

			echo '</div>';
		$output = ob_get_clean();

		ob_start();
			echo '<div class="sq-break-line-inputs">';
			$current_context = array_get_index($settings, 'context_to_select', NULL);

			$contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
			$context_list = Array(
								'-default-' => translate('Use currently active context at time of firing'),

								'-sep-'		=> '----------------------',
							);
			foreach ($contexts as $context_item_id => $context_item) {
				if ((int)$current_context === (int)$context_item_id) {
					$context_name = $context_item['name'];
				}

				$context_list[rawurlencode($context_item['name'])] = $context_item['name'];
			}

			$default = Array('-default-');
			if (array_key_exists($current_context, $contexts) === TRUE) {
				$default = Array($current_context);
			}

			if ($metadata_fieldid) {
				label(translate('Set value in this context:'), $prefix.'[context_to_select]');

				echo ' ';
				if ($write_access) {
					combo_box($prefix.'[context_to_select]', $context_list, FALSE, $default);
				} else {
					if ($current_context === NULL) {
						echo '<strong>';
						echo translate('Use currently active context at time of firing');

						echo '</strong>';
					} else if (array_key_exists($current_context, $contexts) === TRUE) {
						echo '<strong>';
						echo $contexts[$current_context]['name'];
						echo '</strong>';
					} else {
						echo '<span class="sq-backend-warning">'.translate('Set context has been deleted. Trigger will fail until the context is changed to an existing one.').'</span>';

					}
				}
			}
			echo '</div>';
		$output_context_select = ob_get_clean();
		$output .= $output_context_select;

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
		$fieldid = $fieldid_data['assetid'];

		if ($fieldid) {
			$settings['fieldid'] = $fieldid;
			foreach (Array('offset_months', 'offset_days', 'offset_hours', 'offset_mins') as $field_name) {
				$settings[$field_name] = array_get_index($request_data, $field_name, '0');
			}

			$metadata_field =& $GLOBALS['SQ_SYSTEM']->am->getAsset($fieldid);

			// Can we edit the destination field? If we can't then what's the point;
			// we can't even change it through a trigger
			if (!$metadata_field->attr('editable')) {
				trigger_localised_error('CORE0246', sprintf(translate('Metadata field "%1$s" (Id: #%2$s) is marked as not editable; it cannot be set as the field to set'), $metadata_field->name, $metadata_field->id), E_USER_WARNING)
				return FALSE;
			}

			$settings['field_name']	= $metadata_field->name;

			$new_context = array_get_index($request_data, 'context_to_select', 0);
			if (is_numeric($new_context) === TRUE) {
				$new_context = (int)$new_context;
				$contexts = $GLOBALS['SQ_SYSTEM']->getAllContexts();
				if (array_key_exists($new_context, $contexts) === TRUE) {
					$settings['contextid'] = $new_context;
				} else {
					trigger_localised_error('TRIG0005', sprintf(translate('Unable to select context #%1$s for use in trigger, the context no longer exists'), $new_context), E_USER_WARNING)
				}
			} else if ($new_context === '-default-') {
				$settings['contextid'] = NULL;
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
