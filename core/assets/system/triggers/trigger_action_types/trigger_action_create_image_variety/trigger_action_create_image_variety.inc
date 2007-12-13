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
* $Id: trigger_action_create_image_variety.inc,v 1.20 2007/12/13 02:52:10 rong Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Trigger Action Create Image Variety
*
* A trigger action that creates an resizing image variety
*
*
* @author   Andrei Railean <arailean@squiz.net>
* @author   Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.20 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Create_Image_Variety extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:
	* <PRE>
	* Array(
	*       'name'      => string, // name of the image variety
	*       'constrain' => string, // dimension to constrain by
	*       'width'     => int,    // may be zero, see action description
	*       'height'    => int     // may be zero, see action description
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
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		// asset must be image
		if (!($state['asset'] instanceof Image)) {
			trigger_error('Unable to create image variety for a '.$state['asset']->type().' asset', E_USER_NOTICE);
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('image_variety');

		$variety = new Image_Variety();
		$variety->setAttrValue('variety_type', 'resize');

		// set & check settings, state

		// name
		if (!@$variety->setAttrValue('name', $settings['name'])) {
			return FALSE;
		}

		// constrain
		if (!@$variety->setAttrValue('constrain', $settings['constrain'])) {
			return FALSE;
		}

		// width
		if (!@$variety->setAttrValue('width', $settings['width'])) {
			return FALSE;
		}

		// height
		if (!@$variety->setAttrValue('height', $settings['height'])) {
			return FALSE;
		}

		// rely on image_variety to check whether the variety already exists
		$vid = $state['asset']->saveVariety($variety);

		return Array(
				'imageid'	=> $state['asset']->id,
				'varietyid'	=> $state['asset']->id.':'.$vid,
				'name'		=> $variety->name,
				'width'		=> $variety->attr('width'),
				'height'	=> $variety->attr('height'),
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
		// set defaults
		$settings['name']      = array_get_index($settings, 'name', '');
		$settings['constrain'] = array_get_index($settings, 'constrain', 'width');
		$settings['width']     = array_get_index($settings, 'width', 0);
		$settings['height']    = array_get_index($settings, 'height', 0);


		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		// begin buffering basic options
		ob_start();
			text_box($prefix.'[name]', $settings['name'], '', '', FALSE, $form_element_extras);
			$basic_part_1 = ob_get_contents();
		ob_end_clean();

		ob_start();
			$options = Array(
						'width'		=> 'Width',
						'height'	=> 'Height',
						'dimension' => 'Both',
						'none'		=> 'Neither',
					   );
			combo_box($prefix.'[constrain]', $options, FALSE, $settings['constrain'], 0, $form_element_extras);
			$basic_part_2 = ob_get_contents();
		ob_end_clean();

		ob_start();
			int_text_box($prefix.'[width]', $settings['width'], FALSE, '', NULL, NULL, '', FALSE, FALSE, $form_element_extras);
			$basic_part_3 = ob_get_contents();
		ob_end_clean();

		ob_start();
			int_text_box($prefix.'[height]', $settings['height'], FALSE, '', NULL, NULL, '', FALSE, FALSE, $form_element_extras);
			$basic_part_4 = ob_get_contents();
		ob_end_clean();

		return translate('trigger_create_image_variety_basic', $basic_part_1, $basic_part_2, $basic_part_3, $basic_part_4);

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action
	*								as specified by the $prefix in getInterface
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		if (empty($request_data)) {
			return translate('trigger_input_data_missing');
		}

		// check if the name, constrain, width and height are set
		if (empty($request_data['name'])) {
			return translate('trigger_variety_name_cannot_be_empty');
		}

		if (!isset($request_data['constrain']) || !isset($request_data['width']) || !isset($request_data['height'])) {
			return translate('trigger_require_constrain_width_height');
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('image_variety');

		$variety = new Image_Variety();
		$variety->setAttrValue('variety_type', 'resize');

		// name
		if (!$variety->setAttrValue('name', $request_data['name'])) {
			return translate('trigger_cannot_set_variety_value', translate('name'));
		}

		// constrain
		if (!$variety->setAttrValue('constrain', $request_data['constrain'])) {
			return translate('trigger_cannot_set_variety_value', translate('constrain'));
		}

		// width
		if (!$variety->setAttrValue('width', $request_data['width'])) {
			return translate('trigger_cannot_set_variety_value', translate('width'));
		}

		// height
		if (!$variety->setAttrValue('height', $request_data['height'])) {
			return translate('trigger_cannot_set_variety_value', translate('height'));
		}

		unset($variety);

		// save settings
		$settings['name'] = $request_data['name'];
		$settings['constrain'] = $request_data['constrain'];
		$settings['width'] = strval($request_data['width']);
		$settings['height'] = strval($request_data['height']);

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs acquire before executing
	*
	* @param array	$settings	a container for any data the action might want to save
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
