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
* $Id: trigger_action_create_image_variety.inc,v 1.1 2005/02/17 06:34:07 rhoward Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Trigger Action Create Image Variety
*
* A trigger action that creates an image variety
*
*
* @author	Andrei Railean <arailean@squiz.net>
* @author	Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Create_Image_Variety extends Trigger_Action
{

	/**
	* Execute this action
	*
	* @param array(string=>mixed)	$settings	the stored settings for this action
	* @param array(string=>mixed)	&$state	the state that describes the asset the action is executing on
	*
	* @return boolean
	* @access public
	*/
	function execute($settings, &$state)
	{
		/* settings used by this action:
		$settings = Array(
			'name'		=> string,	// name of the image variety
			'width'		=> int,		// may be zero, see action description
			'height'	=> int		// may be zero, see action description
		);
		*/

		$am = &$GLOBALS['SQ_SYSTEM']->am;

		// check settings, state
		// name
		if (empty($settings['name'])) {
			return false;
		}

		// type - must be image
		$asset_type = $am->getAssetTypeInfo(Array($state['assetid']), Array('image'), true);
		if (empty($asset_type)) {
			return false;
		}

		// height & width
		if (!isset($settings['width']) || !isset($settings['height'])) {
			// must have both height and width set, even if one is zero
			return false;
		}

		// height & width must be numeric
		if (!is_numeric($settings['width']) || !is_numeric($settings['height'])) {
			return false;
		}

		// at least one dimension must be greater than zero
		if ( !$settings['width'] && !$settings['height']) {
			return false;
		}


		// state
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return false;
			} else {
				$state['asset'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}


		// execute
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('image_variety');

		// rely on image_variety to check whether the variety already exists
		$variety = new Image_Variety();

		$name = $settings['name'];
		$variety->setAttrValue('name', $name);
		$variety->setAttrValue('width', $settings['width']);
		$variety->setAttrValue('height', $settings['height']);

		$state['asset']->saveVariety($variety);
		return true;

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param Array 		$settings  settings that this condition saves in processInterface()
	* @param string 	$prefix    unique prefix
	*
	* @access public
	* @return boolean
	*/
	function getInterface($settings, $prefix)
	{
		// set defaults
		$settings['name'] = array_get_index($settings, 'name', '');
		$settings['width'] = array_get_index($settings, 'width', 0);
		$settings['height'] = array_get_index($settings, 'height', 0);

		// capture anything printed
		ob_start();

		?>
		Create an Image Variety with the name: <?php text_box($prefix.'[name]', $settings['name']); ?><br />
		and size it to: (width)
		<?php
		int_text_box($prefix.'[width]', $settings['width'], false);
		?>
		by (height)
		<?php
		int_text_box($prefix.'[height]', $settings['height'], false);

		return ob_get_clean();

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param Array			$settings		a container for any data the action might want to save
	* @param string			$request_data	array of data corresponding to this action as specified by the $prefix in getInterface.
	*
	* @access public
	* @return boolean
	*/
	function processInterface(&$settings, $request_data)
	{
		if (empty($request_data) || empty($request_data['name'])) {
			return false;
		}

		// check if the width/heights are valid
		if (!isset($request_data['width']) || !isset($request_data['height'])) {
			// must have both height and width set, even if one is zero
			return false;
		} else {
			// at least one dimension must be greater than zero
			if 	( !( ($request_data['width'] > 0 && $request_data['height'] >= 0) || ($request_data['width'] >= 0 && $request_data['height'] > 0) ) ) {
				// both zero or less.
				return false;
			}
		}

		// save settings
		$settings['name'] = $request_data['name'];
		$settings['width'] = strval($request_data['width']);
		$settings['height'] = strval($request_data['height']);

		return true;

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
		return Array(
			$state['assetid']	=> Array(
				'all'
			)
		);

	}//end getLocks()

}//end class

?>
