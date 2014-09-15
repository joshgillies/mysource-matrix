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
* $Id: trigger_action_set_thumbnail.inc,v 1.5 2012/08/30 01:09:14 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_ATTRIBUTES_PATH.'/parameter_map/parameter_map.inc';

/**
* Trigger Action Set Thumbnail
*
* A trigger action that creates a thumbnail
*
*
* @author   Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Thumbnail extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:
	* <PRE>
	* Array(
	*		'index'		=> string,	// the index of the $_GET, $_POST or $_SESSION request
	*       'imageid'   => string,	// id of the image to set as the thumbnail
	* );
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
		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				// Fail silently if no asset
				return TRUE;
			} else {
				$state['asset'] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (isset($settings['imageid']) && !empty($settings['imageid'])) {
			$thumbnail = $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['imageid']);
		} else {
			$parameter_map_value = array_get_index($settings, 'thumbnail', serialize(Array()));
			$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
			$assetid = $atr_parameter_map->getParameterValue('thumbnail');
			if (!empty($assetid)) {
				$thumbnail = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			} else {
				// Fail silently if empty
				return TRUE;
			}
		}

		// asset must be image (and fail silently)
		if (!is_a($thumbnail, 'image')) return TRUE;

		// Set the thumbnail to the asset passed
		// Delete any thumbnail links
		$grab_link = $GLOBALS['SQ_SYSTEM']->am->getLink($state['assetid'], SQ_LINK_NOTICE, '', FALSE, 'thumbnail');

		if (is_array($grab_link) && isset($grab_link['linkid'])) {
			$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($grab_link['linkid']);
		}

		// Create the new thumbnail link
		$tmblink = $GLOBALS['SQ_SYSTEM']->am->createAssetLink($state['asset'], $thumbnail, SQ_LINK_NOTICE, 'thumbnail');

		return Array(
				'linkid'	=> $tmblink,
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
		$settings['thumbnail']  = array_get_index($settings, 'thumbnail', Array());

		$pmap = new Asset_Attribute_Parameter_Map();
		$pmap->setParameter('thumbnail', 'Set new Thumbnail');
		$pmap->setValue($settings['thumbnail']);

		// begin buffering basic options
		ob_start();
			// Save the prefix
			$new_prefix = str_replace(array('[',']'), '_', $prefix);
			hidden_field($prefix.'[new_prefix]', $new_prefix);

			echo $pmap->paint($new_prefix, !$write_access);
			$basic_part_1 = ob_get_contents();
		ob_end_clean();

		return sprintf(translate('trigger_set_thumbnail'), $basic_part_1);

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

		// Get the prefix
		$new_prefix = str_replace(array('[',']'), '_', array_get_index($request_data, 'new_prefix', ''));

		// check if the name, constrain, width and height are set
		$pmap = new Asset_Attribute_Parameter_Map();
		$pmap->process($new_prefix);
		$settings['thumbnail'] = $pmap->value;

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
