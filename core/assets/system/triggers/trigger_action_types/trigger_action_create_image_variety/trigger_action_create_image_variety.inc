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
* $Id: trigger_action_create_image_variety.inc,v 1.5 2005/04/14 04:58:12 rhoward Exp $
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
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Create_Image_Variety extends Trigger_Action
{


	/**
	* Execute this action
	*
	* The settings used by this action are in the form:
	* <PRE>
	* Array(
	*       'name'      => string,  // name of the image variety
			'constrain'	=> string,	// dimension to constrain by, see action description
	*       'width'     => int,     // may be zero, see action description
	*       'height'    => int      // may be zero, see action description
	*       );
	* </PRE>
	*
	* @param array(string=>mixed)   $settings   the stored settings for this action
	* @param array(string=>mixed)   &$state the state that describes the asset the action is executing on
	*
	* @return boolean
	* @access public
	*/
	function execute($settings, &$state)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		// state
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return false;
			} else {
				$state['asset'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		// type - must be image
		if (!is_a($state['asset'], 'image')) return false;

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('image_variety');

		$variety = new Image_Variety();
		$variety->setAttrValue('variety_type', 'resize');

		// set & check settings, state

		// name
		if (!@$variety->setAttrValue('name', $settings['name'])) {
			return false;
		}

		// constrain
		if (!@$variety->setAttrValue('constrain', $settings['constrain'])) {
			return false;
		}

		// width
		if (!@$variety->setAttrValue('width', $settings['width'])) {
			return false;
		}

		// height
		if (!@$variety->setAttrValue('height', $settings['height'])) {
			return false;
		}

		// execute

		// rely on image_variety to check whether the variety already exists
		$state['asset']->saveVariety($variety);

		return true;

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param Array(string=>mixed)   $settings      settings that this condition saves in processInterface()
	* @param string                 $prefix        unique prefix
	* @param boolean                $write_access  determines whether the interface is editable
	*
	* @access public
	* @return boolean
	*/
	function getInterface($settings, $prefix, $write_access=false)
	{
		// set defaults
		$settings['name']      = array_get_index($settings, 'name', '');
		$settings['constrain'] = array_get_index($settings, 'constrain', 'width');
		$settings['width']     = array_get_index($settings, 'width', 0);
		$settings['height']    = array_get_index($settings, 'height', 0);

		// capture anything printed
		ob_start();

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		echo 'Create an Image Variety with the name: ';
		text_box($prefix.'[name]', $settings['name'], '', '', false, $form_element_extras);
		echo '<br />';
		echo 'and constrain by:';
		$options = Array(
					'width'		=> 'Width',
					'height'	=> 'Height',
					'none'		=> 'Neither',
				   );
		combo_box($prefix.'[constrain]', $options, false, $settings['constrain'], 0, $form_element_extras);
		echo 'and size it to: (width)';
		int_text_box($prefix.'[width]', $settings['width'], false, '', '', false, $form_element_extras);
		echo 'by (height) ';
		int_text_box($prefix.'[height]', $settings['height'], false, '', '', false, $form_element_extras);

		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param Array          $settings       a container for any data the action might want to save
	* @param string         $request_data   array of data corresponding to this action as specified by the $prefix in getInterface.
	*
	* @access public
	* @return boolean
	*/
	function processInterface(&$settings, $request_data)
	{

		if (empty($request_data)) {
			return 'Input Data is missing';
		}

		// check if the name, constrain, width and height are set
		if (empty($request_data['name'])) {
			return 'Variety name cannot be empty';
		}

		if (!isset($request_data['constrain']) || !isset($request_data['width']) || !isset($request_data['height'])) {
			return 'must have constrain, height and width set, even if one is zero';
		}

		$GLOBALS['SQ_SYSTEM']->am->includeAsset('image_variety');

		$variety = new Image_Variety();
		$variety->setAttrValue('variety_type', 'resize');

		// name
		if (!$variety->setAttrValue('name', $request_data['name'])) {
			return 'Cannot set variety\'s "name" value';
		}

		// constrain
		if (!$variety->setAttrValue('constrain', $request_data['constrain'])) {
			return 'Cannot set variety\'s "constrain" value';
		}

		// width
		if (!$variety->setAttrValue('width', $request_data['width'])) {
			return 'Cannot set variety\'s "width" value';
		}

		// height
		if (!$variety->setAttrValue('height', $request_data['height'])) {
			return 'Cannot set variety\'s "height" value';
		}

		unset($variety);

		// save settings
		$settings['name'] = $request_data['name'];
		$settings['constrain'] = $request_data['constrain'];
		$settings['width'] = strval($request_data['width']);
		$settings['height'] = strval($request_data['height']);

		return false;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs acquire before executing.
	*
	* @access public
	* @return array(assetid=>locktype)
	*/
	function getLocks($settings, &$state)
	{
		// get 'all' locks on the firing asset
		return Array($state['assetid'] => Array('all'));

	}//end getLocks()


}//end class

?>
