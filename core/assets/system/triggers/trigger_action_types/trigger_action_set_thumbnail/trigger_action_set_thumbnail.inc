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
* $Id: trigger_action_set_thumbnail.inc,v 1.1 2008/04/03 06:19:18 bpearson Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Trigger Action Set Thumbnail
*
* A trigger action that creates a thumbnail
*
*
* @author   Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.1 $
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
			if (!empty($settings['index'])) {
				$vars_index = $settings['index'];

				// Grab the image id from the request vars, fail if nothing set
				if (isset($_SESSION[$vars_index]) && (!(trim($_SESSION[$vars_index]) == ''))) {
					$imageid = $_SESSION[$vars_index];
				} else if (isset($_POST[$vars_index]) && (!(trim($_POST[$vars_index]) == ''))) {
					$imageid = $_POST[$vars_index];
				} else if (isset($_GET[$vars_index]) && (!(trim($_GET[$vars_index]) == ''))) {
					$imageid = $_GET[$vars_index];
				} else {
					// Fail silently if nothing found
					return TRUE;
				}

				$thumbnail = $GLOBALS['SQ_SYSTEM']->am->getAsset($imageid);
			} else {
				// Fail silently if empty
				return TRUE;
			}
		}

		// asset must be image (and fail silently)
		if (!is_a($thumbnail, 'image')) return TRUE;

		// Set the thumbnail to the asset passed
		if (Trigger_Action_Set_Thumbnail::_checkImage($thumbnail)) {
			// Delete any thumbnail links
			$grab_link = $GLOBALS['SQ_SYSTEM']->am->getLink($state['assetid'], SQ_LINK_NOTICE, '', FALSE, 'thumbnail');

			if (is_array($grab_link) && isset($grab_link['linkid'])) {
				$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($grab_link['linkid']);
			}

			// Create the new thumbnail link
			$tmblink = $GLOBALS['SQ_SYSTEM']->am->createAssetLink($state['asset'], $thumbnail, SQ_LINK_NOTICE, 'thumbnail');

		} else {
			// Fail silently if image is invalid
			return TRUE;
		}

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
		$settings['index']      = array_get_index($settings, 'index', '');

		if (!$write_access) {
			$form_element_extras = 'disabled="disabled"';
		} else {
			$form_element_extras = '';
		}

		// begin buffering basic options
		ob_start();
			text_box($prefix.'[index]', $settings['index'], '', '', FALSE, $form_element_extras);
			$basic_part_1 = ob_get_contents();
		ob_end_clean();

		return translate('trigger_set_thumbnail', $basic_part_1);

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
		if (empty($request_data['index'])) {
			return translate('trigger_thumbnail_index_cannot_be_empty');
		}

		// save settings
		$settings['index'] = $request_data['index'];

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


	/**
	* Check the image for the right stuff
	*
	* @param mixed	&$image	the image asset
	*
	* @return boolean
	* @access private
	*/
	public static function _checkImage(&$image)
	{
		if (empty($image)) return FALSE;

		$max_file_size = 20; 	// Kilobytes
		$max_file_size *= 1024;	// Convert to bytes

		if ($image->attr('size') > $max_file_size) {
			return FALSE;
		}

		$fileInfo = get_file_type($image->attr('name'));
		$validExt = Array('png','jpeg','jpg','gif','PNG','JPEG','JPG','GIF');
		if (array_search($fileInfo, $validExt) === FALSE) {
			return FALSE;
		}
		if (($image->attr('width') > 110) || ($image->attr('height') > 150)) {
			return FALSE;
		}

		return TRUE;

	}//end _checkImage()


}//end class

?>
